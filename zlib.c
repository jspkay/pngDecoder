//
// Created by salvo on 08/12/19.
//

#include "zlib.h"
#include "huffmanTree.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

#define END_OF_BLOCK 0x100
#define CHUNK 16384

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

unsigned char __nextByte(zlib_data *data, int *bc, int *i, int *zdc){
    unsigned char r;
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
unsigned short int __getExtraBitsML(int extraBits, zlib_data *data, int*bc, int*i, int*zdc){
    unsigned short int res=0x0;
    for(int j=0; j<extraBits; j++){
        res <<= (unsigned) 1;
        res += __nextBit(data, bc, i, zdc);
    }
    return res;
}
unsigned short int __getExtraBitsLM(int extraBits, zlib_data *data, int*bc, int*i, int*zdc){
    unsigned short int res=0x0;
    for(int j=0; j<extraBits; j++){
        unsigned short int v = __nextBit(data, bc, i, zdc);
        v <<= (unsigned) j;
        res += v;
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
    res->data[(res->l)++] = byte;
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
huff_tree __readAndGenerateDynamicHuffmanTree(huff_tree *distanceHf, zlib_data *data, int*bc, int*i, int*zdc){
    unsigned short int HLIT, HDIST, HCLEN;
    
    // Reading HLIT (5 bits)
    HLIT = 0;
    for(int j=0; j<5; j++){
        unsigned int r = __nextBit(data, bc, i, zdc);
        r <<= (unsigned) j;
        HLIT += r;
    }
    
    // Reading HDIST (5 bits)
    HDIST = 0;
    for(int j=0; j<5; j++){
        unsigned int r = __nextBit(data, bc, i, zdc);
        r <<= (unsigned) j;
        HDIST += r;
    }
    
    // Reading HCLEN (4 bits)
    HCLEN = 0;
    for(int j=0; j<4; j++){
        unsigned int r = __nextBit(data, bc, i, zdc);
        r <<= (unsigned) j;
        HCLEN += r;
    }
    
    unsigned short int huffmanCodeLenghts[19] = {0};
    unsigned char readingOrder[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
                                            11, 4, 12, 3, 13, 2, 14, 1, 15};
    
    unsigned short int j;
    for(j=0; j<(HCLEN+4); j++){
        unsigned short int k, value=0;
        unsigned short r=0;
        for(k=0; k<3; k++){
            r = __nextBit(data, bc, i, zdc);
            r <<= k;
            value += r;
        }
        huffmanCodeLenghts[readingOrder[j]] = value;
        ////printf("code length %2d: %d\n", readingOrder[j], value);
    }
    
    // Generating huffman tree to read code lengths sequences
    unsigned short int *lenghts = malloc(19 * sizeof(unsigned short int));
    int lenghtsIndex = 0;
    char codeAlphabet[1000] = {0};
    char *string = codeAlphabet;
    for(j=0; j<19; j++){
        if(huffmanCodeLenghts[j] != 0) {
            sprintf(string, "%d ", j);
            string = codeAlphabet + strlen(codeAlphabet);
            lenghts[lenghtsIndex++] = huffmanCodeLenghts[j];
        }
    }
    codeAlphabet[strlen(codeAlphabet)-1] = '\0';
    huff_tree hf = huff_generateTree((char*)codeAlphabet, lenghts);
    assert(huff_validateTree(hf));
    
    // Reading actual huffman code for data
    for(j=0; j < (unsigned short) strlen(codeAlphabet); j++) codeAlphabet[j] = 0;
    string = codeAlphabet;
    unsigned short int limit = HLIT + 257;
    unsigned short int value, last = 0;
    lenghts = realloc(lenghts, limit*sizeof(unsigned short int));
    lenghtsIndex=0;
    j=0;
    while(j<limit){
        value = __getHuffmanValue(hf, data, bc, i, zdc);
        if(value <= 15){
            if(value != 0) {
                sprintf(string, "%u ", j);
                string = codeAlphabet + strlen(codeAlphabet);
                lenghts[lenghtsIndex++] = value;
                ////printf("littlelen %d %d\n", j, value);
            }
            last = value;
            j++;
        }
        else if(value == 16){
            value = __getExtraBitsLM(2, data, bc, i, zdc);
            value += 3;
            j += value;
            sprintf(string, "%d-%d ", j-value, j-1);
            string = codeAlphabet + strlen(codeAlphabet);
            lenghts[lenghtsIndex++] = last;
            ////printf("littlelen repeat %d %d\n", last, value);
        }
        else if (value == 17){
            value = __getExtraBitsLM(3, data, bc, i, zdc);
            value += 3;
            j += value;
            ////printf("codes of length 0: %d\n", value);
        }
        else if (value == 18){
            value = __getExtraBitsLM(7, data, bc, i, zdc);
            value += 11;
            j += value;
            ////printf("codes of length 0: %d\n", value);
        }
    }
    ////printf("%s\n", codeAlphabet);
    
    codeAlphabet[strlen(codeAlphabet)-1] = '\0';
    huff_tree res = huff_generateTree((char*)codeAlphabet, lenghts);
    assert(huff_validateTree(res));
    
    // Generating distance huffman code
    //  -------------------------
    
    for(j=0; j < (unsigned short) strlen(codeAlphabet); j++) codeAlphabet[j] = 0;
    string = codeAlphabet;
    limit = HDIST+1;
    lenghts = realloc(lenghts, limit*sizeof(unsigned short int));
    lenghtsIndex=0;
    j=0;
    huff_tree hfd;
    while(j<limit){
        value = __getHuffmanValue(hf, data, bc, i, zdc);
        if(value <= 15){
            if(value != 0) {
                sprintf(string, "%u ", j);
                string = codeAlphabet + strlen(codeAlphabet);
                lenghts[lenghtsIndex++] = value;
                ////printf("dist %d %d\n", j, value);
            }
            last = value;
            j++;
        }
        else if(value == 16){
            value = __getExtraBitsLM(2, data, bc, i, zdc);
            value += 3;
            j += value;
            sprintf(string, "%d-%d ", j-value, j-1);
            string = codeAlphabet + strlen(codeAlphabet);
            lenghts[lenghtsIndex++] = last;
            ////printf("dist repeat %d %d\n", last, value);
        }
        else if (value == 17){
            value = __getExtraBitsLM(3, data, bc, i, zdc);
            value += 3;
            j += value;
            ////printf("dist codes of length 0: %d\n", value);
        }
        else if (value == 18){
            value = __getExtraBitsLM(7, data, bc, i, zdc);
            value += 11;
            j += value;
            ////printf("dist codes of length 0: %d\n", value);
        }
    }
    ////printf("%s\n", codeAlphabet);
    
    codeAlphabet[strlen(codeAlphabet)-1] = '\0';
    hfd = huff_generateTree((char*)codeAlphabet, lenghts);
    assert(huff_validateTree(res));
    
    free(lenghts);
    huff_freeTree(hf);
    *distanceHf = hfd;
    return res;
}

zlib_data zlib_inflate(zlib_data *data, int n, int *newN){
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    zlib_data res = malloc(*newN * sizeof(struct zlib_data_st));
    res->l = 0;
    res->data = malloc(sizeof(byte));

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if( ret != Z_OK) return NULL;

    int cycles=0;
    do {

        if(data[cycles]->l<CHUNK) {
            for (int i = 0; i<data[cycles]->l; i++) {
                in[i] = data[cycles]->data[i];
            }
            strm.avail_in = data[cycles++]->l;
            strm.next_in = in;
        }

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return NULL;
            }

            have = CHUNK - strm.avail_out;


            if(have > 0){
                int total = res->l + have + 1;
                res->data = realloc(res->data, total * sizeof(byte));
                printf("REALLOCATED: %d(old) - %d(new)\n", res->l, total);
                assert(res->data != NULL);
                for(int i=0; i<have; i++){
                    res->data[res->l+i] = out[i];
                }
                res->l += have;
            }

        } while (strm.avail_out==0);
    }while(ret != Z_STREAM_END);
    ret = inflateEnd(&strm);

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
        //printf("---------------------BLOCK: %d %d\n", lastBlock, type);
        
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
                //printf("Error in file.\n");
                exit(-1);
            }
            
            for(int j=0; j<LEN; j++){
                char byte = __nextByte(data, bc, i, zdc);
                resAlloc = __addOutputStreamByte(byte, res, resAlloc, LEN);
            }
        }else{ // Huffman codes
            huff_tree hf, dhf;
            
            if(type == 0b10) //dinamic huffman codes
                hf = __readAndGenerateDynamicHuffmanTree(&dhf, data, bc, i, zdc);
            else{ //type == 0b01 -> fixed huffman codes
                unsigned short int lenghts[4] = {8, 9, 7, 8};
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
                    //printf("res->l: %d --- literal: %d\n", res->l, read);
                }
                else if(read > 256){
                    unsigned int length, distance;
                    switch(read){
                        default:
                        case 257 ... 264:
                            length = read-254;
                            break;
                        case 265 ... 268:
                            length = 11 + 2 * (read - 265);
                            length += __getExtraBitsLM(1, data, bc, i, zdc);
                            break;
                        case 269 ... 272:
                            length = 19 + 4 * (read - 269);
                            length += __getExtraBitsLM(2, data, bc, i, zdc);
                            break;
                        case 273 ... 280:
                            length = 35 + 8 * (read - 273);
                            length += __getExtraBitsLM(3, data, bc, i, zdc);
                            break;
                        case 281 ... 284:
                            length = 131 + 16 * (read - 281);
                            length += __getExtraBitsLM(4, data, bc, i, zdc);
                            break;
                        case 285:
                            length = 258;
                            break;
                    }
                    
                    distance = 0;
                    if(type == 0b01) {
                        for(int j = 0; j<5; j++) {
                            unsigned short int r = __nextBit(data, bc, i, zdc);
                            r <<= (unsigned) j;
                            distance += r;
                        }
                    }else{ // type == 0b10
                        distance = __getHuffmanValue(dhf, data, bc, i, zdc);
                    }
                    
                    int eb = (distance/2)-1;
                    if(distance<4) {
                        distance += 1;
                    } else {
                        int last = 4; // last value of distance
                        int r = 4, i1 = 1, i2 = 0; // r == read value as distance
                        while(r<distance) {
                            if(i2==2) {
                                i1++;
                                i2 = 0;
                            }
                            last += __twoPower(i1);
                            i2++;
                            r++;
                        }
                        distance = last + 1;
                    }
                    distance += __getExtraBitsLM(eb, data, bc, i, zdc);
    
                    //printf("(BEFORE) res->l: %d --- L: %d, D: %d\n", res->l, length, distance);
                    fflush(stdout);
                    
                    assert(distance < res->l);
                    unsigned char *startingPoint = res->data + res->l - distance;
                    for(int j=0; j<length; j++){
                        unsigned char byte = *startingPoint;
                        resAlloc = __addOutputStreamByte(byte, res, resAlloc, length);
                        startingPoint++;
                    }
                    //printf("(AFTER) res->l: %d --- L: %d, D: %d\n", res->l, length, distance);
                    fflush(stdout);
                }
                
            }while(read != END_OF_BLOCK);
        }
    
    }while(!lastBlock);
    
    return res;
}
void zlib_freeData(zlib_data data){
    free(data->data);
    free(data);
}