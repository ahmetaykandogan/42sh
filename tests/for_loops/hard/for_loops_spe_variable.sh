#!/bin/sh

# For loops with special variables

set a b c; for arg in "$@"; do echo $arg; done
