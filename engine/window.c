#include <image_headfile.h>
#include "window.h"

void window_set(INT32 x0, INT32 y0, INT32 x1, INT32 y1, WINDOW *a)
{
    a->x0 = x0;
    a->y0 = y0;
    a->x1 = x1;
    a->y1 = y1;
}

/* a=intersect(a,b), return overlap bit */
INT32 window_clip(WINDOW *a, const WINDOW *b)
{
    INT32 overlap;

    overlap = window_overlap(a, b);
    window_intersect(a, b, a);
    return overlap;
}

/* c = intersect(a,b) */
void window_intersect(const WINDOW *a, const WINDOW *b, WINDOW *c)
{
    c->x0 = MAX(a->x0, b->x0);
    c->y0 = MAX(a->y0, b->y0);
    c->x1 = MIN(a->x1, b->x1);
    c->y1 = MIN(a->y1, b->y1);
}

INT32 window_overlap(const WINDOW *a, const WINDOW *b)
{
    return a->x0<=b->x1 && a->x1>=b->x0 && a->y0<=b->y1 && a->y1>=b->y0;
}


void window_box_intersect(const WINDOW_BOX *a, const WINDOW_BOX *b, WINDOW_BOX *c)
{
    c->x0 = MAX(a->x0, b->x0);
    c->y0 = MAX(a->y0, b->y0);
    c->x1 = MIN(a->x1, b->x1);
    c->y1 = MIN(a->y1, b->y1);
    window_box_set_size(c);
}

void window_box_set_max(WINDOW_BOX *a)
{
    a->x1 = a->x0+a->nx-1;
    a->y1 = a->y0+a->ny-1;
}

void window_box_set_size(WINDOW_BOX *a)
{
    a->nx = a->x1-a->x0+1;
    a->ny = a->y1-a->y0+1;
}
