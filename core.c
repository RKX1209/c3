/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <c3_core.h>
#include <c3_utils.h>
#include <parser/parsecnf.h>
#include <parser/parser.h>
#include <tosat/bitblaster.h>

typedef enum {
  SMTLIB2,
  DIMACS,
}Format;

char *status_str[] = {"UNSATISFIABLE", "SATISFIABLE", "ERROR" };
C3 c3;

static int help() {
  const char *msg = \
  "Usage: c3 [options] <input-file>\n"
  " where input is SMTLIB2 or DIMACS\n"
  "\n"
  " -h [ --help]\t\tprint this help\n"
  " -v [ --version]\tprint version number\n"
  "\n"
  "Input options:\n"
  " -S [--SMTLIB2]\t\tuse the SMT-LIB2 format\n"
  " -D [--DIMACS]\t\tuse the SMT-LIB2 format\n"
  "\n";
  puts (msg);
  return 0;
}

static int version() {
  printf ("C3 version 1.0.0 \n");
  return 0;
}

C3List* c3_dup_cnf (C3List *cnf) {
  C3ListIter *iter;
  C3List *res, *disj;
  res = c3_list_new ();
  c3_list_foreach (cnf, iter, disj) {
    C3List *disj2 = c3_list_clone (disj, c3_copy_value);
    c3_list_append (res, disj2);
  }
  return res;
}

void c3_del_cnf (C3List *cnf) {
  C3ListIter *iter;
  C3List *disj, *next;
  iter = cnf->head;
  while (iter) {
    disj = (C3List*)iter->data;
    next = iter->n;
    c3_list_free (disj);
    iter = next;
  }
  free (cnf);
}

/* Delete '-l' entries and entire lists that contain 'l' */
static void _c3_dpll_remove2 (C3 *c3, C3List *cnf, int32_t l) {
  C3ListIter *iter, *next;
  C3List *disj;
  iter = cnf->head;
  while (iter && (disj = iter->data)) {
    next = iter->n;
    C3ListIter *fnd;
    if (fnd = c3_list_find (disj, (void*)&l, c3_compare_symbol)) {
      int32_t* found = c3_list_get_data (fnd);
      debug_log (2, "found: %d\n",*found);
      if ((l < 0 && *found < 0) || (l > 0 && *found > 0)) {
        /* same literal (same sign) */
        /* Delete all elements of list */
        c3_list_clear (disj);
        c3_listiter_delete (cnf, iter);
      } else {
        /* different sign */
        /* Delete only one literal */
        c3_listiter_delete (disj, fnd);
      }
    }
    iter = next;
  }
}

/* one literal rule */
static C3_STATUS _c3_dpll_simplify1(C3 *c3, C3List *cnf, int32_t *res) {
  bool simplify;
  C3_STATUS status = C3_NONE;
  C3ListIter *iter, *next;
  C3List *disj;
  do {
    simplify = false;
    iter = cnf->head;
    while (iter && (disj = iter->data)) {
      next = iter->n;
      if (c3_list_length (disj) == 1) {
        /* Found alone literal */
        int32_t* onep = c3_list_head_data (disj);
        int32_t one = *onep;
        int32_t oneab = abs(one);
        int32_t value = (one < 0) ? -1 : 1;
        debug_log (1, "found one literal %d(%p)\n", one, onep);
        if (res[oneab - 1] && res[oneab - 1] != value) {
          /* literal is already set. inconsistent */
          return C3_UNSAT;
        }
        res[oneab - 1] = value;
        simplify = true;
        _c3_dpll_remove2 (c3, cnf, one);
        status = C3_SIMPLIFY;
        _c3_print_cnf (cnf);
      }
      iter = next;
    }
  } while (simplify);
  return status;
}

/* pure literal rule */
static bool _c3_dpll_simplify2(C3 *c3, C3List *cnf, int32_t *res) {
  C3ListIter *iter, *iter2, *next;
  C3List *disj;
  int32_t *num, i, absi;
  bool *flag;
  C3_STATUS status = C3_NONE;

  /* Memorize all symbols */
  c3_list_foreach (cnf, iter, disj) {
    c3_list_foreach (disj, iter2, num) {
      flag = malloc (sizeof(bool));
      if (!flag) {
        return false;
      }
      *flag = true;
      c3_hmap_add_int32 (c3->literals, *num, flag);
      //c3_bstree_add (c3->literals2, num, c3_compare_value);
    }
  }

  /* Delete if literal is pure */
  for (i = -c3->valnum; i <= c3->valnum; i++) {
    if (!c3_hmap_get_int32 (c3->literals, i)) {
      continue;
    }
    if (c3_hmap_get_int32 (c3->literals, -i)) {
      /* not pure. */
      continue;
    }
    absi = abs(i);
    int32_t value = (i < 0) ? -1 : 1;
    if (res[absi - 1] && res[absi - 1] != value) {
      /* literal is already set. inconsistent */
      return C3_UNSAT;
    }
    res[absi - 1] = value;
    iter = cnf->head;
    debug_log (1, "pure literal %d\t%p\n", i, iter);
    while (iter) {
      disj = iter->data;
      next = iter->n;
      if (c3_list_find (disj, &i, c3_compare_value)) {
        c3_list_clear (disj);
        c3_listiter_delete (cnf, iter);
      }
      iter = next;
    }
    status = C3_SIMPLIFY;
    _c3_print_cnf (cnf);
  }
  c3_hmap_clear (c3->literals);
  return status;
}

