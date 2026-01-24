#!/bin/sh

# If elif and else with the cond in the elif

if false; then echo a; elif true; then echo b; else echo c; fi
