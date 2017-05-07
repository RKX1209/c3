# C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209
include Makefile.common

SUBDIRS	+=		parser
OBJS		+=		core.o list.o hashmap.o bstree.o parser/parser.o

.PHONY: all build-all install test clean
all: build-all $(OBJS)
	$(CC) -o $(C3) $(OBJS)
build-all: $(SUBDIRS)
	$(INFO) 'build-all'
	$(MAKE) -C $<
install:
	$(CP) $(C3) $(INSTALL)
test:
	./run_tests.sh
clean: $(SUBDIRS)
	rm -rf *.o $(C3)
	$(MAKE) clean -C $<
.c.o :
	$(CC) $(CFLAGS-$(DEBUG)) -c -o $@ $<
