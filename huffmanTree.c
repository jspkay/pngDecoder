//
// Created by salvo on 10/12/19.
//

#include <stdio.h>
#include <malloc.h>
#include "huffmanTree.h"

typedef struct node_st *node;
struct node_st{
    signed short int val; // Ho 285 (16 bit mi avanzano!) valori
    node l, r;
};
struct count_st{
    short int count; //how many with that codeLength
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
char * __scanString(char *string, short int *s, short int *e){
    if(*string == '\0') return NULL;
    *s = 0;
    for(  ; *string!='-'; string++){
        if(*string == ' ') continue;
        if(*string < '0' || *string > '9'){
            *s = *e = -1;
            return NULL;
        }
        *s = (*s) * (short) 10;
        *s += *string - '0';
    }
    string++;
    *e = 0;
    for(  ; *string!='\0' && *string != ' '; string++){
        if(*string < '0' || *string > '9'){
            *s = *e = -1;
            return NULL;
        }
        *e = (*e) * (short) 10;
        *e += *string - '0';
    }
    return string;
}
huff_tree __getTreeStructure(short int *codes,const short int *lengths, int n){
   huff_tree res = malloc(sizeof(struct node_st));
    res->val = -1;
    res->l = NULL; res->r = NULL;
    node gn, pn; //gn mean generic node; pn mean present node
    
    printf("\n\nCODICI:\n");
    for(int i=0; i<n; i++){
        pn = res;
        printf("%d -> ", i);
        codes[i] <<= (16-lengths[i]);
        for(int j=0; j<lengths[i]; j++){
            int m = (codes[i] & (unsigned) 0x8000) >> (unsigned) 15;
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
            printf("%d", (codes[i] & (unsigned) 0x8000) >> (unsigned) 15);
            codes[i] <<= 1;
        }
        pn->val = (short) i;
        printf(" \n");
    }
    
    return res;
}

huff_tree huff_generateTree(char *lengths, short int *l) {
    int allocatedLenBits=4, lastLenBitIndex=0;
    short int min=1000;
    struct count_st *bl_count = malloc(allocatedLenBits*sizeof(struct count_st));
    signed short int *dictionary;
    signed short int *codeLengths=NULL;
    
    short int start, end, cycle=0;
    while( (lengths= __scanString(lengths, &start, &end)) != NULL){
        /*
         * Following documentation (RFC 1951)
         */
    
        codeLengths = realloc(codeLengths, (end+1)* sizeof(short int));
        for(int i=start; i<=end; i++){
            codeLengths[i] = l[cycle];
        }
        
        //STEP 1
        short int c = end-start+(short)1;
        int done = 0;
        for(int i=0; i<lastLenBitIndex && !done; i++){
            if(bl_count[i].codeLength == l[cycle]){ // se ho già codici con quella lunghezza
                bl_count[i].count += c; // mi ricordo quanti codici con quella lunghezza
                done = 1;
            }
        }
        if(!done){
            if(lastLenBitIndex >= allocatedLenBits){
                allocatedLenBits += 4;
                bl_count = realloc(bl_count, allocatedLenBits * sizeof(struct count_st));
            }
            bl_count[lastLenBitIndex].codeLength = l[cycle];
            bl_count[lastLenBitIndex++].count = c;
        }
        if(l[cycle] < min){ // mi ricordo quant è lungo il codice più corto
            min = l[cycle];
        }
        
        cycle++;
    }
    
    if(lastLenBitIndex >= allocatedLenBits){
        allocatedLenBits += 4;
        bl_count = realloc(bl_count, allocatedLenBits * sizeof(struct count_st));
    }
    bl_count[lastLenBitIndex].codeLength = min-(short)1;
    bl_count[lastLenBitIndex++].count = 0;
    
    /*
     * lastLenBitIndex is actually the length of the array now (so it's
     * not the last index, but the last index + 1)
     *
     */
    
    bl_count = realloc(bl_count, lastLenBitIndex*sizeof(struct count_st));
    struct count_st **bl_count_ord = malloc(lastLenBitIndex * sizeof(struct count_st*));
    for(int i=0; i<lastLenBitIndex; i++) bl_count_ord[i] = bl_count+i;
    __ordina(bl_count_ord, lastLenBitIndex);
    
    lastLenBitIndex--; // now name reflects truth
    
    //STEP 2
    short int code = 0;
    for(int i=1; i<=lastLenBitIndex; i++){
        code = (code + bl_count_ord[i-1]->count) << (unsigned) 1;
        bl_count_ord[i] -> startingCode = code;
    }
    
    dictionary = malloc((end+1) * sizeof(short int));
    
    //STEP 3 -- Constructing codes
    /*
     * end is the last value read from arguments
     */
    
    for(int j=1; j<=lastLenBitIndex; j++) {
        int len = bl_count_ord[j] -> codeLength;
        short int nextCode = bl_count_ord[j]->startingCode;
    
        int i;
        for(i = 0; i<=end; i++) {
            if(codeLengths[i] == len)
                dictionary[i] = nextCode++;
        }
    }
    
    return __getTreeStructure(dictionary, codeLengths, end+1);
}
signed short int huff_getValue(huff_tree hf){
    return hf->val;
}
huff_tree huff_getNextNode(huff_tree hf, char value){
    if(hf->val != -1) return hf;
    else if(value == 0) return hf->l;
    return hf->r;
}