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

/* (a == b) <=> (a[0] <-> b[0] && a[1] <-> b[1] && ... a[n] <-> b[n]) */
ASTNode *c3_bitblast_equal(ASTVec left, ASTVec right) {
  ASTNode *result, *node, *node2;
  C3ListIter *iter, *iter2;
  ASTVec ands = ast_vec_new ();
  if (c3_list_length (left) > 1) {
    iter2 = right->head;
    c3_list_foreach (left, iter, node) {
      node2 = (ASTNode *)iter2->data;
      ASTNode *biteq = ast_create_node2 (IFF, node, node2);
      ast_vec_add (ands, biteq);
      iter2 = iter2->n;
    }
    result = ast_create_node (AND, ands);
  } else {
    result = ast_create_node2 (IFF, (ASTNode *)left->head->data, (ASTNode *)right->head->data);
  }
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

static ASTNode *c3_bitblast_bvle (ASTVec left, ASTVec right, bool sign, bool bvlt) {
  C3ListIter *lit, *rit;
  ASTNode *prevbit = bb->ASTTrue;
  lit = left->head;
  rit = right->head;
  while (lit->n != left->tail) {
    ASTNode *lnode = (ASTNode *)lit->data;
    ASTNode *rnode = (ASTNode *)rit->data;
    ASTNode *thisbit = ast_create_node3 (ITE, ast_create_node2 (IFF, rnode, lnode), prevbit, rnode);
    prevbit = thisbit;
    lit = lit->n;
    rit = rit->n;
  }
  ASTNode *lmsb = (ASTNode *)lit->data;
  ASTNode *rmsb = (ASTNode *)lit->data;
  if (sign) {
    lmsb = ast_create_node1 (NOT, lmsb);
    rmsb = ast_create_node1 (NOT, rmsb);
  }
  ASTNode *msb = ast_create_node3 (ITE, ast_create_node2 (IFF, rmsb, lmsb), prevbit, rmsb);
  if (bvlt) {
    msb = ast_create_node1 (NOT, msb);
  }
  return msb;
}

ASTNode *c3_bitblast_cmp(ASTNode *form) {
  ASTNode *result;
  ASTVec children = form->children;
  ASTVec left = c3_bitblast_term ((ASTNode *)children->head->data);
  ASTVec right = c3_bitblast_term ((ASTNode *)children->head->n->data);
  const ASTKind k = ast_get_kind (form);
  switch (k) {
    case BVLE:
     {
       result = c3_bitblast_bvle (left, right, false, false);
       break;
     }
     case BVGE:
     {
       result = c3_bitblast_bvle (right, left, false, false);
       break;
     }
     case BVGT:
     {
       result = c3_bitblast_bvle (right, left, false, true);
       break;
     }
     case BVLT:
     {
       result = c3_bitblast_bvle (left, right, false, true);
       break;
     }
     case BVSLE:
     {
       result = c3_bitblast_bvle (left, right, true, false);
       break;
     }
     case BVSGE:
     {
       result = c3_bitblast_bvle (right, left, true, false);
       break;
     }
     case BVSGT:
     {
       result = ast_create_node1 (NOT, c3_bitblast_bvle (left, right, true, false));
       break;
     }
     case BVSLT:
     {
       //result = ast_create_node1 (NOT, c3_bitblast_bvle (right, left, true, false));
       result = form;
       break;
     }
     default:
      break;
  }
  return result;
}

ASTVec c3_bitblast_term(ASTNode *term) {
  const ASTKind k = ast_get_kind (term);
  ASTVec children = term->children, result;
  unsigned int numbits = term->value_width;

  switch (k) {
    case BVNOT:
    {
      const ASTVec kids = c3_bitblast_term ((ASTNode *)children->head->data);
      result = c3_bitblast_neg (kids);
      break;
    }
    case BVSX:
    case BVZX:
    {
      printf ("BVSX¥n");
      /* TODO: */
      result = term;
      break;
    }
    case BVCONST:
    {
      printf ("BVCONST¥n");
      ASTVec cnts = ast_vec_new ();
      ASTBVConst *bvconst = term->bvconst;
      unsigned int i;
      for (i = 0; i < numbits; i++) {
        ast_vec_add (cnts, ast_bvc_bit_test (bvconst, i) ? bb->ASTTrue : bb->ASTFalse);
      }
      result = cnts;
      break;
    }
    default:
      break;
  }
  return result;
}

ASTNode *c3_bitblast_form(ASTNode *form) {
  ASTNode *result, *child;
  ASTVec children = form->children;
  ASTVec echildren = ast_vec_new ();
  C3ListIter *iter;
  const ASTKind k = ast_get_kind (form);

  switch (k) {
    case TRUE:
      result = bb->ASTTrue;
      break;
    case FALSE:
      result = bb->ASTFalse;
      break;
    case NOT:
      result = ast_create_node1 (NOT, c3_bitblast_form ((ASTNode *)children->head->data));
      break;
    case ITE:
      result = ast_create_node3 (ITE,
                                c3_bitblast_form ((ASTNode *)children->head->data),
                                c3_bitblast_form ((ASTNode *)children->head->n->data),
                                c3_bitblast_form ((ASTNode *)children->head->n->n->data));
      break;
    case AND:
    case OR:
    case NAND:
    case NOR:
    case IFF:
    case XOR:
    case IMPLIES:
    {
      printf ("TWO¥n");
      c3_list_foreach (children, iter, child) {
        c3_list_append (echildren, c3_bitblast_form (child));
      }
      result = ast_create_node (k, echildren);
      break;
    }
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
      printf ("BVSLT¥n");
      result = c3_bitblast_cmp (form);
      break;
    }
    default:
      break;
  }
  return result;
}

void c3_bitblast(C3 *c3, ASTNode *assertq) {
  bb = c3_bitblast_new ();
  assertq = c3_bitblast_form (assertq);
  c3_bitblast_free (bb);
}
