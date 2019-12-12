//
// Created by salvo on 10/12/19.
//

#ifndef PNGLIB_BINARYTREE_H
#define PNGLIB_BINARYTREE_H

//#include <stdarg.h>

typedef struct node_st *huff_tree;

huff_tree huff_generateTree(char *lengths, short int *l);
huff_tree huff_getNextNode(huff_tree t, char value);

#endif //PNGLIB_BINARYTREE_H
