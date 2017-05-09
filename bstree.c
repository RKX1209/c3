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

  /* Balancing */
  //c3_bstree_balance (bstree);
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

static int32_t _c3_bstree_depth (C3BsTreeIter *iter) {
  int32_t depth_l = 0, depth_r = 0;
  if (iter->l) {
    depth_l = _c3_bstree_depth (iter->l);
  }
  if (iter->r) {
    depth_r = _c3_bstree_depth (iter->r);
  }
  return depth_r > depth_l ? ++depth_r : ++depth_l;
}

static int32_t _c3_bstree_bfact (C3BsTreeIter *iter) {
  int32_t res = 0;
  if (iter->l) {
    res += _c3_bstree_depth (iter->l);
  }
  if (iter->r) {
    res -= _c3_bstree_depth (iter->r);
  }
  return res;
}

/* Left Right Rotate */
static C3BsTreeIter *_c3_bstree_rotate_lr (C3BsTreeIter *iter) {
  C3BsTreeIter *left = iter->l;
  C3BsTreeIter *right = left->r;
  iter->l = right->r;
  left->r = right->l;
  right->l = left;
  right->r = iter;
  return right;
}

/* Left Left Rotate */
static C3BsTreeIter *_c3_bstree_rotate_ll (C3BsTreeIter *iter) {
  C3BsTreeIter *left = iter->l;
  iter->l = left->r;
  left->r = iter;
  return left;
}

/* Right Left Rotate */
static C3BsTreeIter *_c3_bstree_rotate_rl (C3BsTreeIter *iter) {
  C3BsTreeIter *right = iter->r;
  C3BsTreeIter *left = right->l;
  iter->r = right->l;
  right->l = left->r;
  left->l = iter;
  left->r = right;
  return left;
}

/* Left Left Rotate */
static C3BsTreeIter *_c3_bstree_rotate_rr (C3BsTreeIter *iter) {
  C3BsTreeIter *right = iter->r;
  iter->r = right->l;
  right->l = iter;
  return right;
}

static C3BsTreeIter *_c3_avl_balance (C3BsTreeIter *iter) {
  C3BsTreeIter *newroot = NULL;
  if (iter->l) {
    iter->l = _c3_avl_balance (iter->l);
  }
  if (iter->r) {
    iter->r = _c3_avl_balance (iter->r);
  }

  int32_t bfact = _c3_bstree_bfact (iter);

  if (bfact >= 2) {
    /* left heavy*/
    if (_c3_bstree_bfact (iter->l) <= -1) {
      /* left right heavy case */
      newroot = _c3_bstree_rotate_lr (iter);
    } else {
      /* left left heavy case */
      newroot = _c3_bstree_rotate_ll (iter);
    }
  } else if (bfact <= -2) {
    /* right heavy*/
    if (_c3_bstree_bfact (iter->r) >= 1) {
      /* right left heavy case */
      newroot = _c3_bstree_rotate_rl (iter);
    } else {
      /* right right heavy case */
      newroot = _c3_bstree_rotate_rr (iter);
    }
  } else {
    newroot = iter;
  }
  return newroot;
}

void c3_bstree_balance (C3BsTree *bstree) {
  C3BsTreeIter *newroot = NULL;
  newroot = _c3_avl_balance (bstree->root);
  if (newroot != bstree->root) {
    bstree->root = newroot;
  }
}
