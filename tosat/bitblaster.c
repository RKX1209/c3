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

ASTNode *c3_bitblast_form(C3 *c3, ASTNode *form) {
  ASTNode *result;
  const ASTKind k = form->kind;
  switch (k) {
    case TRUE:
      result = bb->ASTTrue;
      break;
    case FALSE:
      result = bb->ASTFalse;
      break;
    case NOT:

      break;
    default:
      break;
  }
  return result;
}

void c3_bitblast(C3 *c3, ASTNode *assertq) {
  bb = c3_bitblast_new ();
  c3_bitblast_form (c3, assertq);
  c3_bitblast_free (bb);
}
