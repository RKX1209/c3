# C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209
CC 			=		gcc
CXX			=		g++
CP			=		cp
MAKE		=		make
OBJS		=		core.o list.o hashmap.o bstree.o
C3			=		c3
INCLUDE =		include
DEBUG		=		y
CFLAGS	=		-I$(INCLUDE)
CFLAGS-y= 	-g $(CFLAGS)
.PHONY: all install test clean
all: $(OBJS)
	$(CC) -o $(C3) $(OBJS)
install:
	$(CP) $(C3) /usr/bin/
test:
	./test/run_tests.sh
clean:
	rm -rf *.o $(C3)
.c.o :
	$(CC) $(CFLAGS-$(DEBUG)) -c -o $@ $<
