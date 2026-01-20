SUBDIRS = \
  read_file \
  create_lextoken \
  lexing \
  case_handler

noinst_LIBRARIES = liblexer.a

liblexer_a_LIBADD = \
  $(top_builddir)/src/lexer/read_file/libread.a \
  $(top_builddir)/src/lexer/create_lextoken/liblextoken.a \
  $(top_builddir)/src/lexer/lexing/liblexing.a \
  $(top_builddir)/src/lexer/case_handler/libhandler.a 

liblexer_a_SOURCES =  \
  lexer.c \
  lexer.h \
  struct_lexer.h

liblexer_a_CPPFLAGS = \
  -I$(top_srcdir)/src \
  -I$(srcdir)/read_file \
  -I$(srcdir)/create_lextoken \
  -I$(srcdir)/lexing \
  -I$(srcdir)/case_handler

liblexer_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla
