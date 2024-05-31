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
// Struktura do przekazania informacji do wątków
typedef struct {
    int client_socket_descriptor;
} thread_data_t;

// Funkcja wątku do odczytu wiadomości z serwera
void* read_from_server(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int client_socket_descriptor = data->client_socket_descriptor;
    char response[MESSAGE_SIZE];

    while (1) {
        memset(response, 0, sizeof(response));
        int bytes_read = read(client_socket_descriptor, response, sizeof(response) - 1);
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

    int client_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_descriptor < 0) {
        perror("Error while creating socket");
        exit(1);
    }

    

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    server_addr.sin_port = htons(PORT);

    connect(client_socket_descriptor, (struct sockaddr *) &server_addr, sizeof(server_addr));
    char message[MESSAGE_SIZE];
    sprintf(message, "SETID %s", CLIENT_ID);
    write(client_socket_descriptor, message, strlen(message) + 1);

    pthread_t read_thread;
    thread_data_t thread_data;
    thread_data.client_socket_descriptor = client_socket_descriptor;

    // Utworzenie wątku do odczytu z serwera
    if (pthread_create(&read_thread, NULL, read_from_server, (void*)&thread_data) != 0) {
        perror("Error creating thread");
        close(client_socket_descriptor);
        exit(1);
    }



    while(1) {
        char user_input[256];
        fgets(user_input, 256, stdin);
        // sprintf(message, "%s> %s", CLIENT_ID, user_input);
        write(client_socket_descriptor, user_input, strlen(user_input) + 1);
    }
    close(client_socket_descriptor);
}