#!/bin/sh

# Pipeline exit status (last true)

false | true; echo $?