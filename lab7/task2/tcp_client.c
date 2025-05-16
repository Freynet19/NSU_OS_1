#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <SERVER_IP> <PORT>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in serverAddr = {};
    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(port);

    const struct sockaddr* serverAddrPtr = (const struct sockaddr *)&serverAddr;

    printf("TCP Echo Client\n"
       "Type 'exit' to quit\n");

    if (connect(socketFD, serverAddrPtr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        close(socketFD);
        return EXIT_FAILURE;
    }

    printf("Connected to server at %s:%d\n", ip, port);

    while (1) {
        printf("Enter message: ");
        fgets(sendbuf, sizeof(sendbuf), stdin);
        sendbuf[strcspn(sendbuf, "\n")] = '\0';
        if (strcmp(sendbuf, "exit") == 0) break;

        ssize_t msgLen = (ssize_t)strlen(sendbuf);

        ssize_t totalWritten = 0;
        while (totalWritten < msgLen) {
            ssize_t writtenNow = write(socketFD, sendbuf + totalWritten, msgLen - totalWritten);
            if (writtenNow == -1) {
                perror("write failed");
                // break;
                close(socketFD);
                return EXIT_FAILURE;
            }
            totalWritten += writtenNow;
        }

        ssize_t bytesRead = read(socketFD, recvbuf, sizeof(recvbuf));
        if (bytesRead > 0) {
            recvbuf[bytesRead] = '\0';
            printf("Server reply: %s\n", recvbuf);
        }
    }

    close(socketFD);
    return EXIT_SUCCESS;
}
