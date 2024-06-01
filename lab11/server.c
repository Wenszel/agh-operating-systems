#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <time.h>
#include "command_utils.h"

#define MAX_CLIENTS 10
#define CLIENT_ID_SIZE 256
#define MESSAGE_SIZE 256
#define TOTAL_MESSAGE_SIZE 512 
#define SETID_COMMAND "SETID"
#define TO_ALL_COMMAND "2ALL"
#define LIST_COMMAND "LIST\n"
#define TO_ONE_COMMAND "2ONE"
#define STOP_COMMAND "STOP\n"

struct user {
    int connected;
    struct sockaddr_in addr;
    char* client_id;
};

char* find_sender(struct user* users, struct sockaddr_in client_addr) {
    for (int j = 0; j < MAX_CLIENTS; j++) {
        if(users[j].connected != -1 && memcmp(&users[j].addr, &client_addr, sizeof(client_addr)) == 0) {
            return users[j].client_id;
        }
    }
    return NULL; 
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Usage: %s IP_ADDRESS PORT\n", argv[0]);
        exit(1);
    }
    int PORT = atoi(argv[2]);
    char* IP_ADDRESS = argv[1];
    
    int server_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket_descriptor < 0) {
        perror("Error while creating socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket_descriptor, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error while binding socket");
        close(server_socket_descriptor);
        exit(1);
    }
    
    printf("UDP Server is running on port %d\n", PORT);
    
    struct user users[MAX_CLIENTS];
    struct pollfd poll_fds[1];
    
    char buffer[MESSAGE_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        users[i].connected = -1;
        users[i].client_id = (char*)malloc(256);
    }

    poll_fds[0].fd = server_socket_descriptor;
    poll_fds[0].events = POLLIN;

    while (1) {
        if (poll(poll_fds, 1, -1) < 0) {
            perror("error while polling");
            close(server_socket_descriptor);
            exit(1);
        }

        if (poll_fds[0].revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = recvfrom(poll_fds[0].fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addr_len);
            if (bytes_read <= 0) {
                if (bytes_read == 0) {
                    printf("Client disconnected from server\n");
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (memcmp(&users[j].addr, &client_addr, sizeof(client_addr)) == 0) {
                            users[j].connected = -1;
                            break;
                        }
                    }
                } else {
                    perror("Error while reading data from client");
                }
                
            } else{
                buffer[bytes_read] = '\0';
                printf("Received: %s\n", buffer);
                char* command = get_command(buffer);
                if (strcmp(command, SETID_COMMAND) == 0) {
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (users[j].connected == -1) {
                            users[j].connected = 1;
                            memcpy(&users[j].addr, &client_addr, sizeof(client_addr));
                            strcpy(users[j].client_id, get_string_after_command(buffer));
                            break;
                        }
                    }
                }

                else if (strcmp(command, TO_ALL_COMMAND) == 0) {
                    char sender[CLIENT_ID_SIZE] = {0};
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if(users[j].connected != -1 && memcmp(&users[j].addr, &client_addr, sizeof(client_addr)) == 0) {
                            strcpy(sender, users[j].client_id);
                            break;
                        }
                    }

                    char message[TOTAL_MESSAGE_SIZE];
                    sprintf(message, "%s> %s", sender, get_string_after_command(buffer));
                    for (int j = 1; j < MAX_CLIENTS; j++) {
                        if (users[j].connected != -1 && memcmp(&users[j].addr, &client_addr, sizeof(client_addr)) != 0) {
                            sendto(poll_fds[0].fd, message, strlen(message) + 1, 0, (struct sockaddr*)&users[j].addr, sizeof(users[j].addr));
                        }
                    }

                }
                else if (strcmp(command, TO_ONE_COMMAND) == 0) {
                    char* message = get_string_after_param(buffer);
                    char* recipient = get_param_after_command(buffer);
                    char* sender = find_sender(users, client_addr);
                    if (sender == NULL) {
                        printf("Sender not found\n");
                        continue;
                    }
                    char total_message[TOTAL_MESSAGE_SIZE];
                    time_t t;
                    time(&t);
                    char time_str[32];
                    strcpy(time_str, ctime(&t));
                    char* end = strchr(time_str, '\n');
                    if (end != NULL) {
                        *end = '\0';
                    }
                    sprintf(total_message, "%s: %s> %s", time_str, sender, message);
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (users[j].connected != -1 && strcmp(users[j].client_id, recipient) == 0) {
                            sendto(poll_fds[0].fd, total_message, strlen(total_message) + 1, 0, (struct sockaddr*)&users[j].addr, sizeof(users[j].addr));
                            break;
                        }
                    }
                }
                else if (strcmp(command, LIST_COMMAND) == 0) {
                    char message[TOTAL_MESSAGE_SIZE];
                    strcpy(message, "Users: ");
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (users[j].connected != -1) {
                            strcat(message, users[j].client_id);
                            strcat(message, " ");
                        }
                    }
                    sendto(poll_fds[0].fd, message, strlen(message) + 1, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
                }
                else if (strcmp(command, STOP_COMMAND) == 0) {
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (memcmp(&users[j].addr, &client_addr, sizeof(client_addr)) == 0) {
                            users[j].connected = -1;
                            break;
                        }
                    }
                }
                memset(buffer, 0, sizeof(buffer));
            }
        }
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        free(users[i].client_id);
    }
    close(server_socket_descriptor);

    return 0;
}
