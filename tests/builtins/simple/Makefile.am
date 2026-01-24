check_PROGRAMS = echo \
		 true_false

echo_SOURCES =  \
    echo.c

echo_CPPFLAGS = -I$(top_srcdir)/src

echo_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla

echo_LDADD =  \
    $(top_srcdir)/src/builtins/libbuiltins.a

echo_LDFLAGS = -lcriterion

true_false_SOURCES =  \
    true_false.c

true_false_CPPFLAGS = -I$(top_srcdir)/src

true_false_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla

true_false_LDADD =  \
    $(top_srcdir)/src/builtins/libbuiltins.a

true_false_LDFLAGS = -lcriterion
