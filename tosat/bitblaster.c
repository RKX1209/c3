/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <parser/ast.h>
#include <tosat/bitblaster.h>
#include <stdlib.h>

C3BitBlast *bb;
C3BitBlast *c3_bitblast_new() {
  C3BitBlast *bitblast;
  bitblast = (C3BitBlast *) malloc (sizeof (C3BitBlast));
  if (!bitblast) {
    return NULL;
  }
  bitblast->ASTTrue = ast_create_node0 (TRUE);
  bitblast->ASTFalse = ast_create_node0 (FALSE);
  return bitblast;
}

void c3_bitblast_free(C3BitBlast *bitblast) {
  ast_free_node (bitblast->ASTTrue);
  ast_free_node (bitblast->ASTFalse);
  free (bitblast);
}

ASTNode *c3_bitblast_equal(ASTVec left, ASTVec right) {
  ASTNode *result;
  return result;
}

ASTVec c3_bitblast_neg(ASTVec vec) {
  ASTVec result;
  ASTNode *node;
  C3ListIter *iter;
  result = ast_vec_new ();
  c3_list_foreach (vec, iter, node) {
    ast_vec_add (result, ast_create_node1 (NOT, node));
  }
  return result;
}

ASTNode *c3_bitblast_cmp(ASTVec left, ASTVec right) {
  ASTNode *result;
  return result;
}

ASTVec c3_bitblast_term(ASTNode *term) {
  const ASTKind k = term->kind;
  ASTVec children = term->children, result;
  switch (k) {
    case BVNOT:
    {
      const ASTVec kids = c3_bitblast_term ((ASTNode *)children->head->data);
      result = c3_bitblast_neg (kids);
      break;
    }
    default:
      break;
  }
  return result;
}

ASTNode *c3_bitblast_form(ASTNode *form) {
  ASTNode *result;
  ASTVec children = form->children;
  const ASTKind k = form->kind;
  switch (k) {
    case TRUE:
      result = bb->ASTTrue;
      break;
    case FALSE:
      result = bb->ASTFalse;
      break;
    case NOT:
      result = ast_create_node1 (NOT, (ASTNode *)children->head->data);
      break;
    case ITE:
      result = ast_create_node3 (ITE, (ASTNode *)children->head->data, (ASTNode *)children->head->n->data, (ASTNode *)children->head->n->n->data);
      break;
    case AND:
    case OR:
    case NAND:
    case NOR:
    case IFF:
    case XOR:
    case IMPLIES:
      result = ast_create_node (k, children);
      break;
    case EQ:
    {
      ASTVec left = c3_bitblast_term ((ASTNode *)children->head->data);
      ASTVec right = c3_bitblast_term ((ASTNode *)children->head->n->data);
      result = c3_bitblast_equal (left, right);
      break;
    }
    case BVLE:
    case BVGE:
    case BVGT:
    case BVLT:
    case BVSLE:
    case BVSGE:
    case BVSGT:
    case BVSLT:
    {
      ASTVec left = c3_bitblast_term ((ASTNode *)children->head->data);
      ASTVec right = c3_bitblast_term ((ASTNode *)children->head->n->data);
      result = c3_bitblast_cmp (left, right);
      break;
    }
    default:
      break;
  }
  return result;
}

void c3_bitblast(C3 *c3, ASTNode *assertq) {
  bb = c3_bitblast_new ();
  c3_bitblast_form (assertq);
  c3_bitblast_free (bb);
}
