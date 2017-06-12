#ifndef AST_H
#define AST_H
#include <stdbool.h>
#include <c3_list.h>
#include "astkind.h"

typedef C3List* ASTVec;

typedef enum {
  UNKNOWN_TYPE,
  BOOLEAN_TYPE,
  ARRAY_TYPE,
  BITVECTOR_TYPE,
} Type;

typedef struct ast_bvconst_t {
  unsigned char *bits;
  size_t size;  //'byte' size of const
}ASTBVConst;

typedef struct ast_node_t {
  ASTKind kind;
  ASTVec children;
  unsigned int value_width; //'bits' size of value
  unsigned int index_width;
  char *name;
  ASTBVConst *bvconst; // This AST is pointing
}ASTNode;

ASTNode* ast_create_node(ASTKind kind, ASTVec children);
ASTNode* ast_create_node0(ASTKind kind);
ASTNode* ast_create_node1(ASTKind kind, ASTNode *n);
ASTNode* ast_create_node1w(ASTKind kind, const unsigned int width, ASTNode *n);
ASTNode* ast_create_node2(ASTKind kind, ASTNode* n1, ASTNode* n2);
ASTNode* ast_create_node2w(ASTKind kind, const unsigned int width, ASTNode* n1, ASTNode* n2);
ASTNode* ast_create_node3(ASTKind kind, ASTNode* n1, ASTNode* n2, ASTNode* n3);
void ast_free_node (ASTNode *n);
void ast_free_node_rec (ASTNode *root);
ASTNode* ast_dup_node(ASTNode* n);
void ast_del_node(ASTNode *n);
ASTVec   ast_vec_new();
ASTNode* ast_vec_add(ASTVec vec, ASTNode* node);
size_t   ast_vec_size(ASTVec vec);
Type     ast_get_type(ASTNode *node);
ASTNode* ast_create_bvc(unsigned int width, unsigned long long bvconst);
ASTBVConst* ast_bvc_create(unsigned int bits);
size_t   ast_bvc_size(unsigned int bits);
void ast_bvc_resize(ASTBVConst *bvconst, unsigned int width);
bool ast_bvc_bit_test (ASTBVConst *bvconst, unsigned int index);
void ast_print(ASTNode *root);
#endif
