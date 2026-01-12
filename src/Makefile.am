# define the subdirectories
SUBDIRS = ast \
      parser \
      lexer \
      utils \
      builtins \
      pretty_printer

bin_PROGRAMS = 42sh

42sh_SOURCES = main.c

42sh_CPPFLAGS = -I%D%

42sh_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla -g

42sh_LDADD =  \
       ast/libast.a \
       parser/libparser.a \
       lexer/liblexer.a \
       builtins/libbuiltins.a \
       pretty_printer/libpretty.a \
       utils/libextra.a 

