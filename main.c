#include <stdio.h>
#include <stdlib.h>
#include "pnglib.h"

int main(){
    pngInitialize();
    pngID p1 = pngOpenFile("/home/salvo/Immagini/wallpaper", "r"),
        p2 = pngOpenFile("/home/salvo/Immagini/test/2.png", "r");
    printf("p1 is png: %c\np2 is png: %c\n",
        pngVerifyType(p1)?'t':'f',
        pngVerifyType(p2)?'t':'f');

    int size;
    if(p2 < 0){
        printf("Error in reading image!\n");
        exit(0);
    }
    pngFileChunk pc = pngReadChunks(p2);

    /* pngChunk IDAT;
    IDAT = pngGetIDATChunk(pc);
    for(int i=0; i<IDAT->length; i++){
        printf("%x", IDAT->data[i]);
    }*/

    for(int j=0; j<pc->n; j++){
        printf("%d.", j);
        pngPrintChunk(pc->chunks[j], stdout);
        printf("\n");
    };
    
    return 0;
}