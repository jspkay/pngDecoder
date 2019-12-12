#include "pnglib.h"

#include <stdio.h>
#include <stdlib.h>

//#define DEBUG

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

pngID pngOpenFile(char *fileName, char *mode){
    if(lastPngFileID >= maxPngFiles){
        lastPngFileID += 2;
        IDTable = realloc(IDTable, lastPngFileID*sizeof(pngFile));
    }
    FILE *fp = fopen(fileName, mode);
    void *ptr = NULL;
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
        endBytesInt += endBytes[i] << ((3-i) * 8);
    }
    return endBytesInt == pc->type;
}
static unsigned int _readByte(FILE *fp){
    unsigned int res = 0;
    unsigned char byte;
    for(int i=0; i<4; i++){
        byte = (unsigned char) fgetc(fp);
        res += byte << ((3-i)*8);
    }
    return res;
}
static unsigned char* _readData(FILE *fp, int lenght){
    unsigned char *res = malloc(lenght * sizeof(unsigned char));
    for(int i=0; i<lenght; i++){
            res[i] = (unsigned  char) fgetc(fp);
    }
    return res;
}

void pngPrintChunk(pngChunk pc, FILE *output){
    fprintf(output, "CHUNK:\n");
    fprintf(output, "LENGHT: %u\n", pc->length);
    fprintf(output, "TYPE: ");
    for(int i=0; i<4; i++){
        printf( "%c", (pc->type >> ((3-i)*8)) );
    }
    fprintf(output, "\n");
    fprintf(output, "CRC: %x\n", pc->CRC);
}
pngFileChunk pngReadChunks(int file){
    pngFile p = IDTable[file];
    rewind(p->fp);
    if( !pngVerifyType(file) )return NULL;

    int maxc=3, lastc=0;
    pngChunk *chunks = malloc(maxc * sizeof(pngChunk));
    pngFileChunk res = malloc(sizeof(struct pngFileChunk_st));
    pngChunk pc;
    char byte;
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

pngChunk *pngGetIDATChunks(pngFileChunk pc, int *qty){
    unsigned int id=0;
    unsigned char name[4] = {'T', 'A', 'D', 'I'};
    for(int i=0; i<4; i++) id += name[i] << (i*8);
    
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
    res->h = res->w = 0;
    for(int i=0; i<4; i++){
        res->w += hdr->data[i] << ((3-i) * 8);
        res->h += hdr->data[i+4] << ((3-i) * 8);
    }
    
    return res;
}