//
// Created by salvo on 05/01/20.
//

#include "pnglib.h"
#define DEBUG
#include <malloc.h>

static unsigned char paethPredictor(unsigned char a, unsigned char b, unsigned char c){
    // a->left, b->above, c->upperLeft
    short int p, pa, pb, pc;
    p = a+b-c;
    pa = p - a; if(pa<0) pa = -pa;
    pb = p - b; if(pb<0) pb = -pb;
    pc = p - c; if(pc<0) pb = -pc;
    if(pa <= pb && pa <= pc) return a;
    else if (pb <= pc) return b;
    else return c;
}

// type = 0
void __noneFilteringColorsAlpha(pixel *line, const unsigned char*rawImage, int i,pngImage image){
    int bpp = 4;
    for(int j=0; j<image->w; j++){
        line[j].r = rawImage[ i*image->w*bpp + j * bpp + 1]; // offset of one because
        line[j].g = rawImage[ i*image->w*bpp + j * bpp + 2]; // the first byte is the
        line[j].b = rawImage[ i*image->w*bpp + j * bpp + 3]; // filtering type
        line[j].a = rawImage[ i*image->w*bpp + j * bpp + 4]; // so we add 1 to correct

        #ifdef DEBUG
        printf("%d\t%3d %3d %3d %3d - %d\n", j, line[j].r, line[j].g, line[j].b, line[j].a, i*image->w*bpp + j*bpp);
        fflush(stdout);
        #endif
    }
}

//type = 1
void __subFilteringColorsAlpha(pixel *line, const unsigned char*rawImage, int i, pngImage image){
    int bpp = 4;
    int lineLen = image->w * bpp +1;

    // Storing the first untouched byte (which is 0 + raw)
    line[0].r = rawImage[i*lineLen + 1] % 256;
    line[0].g = rawImage[i*lineLen + 2] % 256;
    line[0].b = rawImage[i*lineLen + 3] % 256;
    line[0].a = rawImage[i*lineLen + 4] % 256;
    printf("%d\t%d %d %d %d - %d\n", i*lineLen+0, line[0].r, line[0].g, line[0].b, line[0].a, i*lineLen + 1 );


    for(int j=1; j<image->w; j++){
        line[j].r = (line[j - 1].r + rawImage[i * lineLen + j * bpp + 1]) % 256;
        line[j].g = (line[j - 1].g + rawImage[i * lineLen + j * bpp + 2]) % 256;
        line[j].b = (line[j - 1].b + rawImage[i * lineLen + j * bpp + 3]) % 256;
        line[j].a = (line[j - 1].a + rawImage[i * lineLen + j * bpp + 4]) % 265;
        #ifdef DEBUG
        printf("%d\t%d %d %d %d - %d\n", i*lineLen+j, line[j].r, line[j].g, line[j].b, line[j].a, i*lineLen + j*bpp +1 );
        fflush(stdout);
        /*
        line[j].g=0;
        line[j].b = 0;
        line[j].a = 0;
          */
        #endif
    }
    
    #ifdef DEBUG
    printf("\n");
    fflush(stdout);
    #endif
}

void __upFilteringColorsAlpha(pixel *line, const pixel* upLine, const unsigned char*rawImage, int i, pngImage image){
    if(i == 0) {//fisrt scanline
        // equivalent according to specs
        __noneFilteringColorsAlpha(line, rawImage, i, image);
    }
    else{
        int bpp = 4;
        #ifdef DEBUG
        printf("line: %d\n", i);
        #endif

        int lineLen = image->w * bpp + 1;

        for(int j=0; j<image->w; j++){
            line[j].r = (upLine[j].r + rawImage[i*lineLen + j * bpp + 1]) % 256;
            line[j].g = (upLine[j].g + rawImage[i*lineLen + j * bpp + 2]) % 256;
            line[j].b = (upLine[j].b + rawImage[i*lineLen + j * bpp + 3]) % 256;
            line[j].a = (upLine[j].a + rawImage[i*lineLen + j * bpp + 4]) % 265;
#ifdef DEBUG
            printf("%d\t%d %d %d %d - %d\n", i*lineLen+j, line[j].r, line[j].g, line[j].b, line[j].a, i*lineLen + j*bpp + 1);
            fflush(stdout) ;
/*            line[j].r = 0;
            line[j].b = 0;
            line[j].a = 0; */
#endif
        }
        #ifdef DEBUG
        printf("\n");
        fflush(stdout);
        #endif
    }
}

