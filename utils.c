/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c3_core.h>
#include <c3_utils.h>

#define DEBUG_LEVEL 1

inline void debug_log (int level, const char *format, ...) {
  va_list ap;
  va_start (ap, format);
  if (level <= DEBUG_LEVEL) {
    vfprintf (stderr, format, ap);
  }
  va_end (ap);
}

int c3_compare_symbol (const int32_t *a, const int32_t *b) {
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

int c3_compare_value (const int32_t *a, const int32_t *b) {
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

void* c3_copy_value (const int32_t *a) {
  int32_t *res = (int32_t*) malloc (sizeof(int32_t));
  *res = *a;
  return (void *)res;
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

char *c3_str_tolower (char *str) {
  while (str) {
    *str = tolower (*str);
    str++;
  }
  return str;
}
