/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <c3_list.h>

size_t c3_list_length(C3List *list) {
  return list ? list->length : 0;
}

bool c3_list_empty(C3List *list) {
  return list ? list->length == 0 : false;
}

C3List *c3_list_sub(C3List *list, C3ListIter *start, C3ListIter *end) {
  C3List *clone;
  C3ListIter *iter = start;
  void *data;
  clone = c3_list_new ();
  while (iter) {
    c3_list_append (clone, iter->data);
    iter = iter->n;
    if (iter == end) break;
  }
  return clone;
}

C3List *c3_list_clone(C3List *list) {
  return c3_list_sub (list, list->head, list->tail);
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
  list->sorted = false;
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
    if (cmp (data, v) == 0) {
      return iter;
    }
  }
  return NULL;
}

C3ListIter *_c3_list_split (C3List *list, C3ListIter *iter) {
  if (!iter) {
    return NULL;
  }
  if (iter->p) {
    iter->p->n = NULL;
    iter->p = NULL;
  }
  return iter;
}

C3ListIter *_c3_list_half_split (C3List *list, C3ListIter *head) {
  C3ListIter *iter = head;
  size_t sz = 0, i = 0;
  while (iter) {
    iter = iter->n;
    sz++;
  }
  iter = head;
  while (iter) {
    if (i == sz / 2) {
      break;
    }
    iter = iter->n;
    i++;
  }
  return _c3_list_split (list, iter);
}

C3ListIter *_c3_merge(C3List *list, C3ListIter *first, C3ListIter *second, C3ListComparator cmp) {
  C3ListIter *iter, *iter2, *next;
  iter = first;
  // printf ("merge [");
  // while (iter) {
  //   printf ("%d ", *((int32_t*)iter->data));
  //   iter = iter->n;
  // }
  // printf ("] ");
  // iter = second;
  // printf ("[");
  // while (iter) {
  //   printf ("%d ", *((int32_t*)iter->data));
  //   iter = iter->n;
  // }
  // printf ("]\n");
  iter = first;
  iter2 = second;
  while (iter && iter2) {
    //printf ("cmp %d, %d\n", *((int32_t*)iter->data), *((int32_t*)iter2->data));
    if (cmp (iter->data, iter2->data) > 0) {
      next = iter2->n;
      iter2->n = iter;
      iter2->p = iter->p;
      if (iter->p) {
        //printf ("insert %d\n", *((int32_t*)iter->p->data));
        iter->p->n = iter2;
      } else {
        first = iter2;
      }
      iter->p = iter2;
      iter2 = next;
    } else {
      if (!iter->n) break;
      iter = iter->n;
    }
  }
  while (iter2) {
    iter->n = iter2;
    iter2->p = iter;
    iter = iter->n;
    iter2 = iter2->n;
  }
  return first;
}

C3ListIter *_c3_merge_sort(C3List *list, C3ListIter *head, C3ListComparator cmp) {
  C3ListIter *second;

  if (!head || !head->n) {
    return head;
  }

  second = _c3_list_half_split (list, head);
  head = _c3_merge_sort (list, head, cmp);
  second = _c3_merge_sort (list, second, cmp);
  return _c3_merge (list, head, second, cmp);
}

void c3_list_merge_sort(C3List *list, C3ListComparator cmp) {
  if (!list) {
    return;
  }
  if (!list->sorted && list->head && cmp) {
    C3ListIter *iter;
    list->head = _c3_merge_sort (list, list->head, cmp);
    iter = list->head;
    while (iter && iter->n) {
      iter = iter->n;
    }
    list->tail = iter;
  }
  list->sorted = true;
}

C3ListIter *_c3_get_pivot (C3ListIter *head) {
  return head;
}

C3ListIter *_c3_split_part (C3List *list, C3ListIter *head, C3ListIter *tail, C3ListIter *pivot, C3ListComparator cmp) {
  int32_t li = 0, ri = 0;
  C3ListIter *l = head, *r = tail, *t = head;
  void *tmp;

  while (t) {
    ri++;
    if (t == tail) break;
    t = t->n;
  }
  while (li <= ri) {
    while (li <= ri && cmp (l->data, pivot->data) < 0) {
      l = l->n;
      li++;
    }
    while (r != head && cmp (r->data, pivot->data) > 0) {
      r = r->p;
      ri--;
    }
    if (li > ri) break;
    tmp = r->data;
    r->data = l->data;
    l->data = tmp;
    li++; ri--;
  }
  return l;
}

void _c3_quick_sort (C3List *list, C3ListIter *head, C3ListIter *tail, C3ListComparator cmp) {
  if (!head || !tail) {
    /* after only one element */
    return;
  }
  if (head == tail) {
    return;
  }
  C3ListIter *pivot = _c3_get_pivot (head);
  C3ListIter *iter; void *data;
  C3ListIter *split = _c3_split_part (list, head, tail, pivot, cmp);
  _c3_quick_sort (list, head, split, cmp);
  _c3_quick_sort (list, split->n, tail, cmp);
}

void c3_list_quick_sort(C3List *list, C3ListComparator cmp) {
  if (!list || c3_list_empty (list)) {
    return;
  }
  _c3_quick_sort (list, list->head, list->tail, cmp);
}
