#!/bin/sh

# Redirection with variable

FILE=/tmp/varfile.txt; echo content > $FILE; cat $FILE; rm $FILE
