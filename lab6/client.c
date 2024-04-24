#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_QUEUE "/server_queue"
#define MAX_MESSAGE_SIZE 1024

void *receiver(void *arg) {
    mqd_t client_queue = *(mqd_t *)arg;
    char buffer[MAX_MESSAGE_SIZE];

    while (1) {
        ssize_t bytes_read = mq_receive(client_queue, buffer, MAX_MESSAGE_SIZE, NULL);
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            printf("Received: %s\n", buffer);
        } else {
            perror("Receiver error");
        }
    }
    return NULL;
}

int main() {
    char client_queue_name[64];
    sprintf(client_queue_name, "/client_queue_%d", getpid());

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = MAX_MESSAGE_SIZE,
        .mq_curmsgs = 0
    };

    mqd_t client_queue = mq_open(client_queue_name, O_CREAT | O_RDONLY, 0644, &attr);
    mqd_t server_queue = mq_open(SERVER_QUEUE, O_WRONLY);

    if (client_queue == (mqd_t)-1 || server_queue == (mqd_t)-1) {
        perror("Error opening queue");
        exit(1);
    }

    char init_message[64];
    sprintf(init_message, "INIT %s", client_queue_name);
    mq_send(server_queue, init_message, strlen(init_message) + 1, 0);

    char client_id_str[10];
    mq_receive(client_queue, client_id_str, 10, NULL);
    int client_id = atoi(client_id_str);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, receiver, &client_queue);

    while (1) {
        char message[MAX_MESSAGE_SIZE];
        fgets(message, MAX_MESSAGE_SIZE, stdin);
        char full_message[MAX_MESSAGE_SIZE];
        sprintf(full_message, "%d %s", client_id, message);
        mq_send(server_queue, full_message, strlen(full_message) + 1, 0);
    }

    mq_close(client_queue);
    mq_close(server_queue);
    mq_unlink(client_queue_name);

    return 0;
}
