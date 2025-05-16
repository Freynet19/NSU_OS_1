#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int socketFD;

void shutdownServer(int) {
    printf("\nServer shutting down...\n");
    close(socketFD);
    exit(0);
}

int main() {
    signal(SIGINT, shutdownServer);

    struct sockaddr_in serverAddr = {}, clientAddr = {};
    char buffer[BUFFER_SIZE];
    socklen_t addrLen = sizeof(clientAddr);

    socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFD < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    struct sockaddr* serverAddrPtr = (struct sockaddr *)&serverAddr;
    struct sockaddr* clientAddrPtr = (struct sockaddr *)&clientAddr;

    if (bind(socketFD, serverAddrPtr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP Echo Server listening on port %d\n", PORT);
    printf("Press Ctrl+C to stop the server\n");

    while (1) {
        ssize_t len = recvfrom(socketFD, buffer, BUFFER_SIZE, 0, clientAddrPtr, &addrLen);
        if (len < 0) {
            perror("recvfrom failed");
            continue;
        }
        buffer[len] = '\0';
        printf("Message received: %s\n", buffer);

        sendto(socketFD, buffer, len, 0, clientAddrPtr, addrLen);
    }
}
