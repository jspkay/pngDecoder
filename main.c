#include <stdio.h>
#include <stdlib.h>
#include "pnglib.h"

int main(){
    pngInitialize();
    pngID p1 = pngOpenFile("/home/salvo/Immagini/spotify.svg", "r"),
        p2 = pngOpenFile("/home/salvo/Immagini/wallpaper.png", "r");
    printf("p1 is png: %c\np2 is png: %c\n",
        pngVerifyType(p1)?'t':'f',
        pngVerifyType(p2)?'t':'f');

    int size;
    if(p2 < 0){
        printf("Error in reading image!\n");
        exit(0);
    }
    pngChunk *pc = pngReadChunks(p2, &size);

    for(int j=0; j<size; j++){
        printf("%d.", j);
        pngPrintChunk(pc[j], stdout);
        printf("\n");
    } 
    
    return 0;
}