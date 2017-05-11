#ifndef AST_H
#define AST_H
#include <stdbool.h>
#include <c3_list.h>
#include "astkind.h"

typedef C3List* ASTVec;

typedef enum {
  BOOLEAN_TYPE,
  ARRAY_TYPE,
  BITVECTOR_TYPE,
} Type;

typedef struct ast_node_t {
  ASTKind kind;
  ASTVec children;
  Type type;
}ASTNode;

ASTNode* ast_create_node(ASTKind kind, ASTVec children);
ASTNode* ast_create_node2(ASTKind kind, ASTNode* n1, ASTNode* n2);
ASTNode* ast_dup_node(ASTNode* n);
void ast_del_node(ASTNode *n);
ASTVec   ast_vec_new();
ASTNode* ast_vec_add(ASTVec vec, ASTNode* node);
Type     ast_get_type(ASTNode *node) { return node->type; }
#endif
