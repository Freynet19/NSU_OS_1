#!/bin/bash
gcc -c hello_dynamic.c -o libhellodynamic.o -Wall -Wextra -pedantic -Werror
gcc -shared -fPIC -o libhellodynamic.so libhellodynamic.o -Wall -Wextra -pedantic -Werror
gcc hello3.c -o hello3 -L . -l hellodynamic -Wall -Wextra -pedantic -Werror
LD_LIBRARY_PATH=./ ./hello3

