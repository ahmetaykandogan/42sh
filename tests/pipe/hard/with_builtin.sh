#!/bin/sh

# Utilising the builtin only step 1 for the moment

echo 1 | true | false | wc -c
