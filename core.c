/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c3_list.h>
#include <c3_hashmap.h>
#include <c3_bstree.h>

#define DEBUG_LEVEL -1

#define C3_UNSAT    0   //unatisfiable
#define C3_SAT      1   //satisfiable
#define C3_INVALID  2   //error
#define C3_SIMPLIFY 3   //can still simplify
#define C3_NONE     4   //none

typedef uint8_t C3_STATUS;
char *status_str[] = {"UNSATISFIABLE", "SATISFIABLE", "ERROR" };

typedef struct c3_t {
  int32_t valnum;
  int32_t disjnum;
  C3List *cnf; // <cnf[i] = i-th Disjunction>
  C3Hmap *literals;
  C3BsTree *literals2;
}C3;
static C3 c3;

static inline debug_log (int level, const char *format, ...) {
  va_list ap;
  va_start (ap, format);
  if (level <= DEBUG_LEVEL) {
    vfprintf (stderr, format, ap);
  }
  va_end (ap);
}

static int help() {
  printf ("Usage: c3 <cnf-file>\n");
  return 0;
}

static int version() {
  printf ("c3 1.0.0 \n");
  return 0;
}

static int c3_compare_symbol (const int32_t *a, const int32_t *b) {
  int32_t ai, bi;
  ai = (*a < 0) ? -(*a) : *a;
  bi = (*b < 0) ? -(*b) : *b;
  if (ai == bi) {
    return 0;
  } else if (ai < bi) {
    return -1;
  }
  return 1;
}

static int c3_compare_value (const int32_t *a, const int32_t *b) {
  int32_t ai, bi;
  ai = *a;
  bi = *b;
  if (ai == bi) {
    return 0;
  } else if (ai < bi) {
    return -1;
  }
  return 1;
}

static void* c3_copy_value (const int32_t *a) {
  int32_t *res = (int32_t*) malloc (sizeof(int32_t));
  *res = *a;
  return (void *)res;
}

void _c3_print_cnf (C3List *cnf) {
  C3ListIter *iter, *iter2;
  C3List *disj;
  int32_t *num;
  c3_list_foreach (cnf, iter, disj) {
    debug_log (-1, "(");
    debug_log (2, "[%p] ", iter);
    c3_list_foreach (disj, iter2, num) {
      if (*num < 0) {
        debug_log (-1, "!x%d", -(*num));
        debug_log (2, "(%p)", num);
      } else {
        debug_log (-1, "x%d", *num);
        debug_log (2, "(%p)", num);
      }
      if (iter2 != disj->tail) { //XXX
        debug_log (-1, " or ");
      }
    }
    debug_log (-1, ")");
    if (iter == cnf->tail) {
      debug_log (-1, "\n");
    } else {
      debug_log (-1, " and ");
    }
  }
}

