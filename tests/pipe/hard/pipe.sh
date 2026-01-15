#!/bin/sh

#COLOR
RED='\033[0;31m'
GREEN='\033[0;32m'
MAGENTA='\033[0;35m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
NC='\033[0m'

BIN=./src/42sh

#FILE
REF=".ref.out"
TEST=".test.out"

echo -e "$BLUE============================HARD PIPE=============================$NC"

#diff -u pour affichage comme moulinette -e pour les echo si pb

#Var global
nb_test=0
OK_test=0
KO_test=0

testcase()
{
    nb_test=$((nb_test + 1))

    # $1 is the testname
    echo "$YELLOW test: $1 $NC"
    shift

    $BIN -c "$@" > "$TEST"
    $@ > "$REF" # verif si ca marche ou si besoin de changer

    $BIN $@
    got_return_code = $? 

    $@
    expected_return_code = $?

    if diff -u "$TEST" "$REF" > /dev/null ; then
        if [ $got_return_code -eq $expected_return_code ]; then
            echo "OK"
            OK_test=$((OK_test + 1))
        else
            echo "KO"
            echo expected return code was $expected_return_code but got $got_return_code
            KO_test=$((KO_test + 1))
    else
        echo "KO"
        diff -u "$TEST" "$REF"
        KO_test=$((KO_test + 1))
    fi
    echo "$BLUE===================================================================$NC"
}

echo "$GREEN===================================================================$NC"
echo Test pipeline
echo "$GREEN===================================================================$NC"
testcase long_pipeline echo "hello world" | cat | cat | cat | cat | wc -w
testcase without_output true | cat | wc -l
testcase early_exit yes | head -n 5 | wc -l
testcase without_output echo "hello world" | cat | cat | cat | cat | wc -w
testcase with_builtin cd / | pwd | wc -c
testcase huge_pipe seq 1 10000 | grep 5 | wc -l


echo "$BLUE===================================================================$NC"
echo "Nombre de test: $nb_test  |  Test passed: $OK_test / $nb_test  |  Test KO: $KO_test / $nb_test"
echo "$BLUE===================================================================$NC"