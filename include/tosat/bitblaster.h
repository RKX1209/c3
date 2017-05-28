#ifndef BITBLASTER_H
#define BITBLASTER_H

#include <c3_core.h>

typedef struct c3_bitblast_t {
  ASTNode *ASTTrue, *ASTFalse;
} C3BitBlast;

C3BitBlast *c3_bitblast_new();
void c3_bitblast_free(C3BitBlast *bitblast);
ASTNode *c3_bitblast_equal(ASTVec left, ASTVec right);
ASTNode *c3_bitblast_cmp(ASTNode *form);
ASTVec c3_bitblast_term(ASTNode *term);
ASTNode *c3_bitblast_form(ASTNode *form);
void c3_bitblast(C3 *c3, ASTNode *assertq);

#endif
