/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <stdbool.h>
#include <stdlib.h>
#include <c3_list.h>

size_t c3_list_length(C3List *list) {
  return list ? list->length : 0;
}

void *c3_list_head_data (C3List *list) {
  if (list && list->head) {
    return list->head->data;
  }
  return NULL;
}

void *c3_list_get_data (C3ListIter *iter) {
  if (iter) {
    return iter->data;
  }
  return NULL;
}

void c3_list_init(C3List *list) {
  list->head = NULL;
  list->tail = NULL;
  list->length = 0;
}

void c3_listiter_delete(C3List *list, C3ListIter *iter) {
  if (list && iter) {
    if (iter == list->head) {
      list->head = iter->n;
    }
    if (iter == list->tail) {
      list->tail = iter->p;
    }
    if (iter->p) {
      iter->p->n = iter->n;
    }
    if (iter->n) {
      iter->n->p = iter->p;
    }
    c3_listiter_free (iter);
    list->length--;
  }
}

bool c3_list_delete_data(C3List *list, void *data, C3ListComparator cmp) {
  C3ListIter *iter;
  iter = c3_list_find (list, data, cmp);
  if (iter) {
    c3_listiter_delete (list, iter);
    return true;
  }
  return false;
}

void c3_listiter_free(C3ListIter *iter) {
  if (iter) {
    if (iter->data) {
      free (iter->data);
    }
    free (iter);
  }
}

void c3_list_clear(C3List *list) {
  if (list) {
    C3ListIter *it, *next;
    it = list->head;
    while (it) {
      next = it->n;
      c3_listiter_delete (list, it);
      it = next;
    }
  }
}

void c3_list_free(C3List *list) {
  if (list) {
    c3_list_clear (list);
    free (list);
  }
}

C3List* c3_list_new() {
  C3List *list = (C3List*)calloc (1, sizeof(C3List));
  if (!list) {
    return NULL;
  }
  c3_list_init (list);
  return list;
}

C3ListIter *c3_list_append(C3List *list, void *data) {
  C3ListIter *iter;
  if (list && data) {
    iter = (C3ListIter*)calloc (1, sizeof(C3ListIter));
    if (list->tail) {
      list->tail->n = iter;
    }
    iter->data = data;
    iter->p = list->tail;
    iter->n = NULL;
    list->tail = iter;
    if (!list->head) {
      list->head = iter;
    }
    list->length++;
  }
  return iter;
}

C3ListIter *c3_list_find(C3List *list, const void *data, C3ListComparator cmp) {
  C3ListIter *iter;
  void *v;
  c3_list_foreach (list, iter, v) {
    /* TODO: Add compare function support*/
    if (cmp (data, v) == 0) {
      return iter;
    }
  }
  return NULL;
}
