#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    printf("PID: %d\n", getpid());
    sleep(1);

    // if (!getenv("EXECUTED")) {
    //     setenv("EXECUTED", "1", 1);
    //     if (execv(argv[0], argv) == -1) {
    //         perror("execv");
    //         return EXIT_FAILURE;
    //     }
    // }

    if (execv(argv[0], argv) == -1) {
        perror("execv");
        return EXIT_FAILURE;
    }

    printf("Hello world\n");

    return EXIT_SUCCESS;
}
