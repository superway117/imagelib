#ifndef IMAGELIB_ZOOM_H
#define IMAGELIB_ZOOM_H

typedef struct 
{	/* SOURCE TO DEST COORDINATE MAPPING */
    double sx, sy;	/* x and y scales */
    double tx, ty;	/* x and y translations */
    double ux, uy;	/* x and y offset used by MAP, private fields */
} Mapping;


#endif