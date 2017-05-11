# C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209
.PHONY: default
default: all

include Makefile.common

SUBDIRS	+=		parser
OBJS		+=		core.o list.o hashmap.o bstree.o utils.o ordered_map.o

include parser/Makefile

.PHONY: all install test clean
all: $(SUBDIRS)
	$(INFO) 'build-all'
	$(MAKE) -C $<
	$(MAKE) $(C3)
$(C3): $(OBJS)
	$(CC) -o $(C3) $(OBJS) $(LDFLAGS)
install:
	$(CP) $(C3) $(INSTALL)
test:
	./run_tests.sh
clean:
	rm -rf *.o */*.o $(C3)
	rm parser/lexsmt2.c parser/parsesmt2.c parser/parsesmt2.h parser/*.output
.c.o :
	$(CC) $(CFLAGS-$(DEBUG)) -c -o $@ $<
