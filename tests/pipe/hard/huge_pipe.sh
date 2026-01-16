#!/bin/sh

# Huge pipe using a seq to try and break it

seq 1 10000 | grep 5 | wc -l
