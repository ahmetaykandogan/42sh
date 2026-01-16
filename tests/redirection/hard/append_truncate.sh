#!/bin/sh

# Truncate a file then append to see if ok

echo A > file; echo B >> file; cat file