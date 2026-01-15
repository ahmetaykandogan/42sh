# define the subdirectories
SUBDIRS = \
      utils \
      lexer \
      ast \
      parser \
      builtins \
      pretty_printer

bin_PROGRAMS = 42sh

42sh_SOURCES = main.c

42sh_CPPFLAGS = -I$(top_srcdir)/src

42sh_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla -g

42sh_LDADD = \
      ast/libast.a \
      parser/libparser.a \
      lexer/liblexer.a \
      lexer/lexing/liblexing.a \
      lexer/create_lextoken/liblextoken.a \
      lexer/read_file/libread.a \
      builtins/libbuiltins.a \
      pretty_printer/libpretty.a \
      utils/libutils.a \
      utils/fifo/libfifo.a \
      utils/check_char/libcchar.a \
      utils/malloc_err/libmallocer.a

