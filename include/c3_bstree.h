/* Binary Search Tree */
#ifndef C3_BSTREE_H
#define C3_BSTREE_H

typedef struct c3_bstree_t {
  struct c3_bstree_t *l, *r, *pr;
  void *data;
}C3BsTreeIter;

typedef struct c3_bstree {
  C3BsTreeIter *root;
}C3BsTree;

typedef int (*C3BsComparator)(const void *a, const void *b);
C3BsTree *c3_bstree_new (void);
void c3_bstree_free (C3BsTree *bstree);
C3BsTreeIter *c3_bstree_find (C3BsTree *bstree, void *data, C3BsComparator cmp);
C3BsTreeIter *c3_bstree_add (C3BsTree *bstree, void *data, C3BsComparator cmp);
C3BsTreeIter *c3_bstree_delete (C3BsTree *bstree, C3BsTreeIter *iter, C3BsComparator cmp);
C3BsTreeIter *c3_bstree_sub_min (C3BsTreeIter *root, C3BsComparator cmp);
C3BsTreeIter *c3_bstree_min (C3BsTree *bstree, C3BsComparator cmp);
#endif
