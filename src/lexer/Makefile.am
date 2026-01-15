SUBDIRS = \
  read_file \
  create_lextoken \
  lexing

noinst_LIBRARIES = liblexer.a

liblexer_a_LIBADD = \
  $(top_builddir)/src/lexer/read_file/libread.a \
  $(top_builddir)/src/lexer/create_lextoken/liblextoken.a \
  $(top_builddir)/src/lexer/lexing/liblexing.a

liblexer_a_SOURCES =  \
  lexer.c \
  lexer.h \
  struct_lexer.h

liblexer_a_CPPFLAGS = \
  -I$(top_srcdir)/src \
  -I$(srcdir)/read_file \
  -I$(srcdir)/create_lextoken \
  -I$(srcdir)/lexing


liblexer_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla
