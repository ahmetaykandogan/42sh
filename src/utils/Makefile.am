SUBDIRS = \
  check_char \
  fifo \
  malloc_err

noinst_LIBRARIES = libutils.a

libutils_a_SOURCES =

libutils_a_LIBADD = \
  $(top_builddir)/src/utils/check_char/libcchar.a \
  $(top_builddir)/src/utils/fifo/libfifo.a \
  $(top_builddir)/src/utils/malloc_err/libmallocer.a

libutils_a_CPPFLAGS = \
  -I$(top_srcdir)/src \
  -I$(srcdir)/check_char \
  -I$(srcdir)/fifo \
  -I$(srcdir)/malloc_err

libutils_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla
