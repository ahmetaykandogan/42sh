SUBDIRS = \
  check_char \
  fifo \
  malloc_err

lib_LIBRARIES = libutils.a

libutils_a_CPPFLAGS = \
-I$(top_srcdir)/src \
-I$(srcdir)/check_char \
-I$(srcdir)/fifo \
-I$(srcdir)/malloc_err

libutils_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

noinst_LIBRARIES = libutils.a
