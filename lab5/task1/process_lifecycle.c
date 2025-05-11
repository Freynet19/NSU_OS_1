#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAKE_ZOMBIE 0

int global_var = 10;

int main() {
    int local_var = 20;

    printf("Variable addresses and values:\n");
    printf("Global: %p -> %d\n", &global_var, global_var);
    printf("Local:  %p -> %d\n", &local_var, local_var);
    printf("Parent PID: %d\n\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }
    if (pid == 0) {
        printf("Child process:\n");
        printf("PID: %d, PPID: %d\n", getpid(), getppid());

        printf("Original values:\n");
        printf("Global: %p -> %d\n", &global_var, global_var);
        printf("Local:  %p -> %d\n", &local_var, local_var);

        printf("Modified values:\n");
        global_var = 30;
        local_var = 40;
        printf("Global: %p -> %d\n", &global_var, global_var);
        printf("Local:  %p -> %d\n", &local_var, local_var);

        exit(5);
    }

    sleep(20);

    printf("\nParent process:\n");
    printf("Global: %p -> %d\n", &global_var, global_var);
    printf("Local:  %p -> %d\n", &local_var, local_var);

    if (MAKE_ZOMBIE == 0) {
        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            printf("Child exited with status: %d\n", WEXITSTATUS(status));
        } else {
            printf("Child terminated abnormally\n");
        }
    }

    int sleepFor = 10;
    printf("Shutting down in %d sec...\n", sleepFor);
    sleep(sleepFor);

    return 0;
}