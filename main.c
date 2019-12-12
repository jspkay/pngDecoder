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

int main() {
    huffman();
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
    short int lenghts[4] = {8, 9, 7, 8};
    huff_generateTree("0-143 144-255 256-279 280-287", lenghts);
}

void png(){
    pngInitialize();
    pngID p1 = pngOpenFile("/home/salvo/Immagini/vlcsnap-2017-09-09-15h01m19s799.png", "r"),
        p2 = pngOpenFile("/home/salvo/Immagini/test/2.png", "r");
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
    for(int i=0; i<q; i++){
        pngPrintChunk(IDAT[i], stdout);
    }
    
    zlib_data data = malloc(sizeof(struct zlib_data_st));
    data->l = IDAT[0]->length;
    data->data = IDAT[0]->data;
    zlib_CMF CMF= zlib_getCM(data);
    zlib_FLG FLG = zlib_getFLG(data);
    printf("CMF --> CM: %x, CINFO: %x\n", CMF->CM, CMF->CINFO);
    printf("FLG --> FCHECK: %x, FDICT: %x, FLEVEL: %x\n", FLG->FCHECK, FLG->FDICT, FLG->FLEVEL);
    pngImage pi = pngGetImage(pc);
    printf("w: %d, h: %d\n", pi->w, pi->h);
    zlib_deflate(data, 0);
    //stampaBit(data->data, 10);
}

void stampaBit(unsigned char *ptr, int len){
    unsigned char stringaBit[10000];
    int k=0;
    for(int i=0; i<len; i++) {
        unsigned char byte = *ptr;
        printf("%.1x: ", byte);
        for(int j=0; j<8; j++) {
            printf("%d ", byte % 2);
            stringaBit[k++] = ((byte & (unsigned char)128) >> 7) + (unsigned char) '0';
            byte >>= 1;
        }
        printf("\n");
        ptr++;
    }
    
    //CERCO IL COMPLEMENTO
    stringaBit[k] = '\0';
    unsigned char byte[9], car; byte[8] = '\0';
    //strcpy(stringaBit, "000111000111000110001010011101000000011100011100011000101001110100000001110001110001100010100111010000");
    unsigned char *c, *b;
    int i,j;
    for(i=0; i<k-7; i++){
        c = stringaBit+i;
        for(j=0; j<8; j++){
            car = *(c+j);
            if(car == '0') byte[j] = '1';
            else byte[j] = '0';
        }
        b = c+8; int flag=1;
        for(j=0; j<8 && flag; j++, b++){
            if(*b != byte[j]) flag = 0;
        }
        if(flag){
            char str[9]; str[8] = '\0';
            for(j=0; j<8; j++)
                str[j] = *(c++);
            printf("TROVATO: %d(%d,%d), %s - %s\n", i, i/8, i%8, str, byte);
        }
    }
    //printf("\n\n\n%s - %s", stringaBit, byte);
}
