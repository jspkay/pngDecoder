#include <stdio.h>
#include <stdlib.h>
#include "pnglib.h"
#include "zlib.h"
#include "huffmanTree.h"
#include "UI.h"


int main(int argc, char **argv){
    pngInitialize();

    if(argc < 2){
        printf("Usage: %s file1.png [file2.png [other files]] "
               "\nEach image is displayed after the previous is closed", argv[0]);
    }

    /*
    pngID p1 = pngOpenFile("../img/img-white10x10.png", "r"),
            //p2 = pngOpenFile("/home/salvo/Immagini/Schermata da 2020-02-23 13-06-11.png", "r");
            //p2 = pngOpenFile("../img/wallpaperHuffman.png", "r");
            p2 = pngOpenFile("/home/salvo/Scaricati/Marvel-Transparent.png", "r");
            //p2 = pngOpenFile("/home/salvo/Scaricati/img1.png", "r");
            */

    for(int i=1; i<argc; i++) {
        pngID p1 = pngOpenFile(argv[i], "r");
        printf("%s is valid png: %s", argv[i], pngVerifyType(p1) ? "true" : "false");

        if (p1<0) {
            printf("Error in reading image!\n");
            exit(0);
        }
        pngFileChunk pc = pngReadChunks(p1);
        pngCloseFile(p1);

        pngChunk *IDAT;
        int q;
        IDAT = pngGetIDATChunks(pc, &q);
        pngImage pi = pngGetImage(pc);

        printf(" w: %d, h: %d\n", argv[i], pi->w, pi->h);
        UI_displayImage(pi);
    }
}

void huffman(){
    /*
    short int lenghts[4] = {8, 9, 7, 8};
    huff_tree hf = huff_generateTree("0-143 144-255 256-279 280-287", lenghts);
    */
    short int lengths[5] = {1, 2, 0, 2};
    huff_tree hf = huff_generateTree("1 2 3-17 18", lengths);

}