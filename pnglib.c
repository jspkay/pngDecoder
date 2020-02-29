#include "pnglib.h"
#include "zlib.h"
#include "filterAlgorithms.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//#define DEBUG
#define BYTE_SIZE 8

struct pngFile_st{
    FILE *fp;
    int l;
};
static int maxPngFiles = INITLENGTH;
static int lastPngFileID;
static pngFile *IDTable;

void pngInitialize(){
    IDTable = malloc(maxPngFiles * sizeof(pngFile));
    lastPngFileID = 0;;
}
void pngClose(){
    for(int i=0; i<lastPngFileID; i++)
        pngCloseFile(i);
    free(IDTable);
}

pngID pngOpenFile(char *fileName, char *mode){
    if(lastPngFileID >= maxPngFiles){
        lastPngFileID += 2;
        IDTable = realloc(IDTable, lastPngFileID*sizeof(pngFile));
    }
    FILE *fp = fopen(fileName, mode);
    if(fp == NULL) return -1;
    fflush(stdout);
    pngFile pf = malloc(sizeof(struct pngFile_st));
    pf->fp = fp;
    pf->l = lastPngFileID;
    IDTable[lastPngFileID] = pf;
    return lastPngFileID++;
}
bool pngCloseFile(int pf){
    pngFile p = IDTable[pf];
    if(p == NULL) return false;
    free(p);
    for(int i=pf; i<lastPngFileID-1; i++){
        IDTable[i] = IDTable[i+1];
    }
    lastPngFileID--;
    return true;
}
bool pngVerifyType(int pf){
    if(pf < 0){
        return 0;
    }
    pngFile p = IDTable[pf];
    rewind(p->fp);
    unsigned char c;
    unsigned char magicBytes[8] = {
        0x89, 0x50, 0x4e, 0x47,
        0xd, 0xa, 0x1a, 0xa
    };
    for(int i=0; i<8; i++){
        c = (unsigned char) fgetc(p->fp);
        if(c != magicBytes[i])
            return 0;
    }
    return 1;
}

static bool _pngEndChunk(pngChunk pc){
    unsigned char endBytes[4] = {0x49, 0x45, 0x4e, 0x44};
    unsigned int endBytesInt=0;
    for(int i=0; i<4; i++){
        endBytesInt += endBytes[i] << ((3-(unsigned) i) * 8);
    }
    return endBytesInt == pc->type;
}
static unsigned int _readByte(FILE *fp){
    unsigned int res = 0;
    unsigned char byte;
    for(unsigned int i=0; i<4; i++){
        byte = (unsigned char) fgetc(fp);
        res += byte << ((3-i)*8);
    }
    return res;
}
static unsigned char* _readData(FILE *fp, unsigned int length){
    unsigned char *res = malloc(length * sizeof(unsigned char));
    for(int i=0; i<length; i++){
            res[i] = (unsigned  char) fgetc(fp);
    }
    return res;
}

void pngPrintChunk(pngChunk pc, FILE *output){
    fprintf(output, "CHUNK:\n");
    fprintf(output, "LENGHT: %u\n", pc->length);
    fprintf(output, "TYPE: ");
    for(int i=0; i<4; i++){
        printf( "%c", (pc->type >> ((3-(unsigned)i)*8)) );
    }
    fprintf(output, "\n");
    fprintf(output, "CRC: %x\n", pc->CRC);
}
pngFileChunk pngReadChunks(int file){
    pngFile p = IDTable[file];
    rewind(p->fp);
    if(!pngVerifyType(file))
        return NULL;

    int maxc=3, lastc=0;
    pngChunk *chunks = malloc(maxc * sizeof(pngChunk));
    pngFileChunk res = malloc(sizeof(struct pngFileChunk_st));
    pngChunk pc;
    do{
        pc = malloc(sizeof(struct pngChunk_st));
        pc->length = 0;
        pc->type = 0;
        pc->CRC = 0;

        pc->length = _readByte(p->fp);
        pc->type = _readByte(p->fp);
        pc->data = _readData(p->fp, pc->length);
        pc->CRC = _readByte(p->fp);
        
        if(lastc >= maxc){
            maxc += 3;
            chunks = realloc(chunks, maxc * sizeof(pngChunk));
        }
        chunks[lastc++] = pc;
    }while( !_pngEndChunk(pc) );
    res->n = lastc;
    res->chunks = chunks;
    return res;
}
void pngFreeChunks(pngFileChunk pfc){
    for(int i=0; i<pfc->n; i++){
        free(pfc->chunks[i]->data);
        free(pfc->chunks[i]);
    }
    free(pfc->chunks);
    free(pfc);
}

