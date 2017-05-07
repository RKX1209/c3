%{
  /* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
  int yyerror(const char *s) {
    //printf ("");
    return 1;
  }
%}

%union {
  unsigned uintval;
};

%start cmd

%%
cmd:
  { }
%%
