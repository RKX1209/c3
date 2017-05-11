/* Ordered Map implmented as binary search tree. */
#ifndef C3_ORDERED_MAP_H
#define C3_ORDERED_MAP_H

#include <c3_bstree.h>

typedef C3BsTreeIter C3MapIter;
typedef C3BsTree C3Map;
typedef struct c3_pair {
  char *key;
  void *value;
}C3Pair;

C3Map *c3_map_new (void);
void c3_map_free (C3Map *map);
C3MapIter *c3_map_add (C3Map *map, char *key, void *value);
C3MapIter *c3_map_find (C3Map *map, char *key);
void *c3_map_get (C3Map *map, char *key);
C3MapIter *c3_map_set (C3Map *map, char *key, void *value);
#endif
