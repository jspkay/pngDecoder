#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pnglib.h"
#include "zlib.h"
#include "huffmanTree.h"


void png();
void try();
void stampaBit(unsigned char *ptr, int len);
void huffman();
void disegna();

int main(){
    png();
    //huffman();
}

void try(){
    unsigned short int a=1;
    unsigned short int oldA=0;
    int i;
    for(i=0; oldA < a; i++) {
        oldA = a;
        a = (a << (unsigned short) 1) + (short) 1;
        printf("%d\n", a);
    }
    printf("%d\n", i+1);
}
void huffman(){
    /*
    short int lenghts[4] = {8, 9, 7, 8};
    huff_tree hf = huff_generateTree("0-143 144-255 256-279 280-287", lenghts);
    */
    short int lengths[5] = {1, 2, 0, 2};
    huff_tree hf = huff_generateTree("1 2 3-17 18", lengths);

}

void png(){
    pngInitialize();
    pngID p1 = pngOpenFile("../img/img-white10x10.png", "r"),
        p2 = pngOpenFile("../img/wallpaperHuffman.png", "r");
    printf("p1 is png: %c\np2 is png: %c\n",
        pngVerifyType(p1)?'t':'f',
        pngVerifyType(p2)?'t':'f');
    
    if(p2 < 0){
        printf("Error in reading image!\n");
        exit(0);
    }
    pngFileChunk pc = pngReadChunks(p1);
    pngCloseFile(p1); pngCloseFile(p2);
    
    pngChunk *IDAT; int q;
    IDAT = pngGetIDATChunks(pc, &q);
    /*for(int i=0; i<q; i++){
        pngPrintChunk(IDAT[i], stdout);
    }*/
    
    zlib_data data = malloc(sizeof(struct zlib_data_st));
    data->l = IDAT[0]->length;
    data->data = IDAT[0]->data;
    zlib_CMF CMF= zlib_getCM(data);
    zlib_FLG FLG = zlib_getFLG(data);
    printf("CMF --> CM: %x, CINFO: %x\n", CMF->CM, CMF->CINFO);
    printf("FLG --> FCHECK: %x, FDICT: %x, FLEVEL: %x\n", FLG->FCHECK, FLG->FDICT, FLG->FLEVEL);
    pngImage pi = pngGetImage(pc);
    printf("\nw: %d, h: %d\n", pi->w, pi->h);
    
    //stampaBit(data->data, 10);
}

void disegna(){

}