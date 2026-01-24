#!/bin/sh

# Exit status of the pipe with a negation

! false | true; echo 0
