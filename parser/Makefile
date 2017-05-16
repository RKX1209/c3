# C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209
BISON 		= 	bison
FLEX			= 	flex
FORMATS		=		smt2
OBJS			+=	parser/parsecnf.o parser/parsesmt2.o parser/lexsmt2.o parser/ast.o parser/astkind.o

define generate_code
	$(BISON) --debug -v -o parse$(1).c -d $(1).y
	$(FLEX) -Cfe -o lex$(1).c $(1).lex
endef

.PHONY: generate
generate :
	$(foreach format, $(FORMATS), $(call generate_code,$(format)))
