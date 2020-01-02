//
// Created by salvo on 10/12/19.
//

#ifndef PNGLIB_BINARYTREE_H
#define PNGLIB_BINARYTREE_H

typedef struct node_st *huff_tree;

int huff_validateTree(huff_tree hf);
huff_tree huff_generateTree(char *lengths, unsigned short *l);
void huff_freeTree(huff_tree hf);
signed short int huff_getValue(huff_tree hf);
huff_tree huff_getNextNode(huff_tree t, char value);

#endif //PNGLIB_BINARYTREE_H
