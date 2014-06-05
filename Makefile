# Makefile for flan.

VERSION=0.0.8

CC = gcc
CFLAGS += -std=c99
CFLAGS += -Werror

# clang's warnings are very aggressive.
# CC = clang
# CFLAGS += -Weverything -Wno-padded -Wno-format-nonliteral -Wno-switch-enum
# # What happens to values outside the enum?
# CFLAGS += -Wno-covered-switch-default
# # If this is the biggest sin we commit...
# CFLAGS += -Wno-float-equal
# # Intentional!
# CFLAGS += -Wno-vla

# Quell flex's insistence on #including stdlib.h before we get a chance to #define anything
CFLAGS += -D_POSIX_SOURCE

# Common CC flags.
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -O2
# CFLAGS += -ggdb
CFLAGS += -pedantic

LDFLAGS += -lm

# Define these if you want to use the Boehm garbage collector.
CFLAGS += -DBOEHM_GC
LDFLAGS += -lgc

# Profiling
#CFLAGS += -pg

LEX      = flex
LEXFLAGS = -t

GENHEADER = ./genHeader.pl

objects = builtins.o errors.o environment.o lexer.o list.o main.o parser.o \
	  prettyprinter.o symbols.o types.o flange.o graphics.o \
	  values.o eval-fos.o eval-hos.o eval-hol.o util.o
others = lexer.c lexer.h flan image.svg

all: flan

flan: $(objects)
	$(CC) $(CFLAGS) $(LDFLAGS) -o flan $(objects)

flange.o: flange.c flange.h builtins.h environment.h errors.h eval.h graphics.h lexer.h symbols.h types.h

graphics.o: graphics.c graphics.h util.h

builtins.o: builtins.c builtins.h environment.h eval.h parser.h symbols.h types.h values.h

environment.o: environment.c environment.h errors.h parser.h prettyprinter.h symbols.h util.h

errors.o: errors.c errors.h lexer.h

eval-fos.o: eval-fos.c builtins.h environment.h errors.h eval.h parser.h prettyprinter.h values.h

eval-hos.o: eval-hos.c builtins.h environment.h errors.h eval.h parser.h prettyprinter.h values.h

eval-hol.o: eval-hol.c builtins.h environment.h errors.h eval.h parser.h prettyprinter.h values.h

lexer.c: lexer.l $(GENHEADER)
	$(LEX) $(LEXFLAGS) lexer.l > lexer.c

lexer.h: lexer.c
	$(GENHEADER) lexer.l

lexer.o: lexer.c lexer.h util.h

list.o: list.h list.c util.h

main.o: main.c builtins.h errors.h eval.h flange.h lexer.h list.h parser.h prettyprinter.h symbols.h types.h util.h values.h

parser.o: parser.c errors.h lexer.h list.h parser.h types.h util.h

prettyprinter.o: prettyprinter.c prettyprinter.h list.h parser.h symbols.h

symbols.o: symbols.c list.h symbols.h util.h

types.o: types.c environment.h errors.h list.h parser.h prettyprinter.h symbols.h types.h util.h

util.o: util.c util.h

values.o: values.c builtins.h errors.h eval.h prettyprinter.h util.h values.h

clean:
	rm -f flan core $(objects) $(others) flan-$(VERSION).tar.gz

# For distribution.
SRC = NOTES,*.[lch],tests,*.pl,Makefile
flan-$(VERSION).tar.gz:
	(cd ..; ln -f -s flan flan-$(VERSION))
	(cd ..; tar -czf flan-$(VERSION).tar.gz flan-$(VERSION)/{$(SRC)})
	mv -f ../flan-$(VERSION).tar.gz .
	rm -f ../flan-$(VERSION)

dist: flan-$(VERSION).tar.gz
