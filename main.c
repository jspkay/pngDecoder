#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pnglib.h"
#include "zlib.h"
#include "huffmanTree.h"
#include "UI.h"


int main(int argc, char **argv){
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
    pngFileChunk pc = pngReadChunks(p2);
    pngCloseFile(p1); pngCloseFile(p2);

    pngChunk *IDAT; int q;
    IDAT = pngGetIDATChunks(pc, &q);
    /*for(int i=0; i<q; i++){
        pngPrintChunk(IDAT[i], stdout);
    }*/

    zlib_data data = malloc(sizeof(struct zlib_data_st));
    data->l = IDAT[0]->length;
    data->data = IDAT[0]->data;
    zlib_CMF CMF = zlib_getCM(data);
    zlib_FLG FLG = zlib_getFLG(data);
    printf("CMF --> CM: %x, CINFO: %x\n", CMF->CM, CMF->CINFO);
    printf("FLG --> FCHECK: %x, FDICT: %x, FLEVEL: %x\n", FLG->FCHECK, FLG->FDICT, FLG->FLEVEL);
    pngImage pi = pngGetImage(pc);

    UI_displayImage(pi);

    printf("\nw: %d, h: %d\n", pi->w, pi->h);
}

void huffman(){
    /*
    short int lenghts[4] = {8, 9, 7, 8};
    huff_tree hf = huff_generateTree("0-143 144-255 256-279 280-287", lenghts);
    */
    short int lengths[5] = {1, 2, 0, 2};
    huff_tree hf = huff_generateTree("1 2 3-17 18", lengths);

}