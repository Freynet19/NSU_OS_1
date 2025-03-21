#!/bin/bash
gcc -c hello_static.c -o libhellostatic.o -Wall -Wextra -pedantic -Werror
ar r libhellostatic.a libhellostatic.o
gcc hello2.c -L . -l hellostatic -o hello2 -Wall -Wextra -pedantic -Werror
./hello2

