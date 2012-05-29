#ifndef IMAGELIB_WINDOW_H
#define IMAGELIB_WINDOW_H

#include <image_types.h>
typedef struct /* WINDOW: A DISCRETE 2-D RECTANGLE */
{	
    INT32 x0, y0;		/* xmin and ymin */
    INT32 x1, y1;		/* xmax and ymax (inclusive) */
} WINDOW;

typedef struct 	/* WINDOW_BOX: A DISCRETE 2-D RECTANGLE */
{
    INT32 x0, y0;		/* xmin and ymin */
    INT32 x1, y1;		/* xmax and ymax (inclusive) */
    INT32 nx, ny;		/* xsize=x1-x0+1 and ysize=y1-y0+1 */
} WINDOW_BOX;

/*
 * note: because of the redundancy in the above structure, nx and ny should
 * be recomputed with window_box_set_size() when they cannot be trusted
 */

/* caution: we exploit positional coincidences in the following: */
#define window_box_overlap(a, b) \
    window_overlap((WINDOW *)(a), (WINDOW *)(b))

extern void window_set(INT32 x0, INT32 y0, INT32 x1, INT32 y1, WINDOW *a);

extern INT32 window_clip(WINDOW *a, const WINDOW *b);

extern void window_intersect(const WINDOW *a, const WINDOW *b, WINDOW *c);

extern INT32 window_overlap(const WINDOW *a, const WINDOW *b);

extern void window_box_intersect(const WINDOW_BOX *a, const WINDOW_BOX *b, WINDOW_BOX *c);

extern void window_box_set_max(WINDOW_BOX *a);

extern void window_box_set_size(WINDOW_BOX *a);

#endif
