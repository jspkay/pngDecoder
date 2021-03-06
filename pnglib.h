#ifndef PNGLIB_H
#define PNGLIB_H

#include <stdio.h>

#define INITLENGTH 2

typedef enum{false, true} bool;

typedef struct pngChunk_st {
    unsigned int length; //4Bytes   <-¬
    unsigned int type; //4 Bytes       |
    unsigned int CRC; //4 Bytes        |
    unsigned char *data; //lenght bytes ---------
} *pngChunk;
typedef struct pngFileChunk_st {
    int n;
    pngChunk *chunks;
} *pngFileChunk;
typedef int pngID;
typedef struct pngFile_st *pngFile;

typedef struct pixel_st{
    unsigned short int r, g, b, a; //values between 0 and 255
} pixel;
typedef struct pngImage_st{
    unsigned int h, w;
    unsigned char bitDepth, colorType, compressioneMethod,
        filterMethod, interlaceMethod;
    pixel **image;
} *pngImage;

void pngInitialize();
pngID pngOpenFile(char *fileName, char *mode);
bool pngCloseFile(int pf);
bool pngVerifyType(int pf);
pngFileChunk pngReadChunks(int file);
void pngPrintChunk(pngChunk pc, FILE *output);
pngChunk *pngGetIDATChunks(pngFileChunk pc, int *qty);

pngImage pngGetImage(pngFileChunk pc);

#endif