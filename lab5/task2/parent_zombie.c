#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    pid_t pid_dad = fork();

    if (pid_dad == 0) {
        pid_t pid_son = fork();

        if (pid_son == 0) {
            printf("son:\nPID=%d, PPID=%d\n\n", getpid(), getppid());
            sleep(20);
            printf("son after sleep:\n"
                   "PID=%d, PPID=%d\n", getpid(), getppid());
            exit(0);
        }

        printf("dad:\nPID=%d, PPID=%d\n", getpid(), getppid());
        sleep(10);
        printf("dad terminating...\n");
        exit(0);
    }

    sleep(25);
    printf("grandpa terminating...\n");
    exit(0);
}
