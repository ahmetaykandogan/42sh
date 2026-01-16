#!/bin/sh

# -------------------------- COLOR -------------------------- #
RED="\e[0;31m"
GREEN="\e[0;32m"
YELLOW="\e[0;33m"
BLUE="\e[0;34m"
PURPLE="\e[0;35m"
CYAN="\e[0;36m"
WHITE="\e[0;37m"
NC='\033[0m'


# -------------------------- GLOBV -------------------------- #
TEST_DIR=$1
MODULE=$2


# -------------------------- TESTS -------------------------- #
if [ -z "$MODULE" ]; then
    TESTS_TO_RUN=$(ls $TEST_DIR) # Fetch all tests if no module specified.
else
    TESTS_TO_RUN=$MODULE
fi

# -------------------------- RECAP -------------------------- #
RECAP=/tmp/recap

echo ============ RECAP ============ > $RECAP

for dir in $TESTS_TO_RUN; do
    cat_total=0
    cat_passed=0
    for test_file in $(find "$TEST_DIR/$dir" -name "*.sh"); do
        ./src/42sh "$test_file" > /tmp/out
        bash --posix "$test_file" > /tmp/ref_out

        ref_out=/tmp/ref_out
        out=/tmp/out

        if diff -u "$out" "$ref_out" > /dev/null; then
            test_passed=true
        else
            test_passed=false
        fi

        cat_total=$((cat_total + 1))
        if $test_passed; then
            cat_passed=$((cat_passed + 1))
        else
            echo -e "${RED}FAILED${NC}: $test_file"
            echo
            echo -e "=== Diff:"
            echo -e "Content of stdout differs:"
            echo -e "Expected: ${GREEN}"
            cat "$ref_out"
            echo -n ${NC}
            echo -e "Got: ${RED}"
            cat "$out"
            echo -n ${NC}
            echo
        fi
    done
    if [ $cat_passed -eq 0 ] && [ $cat_total -eq 0 ]; then
        continue;
    elif [ $cat_passed = $cat_total ]; then
        color=$GREEN
    else
        color=$RED
    fi
    printf "%-15s ${color}%+15s${NC}\n" $dir "($cat_passed/$cat_total passed)" >> $RECAP
done

cat $RECAP
