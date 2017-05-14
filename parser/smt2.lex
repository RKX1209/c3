%{
  /* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
  #include <stdbool.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <c3_core.h>
  #include "parsesmt2.h"

  extern char *yytext;
  char *string_lit = NULL;

  static int lookup(char *s) {
    char * cleaned = NULL;
    size_t len;

    // The SMTLIB2 specifications sez that the outter bars aren't part of the
    // name. This means that we can create an empty string symbol name.
    if (s[0] == '|' && s[(len = strlen(s))-1] == '|') {
        cleaned = (char*) malloc(len);
        strncpy(cleaned,s+1,len-2); // chop off first and last characters.
        cleaned[len-2] = '\0';
        s = cleaned;
    }
    bool found = false;
    ASTNode *sym_ast;
    if ((sym_ast = c3_lookup_symbol(&c3, s))) {
      found = true;
    }
    if (found) {
      if (cleaned) {
        free (cleaned);
      }
      yylval.node = ast_dup_node (sym_ast);
      return FORMID_TOK;
    } else {
      if (cleaned)
        free (cleaned);
      yylval.node = strdup (s);
      //printf ("LITERAL: %s\n", s);
      return STRING_TOK;
    }
  }
%}

%option noyywrap

/* start states */
%x  COMMENT
%x  STRING_LITERAL
%x  SYMBOL

LETTER    ([a-zA-Z])
DIGIT     ([0-9])
OPCHAR    ([~!@$%^&*\_\-+=<>\.?/])

ANYTHING  ({LETTER}|{DIGIT}|{OPCHAR})

%%
[ \n\t\r\f]   { /* skip */ }
{DIGIT}+      { yylval.uintval = strtoul(yytext, NULL, 10); return NUMERAL_TOK; }

bv{DIGIT}+    { yylval.str = strdup(yytext + 2); return BVCONST_DECIMAL_TOK; }
#b{DIGIT}+    { yylval.str = strdup(yytext + 2); return BVCONST_BINARY_TOK; }
#x({DIGIT}|[a-fA-F])+    { yylval.str = strdup(yytext + 2); return BVCONST_HEXIDECIMAL_TOK; }
{DIGIT}+"."{DIGIT}+   { return DECIMAL_TOK; }

";"		{ BEGIN COMMENT; }
<COMMENT>"\n"	{ BEGIN INITIAL; /* return to normal mode */}
<COMMENT>.	{ /* stay in comment mode */ }

<INITIAL>"\""		{ BEGIN STRING_LITERAL;
                  if (string_lit) {
                    free (string_lit);
                  }
                  string_lit = malloc (1);
                  string_lit[0] = '\0';
                                      }
<STRING_LITERAL>"\"\""	{ /* double quote is the only escape. */
                          size_t old = strlen (string_lit);
                          if (!realloc (string_lit, old + 2)) {
                            c3_fatal_error ("string expansion failed\n");
                          }
                          string_lit[old] = '"';
                          string_lit[old + 1] = '\0'; }
<STRING_LITERAL>"\""	{ BEGIN INITIAL;
			                  yylval.str = strdup (string_lit);
                        return STRING_TOK; }
<STRING_LITERAL>.	{
                    size_t old = strlen (string_lit);
                    if (!realloc (string_lit, old + 2)) {
                      c3_fatal_error ("string expansion failed\n");
                    }
                    string_lit[old] = *yytext;
                    string_lit[old + 1] = '\0'; }
<STRING_LITERAL>"\n" {
                    size_t old = strlen (string_lit);
                    if (!realloc (string_lit, old + 2)) {
                      c3_fatal_error ("string expansion failed\n");
                    }
                    string_lit[old] = *yytext;
                    string_lit[old + 1] = '\0'; }

  /* Valid character are: ~ ! @ # $ % ^ & * _ - + = | \ : ; " < > . ? / ( ) */
"("             { return LPAREN_TOK; }
")"             { return RPAREN_TOK; }
"_"             { return UNDERSCORE_TOK; }
"!"             { return EXCLAIMATION_MARK_TOK; }
":"             { return COLON_TOK; }

 /* Set info types */
 /* This is a very restricted set of the possible keywords */
":source"        { return SOURCE_TOK;}
":category"      { return CATEGORY_TOK;}
":difficulty"    { return DIFFICULTY_TOK; }
":smt-lib-version"  { return VERSION_TOK; }
":status"        { return STATUS_TOK; }

  /* Attributes */
":named"        { return NAMED_ATTRIBUTE_TOK; }


 /* COMMANDS */
"assert"                  { return ASSERT_TOK; }
"check-sat"               { return CHECK_SAT_TOK; }
"check-sat-assuming"      { return CHECK_SAT_ASSUMING_TOK;}
"declare-const"           { return DECLARE_CONST_TOK;}
"declare-fun"             { return DECLARE_FUNCTION_TOK; }
"declare-sort"            { return DECLARE_SORT_TOK;}
"define-fun"              { return DEFINE_FUNCTION_TOK; }
"define-fun-rec"          { return DECLARE_FUN_REC_TOK;}
"define-funs-rec"         { return DECLARE_FUNS_REC_TOK;}
"define-sort"             { return DEFINE_SORT_TOK;}
"echo"                    { return ECHO_TOK;}
"exit"                    { return EXIT_TOK;}
"get-assertions"          { return GET_ASSERTIONS_TOK;}
"get-assignment"          { return GET_ASSIGNMENT_TOK;}
"get-info"                { return GET_INFO_TOK;}
"get-model"               { return GET_MODEL_TOK;}
"get-option"              { return GET_OPTION_TOK;}
"get-proof"               { return GET_PROOF_TOK;}
"get-unsat-assumption"    { return GET_UNSAT_ASSUMPTION_TOK;}
"get-unsat-core"          { return GET_UNSAT_CORE_TOK;}
"get-value"               { return GET_VALUE_TOK;}
"pop"                     { return POP_TOK;}
"push"                    { return PUSH_TOK;}
"reset"                   { return RESET_TOK;}
"reset-assertions"        { return RESET_ASSERTIONS_TOK;}
"set-info"                { return NOTES_TOK;  }
"set-logic"               { return LOGIC_TOK; }
"set-option"  		        { return SET_OPTION_TOK; }



 /* Types for QF_BV and QF_ABV. */
"BitVec"        { return BITVEC_TOK;}
"Array"         { return ARRAY_TOK;}
"Bool"          { return BOOL_TOK;}


 /* CORE THEORY pg. 29 of the SMT-LIB2 standard 30-March-2010. */
"true"          { return TRUE_TOK; }
"false"         { return FALSE_TOK; }
"not"           { return NOT_TOK; }
"and"           { return AND_TOK; }
"or"            { return OR_TOK; }
"xor"           { return XOR_TOK;}
"ite"           { return ITE_TOK;} // PARAMETRIC
"="             { return EQ_TOK;}
"=>"       		{ return IMPLIES_TOK; }

 /* CORE THEORY. But not on pg 29. */
"distinct"      { return DISTINCT_TOK; }  // variadic
"let"           { return LET_TOK; }

 /* Functions for QF_BV and QF_AUFBV.  */
"bvshl"         { return BVLEFTSHIFT_1_TOK;}
"bvlshr"        { return BVRIGHTSHIFT_1_TOK;}
"bvashr"        { return BVARITHRIGHTSHIFT_TOK;}
"bvadd"         { return BVPLUS_TOK;}
"bvsub"         { return BVSUB_TOK;}
"bvnot"         { return BVNOT_TOK;}
"bvmul"         { return BVMULT_TOK;}
"bvudiv"        { return BVDIV_TOK;}
"bvsdiv"        { return SBVDIV_TOK;}
"bvurem"        { return BVMOD_TOK;}
"bvsrem"        { return SBVREM_TOK;}
"bvsmod"        { return SBVMOD_TOK;}
"bvneg"         { return BVNEG_TOK;}
"bvand"         { return BVAND_TOK;}
"bvor"          { return BVOR_TOK;}
"bvxor"         { return BVXOR_TOK;}
"bvnand"        { return BVNAND_TOK;}
"bvnor"         { return BVNOR_TOK;}
"bvxnor"        { return BVXNOR_TOK;}
"concat"        { return BVCONCAT_TOK;}
"extract"       { return BVEXTRACT_TOK;}
"bvult"         { return BVLT_TOK;}
"bvugt"         { return BVGT_TOK;}
"bvule"         { return BVLE_TOK;}
"bvuge"         { return BVGE_TOK;}
"bvslt"         { return BVSLT_TOK;}
"bvsgt"         { return BVSGT_TOK;}
"bvsle"         { return BVSLE_TOK;}
"bvsge"         { return BVSGE_TOK;}
"bvcomp"        { return BVCOMP_TOK;}
"zero_extend"   { return BVZX_TOK;}
"sign_extend"   { return BVSX_TOK;}
"repeat"        { return BVREPEAT_TOK;}
"rotate_left"   { return BVROTATE_LEFT_TOK;}
"rotate_right"  { return BVROTATE_RIGHT_TOK;}

 /* Functions for QF_AUFBV. */
"select"        { return SELECT_TOK; }
"store"         { return STORE_TOK; }

({LETTER}|{OPCHAR})({ANYTHING})* { return lookup (yytext); }
\|([^\|]|\n)*\| { return lookup(yytext); }

. { yyerror("Illegal input character."); }
%%
