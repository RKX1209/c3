%{
  #include <stdint.h>
  #include <stdio.h>
  #include <parser/ast.h>
  #include <parser/parser.h>
  #include <c3_core.h>
  #include <c3_utils.h>
  /* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
  extern char* yytext;
  extern int yylineno;

  int yyerror(const char *s) {
    printf ("syntax error: line %d token: %s\n", yylineno, yytext);
    return 1;
  }
%}

%union {
  unsigned uintval;
  char *str;
  ASTNode *node;
  ASTVec *vec;
};

%start cmd

%type <node> status
%type <vec> an_formulas an_terms function_params an_mixed
%type <node> an_term  an_formula function_param

%token <uintval> NUMERAL_TOK
%token <str> BVCONST_DECIMAL_TOK
%token <str> BVCONST_BINARY_TOK
%token <str> BVCONST_HEXIDECIMAL_TOK

 /* We have this so we can parse :smt-lib-version 2.0 */
%token  DECIMAL_TOK

%token <node> FORMID_TOK TERMID_TOK
%token <str> STRING_TOK BITVECTOR_FUNCTIONID_TOK BOOLEAN_FUNCTIONID_TOK

/* set-info tokens */
%token SOURCE_TOK
%token CATEGORY_TOK
%token DIFFICULTY_TOK
%token VERSION_TOK
%token STATUS_TOK
/* ASCII Symbols */
/* Semicolons (comments) are ignored by the lexer */
%token UNDERSCORE_TOK
%token LPAREN_TOK
%token RPAREN_TOK
/* Used for attributed expressions */
%token EXCLAIMATION_MARK_TOK
%token NAMED_ATTRIBUTE_TOK

/*BV SPECIFIC TOKENS*/
%token BVLEFTSHIFT_1_TOK
%token BVRIGHTSHIFT_1_TOK
%token BVARITHRIGHTSHIFT_TOK
%token BVPLUS_TOK
%token BVSUB_TOK
%token BVNOT_TOK //bvneg in CVCL
%token BVMULT_TOK
%token BVDIV_TOK
%token SBVDIV_TOK
%token BVMOD_TOK
%token SBVREM_TOK
%token SBVMOD_TOK
%token BVNEG_TOK //bvuminus in CVCL
%token BVAND_TOK
%token BVOR_TOK
%token BVXOR_TOK
%token BVNAND_TOK
%token BVNOR_TOK
%token BVXNOR_TOK
%token BVCONCAT_TOK
%token BVLT_TOK
%token BVGT_TOK
%token BVLE_TOK
%token BVGE_TOK
%token BVSLT_TOK
%token BVSGT_TOK
%token BVSLE_TOK
%token BVSGE_TOK
%token BVSX_TOK
%token BVEXTRACT_TOK
%token BVZX_TOK
%token BVROTATE_RIGHT_TOK
%token BVROTATE_LEFT_TOK
%token BVREPEAT_TOK
%token BVCOMP_TOK
/* Types for QF_BV and QF_AUFBV. */
%token BITVEC_TOK
%token ARRAY_TOK
%token BOOL_TOK
/* CORE THEORY pg. 29 of the SMT-LIB2 standard 30-March-2010. */
%token TRUE_TOK;
%token FALSE_TOK;
%token NOT_TOK;
%token AND_TOK;
%token OR_TOK;
%token XOR_TOK;
%token ITE_TOK;
%token EQ_TOK;
%token IMPLIES_TOK;
/* CORE THEORY. But not on pg 29. */
%token DISTINCT_TOK;
%token LET_TOK;
%token COLON_TOK
// COMMANDS
%token ASSERT_TOK
%token CHECK_SAT_TOK
%token CHECK_SAT_ASSUMING_TOK
%token DECLARE_CONST_TOK
%token DECLARE_FUNCTION_TOK
%token DECLARE_SORT_TOK
%token DEFINE_FUNCTION_TOK
%token DECLARE_FUN_REC_TOK
%token DECLARE_FUNS_REC_TOK
%token DEFINE_SORT_TOK
%token ECHO_TOK
%token EXIT_TOK
%token GET_ASSERTIONS_TOK
%token GET_ASSIGNMENT_TOK
%token GET_INFO_TOK
%token GET_MODEL_TOK
%token GET_OPTION_TOK
%token GET_PROOF_TOK
%token GET_UNSAT_ASSUMPTION_TOK
%token GET_UNSAT_CORE_TOK
%token GET_VALUE_TOK
%token POP_TOK
%token PUSH_TOK
%token RESET_TOK
%token RESET_ASSERTIONS_TOK
%token NOTES_TOK
%token LOGIC_TOK
%token SET_OPTION_TOK

/* Functions for QF_ABV. */
%token SELECT_TOK;
%token STORE_TOK;
%token END 0 "end of file"

%%
cmd: commands END
{
  YYACCEPT;
}
;

commands: commands LPAREN_TOK cmdi RPAREN_TOK
| LPAREN_TOK cmdi RPAREN_TOK
{}
;

cmdi:
    ASSERT_TOK an_formula:
    {
    }
|
    CHECK_SAT_TOK
    {

    }
|
    CHECK_SAT_ASSUMING_TOK LPAREN_TOK an_term RPAREN_TOK
    {
      c3_unsupported (&c3);
    }
|
    DECLARE_CONST_TOK LPAREN_TOK an_term RPAREN_TOK
    {
      c3_unsupported (&c3);
    }
|
    DECLARE_FUNCTION_TOK var_decl
    {

    }
|
    DEFINE_FUNCTION_TOK function_def
    {

    }
|
    ECHO_TOK STRING_TOK
    {
      printf ("¥"%s¥"¥n", $2);
      free ($2);
    }
|
    EXIT_TOK
    {
       YYACCEPT;
    }
|
    GET_MODEL_TOK
    {
    }
|
    GET_VALUE_TOK LPAREN_TOK an_mixed RPAREN_TOK
    {

    }
|
    SET_OPTION_TOK COLON_TOK STRING_TOK STRING_TOK
    {

    }
|
    SET_OPTION_TOK COLON_TOK STRING_TOK FALSE_TOK
    {

    }
|
    SET_OPTION_TOK COLON_TOK STRING_TOK TRUE_TOK
    {

    }
|
    PUSH_TOK NUMERAL_TOK
    {

    }
|
    POP_TOK NUMERAL_TOK
    {

    }
|
    RESET_TOK
    {

    }
|
    LOGIC_TOK STRING_TOK
    {

    }
|
    NOTES_TOK attribute STRING_TOK
    {

    }
|
    NOTES_TOK attribute DECIMAL_TOK
    {}
|
    NOTES_TOK attribute
    {}
;

function_param:
/* (arg (_ BitVec 8)) */
LPAREN_TOK STRING_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK RPAREN_TOK
{
  ASTNode *ast_sym = c3_create_variable (0, $6, $2);
  $$ = ast_sym;
  c3_add_symbol ($2, ast_sym);
};

/* Returns a vector of parameters.*/
function_params:
function_param
{
  $$ = ast_vec_new ();
  if ($1 != NULL) {
    ast_vec_add ($$, $1);
  }
}
| function_params function_param
{
  $$ = $1;
  ast_vec_add ($$, $2);
};

function_def:
/* (func (<params>) (_ BitVec 8) <term>) */
STRING_TOK LPAREN_TOK function_params RPAREN_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK an_term
{
  if ($10->value_width != $8) {
    char msg[128];
    sprintf(msg, "Different bit-widths specified: %d %d", $10->GetValueWidth(), $8);
    yyerror(msg);
  }
  c3_store_function (&c3, $1, $3, $10);
  free ($1);
  free ($3);
  free ($10);
}
| /* (func () (_ BitVec 8) <term>) */
STRING_TOK LPAREN_TOK RPAREN_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK an_term
{
  if ($9->value_width != $7) {
    char msg [100];
    sprintf(msg, "Different bit-widths specified: %d %d", $9->value_width, $7);
    yyerror(msg);
  }
  ASTVec empty;
  c3_store_function (&c3, $1, empty, $9);
  free ($1);
  free ($9);
}
| /* (func (<params>) Bool <formula>) */
STRING_TOK LPAREN_TOK function_params RPAREN_TOK BOOL_TOK an_formula
{
  c3_store_function (&c3, $1, $3, $6);
  free ($1);
  free ($3);
  free ($6);
}
| /* (func () Bool <formula>) */
STRING_TOK LPAREN_TOK RPAREN_TOK BOOL_TOK an_formula
{
  ASTVec empty;
  c3_store_function (&c3, $1, empty, $5);
  free ($1);
  free ($5);
}
| /* (func (<params>) (Array (_ BitVec 8) (_ BitVec 8)) <term>) */
STRING_TOK LPAREN_TOK function_params RPAREN_TOK LPAREN_TOK ARRAY_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK RPAREN_TOK an_term
{
  c3_unsupported (&c3);
  free ($1);
  free ($18);
}
| /* (func () (Array (_ BitVec 8) (_ BitVec 8)) <term>) */
STRING_TOK LPAREN_TOK RPAREN_TOK LPAREN_TOK ARRAY_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK RPAREN_TOK an_term
{
  c3_unsupported (&c3);
  free ($1);
  free ($17);
}
;

status:
STRING_TOK {

}
;

attribute:
SOURCE_TOK
{}
| CATEGORY_TOK
{}
| DIFFICULTY_TOK
{}
| VERSION_TOK
{}
| STATUS_TOK status
{}
;

var_decl:
/* (func () (_ BitVec 8)) */
STRING_TOK LPAREN_TOK RPAREN_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK
{
  ASTNode *ast_sym = c3_create_variable (0, $7, $1);
  c3_add_symbol ($1, ast_sym);
} /* (func () Bool) */
| STRING_TOK LPAREN_TOK RPAREN_TOK BOOL_TOK
{  ASTNode *ast_sym = c3_create_variable (0, 0, $1);
  c3_add_symbol ($1, ast_sym);
} /* (func () (Array (_ BitVec 8) (_ BitVec 8))) */
| STRING_TOK LPAREN_TOK RPAREN_TOK LPAREN_TOK ARRAY_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK LPAREN_TOK UNDERSCORE_TOK BITVEC_TOK NUMERAL_TOK RPAREN_TOK RPAREN_TOK
{
  ASTNode *ast_sym = c3_create_variable ($9, $14, $1);
  int index_len = $9;
  int value_len = $14;
  if (index_len > 0) {
    ast_sym->index_width = index_len;
  } else {
    debug_log (-1, "Fatal Error: parsing: BITVECTORS must be of positive length: \n");
  }

  if (value_len > 0) {
    ast_sym->value_width = value_len;
  } else {
    debug_log (-1, "Fatal Error: parsing: BITVECTORS must be of positive length: \n");
  }
  c3_add_symbol ($1, ast_sym);
}
;

an_mixed:
an_formula
{
  $$ = ast_vec_new ();
  if ($1 != NULL) {
    ast_vec_add ($$, $1);
  }
}
|
an_term
{
  $$ = ast_vec_new ();
  if ($1 != NULL) {
    ast_vec_add ($$, $1);
  }
}
|
an_mixed an_formula
{
  if ($1 != NULL && $2 != NULL) {
    ast_vec_add ($1, $2);
    $$ = $1;
  }
}
|
an_mixed an_term
{
  if ($1 != NULL && $2 != NULL) {
    ast_vec_add ($1, $2);
    $$ = $1;
  }
}
;

an_formulas:
an_formula
{
  $$ = ast_vec_new ();
  if ($1 != NULL) {
    ast_vec_add ($$, $1);
  }
}
|
an_formulas an_formula
{
  if ($1 != NULL && $2 != NULL) {
    ast_vec_add ($1, $2);
    $$ = $1;
  }
}
;

an_formula:
TRUE_TOK
{
  $$ = ast_create_node0 (TRUE);
}
| FALSE_TOK
{
  $$ = ast_create_node0 (FALSE);
}
| FORMID_TOK
{
  $$ = ast_dup_node ($1);
  ast_del_node ($1);
}
| LPAREN_TOK EQ_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (EQ, $3, $4);
  $$ = n;
}
| LPAREN_TOK DISTINCT_TOK an_terms RPAREN_TOK
{
  $$ = NULL; //TODO
}
| LPAREN_TOK DISTINCT_TOK an_formulas RPAREN_TOK
{
  $$ = NULL; //TODO
}
| LPAREN_TOK BVSLT_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (BVSLT, $3, $4);
  $$ = n;
}
| LPAREN_TOK BVSLE_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (BVSLE, $3, $4);
  $$ = n;
}
| LPAREN_TOK BVSGT_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (BVSGT, $3, $4);
  $$ = n;
}
| LPAREN_TOK BVSGE_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (BVSGE, $3, $4);
  $$ = n;
}
| LPAREN_TOK BVLT_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (BVLT, $3, $4);
  $$ = n;

}
| LPAREN_TOK BVLE_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (BVLE, $3, $4);
  $$ = n;
}
| LPAREN_TOK BVGT_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (BVGT, $3, $4);
  $$ = n;
}
| LPAREN_TOK BVGE_TOK an_term an_term RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (BVGE, $3, $4);
  $$ = n;
}
| LPAREN_TOK an_formula RPAREN_TOK
{
  $$ = $2;
}
| LPAREN_TOK NOT_TOK an_formula RPAREN_TOK
{
  ASTNode *n = ast_create_node1 (NOT, $3);
  $$ = n;
}
| LPAREN_TOK IMPLIES_TOK an_formula an_formula RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (IMPLIES, $3, $4);
  $$ = n;
}
| LPAREN_TOK ITE_TOK an_formula an_formula an_formula RPAREN_TOK
{
  ASTNode *n = ast_create_node3 (ITE, $3, $4, $5);
  $$ = n;
}
| LPAREN_TOK AND_TOK an_formulas RPAREN_TOK
{
  ASTNode *n = ast_create_node (AND, $3);
  $$ = n;
}
| LPAREN_TOK OR_TOK an_formulas RPAREN_TOK
{
  ASTNode *n = ast_create_node (OR, $3);
  $$ = n;
}
| LPAREN_TOK XOR_TOK an_formula an_formula RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (XOR, $3, $4);
  $$ = n;
}
| LPAREN_TOK EQ_TOK an_formula an_formula RPAREN_TOK
{
  ASTNode *n = ast_create_node2 (IFF, $3, $4);
  $$ = n;
}
| LPAREN_TOK BOOLEAN_FUNCTIONID_TOK an_mixed RPAREN_TOK
{
  $$ = NULL; //TODO
}
| BOOLEAN_FUNCTIONID_TOK an_mixed RPAREN_TOK
{
  $$ = NULL; //TODO
}
| BOOLEAN_FUNCTIONID_TOK
{
  $$ = NULL; //TODO
}
;

