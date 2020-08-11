//
// Created by salvo on 05/01/20.
//

#include "pnglib.h"
#include "filterAlgorithms.h"
//#define DEBUG
#include <malloc.h>
#include <math.h>

static unsigned char paethPredictor(unsigned char a, unsigned char b, unsigned char c){
    // a->left, b->above, c->upperLeft
    short int p, pa, pb, pc;
    p = a+b-c;
    pa = p - a; if(pa<0) pa = -pa;
    pb = p - b; if(pb<0) pb = -pb;
    pc = p - c; if(pc<0) pc = -pc;
    if(pa <= pb && pa <= pc) return a;
    else if (pb <= pc) return b;
    else return c;
}
static unsigned int average(unsigned char a, unsigned char b){
    double avg = ((double)a + (double)b) / 2.0;
    double favg = floor(avg);
    //printf("Average: %d, %d - %d\n", a, b, (int) favg);
    return (int) favg;
}

//ColorType=0, BithDepth={1,2,4,8}
void __subFilteringGrayscale(struct params *p){}
void __noneFilteringGrayscale(struct params *p){}
void __upFilteringGrayscale(struct params *p){}
void __averageFilteringGrayscale(struct params *p){}
void __paethFilteringGrayscale(struct params *p){}

//ColorType=0, BithDepth={1,2,4,8}
void __subFilteringGrayscale16bit(struct params *p){}
void __noneFilteringGrayscale16bit(struct params *p){}
void __upFilteringGrayscale16bit(struct params *p){}
void __averageFilteringGrayscale16bit(struct params *p){}
void __paethFilteringGrayscale16bit(struct params *p){}

