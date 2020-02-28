//
// Created by salvo on 27/02/20.
//

#ifndef PNGLIB_FILTERALGORITHMS_H
#define PNGLIB_FILTERALGORITHMS_H

#include "pnglib.h"

typedef struct params{
    pixel *line;
    unsigned char *rawImage;
    pixel *upline;
    int i;
    int imgWidth;
    int bpp;
} *params;

//ColorType=0, BithDepth={1,2,4,8}
void __subFilteringGrayscale(struct params *p);
void __noneFilteringGrayscale(struct params *p);
void __upFilteringGrayscale(struct params *p);
void __averageFilteringGrayscale(struct params *p);
void __paethFilteringGrayscale(struct params *p);

//ColorType=0, BithDepth={1,2,4,8}
void __subFilteringGrayscale16bit(struct params *p);
void __noneFilteringGrayscale16bit(struct params *p);
void __upFilteringGrayscale16bit(struct params *p);
void __averageFilteringGrayscale16bit(struct params *p);
void __paethFilteringGrayscale16bit(struct params *p);

//ColorType=2, BitDepth=8
void __subFilteringColors8bit(struct params *p);
void __noneFilteringColors8bit(struct params *p);
void __upFilteringColors8bit(struct params *p);
void __averageFilteringColors8bit(struct params *p);
void __paethFilteringColors8bit(struct params *p);

//ColorType=2, BitDepth=16
void __subFilteringColors16bit(struct params *p);
void __noneFilteringColors16bit(struct params *p);
void __upFilteringColors16bit(struct params *p);
void __averageFilteringColors16bit(struct params *p);
void __paethFilteringColors16bit(struct params *p);

//ColorType=4, BitDepth=8
void __subFilteringGrayscaleAlpha8bit(struct params *p);
void __noneFilteringGrayscaleAlpha8bit(struct params *p);
void __upFilteringGrayscaleAlpha8bit(struct params *p);
void __averageFilteringGrayscaleAlpha8bit(struct params *p);
void __paethFilteringGrayscaleAlpha8bit(struct params *p);

//ColorType=4, BitDepth=16
void __subFilteringGrayscaleAlpha16bit(struct params *p);
void __noneFilteringGrayscaleAlpha16bit(struct params *p);
void __upFilteringGrayscaleAlpha16bit(struct params *p);
void __averageFilteringGrayscaleAlpha16bit(struct params *p);
void __paethFilteringGrayscaleAlpha16bit(struct params *p);

//ColorType=6, BitDepth=8
void __noneFilteringColorsAlpha8bit(struct params *p);
void __subFilteringColorsAlpha8bit(struct params *p);
void __upFilteringColorsAlpha8bit(struct params *p);
void __averageFilteringColorsAlpha8bit(struct params *p);
void __paethFilteringColorsAlpha8bit(struct params *p);

//ColorType=6, BitDepth=16
void __noneFilteringColorsAlpha16bit(struct params *p);
void __subFilteringColorsAlpha16bit(struct params *p);
void __upFilteringColorsAlpha16bit(struct params *p);
void __averageFilteringColorsAlpha16bit(struct params *p);
void __paethFilteringColorsAlpha16bit(struct params *p);

#endif //PNGLIB_FILTERALGORITHMS_H