an_terms:
an_term
{
  $$ = ast_vec_new ();
  if ($1 != NULL) {
    ast_vec_add ($$, $1);
  }
}
|
an_terms an_term
{
  if ($1 != NULL && $2 != NULL) {
    ast_vec_add ($$, $2);
    $$ = $1;
  }
}
;

an_term:
TERMID_TOK
{
  $$ = ast_dup_node ($1);
}
| LPAREN_TOK an_term RPAREN_TOK
{
  $$ = $2;
}
| SELECT_TOK an_term an_term
{
  //ARRAY READ TODO
  $$ = NULL;
}
| STORE_TOK an_term an_term an_term
{
  //ARRAY WIRTE TODO
  $$ = NULL;
}
| LPAREN_TOK UNDERSCORE_TOK BVEXTRACT_TOK  NUMERAL_TOK  NUMERAL_TOK RPAREN_TOK an_term
{
  //extract TODO
  $$ = NULL;
}
| LPAREN_TOK UNDERSCORE_TOK BVZX_TOK NUMERAL_TOK RPAREN_TOK an_term
{
  //zero expand TODO
  $$ = NULL;
}
| LPAREN_TOK UNDERSCORE_TOK BVSX_TOK NUMERAL_TOK RPAREN_TOK an_term
{
  //signed expand TODO
  $$ = NULL;
}
| ITE_TOK an_formula an_term an_term
{
  //ITE TODO
  $$ = NULL;
}
| BVCONCAT_TOK an_term an_term
{
  const unsigned int width = $2->value_width + $3->value_width;
  ASTNode *n = ast_create_node2w (BVCONCAT, width, $2, $3);
  $$ = n;
}
| BVNOT_TOK an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVNOT, width, $2);
  $$ = n;
}
| BVNEG_TOK an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVUMINUS, width, $2);
  $$ = n;
}
| BVAND_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVAND, width, $2, $3);
  $$ = n;
}
| BVOR_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVOR, width, $2, $3);
  $$ = n;
}
| BVXOR_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVXOR, width, $2, $3);
  $$ = n;
}
| BVXNOR_TOK an_term an_term
{
  // (bvxnor s t) abbreviates (bvor (bvand s t) (bvand (bvnot s) (bvnot t)))
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVOR, width,
                ast_create_node2w (BVAND, width, $2, $3),
                ast_create_node2w (BVAND, width,
                  ast_create_node1w (BVNOT, width, $2),
                  ast_create_node1w (BVNOT, width, $3)
                )
               );
  $$ = n;
}
| BVCOMP_TOK an_term an_term
{
  //compare TODO
  $$ = NULL;
}
| BVSUB_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVSUB, width, $2, $3);
  $$ = n;
}
| BVPLUS_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVPLUS, width, $2, $3);
  $$ = n;
}
| BVMULT_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVMULT, width, $2, $3);
  $$ = n;
}
| BVDIV_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVDIV, width, $2, $3);
  $$ = n;
}
| BVMOD_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVMOD, width, $2, $3);
  $$ = n;
}
| SBVDIV_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (SBVDIV, width, $2, $3);
  $$ = n;
}
| SBVREM_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (SBVREM, width, $2, $3);
  $$ = n;
}
| SBVMOD_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (SBVMOD, width, $2, $3);
  $$ = n;
}
| BVNAND_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node1w (BVNOT, width, ast_create_node2w (BVAND, width, $2, $3));
  $$ = n;
}
| BVNOR_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node1w (BVNOT, width, ast_create_node2w (BVOR, width, $2, $3));
  $$ = n;
}
| BVLEFTSHIFT_1_TOK an_term an_term
{
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVLEFTSHIFT, width, $2, $3);
  $$ = n;
}
| BVRIGHTSHIFT_1_TOK an_term an_term
{
  // logic right shift
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVRIGHTSHIFT, width, $2, $3);
  $$ = n;
}
| BVARITHRIGHTSHIFT_TOK an_term an_term
{
  // arithmetic right shift
  const unsigned int width = $2->value_width;
  ASTNode *n = ast_create_node2w (BVSRSHIFT, width, $2, $3);
  $$ = n;
}
| LPAREN_TOK UNDERSCORE_TOK BVROTATE_LEFT_TOK  NUMERAL_TOK  RPAREN_TOK an_term
{
  //rotate left TODO
  $$ = NULL;
}
| LPAREN_TOK UNDERSCORE_TOK BVROTATE_RIGHT_TOK  NUMERAL_TOK  RPAREN_TOK an_term
{
  //rotate right TODO
  $$ = NULL;
}
| LPAREN_TOK UNDERSCORE_TOK BVREPEAT_TOK  NUMERAL_TOK RPAREN_TOK an_term
{
  //repeat TODO
  $$ = NULL;
}
| UNDERSCORE_TOK BVCONST_DECIMAL_TOK NUMERAL_TOK
{
  unsigned long decimal = strtoul ($2, NULL, 10);
  ASTNode *n = ast_create_bvc(*$3, decimal);
  $$ = n;
}
| BVCONST_HEXIDECIMAL_TOK
{
  unsigned long hex = strtoul ($1, NULL, 16);
  size_t width = strlen($1) * 4;
  ASTNode *n = ast_create_bvc(width, hex);
  $$ = n;
}
| BVCONST_BINARY_TOK
{
  unsigned long binary = strtoul ($1, NULL, 2);
  size_t width = strlen($1);
  ASTNode *n = ast_create_bvc(width, binary);
  $$ = n;
}
| LPAREN_TOK BITVECTOR_FUNCTIONID_TOK an_mixed RPAREN_TOK
{
  //apply function TODO
  $$ = NULL;
}
| BITVECTOR_FUNCTIONID_TOK
{
  //apply function TODO
  $$ = NULL;
}
| LPAREN_TOK EXCLAIMATION_MARK_TOK an_term NAMED_ATTRIBUTE_TOK STRING_TOK RPAREN_TOK
{
  //attribute TODO
  $$ = NULL;
}
| LPAREN_TOK LET_TOK LPAREN_TOK
{
  //let TODO
  $$ = NULL;
}
;
%%

int c3_smt2_parse() {
  return yyparse();
}
