SUBDIRS = \
  create_lextoken \
  lexing \
  read_file

lib_LIBRARIES = liblexer.a

liblexer_a_SOURCES =  \
  lexer.c \
  lexer.h \
  struct_lexer.h

liblexer_a_CPPFLAGS = \
  -I$(top_srcdir)/src \
  -I$(srcdir)/create_lextoken \
  -I$(srcdir)/lexing \
  -I$(srcdir)/read_file

liblexer_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

noinst_LIBRARIES = liblexer.a
