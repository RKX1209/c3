#ifndef AST_H
#define AST_H

#include <c3_list.h>

typedef C3List* ASTVec;

typedef struct ast_node_t {
  ASTVec children;
}ASTNode;

#endif
