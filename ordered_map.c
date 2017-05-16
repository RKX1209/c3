/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <stdio.h>
#include <stdlib.h>
#include <c3_ordered_map.h>

static int c3_compare_pair (const C3Pair *a, const C3Pair *b) {
  //printf ("compare: %s(%p), %s(%p)\n", a->key, (void*)a->key, b->key, (void*)b->key);
  int cmp = strcmp (a->key, b->key);
  if (cmp == 0) {
    return 0;
  } else if (cmp < 0) {
    return -1;
  }
  return 1;
}

C3Map *c3_map_new (void) {
  return c3_bstree_new ();
}

void c3_map_free (C3Map *map) {
  c3_bstree_free (map);
}

C3MapIter *c3_map_add (C3Map *map, char *key, void *value) {
  C3Pair *new = (C3Pair *)malloc (sizeof (C3Pair));
  C3MapIter *iter;
  if (!new) {
    return NULL;
  }
  new->key = key;
  new->value = value;
  iter = c3_bstree_add (map, new, c3_compare_pair);
  return iter;
}

C3MapIter *c3_map_find (C3Map *map, char *key) {
  C3MapIter *iter;
  C3Pair pair = (C3Pair){key, NULL};
  iter = c3_bstree_find (map, &pair, c3_compare_pair);
  return iter;
}

void *c3_map_get (C3Map *map, char *key) {
  C3MapIter *iter;
  C3Pair pair = (C3Pair){key, NULL}, *res = NULL;
  iter = c3_bstree_find (map, &pair, c3_compare_pair);
  if (!iter) {
    return NULL;
  }
  res = (C3Pair *)iter->data;
  return res->value;
}

C3MapIter *c3_map_set (C3Map *map, char *key, void *value) {
  C3MapIter *iter;
  C3Pair pair = (C3Pair){key, NULL}, *res = NULL;
  iter = c3_bstree_find (map, &pair, c3_compare_pair);
  if (!iter) {
    /* entry is not exist, create new one. */
    iter = c3_map_add (map, key, value);
  }
  return iter;
}
