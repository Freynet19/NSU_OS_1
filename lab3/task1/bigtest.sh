#!/bin/bash

mkdir big_test && touch big_test/big_random.txt
dd if=/dev/random of=big_test/big_random.txt bs=1M count=1024
echo

ls -l big_test/big_random.txt
md5sum big_test/big_random.txt
echo

gcc reverse.c -o reverse -Wall -Wextra -Werror -pedantic
time ./reverse ./big_test
ls -l tset_gib/txt.modnar_gib

rm -r big_test
time ./reverse ./tset_gib
md5sum big_test/big_random.txt

rm -r big_test tset_gib
