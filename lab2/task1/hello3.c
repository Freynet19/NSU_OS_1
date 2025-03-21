#include <unistd.h>
#include <sys/syscall.h>

void write_wrapper(int fd, const char *buf, size_t count) {
    syscall(SYS_write, fd, buf, count);
}

void print_hello(void) {
    const char *msg = "Hello, mom\n";
    write_wrapper(1, msg, 11);
}

int main(void) {
    print_hello();
    return 0;
}

