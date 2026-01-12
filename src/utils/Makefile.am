lib_LIBRARIES = libextra.a

libextra_a_SOURCES =  \
  extra/malloc_err.c \
  extra/malloc_err.h \
  fifo/fifo_access.c \
  fifo/fifo_destroy.c \
  fifo/fifo.h

libextra_a_CPPFLAGS = -I$(top_srcdir)/src

libextra_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

noinst_LIBRARIES = libextra.a
