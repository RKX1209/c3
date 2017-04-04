/* C3 Theorem Prover - Apache v2.0 - Copyright 2017 - rkx1209 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <c3_bstree.h>

C3BsTree *c3_bstree_new (void) {
  C3BsTree *tree = calloc (1, sizeof(C3BsTree));
  if (!tree) {
    return NULL;
  }
  tree->root = NULL;
  return tree;
}

void _c3_bstree_free (C3BsTreeIter *iter) {
  if (!iter) {
    return;
  }
  if (iter->l) {
    _c3_bstree_free (iter->l);
  }
  if (iter->r) {
    _c3_bstree_free (iter->r);
  }
  free (iter->data);
  free (iter);
}

void c3_bstree_free (C3BsTree *bstree) {
  _c3_bstree_free (bstree->root);
}

C3BsTreeIter *c3_bstree_find (C3BsTree *bstree, void *data, C3BsComparator cmp) {
  C3BsTreeIter *iter = bstree->root;
  while (iter != NULL) {
    if (cmp (data, iter->data) < 0) {
      iter = iter->l;
    } else if (cmp (data, iter->data) > 0) {
      iter = iter->r;
    } else {
      break;
    }
  }
  return iter;
}

C3BsTreeIter *c3_bstree_add (C3BsTree *bstree, void *data, C3BsComparator cmp) {
  if (!bstree->root) {
    bstree->root = calloc (1, sizeof(C3BsTreeIter));
    bstree->root->data = data;
    bstree->root->l = bstree->root->r = NULL;
    return bstree->root;
  }
  C3BsTreeIter *iter = bstree->root;
  C3BsTreeIter *parent;
  while (iter) {
    parent = iter;
    if (cmp (data, iter->data) < 0) {
      iter = iter->l;
    } else if (cmp (data, iter->data) > 0){
      iter = iter->r;
    } else {
      /* limit duplicate elements */
      return NULL;
    }
  }
  iter = calloc (1, sizeof(C3BsTreeIter));
  iter->l = iter->r = NULL;
  iter->pr = parent;
  iter->data = data;
  if (cmp (data, parent->data) < 0) {
    parent->l = iter;
  } else {
    parent->r = iter;
  }
}

C3BsTreeIter *c3_bstree_sub_min (C3BsTreeIter *root, C3BsComparator cmp) {
  C3BsTreeIter *iter = root;
  while (iter->l) {
    iter = iter->l;
  }
  return iter;
}

C3BsTreeIter *c3_bstree_min (C3BsTree *bstree, C3BsComparator cmp) {
  return c3_bstree_sub_min (bstree->root, cmp);
}

C3BsTreeIter *c3_bstree_del_min (C3BsTreeIter *iter, C3BsComparator cmp) {
  if (iter->l == NULL) {
    C3BsTreeIter *right = iter->r;
    //Don't delete iter->data
    free (iter);
    return right;
  }
  iter->l = c3_bstree_del_min (iter->l, cmp);
  return iter;
}

static C3BsTreeIter *_c3_bstree_delete (C3BsTreeIter *iter,
                                        C3BsTreeIter *target, C3BsComparator cmp) {
  if (iter == NULL) {
    return NULL;
  }
  if (cmp (target->data, iter->data) == 0) {
    /* Found. So now delete it */
    if (iter->l == NULL) {
      C3BsTreeIter *right = iter->r;
      free (iter->data);
      free (iter);
      return right;
    }
    if (iter->r == NULL) {
      C3BsTreeIter *left = iter->l;
      free (iter->data);
      free (iter);
      return left;
    }
    C3BsTreeIter *min = c3_bstree_sub_min (iter->r, cmp);
    iter->data = min->data;
    c3_bstree_del_min (iter->r, cmp); //delete minimum node of right sub tree
  } else if (cmp (target->data, iter->data) < 0) {
    iter->l = _c3_bstree_delete (iter->l, target, cmp);
  } else {
    iter->r = _c3_bstree_delete (iter->r, target, cmp);
  }
  return iter;
}

C3BsTreeIter *c3_bstree_delete (C3BsTree *bstree, C3BsTreeIter *target, C3BsComparator cmp) {
  return _c3_bstree_delete (bstree->root, target, cmp);
}
