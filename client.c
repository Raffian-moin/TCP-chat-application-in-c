#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include<fcntl.h>

#define SERVER_IP_ADDRESS "127.0.0.1" 
#define SERVER_PORT "3009" 


void set_username(char (*username)[]) {
    while(1) {
        int flag = 1;
        printf("Write your username: ");
        fflush(stdin);
        fgets(*username, sizeof(username), stdin);
        for(int i = 0; i < strlen(*username); i++) {
            char ch = (*username)[i];
            if (ch == '\n') {
                (*username)[i] = '\0';
                break;
            }
            if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||  (ch >= '0' && ch <= '9') || ch == '_') {
                continue;
            } else {
                flag = 0;
                printf("Only alpahneumeric and underscore(_) character is allowed\n");
                break;
            }
        }

        if (flag) break;
    }
}

int main() {

    char username[20];
    set_username(&username);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int get_addr_return_val;
    int socket_fd;
    struct addrinfo *p;

    if ((get_addr_return_val = getaddrinfo("localhost", "3009", &hints, &res)) != 0) {
        printf("%s\n", gai_strerror(get_addr_return_val));
    }

    for(p = res; p != NULL; p = p->ai_next) {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, 0)) == -1) {
            perror("socket\n");
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("connect\n");
            continue;
        }

        break;

    }

    if (p == NULL) {
        printf("attempt to connect to server is failed\n");
        return 1;
    }
    
    freeaddrinfo(res);

    printf("connection successful\n");

    int fds[2];
    fd_set readfds;
    fd_set writefds;
    struct timeval timeout = {2, 0};
    int max_fd = socket_fd;

    if (send(socket_fd, username, strlen(username), 0) == -1) {
        printf("failed to send message to client\n");
    } 

    while(1) { 
        
        FD_ZERO(&readfds);

        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(socket_fd, &readfds);
        
        int readyfds = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
        
        char out[50];
        if (readyfds > 0) {
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                if(fgets(out, sizeof(out), stdin) != NULL) {
                    int size = strcspn(out, "\n");
                    out[size] = '\0';
                    if (strcmp(out, "EXIT") == 0) {
                        close(socket_fd);
                        break;
                    }

                    if (send(socket_fd, out, strlen(out), 0) == -1) {
                        printf("failed to send message to client\n");
                    } 
                    memset(out, 0, sizeof(out));
                }
            }

            if (FD_ISSET(socket_fd, &readfds)) {
                char buff[75];
                memset(buff, 0, sizeof(buff));

                int recvint = recv(socket_fd, buff, sizeof(buff), 0);
                if (recvint > 0) {
                    printf("%s\n", buff);
                }
            }
        }

    }
}
