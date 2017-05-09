/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <stdlib.h>
#include <parser/ast.h>

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

ASTNode* ast_create_node2(ASTKind kind, ASTNode* n1, ASTNode* n2) {
  ASTVec vec = ast_vec_new();
  if (!vec) {
    return NULL;
  }
  ast_vec_add (vec, n1);
  ast_vec_add (vec, n2);
  return ast_create_node (kind, vec);
}

ASTNode* ast_node_dup(ASTNode* n) {
  return ast_create_node (n->kind, n->children);
}
