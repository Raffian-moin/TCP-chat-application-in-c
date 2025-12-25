#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#define SERVER_IP_ADDRESS "127.0.0.1"

int main() {

    struct sockaddr_in server_addr, peer_addr;
    int peer_size = sizeof(peer_size);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_aton(SERVER_IP_ADDRESS, &(server_addr.sin_addr));

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        perror("Socket creation failed");
    }

    if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Socket binding failed");
    }

    if (listen(socket_fd, SOMAXCONN) == -1) {
        perror("Failed listening to the socket");
    }
    
    int connection_fd = accept(socket_fd, (struct sockaddr *) &peer_addr, &peer_size);
    if (connection_fd == -1) {
        perror("Failed to accept the socket");
    }
    
    char buff[1048];
    recv(connection_fd, buff, sizeof(buff), 0);

    printf("%s\n", buff);

    close(socket_fd);

}
