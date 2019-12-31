//
// Created by salvo on 08/12/19.
//

#ifndef PNGLIB_ZLIB_H
#define PNGLIB_ZLIB_H

#include <malloc.h>

typedef unsigned char byte;
typedef struct zlib_data_st{
    unsigned int l;
    byte* data;
} *zlib_data;
typedef struct zlib_CMF_st{
    unsigned short int CMF,CM,CINFO;
} *zlib_CMF;
typedef struct zlib_FLG_st{
    unsigned short int FLG, FCHECK, FDICT, FLEVEL;
} *zlib_FLG;

zlib_CMF zlib_getCM(zlib_data data);
zlib_FLG zlib_getFLG(zlib_data data);
zlib_data zlib_deflate(zlib_data *data, int n, int *newN); //data is the array of data, n is the length of this array, newN is the length of the array returned array
int zlib_getDICTID(zlib_data data);

#endif //PNGLIB_ZLIB_H


