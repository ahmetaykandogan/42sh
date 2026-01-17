#!/bin/sh

# Lot of pipe and print the number of word at the end of the pipe

echo "hello world" | cat | cat | cat | cat | wc -w
