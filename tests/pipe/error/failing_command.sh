#!/bin/sh

# Failing command in the middle

echo test | nonexistent_cmd | wc -c
