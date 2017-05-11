#ifndef C3_CORE_H
#define C3_CORE_H

#include <stdint.h>
#include <c3_list.h>
#include <c3_hashmap.h>
#include <c3_bstree.h>
#include <c3_ordered_map.h>
#include <parser/ast.h>

#define C3_UNSAT    0   //unatisfiable
#define C3_SAT      1   //satisfiable
#define C3_INVALID  2   //error
#define C3_SIMPLIFY 3   //can still simplify
#define C3_NONE     4   //none

typedef uint8_t C3_STATUS;

typedef struct c3_t {
  int32_t valnum;
  int32_t disjnum;
  C3List *cnf; // <cnf[i] = i-th Disjunction>
  C3Hmap *literals;
  C3BsTree *literals2;
  C3Map *symbols;
}C3;

extern C3 c3;

ASTNode* c3_lookup_symbol (C3 *c3, char *symbol);

#endif
