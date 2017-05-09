#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <c3_core.h>
#include <parser/parsecnf.h>

void skip_until_line (char **p) {
  while (**p != '\n' && **p != '\0') {
    ++(*p);
  }
}

void skip_lines (char **p) {
  while (**p == ' ') {
    ++(*p);
  }
}

inline bool c3_is_digit (char c) {
  return '0' <= c && c <= '9';
}

int32_t get_digit_long (char **p) {
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
