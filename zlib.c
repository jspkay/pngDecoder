//
// Created by salvo on 08/12/19.
//

#include "zlib.h"
#include "huffmanTree.h"
#include <stdlib.h>

#define END_OF_BLOCK 0x100

zlib_CMF zlib_getCM(zlib_data data){ //size of the array is always 3
    zlib_CMF res = malloc(sizeof(struct zlib_CMF_st));
    res->CMF = data->data[0];
    res->CM = res->CMF & (unsigned short) 0x0f; //CMF
    res->CINFO = (res->CMF & (unsigned) 0xf0) >> (unsigned) 4; //CINFO
    return res;
}
zlib_FLG zlib_getFLG(zlib_data data){
    zlib_FLG res = malloc(sizeof(struct zlib_FLG_st));
    res->FLG = data->data[1];
    res->FCHECK = res->FLG & (unsigned short)0x1f;
    res->FDICT = (res->FLG & (unsigned)0x20) >> (unsigned) 5;
    res->FLEVEL = (res->FLG & (unsigned)0xc0) >> (unsigned) 6;
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
void zlib_freeCM(zlib_data data){free(data);}
void zlib_freeFLG(zlib_data data){free(data);}
void zlib_freeDICTID(zlib_data data){free(data);}

unsigned short int __nextByte(zlib_data *data, int *bc, int *i, int *zdc){
    short int r;
    if(*bc == 0) { //Se sono all'inizio di un byte, prendo quello
        r = data[*zdc]->data[*i];
        (*i)++;
        if(*i>=data[*zdc]->l) {
            *i = 0;
            (*zdc)++;
        }
    }
    else{ //se sono nel mezzo, prendo il prossimo
        *bc = 0;
        (*i)++;
        if(*i>=data[*zdc]->l) {
            *i = 0;
            (*zdc)++;
        }
        r = data[*zdc]->data[*i];
    }
    return r;
}
unsigned short int __nextBit(zlib_data *data, int *bc, int *i, int *zdc){
    unsigned short int r = (data[*zdc]->data[*i] & ((unsigned)0x1 << (unsigned) *bc)) >> (unsigned) *bc;
    (*bc)++;
    if(*bc >= 8){
        *bc = 0;
        (*i)++;
        if(*i >= data[*zdc]->l){
            *i = 0;
            (*zdc)++;
        }
    }
    return (signed) r;
}
unsigned short int __getExtraBits(int extraBits, zlib_data *data, int*bc, int*i, int*zdc){
    unsigned short int res=0x0;
    for(int j=0; j<extraBits; j++){
        res <<= (unsigned) 1;
        res += __nextBit(data, bc, i, zdc);
    }
    return res;
}
int __twoPower(int exp){
    unsigned int res = 1;
    for(int i=0; i<exp; i++){
        res <<= (unsigned) 0x1;
    }
    return (signed) res;
}
int __addOutputStreamByte(unsigned char byte, zlib_data res, int resAlloc, int pace){
    if(res->l >= resAlloc){
        resAlloc += pace;
        res->data = realloc(res->data, resAlloc * sizeof(struct zlib_data_st));
    }
    res->data[res->l++] = byte;
    return resAlloc;
}
unsigned short int __getHuffmanValue(huff_tree hf, zlib_data *data, int*bc, int*i, int*zdc){
    unsigned short int res;
    huff_tree node;
    node = hf;
    unsigned short int bit;
    while(1){
        bit = __nextBit(data, bc, i, zdc);
        node = huff_getNextNode(node, (char) bit);
        if(huff_getValue(node)!=-1 ) break;
    }
    res = huff_getValue(node);
    return res;
}
zlib_data zlib_deflate(zlib_data *data, int n, int *newN){
    int *bc, *i, *zdc; // bc-> bit counter, i->byte counter, zdc->zlib_data counter
    bc = malloc(sizeof(short int)); *bc=0;
    i = malloc(sizeof(short int)); *i=0;
    zdc = malloc(sizeof(short int)); *zdc=0;
    *newN = n;
    zlib_data res = malloc(sizeof(struct zlib_data_st));
    res->data = malloc(data[0]->l*n * sizeof(byte));
    res->l = 0;
    int resAlloc=n;
    
    if(zlib_getFLG(data[0])->FDICT == 1) *i=6;
    else *i=2;
    
    /*
     * L'idea è procedere bit per bit su uno stream di byte.
     * Per fare questo uso i per avanzare nell'array dei byte
     * e bc per tenere conto del bit che sto guardando.
     * Problema: bc può arrivare solo fino a 7, quindi ogni volta
     * che lo incremento devo controllare se è maggiore di 7 ed in
     * quel caso incrementare i.
     * Per cui ho implementato __nextBit
     */
    
    unsigned short int lastBlock = 0; // BFINAL
    unsigned short int type; // BTYPE
    do{
        lastBlock = __nextBit(data, bc, i, zdc); // Get first bit
        type = 0; // Get the 2nd and 3rd bit
        for(int j=0; j<2; j++){
            unsigned short int read = __nextBit(data, bc, i, zdc);
            read <<= (unsigned) j;
            type += read;
        }
        
        // Looking at the type
        if(type == 0b00){
            __nextByte(data, bc,  i, zdc);
            unsigned short int LEN = 0x0, NLEN = 0x0;
            for(short int j=0; j<2; j++){
                short unsigned int read = __nextByte(data, bc, i, zdc);
                LEN += read << (unsigned short) (8 * j);
            }
            for(unsigned short int j=0; j<2; j++){
                unsigned short int read = __nextByte(data, bc, i, zdc);
                NLEN += read << (unsigned short) (8*j);
            }
            
            // checking errors
            unsigned short int check = 0xFFFF;
            if(check-LEN != NLEN){
                printf("Error in file.\n");
                exit(-1);
            }
            
            for(int j=0; j<LEN; j++){
                char byte = __nextByte(data, bc, i, zdc);
                resAlloc = __addOutputStreamByte(byte, res, resAlloc, LEN);
            }
        }else{ // Huffman codes
            huff_tree hf;
            if(type == 0b10){  // dinamic huffman code
                printf("To be implemented!\n");
                exit(0);
                
                // Reading HLIT (5 bits)
                unsigned short int value=0;
                for(int j=0; j<5; j++){
                    value <<= (unsigned) 1;
                    value += __nextBit(data, bc, i, zdc);
                }
                printf("HLIT: %u\n", value);
                
                // Reading HDIST (5 bits)
                value = 0;
                for(int j=0; j<5; j++){
                    value <<= (unsigned) 1;
                    value += __nextBit(data, bc, i, zdc);
                }
                printf("HDIST: %u\n", value);
                
                // Reading HCLEN (4 bits)
                value = 0;
                for(int j=0; j<4; j++){
                    value <<= (unsigned) 1;
                    value += __nextBit(data, bc, i, zdc);
                }
                printf("HCLEN: %u\n", value);
            }
            else{ //type == 0b01
                short int lenghts[4] = {8, 9, 7, 8};
                hf = huff_generateTree("0-143 144-255 256-279 280-287", lenghts);
            }
            
            // Decoding data with huffman decoding
            unsigned short int read;
            do{
                // read the byte
                read = __getHuffmanValue(hf, data, bc, i, zdc);
                if(read < 256){
                    if(res->l >= resAlloc){
                        resAlloc += n;
                        res->data = realloc(res->data, resAlloc * sizeof(struct zlib_data_st));
                    }
                    res->data[(res->l)++] = read;
                }
                else if(read > 256){
                    int length, distance;
                    switch(read){
                        default:
                        case 257 ... 264:
                            length = read-254;
                            break;
                        case 265 ... 268:
                            length = 11 + 2 * (read - 265);
                            length += __getExtraBits(1, data, bc, i, zdc);
                            break;
                        case 269 ... 272:
                            length = 19 + 4 * (read - 269);
                            length += __getExtraBits(2, data, bc, i, zdc);
                            break;
                        case 273 ... 280:
                            length = 35 + 8 * (read - 273);
                            length += __getExtraBits(3, data, bc, i, zdc);
                            break;
                        case 281 ... 284:
                            length = 131 + 16 * (read - 281);
                            length += __getExtraBits(4, data, bc, i, zdc);
                            break;
                        case 285:
                            length = 258;
                            break;
                    }
                    distance = 0x0;
                    for(int j=0; j<5; j++){
                        unsigned short int r = __nextBit(data, bc, i, zdc);
                        r <<= (unsigned) j;
                        distance += r;
                    }
                    int eb = 0;
                    if(distance < 4) {
                        distance += 1;
                    }
                    else{
                        int last = 4; // last value of distance
                        int r = 4, i1=1, i2=0; // r == read value as distance
                        while(r < distance){
                            if(i2 == 2){
                                i1++;
                                i2 = 0;
                            }
                            last += __twoPower(i1);
                            i2++;
                            r++;
                        }
                        distance = last+1;
                    }
                    distance += __getExtraBits(eb, data, bc, i, zdc);
                    
                    
                    unsigned char *startingPoint = res->data + res->l - distance;
                    for(int j=0; j<length; j++){
                        unsigned char byte = *startingPoint;
                        resAlloc = __addOutputStreamByte(byte, res, resAlloc, length);
                        startingPoint++;
                    }
                    
                    printf("Data output: %d, L: %d, D: %d\n", res->l, length, distance);
                    fflush(stdout);
                }
                
            }while(read != END_OF_BLOCK);
        }
    
    }while(!lastBlock);
    
    return res;
}