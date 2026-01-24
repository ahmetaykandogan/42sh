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
TEST_DIR=${1:-.}
MODULE=$2

# Check for required environment variable
if [ -z "$BIN_PATH" ]; then
    BIN_PATH="$(pwd)/../src/42sh"
fi

# -------------------------- TESTS -------------------------- #
if [ -z "$MODULE" ]; then
    TESTS_TO_RUN=$(ls "$TEST_DIR" 2>/dev/null | grep -v Makefile | grep -v testsuite.sh)
else
    TESTS_TO_RUN=$MODULE
fi

# -------------------------- RECAP -------------------------- #
RECAP=/tmp/recap_$$
echo "============ RECAP ============" > "$RECAP"

total_tests=0
total_passed=0

for dir in $TESTS_TO_RUN; do
    # Skip if not a directory
    if [ ! -d "$TEST_DIR/$dir" ]; then
        continue
    fi
    
    cat_total=0
    cat_passed=0

    # Find all .sh files in this module directory
    for test_file in $(find "$TEST_DIR/$dir" -type f -name "*.sh" 2>/dev/null | sort); do
        cat_total=$((cat_total + 1))
        total_tests=$((total_tests + 1))
        
        # Run with your shell
        "$BIN_PATH" "$test_file" > /tmp/out_$$ 2>&1
        your_exit=$?
        
        # Run with reference bash
        bash --posix "$test_file" > /tmp/ref_out_$$ 2>&1
        ref_exit=$?
        
        ref_out=/tmp/ref_out_$$
        out=/tmp/out_$$
        
        # Compare outputs
        if diff -u "$out" "$ref_out" > /dev/null && [ $your_exit -eq $ref_exit ]; then
            test_passed=true
        else
            test_passed=false
        fi
        
        if $test_passed; then
            cat_passed=$((cat_passed + 1))
            total_passed=$((total_passed + 1))
        else
            echo -e "${RED}FAILED${NC}: $test_file"
            echo
            if [ $your_exit -ne $ref_exit ]; then
                echo -e "Exit codes differ: expected $ref_exit, got $your_exit"
            fi
            echo -e "=== Diff:"
            echo -e "Content of stdout differs:"
            echo -e "Expected: ${GREEN}"
            cat "$ref_out"
            echo -e "${NC}"
            echo -e "Got: ${RED}"
            cat "$out"
            echo -e "${NC}"
            echo
        fi

        # Clean temp files
        rm -f "$out" "$ref_out"
    done

    # Only print recap line if tests were found
    if [ $cat_total -gt 0 ]; then
        if [ $cat_passed -eq $cat_total ]; then
            color=$GREEN
        else
            color=$RED
        fi
        printf "%-15s ${color}%+15s${NC}\n" "$dir" "($cat_passed/$cat_total passed)" >> "$RECAP"
    fi
done

# Find all binary for unary tests
if [ ! -z $COVERAGE ]; then
    cat_total=0
    cat_passed=0
    for dir in $TESTS_TO_RUN; do
        for test_file in $(find "$TEST_DIR/$dir" -type f -executable 2>/dev/null | sort); do
            total_tests=$((total_tests + 1))
            cat_total=$((cat_total + 1))

            ./$test_file
            exit_unit=$?

            if [ $exit_unit -eq 0 ]; then
                total_passed=$((total_passed + 1))
                cat_passed=$((cat_passed + 1))
            fi
        done
    done
    printf "%-15s ${color}%+15s${NC}\n" "unit_test_files" "($cat_passed/$cat_total passed)" >> "$RECAP"
fi

cat "$RECAP"
rm -f "$RECAP"

if [ ! -z $OUTPUT_FILE ]; then
    if [ $cat_total -ne 0 ]; then
        POURCENT=$((($cat_passed / $cat_total) * 100))
        echo $POURCENT > $OUTPUT_FILE
    fi
fi

echo
echo "============ TOTAL ============"
if [ $total_passed -eq $total_tests ]; then
    echo -e "${GREEN}All tests passed: $total_passed/$total_tests${NC}"
else
    echo -e "${RED}Some tests failed: $total_passed/$total_tests${NC}"
fi
