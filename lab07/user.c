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

void perror_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main() {
    int jobs_sem_id = semget(AVAILABLE_JOBS_SEM_KEY, 1, 0666 | IPC_CREAT);
    if (jobs_sem_id == -1) {
        perror_exit("semget");
    }

    struct sembuf new = {0, 1, SEM_UNDO};

    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror_exit("shmget");
    }

    print_queue *queue = (print_queue *)shmat(shmid, NULL, 0);
    if (queue == (void *)-1) {
        perror_exit("shmat");
    }

    srand(time(NULL) ^ getpid());

    semop(jobs_sem_id, &new, 1);

    char job[JOB_SIZE + 1];
    for (int i = 0; i < JOB_SIZE; i++) {
        job[i] = 'a' + rand() % 26;
    }
    job[JOB_SIZE] = '\0';

    strcpy(queue->queue[queue->in], job);
    queue->in = (queue->in + 1) % QUEUE_SIZE;
    queue->count++;

    printf("User %d: Sent job %s\n", getpid(), job);

    sleep(rand() % 5 + 1);

    if (shmdt(queue) == -1) {
        perror_exit("shmdt");
    }

    return 0;
}
