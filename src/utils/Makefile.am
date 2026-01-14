lib_LIBRARIES = libextra.a

libextra_a_SOURCES =  \
  malloc_err.c \
  malloc_err.h \
  fifo_access.c \
  fifo_setup_destroy.c \
  fifo.h \
  check_char.c \
  check_char.h

libextra_a_CPPFLAGS = -I$(top_srcdir)/src

libextra_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

noinst_LIBRARIES = libextra.a
