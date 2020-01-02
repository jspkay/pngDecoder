//
// Created by salvo on 10/12/19.
//

#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include "huffmanTree.h"

typedef struct node_st *node;
struct node_st{
    signed short int val; // Ho 285 (16 bit mi avanzano!) valori
    node l, r;
};
struct count_st{
    int count; //how many with that codeLength
    int codeLength;
    short int startingCode;
};

void __ordina(struct count_st **c, int n){
    int swapped;
    do{
        swapped = 0;
        for(int i=0; i<n-1; i++){
            if(c[i]->codeLength > c[i+1]->codeLength){
                struct count_st *tmp = c[i];
                c[i] = c[i+1];
                c[i+1] = tmp;
                swapped = 1;
            }
        }
    }while(swapped);
}
char * __scanString(char *string, short int *start, short int *end){
    if(*string == '\0') return NULL;
    *start = 0;
    int interval=1;
    for(  ; *string!='\0' && *string!='-'; string++){
        if(*string == ' '){
            interval = 0;
            break;
        }
        if(*string < '0' || *string > '9'){
            *start = *end = -1;
            return NULL;
        }
        *start = (*start) * (short) 10;
        *start += *string - '0';
    }
    if(*string == '\0'){
        *end = *start;
        return string;
    }
    string++;
    if(!interval){
        *end = *start;
    }else{
        *end = 0;
        for(; *string!='\0' && *string!=' '; string++) {
            if(*string<'0' || *string>'9') {
                *start = *end = -1;
                return NULL;
            }
            *end = *end * (unsigned short) 10;
            *end += *string - '0';
        }

        string++; // ignoro lo spazio
    }
    return string;
}
huff_tree __generateTreeStructure(unsigned short int *codes, const short int *lengths, const short int *alphabet, int n){
    huff_tree res = malloc(sizeof(struct node_st));
    res->val = -1;
    res->l = NULL; res->r = NULL;
    node gn, pn; //gn mean generic node; pn mean present node
    
    //printf("\n\nCODICI:\n");
    for(int i=0; i<n; i++){
        if( codes[i] == (unsigned short) -1) continue;
        pn = res;
        //printf("%d [%d] -> ", i, alphabet[i]);
        codes[i] <<= (unsigned) (16-lengths[i]);
        for(int j=0; j<lengths[i]; j++){
            unsigned int m = (codes[i] & (unsigned) 0x8000) >> (unsigned) 15;
            if(m == 1){
                if(pn->r == NULL) {
                    gn = malloc(sizeof(struct node_st));
                    gn->l = gn->r = NULL;
                    gn->val = -1;
                    pn->r = gn;
                }
                pn = pn->r;
            }else{
                if(pn->l == NULL){
                    gn = malloc(sizeof(struct node_st));
                    gn->l = gn->r = NULL;
                    gn->val = -1;
                    pn->l = gn;
                }
                pn = pn->l;
            }
            //printf("%d", (codes[i] & (unsigned) 0x8000) >> (unsigned) 15);
            codes[i] <<= (unsigned) 1;
        }
        pn->val = alphabet[i];
        //printf(" \n");
    }
    
    return res;
}

int huff_validateTree(huff_tree hf){
    if(hf == NULL) return 1;
    // nodo valido
    if(hf->l == NULL && hf->r == NULL && hf->val != -1) return 1;
    else if(hf->l != NULL && hf->r != NULL && hf->val == -1){
        if( huff_validateTree(hf->l) && huff_validateTree(hf->r) ){
            return 1;
        }
    }
    // il nodo non è valido.
    else return 0;
}

