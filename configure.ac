# Init the 42sh project
AC_INIT([42sh], [1.0], [42sh@assistants.epita.fr])

# Setup Automake
AM_INIT_AUTOMAKE([subdir-objects] [foreign])

# Pretty display of Makefile rules
AM_SILENT_RULES([yes])

# Enable ar for Makefile
AM_PROG_AR

# Check if ranlib is available
AC_PROG_RANLIB

# Check if a C compiler is available
AC_PROG_CC

# Check if a compiler has this list of flags
AX_COMPILER_FLAGS([], [], [], [-std=c99 -pedantic -Werror -Wall -Wextra -Wvla])

# List Makefiles in subdirectories
AC_CONFIG_FILES([
                    Makefile
                    src/Makefile
                    src/ast/Makefile
                    src/lexer/Makefile
                    src/lexer/create_lextoken/Makefile
                    src/lexer/lexing/Makefile
                    src/lexer/read_file/Makefile
                    src/lexer/case_handler/Makefile
                    src/parser/Makefile
                    src/utils/Makefile
                    src/utils/check_char/Makefile
                    src/utils/fifo/Makefile
                    src/utils/malloc_err/Makefile
                    src/builtins/Makefile
                    src/pretty_printer/Makefile
                    tests/Makefile
                    tests/pretty_printer/Makefile
                    tests/pretty_printer/hard/Makefile
                    tests/pretty_printer/error/Makefile
                    tests/pretty_printer/simple/Makefile
                    tests/builtins/Makefile
                    tests/builtins/simple/Makefile
                ])
AC_OUTPUT
