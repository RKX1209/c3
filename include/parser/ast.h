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

typedef struct ast_bvconst_t {
  unsigned int *bits;
  size_t size;
}ASTBVConst;

typedef struct ast_node_t {
  ASTKind kind;
  ASTVec children;
  Type type;
  int value_width;
  int index_width;
  char *name;
}ASTNode;

ASTNode* ast_create_node(ASTKind kind, ASTVec children);
ASTNode* ast_create_node0(ASTKind kind);
ASTNode* ast_create_node1(ASTKind kind, ASTNode n);
ASTNode* ast_create_node2(ASTKind kind, ASTNode* n1, ASTNode* n2);
ASTNode* ast_create_node3(ASTKind kind, ASTNode* n1, ASTNode* n2, ASTNode* n3);
ASTNode* ast_dup_node(ASTNode* n);
void ast_del_node(ASTNode *n);
ASTVec   ast_vec_new();
ASTNode* ast_vec_add(ASTVec vec, ASTNode* node);
size_t   ast_vec_size(ASTVec vec);
Type     ast_get_type(ASTNode *node);
ASTBVConst* ast_bvc_create (unsigned int bits);
unsigned int ast_bvc_size (unsigned int bits);
#endif
