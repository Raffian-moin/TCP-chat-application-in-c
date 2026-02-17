#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>

#define SERVER_IP_ADDRESS "127.0.0.1"

void *get_in_address(struct sockaddr *peer_addr) {
        if (peer_addr->sa_family == AF_INET) {
            return &(((struct sockaddr_in *)&peer_addr)->sin_addr);
        } else { 
            return &(((struct sockaddr_in6 *)&peer_addr)->sin6_addr);
        }
}

struct fdstruct {
    int capacity;
    int total;
    int fds[10];
};

int main() {
    
    struct addrinfo hints, *res;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int get_addr_return_val;
    int yes = 1;
    int socket_fd;
    
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_size;
    peer_addr_size = sizeof(peer_addr);
    char peer_addr_text[INET6_ADDRSTRLEN];

     
    if((get_addr_return_val = getaddrinfo(NULL, "3009", &hints, &res)) != 0) {
        printf("%s\n", gai_strerror(get_addr_return_val));
    }

    for(struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, 0)) == -1) {
            perror("socket\n");
            continue;
        }
            
        int flags = fcntl(socket_fd, F_GETFL);
        flags |= O_NONBLOCK; 
        fcntl(socket_fd, F_SETFL, flags);
        
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("Socket option error\n");
        }

        printf("socket fd: %d\n", socket_fd);
        
        if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind\n");
            continue;
        }

        break;

    }

    freeaddrinfo(res);
    
    if (listen(socket_fd, 5) == -1) {
        perror("listen failed\n");
    }

    struct fdstruct fdstr;
    fdstr.capacity = 10;
    fdstr.total = 0;
    fd_set readfds;
    struct timeval timeout = {2, 0};
    int max_fd = socket_fd;

    fdstr.fds[fdstr.total] = socket_fd;
    fdstr.total++;

    while(1) {
        FD_ZERO(&readfds);

        for(int i = 0; i < fdstr.total; i++) {
            FD_SET(fdstr.fds[i], &readfds);
            if (fdstr.fds[i] > max_fd) {
                max_fd = fdstr.fds[i];
            }
        }

        int readyfds = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
        if (readyfds > 0) {
            printf("%d\n", fdstr.total);
            for(int i = 0; i < fdstr.total; i++) {
                printf("ok\n");
                if(FD_ISSET(fdstr.fds[i], &readfds)) {   
                    printf("inside read\n");
                    char buff[50];
                    memset(buff, 0, sizeof(buff));

                    int recvint = recv(fdstr.fds[i], buff, sizeof(buff), 0);
                    if (recvint > 0) {
                        printf("received text: %s", buff);
                        for(int j = 0; j < fdstr.total; j++) {
                            if (fdstr.fds[j] != socket_fd) {
                                if (send(fdstr.fds[j], buff, strlen(buff), 0) == -1) {
                                    printf("failed to send message to client");
                                } 
                            }
                        }
                    }

                }
            }
            

            if(FD_ISSET(socket_fd, &readfds)) {   
                int accept_fd;
                if ((accept_fd = accept(socket_fd, (struct sockaddr *)&peer_addr, &peer_addr_size)) == -1) { 
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        printf("hmm\n");
                        continue;
                    }

                    perror("accept failed\n");
                    continue;
                }

                fdstr.fds[fdstr.total] = accept_fd;
                fdstr.total++;
                inet_ntop(peer_addr.ss_family, get_in_address((struct sockaddr *)&peer_addr), peer_addr_text, peer_addr_size);
                
                printf("%s connected to the server\n", peer_addr_text);
            }

        }
    }
}
