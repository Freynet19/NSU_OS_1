#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024

int serverSocketFD;

void shutdownServer(int) {
    close(serverSocketFD);
    printf("\nServer shutting down...\n");
    exit(EXIT_SUCCESS);
}

void handleClient(int clientSocketFD, const char* clientIp, int clientPort) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(clientSocketFD, buffer, sizeof(buffer) - 1))) {
        if (bytesRead < 0) {
            perror("error reading from socket");
            return;
        }

        buffer[bytesRead] = '\0';
        printf("Message received from %s:%d: %s\n", clientIp, clientPort, buffer);

        ssize_t totalWritten = 0;
        while (totalWritten < bytesRead) {
            ssize_t writtenNow = write(clientSocketFD, buffer + totalWritten, bytesRead - totalWritten);
            if (writtenNow == -1) {
                perror("write failed");
                return;
            }
            totalWritten += writtenNow;
        }
    }
}

int main() {
    signal(SIGINT, shutdownServer);

    const char *ip = IP;
    int port = PORT;
    struct sockaddr_in serverAddr = {}, clientAddr = {};
    socklen_t clientLen = sizeof(clientAddr);

    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD < 0) {
        perror("socket creation failed");
        return EXIT_FAILURE;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(port);

    struct sockaddr* serverAddrPtr = (struct sockaddr *)&serverAddr;
    struct sockaddr* clientAddrPtr = (struct sockaddr *)&clientAddr;

    if (bind(serverSocketFD, serverAddrPtr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        close(serverSocketFD);
        return EXIT_FAILURE;
    }

    if (listen(serverSocketFD, 2) < 0) {
        perror("listen failed");
        close(serverSocketFD);
        return EXIT_FAILURE;
    }

    printf("TCP Echo Server listening on %s:%d\n", ip, port);
    printf("Press Ctrl+C to stop the server\n");

    while (1) {
        int clientSocketFD = accept(serverSocketFD, clientAddrPtr, &clientLen);
        if (clientSocketFD < 0) {
            perror("accept failed");
            continue;
        }

        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
        int clientPort = ntohs(clientAddr.sin_port);

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            close(clientSocketFD);
        } else if (pid == 0) {
            close(serverSocketFD);

            printf("Client %s:%d connected\n", clientIp, clientPort);
            handleClient(clientSocketFD, clientIp, clientPort);
            close(clientSocketFD);
            printf("Client %s:%d disconnected\n", clientIp, clientPort);

            return EXIT_SUCCESS;
        } else {
            close(clientSocketFD);
        }
    }
}
