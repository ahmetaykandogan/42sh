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

echo -e "$BLUE===========================ERROR REDIRECTION======================$NC"

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
testcase redirection_missing_file cat < does_not_exist
testcase bad_file_descriptor echo hi 9>file
testcase redirect_with_clise_fd exec 3>&- echo hi 1>&3
testcase no_target echo hi 2>&


echo "$BLUE===================================================================$NC"
echo "Nombre de test: $nb_test  |  Test passed: $OK_test / $nb_test  |  Test KO: $KO_test / $nb_test"
echo "$BLUE===================================================================$NC"