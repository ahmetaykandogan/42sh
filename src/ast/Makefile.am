lib_LIBRARIES = libast.a

libast_a_SOURCES =  \
  ast.c \
  ast_init.c \
  ast_destroy.c \
  ast_add.c \
  ast_redir.c \
  ast.h \
  variable.c \
  variable.h \
  functions.c \
  functions.h

libast_a_CPPFLAGS = -I$(top_srcdir)/src

libast_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

noinst_LIBRARIES = libast.a
