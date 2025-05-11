#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>

void recursiveStack(int depth) {
    char arr[8 * 1024];
    printf("stack depth: %d\n", depth);
    sleep(1);
    recursiveStack(depth + 1);
}

void heapAllocation() {
    void *ptr;
    for (int i = 0; i < 1024; i++) {
        ptr = malloc(1024 * 1024);
        printf("heap allocated at %p\n", ptr);
        sleep(1);
    }
    free(ptr);
}

void sigsegvHandler() {
    printf("Caught SIGSEGV!\n");
    exit(1);
}

int main() {
    printf("PID: %d\n", getpid());
    sleep(10);

    // recursiveStack(0);

    // return 0;

    // heapAllocation();

    // return 0;

    size_t pageSize = getpagesize();
    void *region = mmap(NULL, 10 * pageSize, PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    printf("Mapped region: %p\n", region);
    sleep(2);

    // return 0;

    signal(SIGSEGV, sigsegvHandler);

    /*mprotect(region, 10 * pageSize, PROT_NONE);
    printf("Trying to read...\n");
    char data = *(char*)region;
    printf("%c\n", data);*/

    // return 0;

    /*mprotect(region, 10 * pageSize, PROT_READ);
    printf("Trying to write...\n");
    *(char*)region = 1;*/

    // return 0;

    munmap((char*)region + 4 * pageSize, 3 * pageSize);
    sleep(10);

    return 0;
}
