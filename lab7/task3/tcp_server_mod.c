#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

int serverSocketFD;

void shutdownServer(int) {
    close(serverSocketFD);
    printf("\nServer shutting down...\n");
    exit(EXIT_SUCCESS);
}

typedef struct {
    int socketFD;
    char buffer[BUFFER_SIZE];
    size_t totalToWrite;
    size_t totalWritten;
} ClientData;

ClientData clientsData[MAX_CLIENTS] = {0};

int setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int handleClient(int clientSocketFD, const char* clientIp, int clientPort) {
    ClientData *clientData = nullptr;
    int emptySlot = -1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientsData[i].socketFD == clientSocketFD) {
            clientData = &clientsData[i];
            break;
        }
        if (emptySlot == -1 && clientsData[i].socketFD == 0) {
            emptySlot = i;
        }
    }

    if (clientData && clientData->totalToWrite > 0) {
        ssize_t writtenNow = write(clientSocketFD,
            clientData->buffer + clientData->totalWritten,
            clientData->totalToWrite - clientData->totalWritten);

        if (writtenNow == -1) {
            if (errno == EWOULDBLOCK) return 1;
            perror("write failed");
            return -1;
        }

        clientData->totalWritten += writtenNow;

        if (clientData->totalWritten >= clientData->totalToWrite) {
            memset(clientData, 0, sizeof(ClientData));
        }
        return 1;
    }

    if (clientData == NULL) {
        if (emptySlot == -1) {
            fprintf(stderr, "No space for new client data\n");
            return -1;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytesRead = read(clientSocketFD, buffer, sizeof(buffer) - 1);

        if (bytesRead < 0) {
            if (errno != EWOULDBLOCK) {
                perror("error reading from socket");
                return -1;
            }
            return 1;
        }
        if (bytesRead == 0) return 0;

        buffer[bytesRead] = '\0';
        printf("Message received from %s:%d: %s\n", clientIp, clientPort, buffer);

        clientData = &clientsData[emptySlot];
        clientData->socketFD = clientSocketFD;
        memcpy(clientData->buffer, buffer, bytesRead);
        clientData->totalToWrite = bytesRead;
        clientData->totalWritten = 0;
    }

    return 1;
}

int main() {
    signal(SIGINT, shutdownServer);

    const char *ip = IP;
    int port = PORT;
    struct sockaddr_in serverAddr = {}, clientAddr = {};
    socklen_t clientLen = sizeof(clientAddr);
    fd_set readFDs, writeFDs;
    int clientSockets[MAX_CLIENTS] = {0};

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

    if (bind(serverSocketFD, serverAddrPtr, sizeof(serverAddr))) {
        perror("bind failed");
        close(serverSocketFD);
        return EXIT_FAILURE;
    }

    if (listen(serverSocketFD, 5)) {
        perror("listen failed");
        close(serverSocketFD);
        return EXIT_FAILURE;
    }

    printf("TCP Echo Multiplex Server listening on %s:%d\n", ip, port);
    printf("Press Ctrl+C to stop the server\n");

    while (1) {
        FD_ZERO(&readFDs);
        FD_ZERO(&writeFDs);
        FD_SET(serverSocketFD, &readFDs);
        int maxFD = serverSocketFD;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientSockets[i] > 0) {
                bool hasToSend = false;
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    ClientData cd = clientsData[j];
                    if (cd.socketFD == clientSockets[i] && cd.totalToWrite > cd.totalWritten) {
                        hasToSend = true;
                        break;
                    }
                }

                if (hasToSend) FD_SET(clientSockets[i], &writeFDs);
                else FD_SET(clientSockets[i], &readFDs);

                if (maxFD < clientSockets[i]) maxFD = clientSockets[i];
            }
        }

        if (select(maxFD + 1, &readFDs, &writeFDs, nullptr, nullptr) < 0) {
            perror("select failed");
            continue;
        }

        if (FD_ISSET(serverSocketFD, &readFDs)) {
            int newSocket = accept(serverSocketFD, clientAddrPtr, &clientLen);
            if (newSocket < 0) {
                perror("accept failed");
                continue;
            }

            if (setNonBlocking(newSocket) < 0) {
                perror("failed to set non-blocking mode");
                close(newSocket);
                continue;
            }

            char clientIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
            int clientPort = ntohs(clientAddr.sin_port);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    printf("Client %s:%d connected\n", clientIp, clientPort);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientSockets[i] > 0) {
                char clientIp[INET_ADDRSTRLEN];
                struct sockaddr_in addr;
                socklen_t addrLen = sizeof(addr);

                getpeername(clientSockets[i], (struct sockaddr*)&addr, &addrLen);
                inet_ntop(AF_INET, &addr.sin_addr, clientIp, INET_ADDRSTRLEN);
                int clientPort = ntohs(addr.sin_port);

                if (FD_ISSET(clientSockets[i], &readFDs) ||
                    FD_ISSET(clientSockets[i], &writeFDs)) {
                    int result = handleClient(clientSockets[i], clientIp, clientPort);
                    if (result <= 0) {
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clientsData[j].socketFD == clientSockets[i]) {
                                memset(&clientsData[j], 0, sizeof(ClientData));
                                break;
                            }
                        }
                        close(clientSockets[i]);
                        printf("Client %s:%d disconnected\n", clientIp, clientPort);
                        clientSockets[i] = 0;
                    }
                }
            }
        }
    }
}
