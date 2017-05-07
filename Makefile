# C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209
.PHONY: default
default: all

include Makefile.common

SUBDIRS	+=		parser
OBJS		+=		core.o list.o hashmap.o bstree.o

include parser/Makefile

.PHONY: all build-all install test clean
all: build-all $(OBJS)
	$(CC) -o $(C3) $(OBJS) $(LDFLAGS)
build-all: $(SUBDIRS)
	$(INFO) 'build-all'
	$(MAKE) -C $<
install:
	$(CP) $(C3) $(INSTALL)
test:
	./run_tests.sh
clean: $(SUBDIRS)
	rm -rf *.o $(C3)
	rm parser/*.c parser/*.h parser/*.output
.c.o :
	$(CC) $(CFLAGS-$(DEBUG)) -c -o $@ $<
