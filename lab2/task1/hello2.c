#include <unistd.h>

void print_hello(void) {
    const char *msg = "Hello, mom\n";
    write(1, msg, 11);
}

int main(void) {
    print_hello();
    return 0;
}

