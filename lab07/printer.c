#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/shm.h>
#include <unistd.h>

#define AVAILABLE_PRINTERS_SEM_KEY 1234
#define AVAILABLE_JOBS_SEM_KEY 3456
#define SHM_KEY 5678
#define SHM_SIZE 1024

#define QUEUE_SIZE 10
#define JOB_SIZE 10

typedef struct {
    char queue[QUEUE_SIZE][JOB_SIZE + 1];
    int in;
    int out;
    int count;
} print_queue;

int main() {
    int printers_sem_id = semget(AVAILABLE_PRINTERS_SEM_KEY, 1, 0666 | IPC_CREAT);
    int jobs_sem_id = semget(AVAILABLE_JOBS_SEM_KEY, 1, 0666 | IPC_CREAT);

    struct sembuf new = {0, 1, SEM_UNDO};
    struct sembuf handle_task = {0, -1, SEM_UNDO};
    struct sembuf printing = {0, -1, SEM_UNDO};
    struct sembuf end = {0, 1, SEM_UNDO};

    semop(printers_sem_id, &new, 1);

    // Tworzenie segmentu pamięci współdzielonej, jeśli nie istnieje
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
    if (shmid == -1) {
        // Jeśli segment już istnieje, uzyskaj jego identyfikator
        shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
        if (shmid == -1) {
            printf("ee");
            exit(0);
        }
    } else {
        // Jeśli segment został właśnie utworzony, zainicjuj go
        print_queue *init_queue = (print_queue *)shmat(shmid, NULL, 0);
        if (init_queue == (void *)-1) {
            printf("ee");
            exit(0);
        }
        init_queue->in = 0;
        init_queue->out = 0;
        init_queue->count = 0;
        if (shmdt(init_queue) == -1) {
            printf("ee");
            exit(0);
        }
    }

    print_queue *queue = (print_queue *)shmat(shmid, NULL, 0);

    while(1) {
        semop(printers_sem_id, &printing, 1);
        semop(jobs_sem_id, &handle_task, 1);

        char job[JOB_SIZE + 1];
        strcpy(job, queue->queue[queue->out]);
        queue->out = (queue->out + 1) % QUEUE_SIZE;
        queue->count--;

        
        printf("Printer %d: Printing job %s\n", getpid(), job);
        for (int i = 0; i < JOB_SIZE; i++) {
            printf("%c", job[i]);
            fflush(stdout);
            sleep(1); 
        }
        printf("\n");
        semop(printers_sem_id, &end, 1);
    } 
    shmdt(queue);
}