huff_tree huff_generateTree(char *lengths, unsigned short *l) {
    int bl_count_AllocSize=4, bl_count_size=0;
    short int min=1000, max = -1;
    struct count_st *bl_count = malloc(bl_count_AllocSize * sizeof(struct count_st));
    short int *codeLengths=NULL, *alphabet=NULL;
    int alphabetIndex=0, alphabetSize=0;
    
    short int start, end, cycle=0;
    while( (lengths= __scanString(lengths, &start, &end)) != NULL){
        /*
         * Following documentation (RFC 1951)
         */
        
        short int intervalLenght = (short) (end - start + 1);
        alphabetSize += intervalLenght;
        
        codeLengths = realloc(codeLengths, alphabetSize * sizeof(short int));
        alphabet = realloc(alphabet, alphabetSize * sizeof(int));
        
        for(int i=start; i<=end; i++){
            alphabet[alphabetIndex] = i;
            codeLengths[alphabetIndex++] = l[cycle];
        }
        
        //STEP 1
        int done = l[cycle] == 0;
        for(int i=0; i<bl_count_size && !done; i++){
            if(bl_count[i].codeLength == l[cycle]){ // se ho già codici con quella lunghezza
                bl_count[i].count += intervalLenght; // mi ricordo quanti codici con quella lunghezza
                done = 1;
            }
        }
        if(!done){
            if(bl_count_size>=bl_count_AllocSize){
                bl_count_AllocSize += 4;
                bl_count = realloc(bl_count, bl_count_AllocSize * sizeof(struct count_st));
            }
            bl_count[bl_count_size].codeLength = l[cycle];
            bl_count[bl_count_size++].count = intervalLenght;
        }
        if(l[cycle] > 0 && l[cycle] < min){ // mi ricordo quant è lungo il codice più corto
            min = l[cycle];
        }
        if(l[cycle] > max) max = l[cycle];
        
        cycle++;
    }
    
    if(bl_count_size>=bl_count_AllocSize){
        bl_count_AllocSize += 4;
        bl_count = realloc(bl_count, bl_count_AllocSize * sizeof(struct count_st));
    }
    bl_count[bl_count_size].codeLength = min - (short)1;
    bl_count[bl_count_size++].count = 0;
    
    /*
     * bl_count_size is actually the length of the array now (so it's
     * not the last index, but the last index + 1)
     *
     */
    
    struct count_st **bl_count_ord = malloc(bl_count_size * sizeof(struct count_st*));
    for(int i=0; i<bl_count_size; i++) bl_count_ord[i] = bl_count + i;
    __ordina(bl_count_ord, bl_count_size);
    int fixedBlCountSize = bl_count_size;
    for(int i=1; i<fixedBlCountSize; i++){
        if(bl_count_ord[i-1]->codeLength != bl_count_ord[i]->codeLength-1){
            for(int j=bl_count_ord[i-1]->codeLength+1; j<bl_count_ord[i]->codeLength; j++){
                if(bl_count_size>=bl_count_AllocSize){
                    bl_count_AllocSize += 4;
                    bl_count = realloc(bl_count, bl_count_AllocSize * sizeof(struct count_st));
                }
                bl_count[bl_count_size].codeLength = j;
                bl_count[bl_count_size++].count = 0;
            }
        }
    }
    
    bl_count_ord = realloc(bl_count_ord, bl_count_size * sizeof(struct count_st));
    for(int i=0; i<bl_count_size; i++) bl_count_ord[i] = bl_count + i;
    __ordina(bl_count_ord, bl_count_size);
    
    //STEP 2
    short int code = 0;
    for(int i=1; i<bl_count_size; i++){
        code = (code + bl_count_ord[i - 1]->count) << (unsigned) 1;
        bl_count_ord[i]->startingCode = code;
    }
    
    unsigned short int *dictionary;
    dictionary = (unsigned short int*) malloc( alphabetSize * sizeof(short int));
    for(int i=0; i<alphabetSize; i++) dictionary[i] = (unsigned short) -1;
    
    //STEP 3 -- Constructing codes
    /*
     * end is the last value read from arguments
     */
    
    assert(codeLengths != NULL);
    
    for(int j=1; j<bl_count_size; j++) {
        int len = bl_count_ord[j] -> codeLength;
        short int nextCode = bl_count_ord[j]->startingCode;
        
        int i;
        for(i = 0; i<alphabetSize; i++) {
            if(codeLengths[i] == len)
                dictionary[i] = nextCode++;
        }
    }
    
    huff_tree res = __generateTreeStructure(dictionary, codeLengths, alphabet, alphabetSize);
    free(dictionary);
    free(bl_count_ord);
    free(bl_count);
    return res;
}
signed short int huff_getValue(huff_tree hf){
    return hf->val;
}
huff_tree huff_getNextNode(huff_tree hf, char value){
    if(hf->val != -1) return hf;
    else if(value == 0) return hf->l;
    return hf->r;
}
void huff_freeTree(huff_tree root){
    if(root->val != -1){
        free(root);
        return;
    }
    
    huff_freeTree(root->r);
    huff_freeTree(root->l);
}