pixel** pngFilter(unsigned char* rawImage, pngImage image){
    if(image->interlaceMethod!=0){
        printf("Interlaced! Cannot process!");
        exit(-1);
    }
    
    pixel **res = malloc(image->h * sizeof(pixel));
    for(int i=0; i<image->h; i++)
        res[i] = malloc(image->w * sizeof(struct pixel_st));
    
    // pixel[i][j] stands for i-th row j-th column
    
    void (*type0)(params), (*type1)(params), (*type2)(params), (*type3)(params), (*type4)(params);
    struct params p = {
            NULL,
            rawImage,
            NULL,
            0,
            image->w,
            0
    };

    switch(image->colorType) {
        case 0 ... 5:
            printf("Not supported.");
            exit(0);
        case 6:
            if(image->bitDepth != 8) {
                printf("Not supported.");
                exit(0);
            }
            p.bpp = 4;
            type0 = __noneFilteringColorsAlpha8bit;
            type1 = __subFilteringColorsAlpha8bit;
            type2 = __upFilteringColorsAlpha8bit;
            type3 = __averageFilteringColorsAlpha8bit;
            type4 = __paethFilteringColorsAlpha8bit;
    }

    /*
    switch(image->colorType){
        case 0: // grayScale (1 or 2 bytes)
            switch(image->bitDepth){
                case 1:
                case 2:
                case 4:
                case 8:
                    p.bpp = 1;
                    type0 = __noneFilteringGrayscale;
                    type1 = __subFilteringGrayscale;
                    type2 = __upFilteringGrayscale;
                    type3 = __averageFilteringGrayscale;
                    type4 = __paethFilteringGrayscale;
                    break;
                case 16:
                    p.bpp = 2;
                    type0 = __noneFilteringGrayscale16bit;
                    type1 = __subFilteringGrayscale16bit;
                    type2 = __upFilteringGrayscale16bit;
                    type3 = __averageFilteringGrayscale16bit;
                    type4 = __paethFilteringGrayscale16bit;
            }
            break;
        case 2: // rgb triple
            if(image->bitDepth == 8){
                p.bpp = 3;
                type0 = __noneFilteringColors8bit;
                type1 = __subFilteringColors8bit;
                type2 = __upFilteringColors8bit;
                type3 = __averageFilteringColors8bit;
                type4 = __paethFilteringColors8bit;
            }
            else{ //MSB first
                p.bpp = 6;
                type0 = __noneFilteringColors16bit;
                type1 = __subFilteringColors16bit;
                type2 = __upFilteringColors16bit;
                type3 = __averageFilteringColors16bit;
                type4 = __paethFilteringColors16bit;
            }
            break;
        case 3: //palette
            printf("Not implemented yet");
            exit(-1);
            p.bpp = 1;
            break;
        case 4: // grayscale + alpha sample
            if(image->bitDepth == 8){
                p.bpp = 2;
                type0 = __noneFilteringGrayscaleAlpha8bit;
                type1 = __subFilteringGrayscaleAlpha8bit;
                type2 = __upFilteringGrayscaleAlpha8bit;
                type3 = __averageFilteringGrayscaleAlpha8bit;
                type4 = __paethFilteringGrayscaleAlpha8bit;
            }
            else{
                p.bpp = 4;
                type0 = __noneFilteringGrayscaleAlpha8bit;
                type1 = __subFilteringGrayscaleAlpha8bit;
                type2 = __upFilteringGrayscaleAlpha8bit;
                type3 = __averageFilteringGrayscaleAlpha8bit;
                type4 = __paethFilteringGrayscaleAlpha8bit;
            }
            break;
        case 6: // rgb triple + alpha sample
            if(image->bitDepth == 8) {
                p.bpp = 4;
                type0 = __noneFilteringColorsAlpha8bit;
                type1 = __subFilteringColorsAlpha8bit;
                type2 = __upFilteringColorsAlpha8bit;
                type3 = __averageFilteringColorsAlpha8bit;
                type4 = __paethFilteringColorsAlpha8bit;
            }
            else{
                p.bpp = 8;
                type0 = __noneFilteringColorsAlpha16bit;
                type1 = __subFilteringColorsAlpha16bit;
                type2 = __upFilteringColorsAlpha16bit;
                type3 = __averageFilteringColorsAlpha16bit;
                type4 = __paethFilteringColorsAlpha16bit;
            }
            break;
    }*/

    printf(" colorType:%d - bitDepth:%d\n", image->colorType, image->bitDepth);
    for(int i=0; i<image->h; i++){ // For each scanline (row)
        // The first byte is the type
        int type = rawImage[i * (image->w*p.bpp+1) ]; //every row has image->w*bpp + 1 bytes

        p.line = res[i];
        p.i = i;
        if(i>0) p.upline = res[i-1];

        #ifdef DEBUG
        printf("line: %4d - type:%d\n", i, type);
        #endif

        switch(type){
            case 0:
                (*type0)(&p);
                break;
            case 1:
                (*type1)(&p);
                break;
            case 2:
                (*type2)(&p);
                break;
            case 3:
                (*type3)(&p);
                break;
            case 4:
                (*type4)(&p);
                break;
        }
        
    }
    
    return res;
}

