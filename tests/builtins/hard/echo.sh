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

echo -e "$BLUE=============================HARD==================================$NC"

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

testcase lot_words .src/42sh echo 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42

echo "$GREEN===================================================================$NC"
echo Test pour le -n
echo "$GREEN===================================================================$NC"
testcase multiple_following_n echo -n -n -n HI hello bonjour Salut
testcase multiple_line_n echo -n \
                                hi \
                                hi \
                                hi \
                                happy

echo "$GREEN===================================================================$NC"
echo Test pour le -e
echo "$GREEN===================================================================$NC"
# '\t' '\n' \\
testcase multiple_following_e echo -e -e -e -e HI '\t' '\n' Goodbye
testcase multiple_\n_\t_\\ echo -e HI '\t' '\t''\t' '\n''\n' '\n' '\\' _'\\' '\\' Hello '\t' '\n' World
testcase multiple_\n_\t_\\ echo -e HI \t \t\t \n\n \n \\ _\\ \\ Hello \t \n World


echo "$GREEN===================================================================$NC"
echo Test pour le -E
echo "$GREEN===================================================================$NC"
# '\t' '\n' '\\''
testcase multiple_following_E echo -E -E -E -E HI '\t' '\n' Goodbye
testcase multiple_\n_\t_\\_E echo -E HI '\t' '\t''\t' '\n''\n' '\n' '\\' _'\\' '\\' Hello '\t' '\n' World
testcase multiple_\n_\t_\\_E echo -E HI \t \t\t \n\n \n \\ _\\ \\ Hello \t \n World

echo "$GREEN===================================================================$NC"
echo Test mixed option
echo "$GREEN===================================================================$NC"
# '\t' '\n' '\\''
testcase multiple_E_e_last_E echo -E -E -E -e -E -E -e -e -e -E HI '\t' '\n' Goodbye
testcase multiple_e_E_last_e echo -E -E -E -e -E -E -e -e -e -e HI '\t' '\n' Goodbye
testcase multiple_e_E_n_last_e echo -n -E -E -E -e -n -E -E -e -e -e -e HI '\t' '\n' Goodbye
testcase multiple_e_E_n_last_e echo -E -E -n -E -e -E -n -E -e -e -e -e HI '\t' '\n' Goodbye

echo "$BLUE===================================================================$NC"
echo "Nombre de test: $nb_test  |  Test passed: $OK_test / $nb_test  |  Test KO: $KO_test / $nb_test"
echo "$BLUE===================================================================$NC"