# C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209
CC 			=		gcc
CXX			=		g++
MAKE		=		make
OBJS		=		core.o list.o
C3			=		c3
INCLUDE =		include
CFLAGS	=		-I$(INCLUDE)
.PHONY: all clean
all: $(OBJS)
	$(CC) -o $(C3) $(OBJS)
clean:
	rm -rf *.o $(C3)
.c.o :
	$(CC) $(CFLAGS) -c -o $@ $<
