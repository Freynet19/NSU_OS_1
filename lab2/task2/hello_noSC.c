#include <stdio.h>

void print_hello(void) {
    const char msg[] = "Hello, mom\n";
    long ret;
    asm volatile (
        "mov $1, %%rax\n"   // 1 - номер сисколла write
        "mov $1, %%rdi\n"   // 1 - номер фд stdout
        "mov %1, %%rsi\n"   // 1й аргумент - указатель на msg
        "mov $11, %%rdx\n"   // 11 - длина строки
        "syscall\n"         // вызов сисколла и прерывания
        "mov %%rax, %0"     // пишем результат (колво записанных байтов) в 0й аргумент 
        : "=r" (ret)
        : "r" (msg)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r11"
    );
}

int main(void) {
    print_hello();
    return 0;
}

