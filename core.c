/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c3_list.h>
#include <c3_hashmap.h>

#define C3_INVALID -1   //error
#define C3_UNSAT    0   //unatisfiable
#define C3_SAT      1   //satisfiable

typedef uint8_t C3_STATUS;
char *status_str[2] = {"UNSATISFIABLE", "SATISFIABLE"};

typedef struct c3_t {
  int32_t valnum;
  int32_t disjnum;
  C3List *cnf; // <cnf[i] = i-th Disjunction>
  C3Hmap *literals;
}C3;
static C3 c3;

static int help() {
  printf ("Usage: c3 <cnf-file>\n");
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

void c3_print_cnf (C3 *c3) {
  C3ListIter *iter, *iter2;
  C3List *disj;
  int32_t *num;
  //printf ("size=%d, val=%d\n", c3->disjnum, c3->valnum);
  c3_list_foreach (c3->cnf, iter, disj) {
    printf ("(");
    c3_list_foreach (disj, iter2, num) {
      if (*num < 0) {
        printf ("!x%d", -(*num));
      } else {
        printf ("x%d", *num);
      }
      if (iter2 != disj->tail) { //XXX
        printf (" or ");
      }
    }
    printf (")");
    if (iter == c3->cnf->tail) {
      printf ("\n");
    } else {
      printf (" and ");
    }
  }
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
  int32_t res = 0, dig = 1;
  int sign = 1;
  if (**p == '-') {
    sign = -1;
    (*p)++;
  }
  //XXX may overflow. and must treat invalid num, like 8p30
  while (c3_is_digit (**p)) {
    res += (**p - '0') * dig;
    dig *= 10;
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
      printf ("size=%d, val=%d\n", c3->disjnum, c3->valnum);
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
          printf ("%d ", inum);
          c3_list_append (disj, inump);
        }
        printf ("\n");
        data++; //skip '\n'
      }
    } else {
      switch (data[0]) {
        case 'c':
          printf("c: comment\n");
          data++; //skip 'c'
          skip_until_line (&data);
          data++; //skip '\n'
          break;
        case 'p':
          printf("p: header\n");
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
          printf ("cannnot read '%c'\n", data[0]);
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

/* one literal rule */
static bool _c3_dpll_simplify1(C3 *c3, int8_t *res) {
  bool simplify;
  bool updated = false;
  C3ListIter *iter, *iter2, *next, *next2;
  C3List *disj, *disj2;
  do {
    simplify = false;
    iter = c3->cnf->head;
    while (iter && (disj = iter->data)) {
      next = iter->n;
      if (c3_list_length (disj) == 1) {
        /* Found alone literal */
        int32_t* onep = c3_list_head_data (disj);
        int32_t oneab = abs(*onep);
        //printf ("found one literal %d\n", *onep);
        res[oneab - 1] = (*onep < 0) ? -1 : 1;
        simplify = true;
        iter2 = c3->cnf->head;
        while (iter2 && (disj2 = iter2->data)) {
          next2 = iter2->n;
          if (disj == disj2) {  // must not iterate same disjunction
            iter2 = next2;
            continue;
          }
          C3ListIter *fnd;
          if (fnd = c3_list_find (disj2, (void*)onep, c3_compare_symbol)) {
            int32_t* found = c3_list_get_data (fnd);
            //printf ("found: %d\n",*found);
            if (*onep < 0 && *found < 0 || *onep > 0 && *found > 0) {
              /* same literal (same sign) */
              /* Delete all elements of list */
              c3_list_clear (disj2);
              c3_listiter_delete (c3->cnf, iter2);
            } else {
              /* different sign */
              /* Delete only one literal */
              c3_listiter_delete (disj2, fnd);
            }
          }
          iter2 = next2;
        }
        c3_list_clear (disj);
        c3_listiter_delete (c3->cnf, iter);
        updated = true;
      }
      iter = next;
    }
  } while (simplify);
  return updated;
}

/* pure literal rule */
static bool _c3_dpll_simplify2(C3 *c3, int8_t *res) {
  C3ListIter *iter, *iter2, *next;
  C3List *disj;
  int32_t *num, i;
  bool *flag;
  bool updated = false;

  /* Memorize all symbols */
  c3_list_foreach (c3->cnf, iter, disj) {
    c3_list_foreach (disj, iter2, num) {
      flag = malloc (sizeof(bool));
      if (!flag) {
        return false;
      }
      *flag = true;
      c3_hmap_add_int32 (c3->literals, *num, flag);
    }
  }

  /* Delete if literal is pure */
  for (i = 1; i <= c3->valnum; i++) {
    if (!c3_hmap_get_int32 (c3->literals, i)) {
      continue;
    }
    if (c3_hmap_get_int32 (c3->literals, -i)) {
      /* not pure. */
      continue;
    }
    //printf ("pure literal %d\n", i);
    iter = c3->cnf->head;
    while (iter) {
      disj = iter->data;
      next = iter->n;
      if (c3_list_find (disj, &i, c3_compare_value)) {
        c3_list_clear (disj);
        c3_listiter_delete (c3->cnf, iter);
      }
      iter = next;
    }
    updated = true;
  }
  c3_hmap_clear (c3->literals);
  return updated;
}

/* Simplification rules */
static void _c3_dpll_simplify(C3 *c3, int8_t *res) {
  while (_c3_dpll_simplify1(c3, res) ||
        _c3_dpll_simplify2(c3, res) );
}

/* DPLL algorithm */
static C3_STATUS _c3_derive_dpll(C3 *c3, int8_t *res) {
  _c3_dpll_simplify (c3, res);
}

C3_STATUS c3_derive_sat(C3 *c3, int8_t *res) {
  if (c3 && res) {
    return _c3_derive_dpll (c3, res);
  }
  return C3_INVALID;
}

void c3_init (C3 *c3) {
  c3->cnf = c3_list_new ();
  c3->literals = c3_hmap_new (50); //TODO: implment rehash function
}

void c3_fini (C3 *c3) {
  C3ListIter *iter;
  C3List *disj;
  c3_list_foreach (c3->cnf, iter, disj) {
    c3_list_free (disj);
  }
  c3_list_free (c3->cnf);
  c3_hmap_free (c3->literals);
}

char* c3_file_read (FILE *fp, long *len) {
  char *buf;
  long sz;
  if (fp) {
    fseek (fp, 0, SEEK_END);
    sz = ftell (fp);
    buf = (char *) calloc (sz, sizeof(char));
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
  fprintf (stderr, "file_read: can not read file\n");
  return NULL;
}

FILE* c3_file_open (const char *file, const char *mode) {
  FILE* fp;
  if (!file || !mode) {
    return NULL;
  }
  if ((fp = fopen (file, mode)) == NULL) {
    fprintf (stderr, "file_open: can not open sucn file\n");
    return NULL;
  }
  return fp;
}

/* XXX: uncessary function. should duplicate it */
void c3_sort_cnf (C3 *c3) {
  C3ListIter *iter;
  C3List *disj;
  int32_t *num;
  c3_list_foreach (c3->cnf, iter, disj) {
    //c3_list_merge_sort (disj, c3_compare_value);
    c3_list_quick_sort (disj, c3_compare_value);
  }
}

int main(int argc, char **argv, char **envp) {
  int c;
  FILE* cnfp;
  char *cnf_path, *cnf;
  int8_t *res;

  if (argc < 2) {
    return help ();
  }
  while ((c = getopt (argc, argv, "h")) != -1) {
    switch (c) {
      case 'h':
        help ();
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
  c3_print_cnf (&c3);
  res = (int8_t*) calloc (c3.valnum, sizeof(int8_t));
  c3_derive_sat (&c3, res);
  c3_print_cnf (&c3);

  /* Finish */
  fclose (cnfp);
  //free (res);
  //c3_fini (&c3);
}
