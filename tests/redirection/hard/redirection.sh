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

echo -e "$BLUE============================HARD REDIRECTION====================$NC"

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

    if diff -u "$TEST" "$REF" > /dev/null ; then
        echo "OK"
        OK_test=$((OK_test + 1))
    else
        echo "KO"
        diff -u "$TEST" "$REF"
        KO_test=$((KO_test + 1))
    fi
    echo "$BLUE===================================================================$NC"
}

echo "$GREEN===================================================================$NC"
echo Test redirection
echo "$GREEN===================================================================$NC"
testcase override_redirection echo first > file1 > file2
testcase append_truncate echo A > file; echo B >> file; cat file
testcase redirection_without_command > only_file # if implemented yet
testcase multiple_redirection echo X > f1 echo Y > f2 cat < f1 < f2
testcase stdin_form_file_to_stdout_to_file cat 0<input.txt 1>out.txt
testcase stdin_and_stderr grep foo 0<input.txt 2>err.txt
testcase all_three grep foo 0<input.txt 1>out.txt 2>err.txt
testcase multiple_override echo hi 1>f1 1>f2 1>f3


echo "$BLUE===================================================================$NC"
echo "Nombre de test: $nb_test  |  Test passed: $OK_test / $nb_test  |  Test KO: $KO_test / $nb_test"
echo "$BLUE===================================================================$NC"