/* Simplification rules */
static C3_STATUS _c3_dpll_simplify(C3 *c3, C3List *cnf, int32_t *res) {
  C3_STATUS status1 = C3_NONE, status2 = C3_NONE;
  do {
    status1 = _c3_dpll_simplify1(c3, cnf, res);
    if (status1 == C3_UNSAT) return C3_UNSAT;
    status2 = _c3_dpll_simplify2(c3, cnf, res);
    if (status2 == C3_UNSAT) return C3_UNSAT;
  } while (status1 == C3_SIMPLIFY || status2 == C3_SIMPLIFY);
  return C3_NONE;
}

static C3_STATUS _c3_derive_dpll(C3 *c3, C3List *cnf, int32_t *res);

/* Split rule */
static C3_STATUS _c3_dpll_split (C3 *c3, C3List *cnf, int32_t *res) {
  C3List *hdisj, *tlist, *flist;
  int32_t l;
  /* Get head literal of CNF. */
  hdisj = c3_list_head_data (cnf);
  l = abs(*(int32_t*)c3_list_head_data (hdisj));
  /* Assume that l = true */
  debug_log (1, "assume %d\n", l);
  tlist = c3_dup_cnf (cnf);
  _c3_dpll_remove2 (c3, tlist, l);
  _c3_print_cnf (tlist);
  res[l - 1] = 1;
  if (_c3_derive_dpll (c3, tlist, res) == C3_SAT) {
    c3_list_free (tlist);
    return C3_SAT;
  }

  /* Assume that l = false */
  debug_log (1, "assume %d\n", -l);
  flist = c3_dup_cnf (cnf);
  _c3_dpll_remove2 (c3, flist, -l);
  _c3_print_cnf (flist);
  res[l - 1] = -1;
  if (_c3_derive_dpll (c3, flist, res) == C3_SAT) {
    c3_list_free (flist);
    return C3_SAT;
  }

  c3_list_free (tlist);
  c3_list_free (flist);
  return C3_UNSAT;
}

/* DPLL algorithm */
static C3_STATUS _c3_derive_dpll(C3 *c3, C3List *cnf, int32_t *res) {
  C3ListIter *iter;
  C3List *disj;
  /* Simplify */
  if (_c3_dpll_simplify (c3, cnf, res) == C3_UNSAT) {
    return C3_UNSAT;
  }
  if (c3_list_empty (cnf)) {
    return C3_SAT;
  }
  c3_list_foreach (cnf, iter, disj) {
    if (c3_list_empty (disj)) {
      return C3_UNSAT;
    }
  }

  return _c3_dpll_split (c3, cnf, res);
}

C3_STATUS c3_derive_sat(C3 *c3, int32_t *res) {
  if (c3 && res) {
    C3List *cnf2 = c3_dup_cnf (c3->cnf);
    C3_STATUS status = _c3_derive_dpll (c3, cnf2, res);
    c3_del_cnf (cnf2);
    return status;
  }
  return C3_INVALID;
}

void c3_init (C3 *c3) {
  c3->cnf = c3_list_new ();
  c3->asserts = c3_list_new ();
  c3->literals = c3_hmap_new (100); //TODO: implment rehash function
  c3->literals2 = c3_bstree_new ();
  c3->symbols = c3_map_new ();
  c3->funcnames = c3_map_new ();
  c3->symbol_num = 0;
}

void c3_fini (C3 *c3) {
  c3_del_cnf (c3->cnf);
  c3_list_free (c3->asserts);
  c3_hmap_free (c3->literals);
  c3_bstree_free (c3->literals2);
  c3_map_free (c3->symbols);
  c3_map_free (c3->funcnames);
}

/* XXX: uncessary function. should duplicate it */
void c3_sort_cnf (C3 *c3) {
  C3ListIter *iter;
  C3List *disj;
  c3_list_foreach (c3->cnf, iter, disj) {
    //c3_list_merge_sort (disj, c3_compare_value);
    c3_list_quick_sort (disj, c3_compare_value);
  }
}

/* Verify the result of SAT */
bool c3_verify_sat (C3 *c3, int32_t *res) {
  C3ListIter *iter, *iter2;
  C3List *disj;
  int32_t *num;
  bool ans = true;
  c3_list_foreach (c3->cnf, iter, disj) {
    bool disjr = false;
    c3_list_foreach (disj, iter2, num) {
      debug_log (2, "%d(%d) ", abs (*num), res[abs(*num) - 1]);
      if (res[abs(*num) - 1] * (*num) >= 0) disjr |= true;
    }
    ans &= disjr;
  }
  debug_log (2, "\n");
  return ans;
}

