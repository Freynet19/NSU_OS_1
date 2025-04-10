#!/bin/bash
for cmd in create_dir list_dir remove_dir create_file cat_file remove_file \
           create_symlink read_symlink follow_symlink remove_symlink \
           create_hardlink remove_hardlink stat_file chmod_file; do
    ln multitool $cmd
done