//ColorType=2, BitDepth=8
void __noneFilteringColors8bit(struct params *p){
    int bpp = 4;

    pixel *line = p->line;
    unsigned char *rawImage = p->rawImage;
    int i = p->i;
    int imgWidth = p->imgWidth;

    for(int j=0; j<imgWidth; j++){
        line[j].r = rawImage[i * imgWidth * bpp+j * bpp+1]; // offset of one because
        line[j].g = rawImage[i * imgWidth * bpp+j * bpp+2]; // the first byte is the
        line[j].b = rawImage[i * imgWidth * bpp+j * bpp+3]; // filtering type
        line[j].a = 255;

#ifdef DEBUG
        printf("%d\t%3d %3d %3d %3d - %d\n", j, line[j].r, line[j].g, line[j].b, line[j].a, i * imgWidth * bpp+j * bpp);
        fflush(stdout);
#endif
    }
}
void __subFilteringColors8bit(struct params *p){
    pixel *line = p->line;
    const unsigned char *rawImage = p->rawImage;
    int i = p->i;
    int imgWidth = p->imgWidth;
    int bpp = p->bpp;

    int lineLen = imgWidth * bpp +1;

    // Storing the first untouched byte (which is 0 + raw)
    line[0].r = rawImage[i*lineLen + 1] % 256;
    line[0].g = rawImage[i*lineLen + 2] % 256;
    line[0].b = rawImage[i*lineLen + 3] % 256;
    line[0].a = 255;

#ifdef DEBUG
    printf("%d\t%d %d %d %d - %d\n", i*lineLen+0, line[0].r, line[0].g, line[0].b, line[0].a, i*lineLen + 1 );
#endif


    for(int j=1; j<imgWidth; j++){
        line[j].r = (line[j - 1].r + rawImage[i * lineLen + j * bpp + 1]) % 256;
        line[j].g = (line[j - 1].g + rawImage[i * lineLen + j * bpp + 2]) % 256;
        line[j].b = (line[j - 1].b + rawImage[i * lineLen + j * bpp + 3]) % 256;
        line[j].a = (line[j - 1].a + rawImage[i * lineLen + j * bpp + 4]) % 256;
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
void __upFilteringColors8bit(struct params *p){
    if(p->i == 0) {//fisrt scanline
        // equivalent according to specs
        __noneFilteringColors8bit(p);
    }
    else{
        pixel *line = p->line;
        pixel *upLine = p->upline;
        const unsigned char *rawImage = p->rawImage;
        int i = p->i;
        int imgWidth = p->imgWidth;
        int bpp = p->bpp;

        int lineLen = imgWidth * bpp + 1;

        for(int j=0; j<imgWidth; j++){
            line[j].r = (upLine[j].r + rawImage[i*lineLen + j * bpp + 1]) % 256;
            line[j].g = (upLine[j].g + rawImage[i*lineLen + j * bpp + 2]) % 256;
            line[j].b = (upLine[j].b + rawImage[i*lineLen + j * bpp + 3]) % 256;
            line[j].a = 255;
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
void __averageFilteringColors8bit(struct params *p){
    pixel *line = p->line;
    pixel *upLine = p->upline;
    const unsigned char *rawImage = p->rawImage;
    int i = p->i;
    int imgWidth = p->imgWidth;
    int bpp = p->bpp;

    int lineLen = imgWidth*bpp + 1;

    if(i == 0 && upLine==NULL) upLine = calloc(imgWidth, sizeof(char));

    line[0].r = rawImage[i*lineLen+1] + upLine[0].r/2;
    line[0].g = rawImage[i*lineLen+2] + upLine[0].g/2;
    line[0].b = rawImage[i*lineLen+3] + upLine[0].b/2;
    line[0].a = 255;

    for(int j=1; j<imgWidth; j++) {
        int index = i*lineLen+j*bpp;
        line[j].r = (rawImage[index+1] + (line[j-1].r + upLine[j].r) / 2) % 256;
        line[j].g = (rawImage[index+2] + (line[j-1].g + upLine[j].g) / 2) % 256;
        line[j].b = (rawImage[index+3] + (line[j-1].b + upLine[j].b) / 2) % 256;
        line[j].a = 255;
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
void __paethFilteringColors8bit(struct params *p){
    pixel *line = p->line;
    pixel *upline = p->upline;
    const unsigned char *rawImage = p->rawImage;
    int i = p->i;
    int imgWidth = p->imgWidth;
    int bpp = p->bpp;

    int lineLen = imgWidth*bpp + 1;

    if(i==0 && upline == NULL) upline = calloc(imgWidth, sizeof(char));

    line[0].r = (rawImage[i*lineLen +1] + paethPredictor(0, upline[0].r, 0) )%256;
    line[0].g = (rawImage[i*lineLen +2] + paethPredictor(0, upline[0].g, 0) )%256;
    line[0].b = (rawImage[i*lineLen +3] + paethPredictor(0, upline[0].b, 0) )%256;
    line[0].a = 255;
#ifdef DEBUG
    printf("Indexes: %d -> %d\n", i*lineLen+1, i*lineLen+4);
    printf("%d\t%d %d %d %d - %d\n", i*lineLen, line[0].r, line[0].g, line[0].b, line[0].a, i*lineLen + 1);
    fflush(stdout);
#endif


    for(int j=1; j<imgWidth; j++){
        line[j].r = ( rawImage[i*lineLen + j*bpp+1] + paethPredictor(line[j-1].r, upline[j].r, upline[j-1].r) )% 256;
        line[j].g = ( rawImage[i*lineLen + j*bpp+2] + paethPredictor(line[j-1].g, upline[j].g, upline[j-1].g) )% 256;
        line[j].b = ( rawImage[i*lineLen + j*bpp+3] + paethPredictor(line[j-1].b, upline[j].b, upline[j-1].b) )% 256;
        line[j].a = 255;
#ifdef  DEBUG
        printf("%d %d %d %d\t", rawImage[i*lineLen + j*bpp+1], rawImage[i*lineLen + j*bpp+2], rawImage[i*lineLen + j*bpp+3],rawImage[i*lineLen + j*bpp+4]);
        printf("%d:%d\t%d %d %d %d - %d\n", i, j, line[j].r, line[j].g, line[j].b, line[j].a, i*lineLen + j*bpp + 1);
        fflush(stdout);
#endif
    }
}

//ColorType=2, BitDepth=16
void __subFilteringColors16bit(struct params *p){}
void __noneFilteringColors16bit(struct params *p){}
void __upFilteringColors16bit(struct params *p){}
void __averageFilteringColors16bit(struct params *p){}
void __paethFilteringColors16bit(struct params *p){}

//ColorType=4, BitDepth=8
void __subFilteringGrayscaleAlpha8bit(struct params *p){}
void __noneFilteringGrayscaleAlpha8bit(struct params *p){}
void __upFilteringGrayscaleAlpha8bit(struct params *p){}
void __averageFilteringGrayscaleAlpha8bit(struct params *p){}
void __paethFilteringGrayscaleAlpha8bit(struct params *p){}

//ColorType=4, BitDepth=16
void __subFilteringGrayscaleAlpha16bit(struct params *p){}
void __noneFilteringGrayscaleAlpha16bit(struct params *p){}
void __upFilteringGrayscaleAlpha16bit(struct params *p){}
void __averageFilteringGrayscaleAlpha16bit(struct params *p){}
void __paethFilteringGrayscaleAlpha16bit(struct params *p){}

//ColorType=6, BitDepth=8
void __noneFilteringColorsAlpha8bit(struct params *p){ // type = 0
    pixel *line = p->line;
    unsigned char *rawImage = p->rawImage;
    int i = p->i;
    int imgWidth = p->imgWidth;
    int bpp = p->bpp;

    int lineLen = imgWidth*bpp+1;
    for(int j=0; j<imgWidth; j++){
        line[j].r = rawImage[i * lineLen +j * bpp+1]; // offset of one because
        line[j].g = rawImage[i * lineLen +j * bpp+2]; // the first byte is the
        line[j].b = rawImage[i * lineLen +j * bpp+3]; // filtering type
        line[j].a = rawImage[i * lineLen +j * bpp+4]; // so we add 1 to correct
        //line[j].a = 0;
        #ifdef DEBUG
        printf("%d\t%3d %3d %3d %3d - %d\n", j, line[j].r, line[j].g, line[j].b, line[j].a, i * imgWidth * bpp+j * bpp);
        fflush(stdout);
        #endif
    }
}
void __subFilteringColorsAlpha8bit(struct params *p){//type = 1
    pixel *line = p->line;
    const unsigned char *rawImage = p->rawImage;
    int i = p->i;
    int imgWidth = p->imgWidth;
    int bpp = p->bpp;

    int lineLen = imgWidth * bpp +1;

    // Storing the first untouched byte (which is 0 + raw)
    line[0].r = rawImage[i*lineLen + 1] % 256;
    line[0].g = rawImage[i*lineLen + 2] % 256;
    line[0].b = rawImage[i*lineLen + 3] % 256;
    line[0].a = rawImage[i*lineLen + 4] % 256;

#ifdef DEBUG
    printf("%d\t%d %d %d %d - %d\n", i*lineLen+0, line[0].r, line[0].g, line[0].b, line[0].a, i*lineLen + 1 );
#endif


    for(int j=1; j<imgWidth; j++){
        line[j].r = (line[j - 1].r + rawImage[i * lineLen + j * bpp + 1]) % 256;
        line[j].g = (line[j - 1].g + rawImage[i * lineLen + j * bpp + 2]) % 256;
        line[j].b = (line[j - 1].b + rawImage[i * lineLen + j * bpp + 3]) % 256;
        line[j].a = (line[j - 1].a + rawImage[i * lineLen + j * bpp + 4]) % 256;
        //line[j].a = 0;
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
void __upFilteringColorsAlpha8bit(struct params *p){//type = 2
    if(p->i == 0) {//fisrt scanline
        // equivalent according to specs
        __noneFilteringColorsAlpha8bit(p);
    }
    else{
        pixel *line = p->line;
        pixel *upLine = p->upline;
        const unsigned char *rawImage = p->rawImage;
        int i = p->i;
        int imgWidth = p->imgWidth;
        int bpp = p->bpp;

        int lineLen = imgWidth * bpp + 1;

        for(int j=0; j<imgWidth; j++){
            line[j].r = (upLine[j].r + rawImage[i*lineLen + j * bpp + 1]) % 256;
            line[j].g = (upLine[j].g + rawImage[i*lineLen + j * bpp + 2]) % 256;
            line[j].b = (upLine[j].b + rawImage[i*lineLen + j * bpp + 3]) % 256;
            line[j].a = (upLine[j].a + rawImage[i*lineLen + j * bpp + 4]) % 256;
            //line[j].a = 0;
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
void __averageFilteringColorsAlpha8bit(struct params *p){// type = 3;
    pixel *line = p->line;
    pixel *upLine = p->upline;
    const unsigned char *rawImage = p->rawImage;
    int i = p->i;
    int imgWidth = p->imgWidth;
    int bpp = p->bpp;

    int lineLen = imgWidth*bpp + 1;

    if(i == 0 && upLine==NULL) upLine = calloc(imgWidth, sizeof(char));

    line[0].r = (rawImage[i*lineLen+1] + upLine[0].r/2)%256;
    line[0].g = (rawImage[i*lineLen+2] + upLine[0].g/2)%256;
    line[0].b = (rawImage[i*lineLen+3] + upLine[0].b/2)%256;
    line[0].a = (rawImage[i*lineLen+4] + upLine[0].a/2)%256;

    for(int j=1; j<imgWidth; j++){
        int index = i*lineLen+j*bpp;
        line[j].r = (rawImage[index+1] + average(line[j-1].r, upLine[j].r)) % 256;
        //printf("AVG: %d - %d --> %d\n", (int) rawImage[index+1], average(line[j-1].r, upLine[j].r), line[j].r);
        line[j].g = (rawImage[index+2] + average(line[j-1].g, upLine[j].g)) % 256;
        line[j].b = (rawImage[index+3] + average(line[j-1].b, upLine[j].b)) % 256;
        line[j].a = (rawImage[index+4] + average(line[j-1].a, upLine[j].a)) % 256;
        //line[j].a = 0;
        /*
        line[j].r = (rawImage[index+1] + (int) floor( ( (double)line[j-1].r+(double)upLine[j].r) / 2.0) ) % 256;
        line[j].g = (rawImage[index+2] + (int) floor( ( (double)line[j-1].g+(double)upLine[j].g) / 2.0) ) % 256;
        line[j].b = (rawImage[index+3] + (int) floor( ( (double)line[j-1].b+(double)upLine[j].b) / 2.0) ) % 256;
        line[j].a = (rawImage[index+4] + (int) floor( ( (double)line[j-1].a+(double)upLine[j].a) / 2.0) ) % 256;
        //line[j].a = 0; */
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
void __paethFilteringColorsAlpha8bit(struct params *p){// type = 4
    pixel *line = p->line;
    pixel *upline = p->upline;
    const unsigned char *rawImage = p->rawImage;
    int i = p->i;
    int imgWidth = p->imgWidth;
    int bpp = p->bpp;

    int lineLen = imgWidth*bpp + 1;

    if(i==0 && upline == NULL) upline = calloc(imgWidth, sizeof(char));

    line[0].r = (rawImage[i*lineLen +1] + paethPredictor(0, upline[0].r, 0) )%256;
    line[0].g = (rawImage[i*lineLen +2] + paethPredictor(0, upline[0].g, 0) )%256;
    line[0].b = (rawImage[i*lineLen +3] + paethPredictor(0, upline[0].b, 0) )%256;
    line[0].a = (rawImage[i*lineLen +4] + paethPredictor(0, upline[0].a, 0) )%256;
    //line[0].a = 0;

#ifdef DEBUG
    printf("Indexes: %d -> %d\n", i*lineLen+1, i*lineLen+4);
    printf("%d\t%d %d %d %d - %d\n", i*lineLen, line[0].r, line[0].g, line[0].b, line[0].a, i*lineLen + 1);
    fflush(stdout);
#endif

    for(int j=1; j<imgWidth; j++){
        line[j].r = ( rawImage[i*lineLen + j*bpp+1] + paethPredictor(line[j-1].r, upline[j].r, upline[j-1].r) )% 256;
        line[j].g = ( rawImage[i*lineLen + j*bpp+2] + paethPredictor(line[j-1].g, upline[j].g, upline[j-1].g) )% 256;
        line[j].b = ( rawImage[i*lineLen + j*bpp+3] + paethPredictor(line[j-1].b, upline[j].b, upline[j-1].b) )% 256;
        line[j].a = ( rawImage[i*lineLen + j*bpp+4] + paethPredictor(line[j-1].a, upline[j].a, upline[j-1].a) )% 256;
        #ifdef  DEBUG
        printf("%d %d %d %d\t", rawImage[i*lineLen + j*bpp+1], rawImage[i*lineLen + j*bpp+2], rawImage[i*lineLen + j*bpp+3],rawImage[i*lineLen + j*bpp+4]);
        printf("%d:%d\t%d %d %d %d - %d\n", i, j, line[j].r, line[j].g, line[j].b, line[j].a, i*lineLen + j*bpp + 1);
        fflush(stdout);
        #endif
    }
    if(i==0) free(upline);
}

//ColorType=6, BitDepth=16
void __noneFilteringColorsAlpha16bit(struct params *p){}
void __subFilteringColorsAlpha16bit(struct params *p){}
void __upFilteringColorsAlpha16bit(struct params *p){}
void __averageFilteringColorsAlpha16bit(struct params *p){}
void __paethFilteringColorsAlpha16bit(struct params *p){}