pngChunk *pngGetIDATChunks(pngFileChunk pc, int *qty){
    unsigned int id=0;
    unsigned char name[4] = {'T', 'A', 'D', 'I'};
    for(unsigned int i=0; i<4; i++) id += name[i] << (i*8);
    
    *qty = 0;
    int max = 2;
    pngChunk *res = malloc(max * sizeof(pngChunk));
    
    int n = pc->n;
    int start;
    for(start=0; pc->chunks[start]->type != id; start++); // Cerco il primo
    for(int i=start; i<n && pc->chunks[i]->type == id; i++) { //inserisco gli altri
        if(*qty >= max){ //se sbordo rialloco
            max *= 2;
            res = realloc(res, max * sizeof(pngChunk));
        }
        res[ (*qty)++ ] = pc->chunks[i];
    }
    res = realloc(res, (*qty) * sizeof(pngChunk)); // rialloco: non spreco memoria inutilmente (*qty < max)
    return res;
}
pngImage pngGetImage(pngFileChunk pc){
    pngImage res = malloc(sizeof(struct pngImage_st));
    
    pngChunk hdr = pc->chunks[0];
    //reading image attributes
    res->h = res->w = 0;
    for(unsigned int i=0; i<4; i++){
        res->w += hdr->data[i] << ((3-i) * 8);
        res->h += hdr->data[i+4] << ((3-i) * 8);
    }
    res->bitDepth = hdr->data[8];
    res->colorType = hdr->data[9];
    res->compressioneMethod = hdr->data[10];
    res->filterMethod = hdr->data[11];
    res->interlaceMethod = hdr->data[12];
    
    int idatN; //Getting pngChunk array of IDAT chunks
    pngChunk *idat = pngGetIDATChunks(pc, &idatN);
    zlib_data *compressedImage = malloc(idatN * sizeof(zlib_data));
    //Generating zlib data stream to be decompressed
    for(int i=0; i<idatN; i++){
        compressedImage[i] = malloc(sizeof(struct zlib_data_st));
        compressedImage[i]->l = idat[i]->length;
        compressedImage[i]->data = idat[i]->data;
    }
    int bytes;
    
    FILE *fp = fopen("../infgen/deflateOut", "w");
    for(int j=0; j<idatN; j++) {
        for(int i = 0; i<compressedImage[j]->l; i++) {
            fprintf(fp, "%c", compressedImage[j]->data[i]);
        }
    }
    fclose(fp);
    
    zlib_data rawImage = zlib_inflate(compressedImage, idatN, &bytes);
    free(compressedImage);
    
    /*
    for(int i = 0; i<rawImage->l; i++) {
        printf("%d ", rawImage->data[i]);
    }*/
    
    // filtering
    res->image = pngFilter(rawImage->data, res);
    zlib_freeData(rawImage);
    
    // composing pixels
    
    return res;
}