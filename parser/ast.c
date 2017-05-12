/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <stdlib.h>
#include <parser/ast.h>

static const unsigned BYTE = 8;
static const unsigned BYTE_MASK = (1 << BYTE) - 1;

ASTVec ast_vec_new() {
  return c3_list_new ();
}

ASTNode* ast_vec_add(ASTVec vec, ASTNode* node) {
  C3ListIter *iter = c3_list_append (vec, node);
  if (!iter) {
    return NULL;
  }
  return (ASTNode*)iter->data;
}

ASTNode* ast_create_node(ASTKind kind, ASTVec children) {
  ASTNode *node;
  if ((node = (ASTNode *) malloc (sizeof (ASTNode))) == NULL) {
    return NULL;
  }
  node->kind = kind;
  node->children = children;
  return node;
}

ASTNode* ast_create_node0(ASTKind kind) {
  return ast_create_node (kind, NULL);
}

ASTNode* ast_create_node1(ASTKind kind, ASTNode* n) {
  ASTVec vec = ast_vec_new();
  if (!vec) {
    return NULL;
  }
  ast_vec_add (vec, n);
  return ast_create_node (kind, vec);
}

ASTNode* ast_create_node2(ASTKind kind, ASTNode* n1, ASTNode* n2) {
  ASTVec vec = ast_vec_new();
  if (!vec) {
    return NULL;
  }
  ast_vec_add (vec, n1);
  ast_vec_add (vec, n2);
  return ast_create_node (kind, vec);
}

ASTNode* ast_create_node3(ASTKind kind, ASTNode* n1, ASTNode* n2, ASTNode* n3) {
  ASTVec vec = ast_vec_new();
  if (!vec) {
    return NULL;
  }
  ast_vec_add (vec, n1);
  ast_vec_add (vec, n2);
  ast_vec_add (vec, n3);
  return ast_create_node (kind, vec);
}

void ast_del_node(ASTNode *n) {
  free (n);
}

ASTNode* ast_dup_node(ASTNode* n) {
  return ast_create_node (n->kind, n->children);
}

size_t ast_vec_size(ASTVec vec) {
  return vec->length;
}

Type ast_get_type(ASTNode *node) {
  return node->type;
}

ASTBVConst* ast_bvc_create (unsigned int bits) {
  ASTBVConst *bvconst = (ASTBVConst *)malloc (sizeof (ASTBVConst));
  size_t size;
  unsigned int *bits;

  if (!bvconst) {
    return NULL;
  }
  size = ast_bvc_size (bits);
  bits = (unsigned int *) malloc (size);
  bvconst->bits = bits;
  bvconst->size = size;
}

unsigned int ast_bvc_size (unsigned int bits) {
  unsigned int size = 0;
  while (bits > 0) {
    if (bits) size++;
    bits >>= 8;
  }
  return size;
}
