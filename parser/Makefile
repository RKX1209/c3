# C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209
include ../Makefile.common
BISON 		= 	bison
FLEX			= 	flex
FORMATS		=		smt2
OBJS			+= 	parsesmt2.o lexsmt2.o

define generate_code
	$(BISON) --debug -v -o parse$(1).c -d -p $(1) $(1).y
	$(FLEX) -Cfe -o lex$(1).c -P$(1) $(1).lex
endef

.PHONY: all generate clean
all : generate $(OBJS)
	$(CC) -o parser.o $(OBJS)
generate :
	$(foreach format, $(FORMATS), $(call generate_code,$(format)))
clean :
	rm *.c *.h *.output
.c.o :
	$(CC) $(CFLAGS-$(DEBUG)) -c -o $@ $<
