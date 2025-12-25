#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#define SERVER_IP_ADDRESS "127.0.0.1" 

int main() {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_aton(SERVER_IP_ADDRESS, &(server_addr.sin_addr));

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        perror("Socket creation failed");
    }

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Failed connect to server");
    }

    char buff[] = "Hello form client!";
    send(socket_fd, buff, sizeof(buff), 0);

    close(socket_fd);
}
