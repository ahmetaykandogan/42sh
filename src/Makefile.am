# define the subdirectories
SUBDIRS = ast \
      parser \
      lexer

bin_PROGRAMS = 42sh

42sh_SOURCES = 42sh.c

42sh_CPPFLAGS = -I%D%

42sh_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

42sh_LDADD =  \
       ast/libast.a \
       parser/libparser.a \
       lexer/liblexer.a 
