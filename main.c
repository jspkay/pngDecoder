#include <stdio.h>
#include <stdlib.h>
#include "pnglib.h"
#include "zlib.h"
#include "huffmanTree.h"
#include "UI.h"

pixel ** convolution(pixel **image, signed short int matrix[][3], int w, int h);

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
    /*        */

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

        int a = 4;
        // signed short int convolutionMatrix[3][3] = { {0, -1, 0}, {-1, 5, -1}, {0, -1, 0} }; //sharpener
      //  signed short int convolutionMatrix[3][3] = { {1, 3, 1}, {3, 5, 3}, {1, 3, 1} }; //blur
       // pi->image = convolution(pi->image, convolutionMatrix, pi->w, pi->h);

        printf(" w: %u, h: %u\n", pi->w, pi->h);
        UI_displayImage(pi);
    }
}

pixel ** convolution(pixel **image, signed short int matrix[][3], int w, int h){
    pixel **res = malloc(h * sizeof(pixel*));
    for(int i=0; i<h; i++) res[i] = malloc(w*sizeof(pixel));

    for(int i=0; i<h; i++){ // each column
        for(int j=0; j<w; j++){ // each row

            int sum[3] = {0};
            for(int mc=0; mc<3; mc++){
                for(int mr=0; mr<3; mr++){
                    int l=i-1+mc, p=j+mr-1; // l->column, p->row
                    if(l<0 || l>=h || p<0 || p>=w) continue;
                    sum[0] += image[l][p].r * matrix[mr][mc]/10;
                    sum[1] += image[l][p].g * matrix[mr][mc]/10;
                    sum[2] += image[l][p].b * matrix[mr][mc]/10;
                }
            }

            res[i][j].r = sum[0] % 256;
            res[i][j].g = sum[1] % 256;
            res[i][j].b = sum[2] % 256;
            res[i][j].a = image[i][j].a;

        }
    }

    for(int i=0; i<h; i++) free(image[i]);
    free(image);

    return res;
}


void huffman(){
    /*
    short int lenghts[4] = {8, 9, 7, 8};
    huff_tree hf = huff_generateTree("0-143 144-255 256-279 280-287", lenghts);
    */
    short int lengths[5] = {1, 2, 0, 2};
    huff_tree hf = huff_generateTree("1 2 3-17 18", lengths);

}