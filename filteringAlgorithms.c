//
// Created by salvo on 05/01/20.
//

#include "pnglib.h"
#define DEBUG

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

void __subFilteringColorsAlpha(pixel *line, const unsigned char*rawImage, int i, pngImage image){
    int bpp = 4;
    // Storing the first untouched byte
    line[0].r = rawImage[i * image->w * bpp + 1];
    line[0].g = rawImage[i * image->w * bpp+ 2];
    line[0].b = rawImage[i * image->w * bpp+ 3];
    line[0].a = rawImage[i * image->w * bpp+ 4];
    
    for(int j=1; j<image->w; j++){
        line[j].r = (line[j - 1].r + rawImage[i * image->w + j * bpp + 1]) % 256;
        line[j].g = (line[j - 1].g + rawImage[i * image->w + j * bpp + 2]) % 256;
        line[j].b = (line[j - 1].b + rawImage[i * image->w + j * bpp + 3]) % 256;
        line[j].a = (line[j - 1].a + rawImage[i * image->w + j * bpp + 4]) % 265;
        #ifdef DEBUG
        printf("%d\t%d %d %d %d - %d\n", j, line[j].r, line[j].g, line[j].b, line[j].a, i * image->w + j*bpp);
        fflush(stdout);
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
        
        for(int j=0; j<image->w; j++){
            line[j].r = (upLine[j].r + rawImage[i * image->w + j * bpp + 1]) % 256;
            line[j].g = (upLine[j].g + rawImage[i * image->w + j * bpp + 2]) % 256;
            line[j].b = (upLine[j].b + rawImage[i * image->w + j * bpp + 3]) % 256;
            line[j].a = (upLine[j].a + rawImage[i * image->w + j * bpp + 4]) % 265;
#ifdef DEBUG
            printf("%d\t%d %d %d %d\n", j, line[j].r, line[j].g, line[j].b, line[j].a);
            fflush(stdout);
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
    
    line[0].r = rawImage[i * image->w + 1] + upLine[0].r/2;
    line[0].g = rawImage[i * image->w + 2] + upLine[0].g/2;
    line[0].b = rawImage[i * image->w + 3] + upLine[0].b/2;
    line[0].a = rawImage[i * image->w + 4] + upLine[0].a/2;
    
    printf("line: %d\n", i);
    for(int j=1; j<image->w; j++) {
        line[j].r = (rawImage[i * image->w + j * bpp + 1] + (line[j - 1].r + upLine[j].r) / 2) % 256;
        line[j].r = (rawImage[i * image->w + j * bpp + 2] + (line[j - 1].g + upLine[j].g) / 2) % 256;
        line[j].r = (rawImage[i * image->w + j * bpp + 3] + (line[j - 1].b + upLine[j].b) / 2) % 256;
        line[j].r = (rawImage[i * image->w + j * bpp + 4] + (line[j - 1].a + upLine[j].a) / 2) % 256;
#ifdef DEBUG
        printf("%d\t%d %d %d %d\n", j, line[j].r, line[j].g, line[j].b, line[j].a);
        fflush(stdout);
#endif
    }
#ifdef DEBUG
    printf("\n");
    fflush(stdout);
#endif
}

void __paethFilteringColorsAlpha(){}