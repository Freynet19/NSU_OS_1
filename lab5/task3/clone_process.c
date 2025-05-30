#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define STACK_SIZE (1024 * 1024)
#define RECURSION_DEPTH 10

void recursiveFunction(int counter) {
    if (counter < 1) return;
    char str[] = "hello world";
    (void)str;
    recursiveFunction(counter - 1);
}

int entryFunction(void *arg) {
    (void)arg;
    recursiveFunction(RECURSION_DEPTH);
    return 0;
}

int main() {
    int stackFD = open("stack_file", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (stackFD < 0) {
        perror("File creation failed");
        return 1;
    }
    ftruncate(stackFD, STACK_SIZE);
    
    void *stackPtr = mmap(nullptr, STACK_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED, stackFD, 0);
    if (stackPtr == MAP_FAILED) {
        perror("Memory mapping failed");
        close(stackFD);
        return 1;
    }
    
    void *stackTop = (char*)stackPtr + STACK_SIZE;
    pid_t pid = clone(entryFunction, stackTop, SIGCHLD, nullptr);
    if (pid < 0) {
        perror("Process creation failed");
        munmap(stackPtr, STACK_SIZE);
        close(stackFD);
        return 1;
    }
    
    waitpid(pid, nullptr, 0);
    munmap(stackPtr, STACK_SIZE);
    close(stackFD);
    return 0;
}
