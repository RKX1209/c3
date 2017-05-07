%{
  /* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
  #include "parsesmt2.h"
  extern char *yytext;
%}

LETTER ([a-zA-Z])
DIGIT ([0-9])

%%
[ \n\t\r\f]   { /* skip */ }
%%
