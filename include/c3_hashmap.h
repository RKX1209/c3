#ifndef C3_HASHMAP_H
#define C3_HASHMAP_H

#include <stdbool.h>
#include <stddef.h>
/* Hash Map for <Key, Value> pair */

typedef struct c3_hmap_iter_t {
  char *key;
  void *data;
  struct c3_hmap_iter_t *next;
}C3HmapIter;

typedef struct c3_hmap_t {
  C3HmapIter **table;
  size_t length;
}C3Hmap;

C3Hmap *c3_hmap_new (size_t size);
void c3_hmap_clear (C3Hmap *hmap);
void c3_hmap_free (C3Hmap *hmap);
bool c3_hmap_add (C3Hmap *hmap, char *key, void *value);
bool c3_hmap_add_int32 (C3Hmap *hmap, int32_t key, void *value);
void *c3_hmap_get (C3Hmap *hmap, char *key);
void *c3_hmap_get_int32 (C3Hmap *hmap, int32_t key);
C3HmapIter *c3_hmap_entry_new (char *key, void *value);
unsigned long c3_hash (C3Hmap *hmap, char *key);

#endif
