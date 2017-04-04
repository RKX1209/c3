# C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209
CC 			=		gcc
CXX			=		g++
MAKE		=		make
OBJS		=		core.o list.o hashmap.o bstree.o
C3			=		c3
INCLUDE =		include
DEBUG		=		y
CFLAGS	=		-I$(INCLUDE)
CFLAGS-y= 	-g $(CFLAGS)
.PHONY: all clean
all: $(OBJS)
	$(CC) -o $(C3) $(OBJS)
clean:
	rm -rf *.o $(C3)
.c.o :
	$(CC) $(CFLAGS-$(DEBUG)) -c -o $@ $<