void c3_print_cnf (C3 *c3) {
  //printf ("size=%d, val=%d\n", c3->disjnum, c3->valnum);
  _c3_print_cnf (c3->cnf);
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

static void skip_until_line (char **p) {
  while (**p != '\n' && **p != '\0') {
    ++(*p);
  }
}

static void skip_lines (char **p) {
  while (**p == ' ') {
    ++(*p);
  }
}

static inline bool c3_is_digit (char c) {
  return '0' <= c && c <= '9';
}

static int32_t get_digit_long (char **p) {
  int32_t res = 0;
  int sign = 1;
  if (**p == '-') {
    sign = -1;
    (*p)++;
  }
  //XXX may overflow. and must treat invalid num, like 8p30
  while (c3_is_digit (**p)) {
    res *= 10;
    res += (**p - '0');
    (*p)++;
  }
  res *= sign;
  return res;
}

int c3_parse_cnffile (C3 *c3, char *data) {
  int32_t inum = 0, i = 0;
  int32_t *inump = NULL;
  C3ListIter *iter;
  C3List *disj;
  bool valid = false;

  while (*data) {
    if (valid) {
      debug_log (1, "size=%d, val=%d\n", c3->disjnum, c3->valnum);
      for (i = 0; i < c3->disjnum; i++) {
        disj = c3_list_new ();
        c3_list_append (c3->cnf, disj);
        /* disjunctive inputs must be terminated by last '0' */
        while (inum = get_digit_long (&data)) {
          if (!(++data)) {// inum != 0 && data_next = '\0' (i.e not ended by '0')
            c3_list_free (disj);
            c3_list_free (c3->cnf);
            goto fail;
          }
          inump = malloc (sizeof(int));
          *inump = inum;
          debug_log (1, "%d ", inum);
          c3_list_append (disj, inump);
        }
        debug_log (1, "\n");
        /* data may be pointing EOF or '\0' when input is not ended with '\n' */
        data++; //skip '\n'
      }
    } else {
      switch (data[0]) {
        case 'c':
          debug_log (1, "c: comment\n");
          data++; //skip 'c'
          skip_until_line (&data);
          data++; //skip '\n'
          break;
        case 'p':
          debug_log (1, "p: header\n");
          if (!(++data) || strncmp (++data, "cnf", 3) != 0) {
            goto fail;
          }
          data += 3;
          if (!(data++) || !(inum = get_digit_long (&data))) {
            goto fail;
          }
          c3->valnum = inum;
          if (!(data++) || !(inum = get_digit_long (&data))) {
            goto fail;
          }
          c3->disjnum = inum;
          skip_until_line (&data);
          data++; //skip '\n'
          valid = true;
          break;
        default:
          fprintf (stderr, "cannnot read '%c'\n", data[0]);
          goto fail;
      }
    }
    skip_lines (&data);
  }
  return 1;
fail:
  fprintf (stderr, "parse_cnf: failed to parse cnf file\n");
  return 0;
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
      if (l < 0 && *found < 0 || l > 0 && *found > 0) {
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
  c3->literals = c3_hmap_new (100); //TODO: implment rehash function
  c3->literals2 = c3_bstree_new ();
}

void c3_fini (C3 *c3) {
  c3_del_cnf (c3->cnf);
  c3_hmap_free (c3->literals);
  c3_bstree_free (c3->literals2);
}

char* c3_file_read (FILE *fp, long *len) {
  char *buf;
  long sz;
  if (fp) {
    fseek (fp, 0, SEEK_END);
    sz = ftell (fp);
    buf = (char *) calloc (sz + 1, sizeof(char));
    if (!buf) {
      return NULL;
    }
    fseek (fp, 0, SEEK_SET);
    fread (buf, sizeof(char), sz, fp);
    if (len) {
      *len = sz;
    }
    return buf;
  }
  fprintf (stderr, "file_read: cannot read file\n");
  return NULL;
}

FILE* c3_file_open (const char *file, const char *mode) {
  FILE* fp;
  if (!file || !mode) {
    return NULL;
  }
  if ((fp = fopen (file, mode)) == NULL) {
    fprintf (stderr, "file_open: cannot open sucn file %s\n", file);
    return NULL;
  }
  return fp;
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

int main(int argc, char **argv, char **envp) {
  int c;
  FILE* cnfp;
  char *cnf_path, *cnf;
  int32_t *res;
  C3_STATUS status;
  bool correct = true;

  if (argc < 2) {
    return help ();
  }
  while ((c = getopt (argc, argv, "hv")) != -1) {
    switch (c) {
      case 'h':
        help ();
        break;
      case 'v':
        version ();
        break;
      default:
        break;
    }
  }
  cnf_path = argv[optind] ? strdup (argv[optind]) : NULL;
  cnfp = c3_file_open (cnf_path, "r");
  if (!cnfp) {
    return 0;
  }
  free ((void*)cnf_path);

  /* Init */
  c3_init (&c3);

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
  /* Finish */
  fclose (cnfp);
  free (res);
  c3_fini (&c3);
}
