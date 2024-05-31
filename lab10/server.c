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

/*
    Sources used:
    https://www.geeksforgeeks.org/socket-programming-cc/
    https://man7.org/linux/man-pages/man2/poll.2.html
*/

#define MAX_CLIENTS 10
#define CLIENT_ID_SIZE 256
#define MESSAGE_SIZE 256
#define TOTAL_MESSAGE_SIZE 512 
#define SETID_COMMAND "SETID"
#define TO_ALL_COMMAND "2ALL"
#define LIST_COMMAND "LIST\n"
#define TO_ONE_COMMAND "2ONE"
#define STOP_COMMAND "STOP\n"

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Usage: %s IP_ADDRESS PORT\n", argv[0]);
        exit(1);
    }
    int PORT = atoi(argv[2]);
    char* IP_ADDRESS = argv[1];
    /* socket(domain, type, protocol) - creates a new socket and returns its descriptor 
        - domain: AF_INET indicates that we are using IPv4
        - type: SOCK_STREAM indicates that we are using a connection-oriented stream protocol (TCP)
        - protocol: indicates the protocol to be used with the socket. A value of 0 indicates that the default protocol (TCP) will be used. */
    int server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor < 0) {
        perror("Error while creating socket");
        exit(1);
    }

    /* struct sockaddr_in 
        short            sin_family;   - this is always set to AF_INET because sockaddr_in is always used with IPv4 
        unsigned short   sin_port;     - port number 
        struct in_addr   sin_addr;     - IP address on which the server will listen. INADDR_ANY means that the server will listen on all available interfaces of the running machine 
        char             sin_zero[8];  - this is padding to make struct the same size as sockaddr */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    /* htons() converts the unsigned short integer hostshort from host byte order to big-endian network byte order (commonly used in networking protocols) */
    server_addr.sin_port = htons(PORT);

    /* bind(socket_descriptor, address structure, address_length) - assigns the address to the socket */
    if (bind(server_socket_descriptor, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error while binding socket");
        close(server_socket_descriptor);
        exit(1);
    }

    /* listen(socket_descriptor, backlog) - marks the socket as a passive socket, that is, as a socket that will be used to accept incoming connection requests using accept
        - backlog: the maximum length to which the queue of pending connections for socket_descriptor may grow */
    if (listen(server_socket_descriptor, 5) < 0) {
        perror("Error while listening");
        close(server_socket_descriptor);
        exit(1);
    }

    printf("Server is listening on port %d\n", PORT);
    

    struct user {
        int fd;
        char* client_id;
    };

    struct user users[MAX_CLIENTS];
    struct pollfd poll_fds[MAX_CLIENTS + 1];

    char buffer[MESSAGE_SIZE];
    int client_socket;
    
    for (int i = 0; i < MAX_CLIENTS + 1; i++) {
        poll_fds[i].fd = -1;
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        users[i].fd = -1;
        users[i].client_id = (char*)malloc(256);
    }
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    poll_fds[0].fd = server_socket_descriptor;
    poll_fds[0].events = POLLIN;

    while (1) {
        // poll(struct pollfd fds[], nfds_t nfds, int timeout) - waits for one of a set of file descriptors to become ready to perform i/o
        if (poll(poll_fds, MAX_CLIENTS+ 1, -1) < 0) {
            perror("error while polling");
            close(server_socket_descriptor);
            exit(1);
        }
        // checks if there is a new connection
        if (poll_fds[0].revents & POLLIN) {
            client_socket = accept(server_socket_descriptor, (struct sockaddr *) &client_addr, &addr_len);
            if (client_socket < 0) {
                perror("error while accepting connection");
                continue;
            }
            printf("new connection, socket fd: %d, ip: %s, port: %d\n", client_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            for (int i = 1; i <= MAX_CLIENTS; i++) {
                if (i == MAX_CLIENTS) {
                    printf("max clients reached. rejecting connection...\n");
                    close(client_socket);
                }
                if (poll_fds[i].fd == -1) {
                    poll_fds[i].fd = client_socket;
                    poll_fds[i].events = POLLIN;
                    break;
                }
            }
        }
        
        for (int i = 1; i < MAX_CLIENTS; i++) {
            if (poll_fds[i].fd != -1 && (poll_fds[i].revents & POLLIN)) {
                
                memset(buffer, 0, sizeof(buffer));
                int bytes_read = read(poll_fds[i].fd, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0) {
                    if (bytes_read == 0) {
                        printf("Client disconnected, socket fd: %d\n", poll_fds[i].fd);
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (users[j].fd == poll_fds[i].fd) {
                                users[j].fd = -1;
                                break;
                            }
                        }
                    } else {
                        perror("Error while reading data from client");
                    }
                    close(poll_fds[i].fd);
                    poll_fds[i].fd = -1;
                } else {
                    buffer[bytes_read] = '\0';
                    char* command = get_command(buffer);
                    if (strcmp(command, SETID_COMMAND) == 0) {
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (users[j].fd == -1) {
                                users[j].fd = poll_fds[i].fd;
                                strcpy(users[j].client_id, get_string_after_command(buffer));
                                break;
                            }
                        }
                    }
                    else if (strcmp(command, TO_ALL_COMMAND) == 0) {
                        char sender[CLIENT_ID_SIZE];
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if(users[j].fd != -1 && poll_fds[i].fd != -1 && users[j].fd == poll_fds[i].fd) {
                                strcpy(sender, users[j].client_id);
                                break;
                            }
                        }
                        char message[TOTAL_MESSAGE_SIZE];
                        sprintf(message, "%s> %s", sender, get_string_after_command(buffer));
                        for (int j = 1; j < MAX_CLIENTS; j++) {
                            if (poll_fds[j].fd != -1 && poll_fds[j].fd != poll_fds[i].fd) {
                                write(poll_fds[j].fd, message, strlen(message) + 1);
                            }
                        }
                    }
                    else if (strcmp(command, LIST_COMMAND) == 0) {
                        char message[TOTAL_MESSAGE_SIZE];
                        strcpy(message, "Users: ");
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (users[j].fd != -1) {
                                strcat(message, users[j].client_id);
                                strcat(message, " ");
                            }
                        }
                        write(poll_fds[i].fd, message, strlen(message) + 1);
                    }
                    else if (strcmp(command, TO_ONE_COMMAND) == 0) {
                        char* message = get_string_after_param(buffer);
                        char* recipient = get_param_after_command(buffer);
                        char sender[CLIENT_ID_SIZE];
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if(users[j].fd != -1 && poll_fds[i].fd != -1 && users[j].fd == poll_fds[i].fd) {
                                strcpy(sender, users[j].client_id);
                                break;
                            }
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
                            if (users[j].fd != -1 && strcmp(users[j].client_id, recipient) == 0) {
                                write(users[j].fd, total_message, strlen(total_message) + 1);
                                break;
                            }
                        }
                    }
                    else if (strcmp(command, STOP_COMMAND) == 0) {
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (users[j].fd == poll_fds[i].fd) {
                                users[j].fd = -1;
                                break;
                            }
                        }
                        close(poll_fds[i].fd);
                        poll_fds[i].fd = -1;
                    }
                    memset(buffer, 0, sizeof(buffer));
                }
            }
        }
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        free(users[i].client_id);
    }
    close(server_socket_descriptor);

    return 0;
}