#ifndef AST_H
#define AST_H
#include <stdbool.h>
#include <c3_list.h>
#include "astkind.h"

typedef C3List* ASTVec;

typedef struct ast_node_t {
  ASTKind kind;
  ASTVec children;
}ASTNode;

ASTNode* ast_create_node(ASTKind kind, ASTVec children);
ASTNode* ast_create_node2(ASTKind kind, ASTNode* n1, ASTNode* n2);
ASTNode* ast_node_dup(ASTNode* n);
ASTVec   ast_vec_new();
ASTNode* ast_vec_add(ASTVec vec, ASTNode* node);

#endif
