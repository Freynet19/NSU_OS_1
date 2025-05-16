#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <SERVER_IP> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr = {};
    char recvbuf[BUFFER_SIZE];
    char sendbuf[BUFFER_SIZE];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);

    const struct sockaddr* serverAddrPtr = (const struct sockaddr *)&serverAddr;

    printf("UDP Echo Client\n"
           "Type 'exit' to quit\n");

    while (1) {
        printf("Enter message: ");
        fgets(sendbuf, BUFFER_SIZE, stdin);
        sendbuf[strcspn(sendbuf, "\n")] = '\0';

        if (strcmp(sendbuf, "exit") == 0) break;

        sendto(sockfd, sendbuf, strlen(sendbuf), 0, serverAddrPtr, sizeof(serverAddr));
        printf("Message sent\n");

        ssize_t len = recvfrom(sockfd, recvbuf, BUFFER_SIZE, 0, nullptr, nullptr);
        if (len < 0) {
            perror("recvfrom failed");
            continue;
        }
        recvbuf[len] = '\0';
        printf("Server reply: %s\n", recvbuf);
    }

    close(sockfd);
    printf("Client closed\n");
    return EXIT_SUCCESS;
}
