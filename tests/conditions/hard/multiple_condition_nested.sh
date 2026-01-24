#!/bin/sh

# Multiples conditions a nested one

if true; then if false; then echo no; else echo yes; fi; fi
