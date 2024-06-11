
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_REINDEER 9
#define DELIVERIES 4
#define STATUS_WAITING 0
#define STATUS_DELIVERING 1

pthread_mutex_t mutex;
pthread_cond_t santa_cond;
pthread_cond_t reindeer_cond;

int reindeer_count = 0;
int deliveries_made = 0;
int status = STATUS_WAITING;

void* santa_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&santa_cond, &mutex);

        if (deliveries_made >= DELIVERIES) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        printf("Santa: Budzę się\n");
        printf("Santa: Dostarczam zabawki\n");
        status = STATUS_DELIVERING;
        pthread_cond_broadcast(&reindeer_cond);
        deliveries_made++;
        while (status == STATUS_DELIVERING) {
            pthread_cond_wait(&santa_cond, &mutex);
        }
        printf("Santa: Zasypiam\n");
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* reindeer_thread(void* arg) {
    int id = *((int*)arg);
    free(arg);

    while (1) {
        sleep(5 + rand() % 6); 

        pthread_mutex_lock(&mutex);
        reindeer_count++;
        printf("Reindeer %d: wrocilem z wakacji liczba reniferow na biegunie %d\n", id, reindeer_count);
        
        if (reindeer_count == NUM_REINDEER) {
            printf("Reindeer %d: Wybudzam Mikolaja\n", id);
            pthread_cond_signal(&santa_cond);
        }
        while (status == STATUS_WAITING) {
            pthread_cond_wait(&reindeer_cond, &mutex);
        }
        if (deliveries_made >= DELIVERIES) {
            pthread_mutex_unlock(&mutex);
            pthread_cond_signal(&santa_cond);
            break;
        }

        pthread_mutex_unlock(&mutex);
        printf("Reindeer %d: Dostarczam zabawki\n", id);
        sleep(2 + rand() % 3); 
        pthread_mutex_lock(&mutex);
        reindeer_count--;
        printf("Reindeer %d: Lecę na wakacje\n", id);
        if (reindeer_count == 0) {
            status = STATUS_WAITING;
            pthread_cond_signal(&santa_cond);
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t santa;
    pthread_t reindeers[NUM_REINDEER];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&santa_cond, NULL);
    pthread_cond_init(&reindeer_cond, NULL);

    pthread_create(&santa, NULL, santa_thread, NULL);

    for (int i = 0; i < NUM_REINDEER; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&reindeers[i], NULL, reindeer_thread, id);
    }

    pthread_join(santa, NULL);

    for (int i = 0; i < NUM_REINDEER; i++) {
        pthread_join(reindeers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&santa_cond);
    pthread_cond_destroy(&reindeer_cond);

    printf("Santa: Koniec dostarczania prezentów\n");

    return 0;
}
