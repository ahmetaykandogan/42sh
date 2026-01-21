lib_LIBRARIES = libast.a

libast_a_SOURCES =  \
  ast.c \
  ast.h \
  variable.c \
  variable.h

libast_a_CPPFLAGS = -I$(top_srcdir)/src

libast_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

noinst_LIBRARIES = libast.a