void __averageFilteringColorsAlpha(pixel* line, const pixel*upLine, const unsigned char*rawImage, int i, pngImage image){
    int bpp = 4;
    int lineLen = image->w*bpp + 1;

    if(i == 0 && upLine==NULL) upLine = calloc(image->w, sizeof(char));

    line[0].r = rawImage[i*lineLen+1] + upLine[0].r/2;
    line[0].g = rawImage[i*lineLen+2] + upLine[0].g/2;
    line[0].b = rawImage[i*lineLen+3] + upLine[0].b/2;
    line[0].a = rawImage[i*lineLen+4] + upLine[0].a/2;


    for(int j=1; j<image->w; j++) {
        int index = i*lineLen+j*bpp+1;
        line[j].r = (rawImage[index] + (line[j-1].r + upLine[j].r) / 2) % 256;
        line[j].g = (rawImage[index] + (line[j-1].g + upLine[j].g) / 2) % 256;
        line[j].b = (rawImage[index] + (line[j-1].b + upLine[j].b) / 2) % 256;
        line[j].a = (rawImage[index] + (line[j-1].a + upLine[j].a) / 2) % 256;
#ifdef DEBUG
        printf("%d\t%d %d %d %d\n", j, line[j].r, line[j].g, line[j].b, line[j].a);
        fflush(stdout); /*
        line[j].r=0;
        line[j].g=0;
        line[j].a = 0; */
#endif
    }
#ifdef DEBUG
    printf("\n");
    fflush(stdout);
#endif
}

// type = 4
void __paethFilteringColorsAlpha(pixel *line, const pixel*upline, unsigned char*rawImage, int i, pngImage image){
    int bpp = 4;
    int lineLen = image->w*bpp + 1;

    if(i==0 && upline == NULL) upline = calloc(image->w, sizeof(char));

    line[0].r = (rawImage[i*lineLen +1] + paethPredictor(0, upline[0].r, 0) )%256;
    line[0].g = (rawImage[i*lineLen +2] + paethPredictor(0, upline[0].g, 0) )%256;
    line[0].b = (rawImage[i*lineLen +3] + paethPredictor(0, upline[0].b, 0) )%256;
    line[0].a = (rawImage[i*lineLen +4] + paethPredictor(0, upline[0].a, 0) )%256;
#ifdef DEBUG
    printf("%d\t%d %d %d %d - %d\n", i*lineLen, line[0].r, line[0].g, line[0].b, line[0].a, i*lineLen + 1);
    fflush(stdout);
#endif

    for(int j=1; j<image->w; j++){
        line[j].r = ( rawImage[i*lineLen + j*bpp+1] + paethPredictor(line[j-1].r, upline[j].r, upline[j-1].r) )% 256;
        line[j].g = ( rawImage[i*lineLen + j*bpp+2] + paethPredictor(line[j-1].g, upline[j].g, upline[j-1].g) )% 256;
        if(i == 6 && j == 5)
            printf("index: %d --- val: %d\n", i*lineLen + j*bpp+3, rawImage[i*lineLen + j*bpp+3]);
        line[j].b = ( rawImage[i*lineLen + j*bpp+3] + paethPredictor(line[j-1].b, upline[j].b, upline[j-1].b) )% 256;
        line[j].a = ( rawImage[i*lineLen + j*bpp+4] + paethPredictor(line[j-1].a, upline[j].a, upline[j-1].a) )% 256;
        #ifdef  DEBUG
        printf("%d\t%d %d %d %d - %d\n", i*lineLen+j, line[j].r, line[j].g, line[j].b, line[j].a, i*lineLen + j*bpp + 1);
        fflush(stdout);
        //line[j].a = 0;
        #endif
    }
}