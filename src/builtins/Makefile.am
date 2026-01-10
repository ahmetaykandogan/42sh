lib_LIBRARIES = libbuiltins.a

libbuiltins_a_SOURCES =  \
  builtins.c \
  builtins.h

libbuiltins_a_CPPFLAGS = -I$(top_srcdir)/src

libbuiltins_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

noinst_LIBRARIES = libbuiltins.a