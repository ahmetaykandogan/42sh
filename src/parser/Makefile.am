lib_LIBRARIES = libparser.a

libparser_a_SOURCES =  \
  parser.c \
  parse_shell.c \
  parse_command.c \
  parse_redirections.c \
  parser.h

libparser_a_CPPFLAGS = -I$(top_srcdir)/src

libparser_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

noinst_LIBRARIES = libparser.a
