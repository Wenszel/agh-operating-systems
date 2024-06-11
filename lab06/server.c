#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>

#define SERVER_QUEUE "/server_queue"
#define MAX_CLIENTS 10
#define MAX_MESSAGE_SIZE 1024

struct client {
    mqd_t queue;
    char queue_name[64];
};

struct client clients[MAX_CLIENTS];
int client_count = 0;

int main() {
    mqd_t server_queue;
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = MAX_MESSAGE_SIZE,
        .mq_curmsgs = 0
    };

    server_queue = mq_open(SERVER_QUEUE, O_CREAT | O_RDONLY, 0644, &attr);

    if (server_queue == (mqd_t) -1) {
        perror("Error opening queue");
        exit(1);
    }

    printf("Server started. Waiting for clients...\n");

    while (1) {
        char buffer[MAX_MESSAGE_SIZE];
        unsigned int priority;
        ssize_t bytes_read;

        bytes_read = mq_receive(server_queue, buffer, MAX_MESSAGE_SIZE, &priority);

        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';

            if (strncmp(buffer, "INIT", 4) == 0) {
                char client_queue_name[64];
                sscanf(buffer, "INIT %s", client_queue_name);

                if (client_count >= MAX_CLIENTS) {
                    fprintf(stderr, "Maximum number of clients reached.\n");
                    continue;
                }

                mqd_t client_queue = mq_open(client_queue_name, O_WRONLY);
                if (client_queue == (mqd_t)-1) {
                    perror("Error opening client queue");
                    continue;
                }

                clients[client_count].queue = client_queue;
                strcpy(clients[client_count].queue_name, client_queue_name);
                char client_id_str[10];
                sprintf(client_id_str, "%d", client_count);
                mq_send(client_queue, client_id_str, strlen(client_id_str) + 1, 0);
                client_count++;
            } else {
                int client_id;
                char message[1024];
                sscanf(buffer, "%d %[^\n]", &client_id, message);
                for (int i = 0; i < client_count; i++) {
                    if (client_id != i){
                        mq_send(clients[i].queue, message, strlen(message) + 1, 0);
                    }
                }
            }
        } else {
            perror("Error receiving message");
        }
    }

    mq_close(server_queue);
    mq_unlink(SERVER_QUEUE);
    return 0;
}
