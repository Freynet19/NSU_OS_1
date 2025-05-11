#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int globalInitialized = 10;
int globalUninitialized;
const int globalConst = 20;

char* createLocalVar() {
    char locVar = 'o';
    printf("ptr inside func: %p\n", &locVar);
    return &locVar;
}

long createLocalVarLong() {
    char locVar = 'o';
    printf("ptr inside func: %p\n", &locVar);
    return (long)&locVar;
}

void heapOperations() {
    char* buffer1 = malloc(100);
    // strcpy(buffer1, "First buffer");
    strcpy(buffer1, "0123456789abcdef0123456789abcdef");
    printf("%s\n", buffer1);
    free(buffer1);
    for (int i = 0; i < 32; ++i) {
        printf("%c", buffer1[i]);
    }
    // printf("%s\n", buffer1);
    printf("\n");

    // char* buffer2 = malloc(100);
    // strcpy(buffer2, "Second buffer");
    // printf("%s\n", buffer2);
    // free(buffer2 + 50);
    // free(buffer2);
}

void printAndModifyEnv() {
    const char* env = getenv("CUSTOM_ENV");
    printf("ENV_VAR: %s\n", env ? env : "(null)");
    setenv("CUSTOM_ENV", "new_value", 1);
    env = getenv("CUSTOM_ENV");
    printf("ENV_VAR: %s\n", env ? env : "(null)");

    printf("%p\n", env);
}

int main() {
    int localVar;
    static int staticVar;
    const int constVar = 30;

    printf("local: %p\n"
           "static: %p\n"
           "const: %p\n"
           "global init: %p\n"
           "global uninit: %p\n"
           "global const: %p\n\n",
           (void*)&localVar,
           (void*)&staticVar,
           (void*)&constVar,
           (void*)&globalInitialized,
           (void*)&globalUninitialized,
           (void*)&globalConst);

    printf("pid: %d\n\n", getpid());

    char* externalLocVar = createLocalVar();
    printf("ptr outside func: %p\n", externalLocVar);

    printf("%p\n", createLocalVarLong());

    heapOperations();



    // printAndModifyEnv();

    // sleep(30);
    return 0;
}
