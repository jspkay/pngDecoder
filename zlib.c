//
// Created by salvo on 08/12/19.
//

#include "zlib.h"
#include "huffmanTree.h"

zlib_CMF zlib_getCM(zlib_data data){ //size of the array is always 3
    zlib_CMF res = malloc(sizeof(struct zlib_CMF_st));
    res->CMF = data->data[0];
    res->CM = res->CMF & (unsigned short) 0x0f; //CMF
    res->CINFO = (res->CMF & (unsigned short) 0xf0) >> (unsigned) 4; //CINFO
    return res;
}
zlib_FLG zlib_getFLG(zlib_data data){
    zlib_FLG res = malloc(sizeof(struct zlib_FLG_st));
    res->FLG = data->data[1];
    res->FCHECK = res->FLG & (unsigned short)0x1f;
    res->FDICT = (res->FLG & (unsigned short)0x20) >> (unsigned) 5;
    res->FLEVEL = (res->FLG & (unsigned short)0xc0) >> (unsigned) 6;
    return res;
}
int zlib_getDICTID(zlib_data data){
    zlib_FLG  f = zlib_getFLG(data);
    if(f->FDICT == 0) return 0;
    int res = 0;
    for(int i=0; i<4; i++){
        res += data->data[i+2] << (unsigned) (i*8);
    }
    return res;
}


zlib_data zlib_deflate(zlib_data data, unsigned short int startingBit){
    for(int j=0; j<2; j++) {
        unsigned short int b = data->data[2+j];
        for(int i = 0; i<8; i++) {
            printf("%d ", b & (unsigned) 1);
            b >>= 1;
        }
        printf("\n");
    }
    
    /*
     * ------  BF  BTYPE  Data
     * OUTPUT: 0   0 1    1 0 1 1 1
     *
     *         1 0 0 0 0 0 1 1
     * Secondo le specifiche (RFC 1951) i dati sono memorizzati
     * in machine order (LSB -> MSB) (infatti è così che vengono stampati)
     * e sono "pacchettizzati" per byte!
     */
    
    short int lenghts[4] = {8, 9, 7, 8};
    huff_tree hf = huff_generateTree("0-143 144-255 256-279 280-287", lenghts);
    
    
    
    printf("\n");
}