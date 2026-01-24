#!/bin/sh

# Redirection plus pipe

echo test | cat > /tmp/pipe.txt; cat /tmp/pipe.txt;