/* Lookup symbol */
ASTNode* c3_lookup_symbol (C3 *c3, char *symbol) {
  return (ASTNode*)c3_map_get (c3->symbols, symbol);
}

/* Add symbol */
void c3_add_symbol (C3 *c3, char *symbol, ASTNode *n) {
  if (!c3_map_find (c3->symbols, symbol)) {
    c3->symbol_num++;
  }
  printf ("Add symbol %s\n", symbol);
  n->name = symbol;
  c3_map_set (c3->symbols, symbol, n);
}

ASTNode *c3_create_variable (int index_width, int value_width, char *name) {
  ASTNode *node = malloc (sizeof(ASTNode));
  if (!node) {
    return NULL;
  }
  node->name = name;
  node->index_width = index_width;
  node->value_width = value_width;
  return node;
}

void c3_store_function(C3 *c3, char *name, ASTVec params, ASTNode* function) {
  Function *f = malloc (sizeof(Function));
  C3ListIter *iter;
  ASTNode *node;
  const char *prefix = "arg";
  size_t i;
  if (!f) {
    return;
  }
  f->name = strdup (name);
  c3_list_foreach (params, iter, node) {
    char *buf = malloc (sizeof (char) * (strlen (prefix) + 34));
    sprintf (buf, "%s_%d", prefix, c3->symbol_num);
    ASTNode *param = c3_create_variable (node->index_width, node->value_width, buf);
    ast_vec_add (f->params, param);
  }
  // XXX:
  f->function = function;
  c3_map_set (c3->funcnames, f->name, f);
}

void c3_unsupported(C3 *c3) {
  debug_log (-1, "unsupported\n");
}

void c3_add_assert (C3 *c3, ASTNode *assert) {
  /* if (ast_get_type (assert) != BOOLEAN_TYPE) {
    c3_fatal_error ("Add assert: Trying to assert a non-formula:\n");
  } */
  c3_list_append (c3->asserts, assert);
  ast_print (assert);
  printf("\n");
}

void c3_solve_by_sat(C3 *c3, ASTNode *assertq) {
  /* Bitblasting: Translate SMT operations to combinational logic */
  //c3_bitblast (c3, assertq);
}

int main(int argc, char **argv, char **envp) {
  int c;
  FILE* cnfp;
  char *cnf_path, *cnf;
  int32_t *res;
  C3_STATUS status;
  bool correct = true;
  Format file_format;
  ASTNode *assertq;
  C3ListIter *iter;
  struct option longopts[] = {
    { "help", no_argument, NULL, 'h'},
    { "version", no_argument, NULL, 'v'},
    { "DIMACS", required_argument, NULL, 'D'},
    { "SMTLIB2", required_argument, NULL, 'S'},
    {0, 0, 0, 0},
  };

  if (argc < 2) {
    return help ();
  }
  while ((c = getopt_long (argc, argv, "hvD:S:", longopts, NULL)) != -1) {
    switch (c) {
      case 'h':
        help ();
        break;
      case 'v':
        version ();
        break;
      case 'D': // SAT format file (DIMACS)
        file_format = DIMACS;
        cnf_path = strdup (optarg);
        break;
      case 'S': // STP format file (SMTLIB2)
        file_format = SMTLIB2;
        cnf_path = strdup (optarg);
        break;
      default:
        break;
    }
  }
  if (!cnf_path) {
    /* default format */
    file_format = SMTLIB2;
    cnf_path = argv[optind] ? strdup (argv[optind]) : NULL;
  }
  cnfp = c3_file_open (cnf_path, "r");
  if (!cnfp) {
    return 0;
  }
  free ((void*)cnf_path);

  /* Init */
  c3_init (&c3);

  if (file_format == DIMACS) {
    /* SAT mode */
    cnf = c3_file_read (cnfp, NULL);
    if (!cnf) {
      fclose (cnfp);
      return 0;
    }

    if (!c3_parse_cnffile (&c3, cnf)) {
      fclose (cnfp);
      free (cnf);
      return 0;
    }
    c3_print_cnf (&c3);
    c3_sort_cnf (&c3);
    res = (int8_t*) calloc (c3.valnum, sizeof(int32_t));
    status = c3_derive_sat (&c3, res);
    //c3_print_cnf (&c3);
    debug_log (-1, "s %s\n", status_str[status]);
    if (status == C3_SAT) {
      /* print result */
      debug_log (-1, "v ");
      int i;
      for (i = 0; i < c3.valnum; i++) {
          if (res[i] >= 0) debug_log (-1, "%d ", i + 1);
          else debug_log (-1, "%d ", -(i + 1));
      }
      debug_log (-1, "0\n");
      correct = c3_verify_sat (&c3, res);
    }
    printf ("Verifying.... [%s]\n", (correct ? "SUCCESS":"FAIL"));
    free (res);
  } else {
    /* SMT mode */
    c3_smt2_parse (cnfp);
    c3_list_foreach (c3.asserts, iter, assertq) {
      c3_solve_by_sat (&c3, assertq);
    }
  }
  /* Finish */
  fclose (cnfp);
  c3_fini (&c3);
}
