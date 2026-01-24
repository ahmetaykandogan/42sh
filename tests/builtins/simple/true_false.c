#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "../../../src/builtins/builtins.h"

// TestSuite definition
TestSuite(TestTrueFalse);

Test(TestTrueFalse, test_basic_true)
{
    int result = builtins_true();
    int expected = 0;
    cr_expect(eq(result, expected));
}

Test(TestTrueFalse, test_basic_false)
{
    int result = builtins_false();
    int expected = 1;
    cr_expect(eq(result, expected));
}
