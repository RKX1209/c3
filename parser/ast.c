/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <parser/ast.h>
#include <c3_core.h>
#include <c3_utils.h>

#define BYTE 8
#define BYTE_MASK ((1 << BYTE) - 1)

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
  node->index_width = -1;
  node->value_width = 0;
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

ASTNode* ast_create_node1w(ASTKind kind, const unsigned int width, ASTNode *n) {
  ASTNode *node = ast_create_node1 (kind, n);
  if (!node) {
    return NULL;
  }
  node->value_width = width;
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

ASTNode* ast_create_node2w(ASTKind kind, const unsigned int width, ASTNode* n1, ASTNode* n2) {
  ASTNode *node = ast_create_node2 (kind, n1, n2);
  if (!node) {
    return NULL;
  }
  node->value_width = width;
  return node;
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
  ASTNode *new = ast_create_node (n->kind, n->children);
  new->value_width = n->value_width;
  new->index_width = n->index_width;
  new->name = n->name;
  new->bvconst = n->bvconst;
}

size_t ast_vec_size(ASTVec vec) {
  return vec->length;
}

Type ast_get_type(ASTNode *node) {
  if (node->index_width == 0 && node->value_width == 0) {
    return BOOLEAN_TYPE;
  }
  if (node->index_width == 0 && node->value_width > 0) {
    return BITVECTOR_TYPE;
  }
  if (node->index_width > 0 && node->value_width > 0) {
    return ARRAY_TYPE;
  }
  return UNKNOWN_TYPE;
}

ASTNode* ast_create_bvc(unsigned int width, unsigned long long bvconst) {
  if (width <= 0 || width > (sizeof (unsigned long long) * 8)) {
    c3_fatal_error ("CreateBVConst: trying to create bvconst of width %d", width);
  }
  ASTNode *node;
  ASTBVConst *astconst;
  node = (ASTNode*)malloc (sizeof(ASTNode));
  if (!node) {
    return NULL;
  }
  astconst = ast_bvc_create (bvconst);
  if (!astconst) {
    free (node);
    return NULL;
  }
  ast_bvc_resize (astconst, width);
  node->bvconst = astconst;
  node->value_width = width;
  return node;
}

ASTBVConst* ast_bvc_create (unsigned int bits) {
  ASTBVConst *bvconst = (ASTBVConst *)malloc (sizeof (ASTBVConst));
  size_t size;
  unsigned char *bitsc;

  if (!bvconst) {
    return NULL;
  }
  size = ast_bvc_size (bits);
  bitsc = (unsigned char *) malloc (size);
  bvconst->bits = bitsc;
  bvconst->size = size;
  return bvconst;
}

size_t ast_bvc_size (unsigned int bits) {
  size_t size = 0;
  while (bits > 0) {
    if (bits) size++;
    bits >>= sizeof (char) * 8;
  }
  return size;
}

void ast_bvc_resize (ASTBVConst *bvconst, unsigned int width) {
  size_t oldsize, newsize, bytes;
  unsigned char *newaddr;
  oldsize = bvconst->size * 8; //width is newsize in bits
  newsize = width;
  if (oldsize < newsize) {
    bytes = newsize / 8;
    if (newsize % 8 != 0) bytes++;
    newaddr = (unsigned char*)realloc (bvconst->bits, bytes);
    if (!newaddr) {
      c3_fatal_error ("BVConst resize is failed");
      return;
    }
    bvconst->bits = newaddr;
  }
}
