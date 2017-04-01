#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c3_hashmap.h"

C3Hmap *c3_hmap_new (size_t size) {
  C3Hmap *hmap;
  int32_t i;
  if (!size) {
    return NULL;
  }
  if ((hmap = (C3Hmap *) malloc (sizeof(C3Hmap))) == NULL) {
    return NULL;
  }
  if ((hmap->table = (C3HmapIter **) malloc (sizeof(C3HmapIter *) * size)) == NULL) {
    return NULL;
  }
  for (i = 0; i < size; i++) {
    hmap->table[i] = NULL;
  }
  hmap->length = size;
  return hmap;
}

void c3_hmap_clear (C3Hmap *hmap) {
  int32_t i;
  C3HmapIter *entry, *next;
  for (i = 0; i < hmap->length; i++) {
    entry = hmap->table[i];
    while (entry) {
      next = entry->next;
      free (entry->key);
      free (entry->data);
      free (entry);
      entry = next;
    }
    hmap->table[i] = NULL;
  }
}

void c3_hmap_free (C3Hmap *hmap) {
  c3_hmap_clear (hmap);
  free (hmap);
}

unsigned long c3_hash (C3Hmap *hmap, char *key) {
  int32_t i = 0;
  unsigned long hashval = 0;
  while (hashval < ULONG_MAX && i < strlen (key)) {
    hashval = hashval << 8;
    hashval += key[i];
    i++;
  }
  return hashval % hmap->length;
}

C3HmapIter *c3_hmap_entry_new (char *key, void *value) {
  C3HmapIter *entry;
  if ((entry = malloc (sizeof(C3HmapIter))) == NULL) {
    return NULL;
  }
  if ((entry->key = strdup (key)) == NULL) {
    return NULL;
  }
  entry->data = value;
  entry->next = NULL;
  return entry;
}

bool c3_hmap_add (C3Hmap *hmap, char *key, void *value) {
  int32_t hash;
  C3HmapIter *entry = NULL, *last = NULL, *newent = NULL;

  hash = c3_hash (hmap, key);
  entry = hmap->table[hash];

  while (entry != NULL && strcmp (key, entry->key) < 0) {
    last = entry;
    entry = entry->next;
  }

  /* Replace exist element */
  if (entry != NULL && strcmp (key, entry->key) == 0) {
    free (entry->data);
    entry->data = value;
    //printf ("exist ");
  } else {
    /* couldn't find it. add new one */
    newent = c3_hmap_entry_new (key, value);
    if (!newent) {
      return false;
    }
    /* not exist any elements */
    if (last == NULL) {
      hmap->table[hash] = newent;
    } else if (last->next == NULL){
      /* tail of linked list */
      last->next = newent;
    } else {
      /* middle of linker list */
      newent->next = last->next;
      last->next = newent;
    }
    //printf ("Add ");
  }
  //printf ("set: hash=%d key=%s\n", hash, key);
  return true;
}

bool c3_hmap_add_int32 (C3Hmap *hmap, int32_t key, void *value) {
  char buf[16];
  sprintf (buf, "%d", key);
  return c3_hmap_add (hmap, buf, value);
}

void *c3_hmap_get (C3Hmap *hmap, char *key) {
  C3HmapIter *entry;
  unsigned long hash;
  hash = c3_hash (hmap, key);
  //printf ("get: hash=%d key=%s\n", hash, key);
  for (entry = hmap->table[hash]; entry; entry = entry->next) {
    if (strcmp (entry->key, key) == 0) {
      return entry;
    }
  }
  return NULL;
}

void *c3_hmap_get_int32 (C3Hmap *hmap, int32_t key) {
  char buf[16];
  sprintf (buf, "%d", key);
  return c3_hmap_get (hmap, buf);
}
