#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> 
#define MESSAGE_SIZE 512

struct sockaddr_in server_addr;
int client_socket_descriptor;
typedef struct {
    int client_socket_descriptor;
} thread_data_t;

void handleDisconnect() {
    char message[MESSAGE_SIZE];
    sprintf(message, "STOP\n");
    sendto(client_socket_descriptor, message, strlen(message) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    close(client_socket_descriptor);
    exit(0);
}

void* read_from_server(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int client_socket_descriptor = data->client_socket_descriptor;
    char response[MESSAGE_SIZE];

    while (1) {
        memset(response, 0, sizeof(response));
        int bytes_read = recv(client_socket_descriptor, response, sizeof(response) - 1, 0);
        if (bytes_read > 0) {
            printf("%s\n", response);
        } else if (bytes_read < 0) {
            perror("Error while reading from server");
            close(client_socket_descriptor);
            pthread_exit(NULL);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s ADDRESS PORT ID\n", argv[0]);
        exit(1);
    }
    char* IP_ADDRESS = argv[1];
    int PORT = atoi(argv[2]);
    char* CLIENT_ID = argv[3];

    client_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket_descriptor < 0) {
        perror("Error while creating socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    server_addr.sin_port = htons(PORT);

    char message[MESSAGE_SIZE];
    sprintf(message, "SETID %s", CLIENT_ID);
    sendto(client_socket_descriptor, message, strlen(message) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    pthread_t read_thread;
    thread_data_t thread_data;
    thread_data.client_socket_descriptor = client_socket_descriptor;

    signal(SIGINT, handleDisconnect);

    if (pthread_create(&read_thread, NULL, read_from_server, (void*)&thread_data) != 0) {
        perror("Error creating thread");
        close(client_socket_descriptor);
        exit(1);
    }

    
    while(1) {
        char user_input[256];
        fgets(user_input, 256, stdin);
        if (strcmp(user_input, "STOP\n") == 0) {
            handleDisconnect();
            break;
        }
        sendto(client_socket_descriptor, user_input, strlen(user_input) + 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    }

    close(client_socket_descriptor);
}