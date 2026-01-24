check_PROGRAMS = pretty_printer

pretty_printer_SOURCES =  \
    pretty_printer.c

pretty_printer_CPPFLAGS = -I$(top_srcdir)/src

pretty_printer_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla

pretty_printer_LDADD =  \
    $(top_srcdir)/src/pretty_printer/libpretty.a

pretty_printer_LDFLAGS = -lcriterion
