

#ifndef IMAGELIB_PIXEL_H
#define IMAGELIB_PIXEL_H


typedef unsigned char			COLOR;

typedef struct {COLOR r, g, b, a;}	PIXEL_RGBA;

#define PIXEL_UNDEFINED -239	/* to flag undefined vbls in various places */

#endif
