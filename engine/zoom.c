

#include <math.h>

#include <image_headfile.h>

#include "zoom.h"
#include "image_effect.h"



/* the mapping from discrete dest coord b to continuous source coord: */
#define MAP(b, scale, offset)  (((b)+(offset))/(scale))


static void make_map_table();


IMAGELIB_ERROR_CODE image_zoom_unfiltered_impl(IMAGELIB_SESSION* _session,IMAGE_INSTANCE *decoder_instance, WINDOW_BOX *decoder_window, IMAGE_INSTANCE *encoder_instance, WINDOW_BOX *encoder_window, Mapping *m, INT32 overlap,IMAGELIB_EFFECT effect_type,INT32 effect_para)
{
    INT32 byi, by, ay, ayold, bx, ax;
    PIXEL_RGBA *abuf, *bbuf, *bp;	/* source and dest scanline buffers */
    PIXEL_RGBA **xmap, **xp;		/* mapping from abuf to bbuf */
    INT32 *ymap;		/* order of dst y coords that avoids feedback */
    char *linewritten;		/* has scanline y been written? (debugging) */
    IMAGELIB_ERROR_CODE error_code =IMAGELIB_OTHER_ERROR;
    IMAGE_ALLOC(abuf, PIXEL_RGBA, decoder_window->nx);
    IMAGE_ALLOC(bbuf, PIXEL_RGBA, encoder_window->nx);
    IMAGE_ALLOC(xmap, PIXEL_RGBA *, encoder_window->nx);
    IMAGE_ALLOC(ymap, INT32, encoder_window->ny);
    IMAGE_ALLOC_ZERO(linewritten, char, MAX(decoder_window->y1, encoder_window->y1)+1);
    if(abuf == NULL || bbuf==NULL || xmap==NULL||ymap==NULL)
    {
    	 IMAGE_FREE(abuf);
	 IMAGE_FREE(bbuf);
	 IMAGE_FREE(xmap);
	 IMAGE_FREE(ymap);
	 IMAGE_FREE(linewritten);
	 return IMAGELIB_RAM_NOT_ENOUGH;
    }

    /* if overlapping src & dst, find magic y ordering that avoids feedback */
    make_map_table(m->sy, m->ty, .5, decoder_window->y0, encoder_window->y0, encoder_window->ny, overlap, ymap);

    for (bx=0; bx<encoder_window->nx; bx++) 
    {
	ax = (INT32) MAP(bx, m->sx, m->ux);
	xmap[bx] = &abuf[ax];
    }

    ayold = -1;		/* impossible value for ay */
    for (byi=0; byi<encoder_window->ny; byi++) 
   {
   	if(_session->state != IMAGELIB_DECODING)
   	{
   		error_code = IMAGELIB_EXTERNAL_STOP;
		break;
   	}
	by = ymap[byi];
	ay = (INT32) MAP(by, m->sy, m->uy);
	/* scan line decoder_window.y0+ay goes to encoder_window.y0+by */
	if (ay!=ayold) 
	{		/* new scan line; read it in */
	    ayold = ay;
	    if (overlap && linewritten[decoder_window->y0+ay])
		fprintf(stderr, "FEEDBACK ON LINE %d\n", decoder_window->y0+ay);
	    /* read scan line into abuf */
	//    if(decoder_window->y0+ay < decoder_window->ny)			//need check
	   	error_code= image_read_row_rgba(decoder_instance, decoder_window->y0+ay, decoder_window->x0, decoder_window->nx, abuf);
	//   else
	//   	printf("error\n");
           if(error_code != IMAGELIB_SUCCESS)
		   break;
	    /* resample in x */
	    for (bp=bbuf, xp=xmap, bx=0; bx<encoder_window->nx; bx++)
		*bp++ = **xp++;
	}
	image_effect_rgba(bbuf,(UINT32)encoder_window->nx,effect_type,effect_para);
	error_code = image_write_row_rgba(encoder_instance, encoder_window->y0+by, encoder_window->x0, encoder_window->nx, bbuf);
	linewritten[encoder_window->y0+by] = 1;
    }
    IMAGE_FREE(abuf);
    IMAGE_FREE(bbuf);
    IMAGE_FREE(xmap);
    IMAGE_FREE(ymap);
    IMAGE_FREE(linewritten);
    return error_code;
}


IMAGELIB_ERROR_CODE image_zoom_unfiltered(IMAGELIB_SESSION* _session,IMAGE_INSTANCE *decoder_instance, WINDOW_BOX *decoder_window, IMAGE_INSTANCE *encoder_instance, WINDOW_BOX *encoder_window, Mapping *m,IMAGELIB_EFFECT effect_type,INT32 effect_para)
{
    INT32 overlap;

    /* do source and dest windows overlap? */
    overlap = decoder_instance==encoder_instance && window_box_overlap(decoder_window, encoder_window);

    return image_zoom_unfiltered_impl (_session,decoder_instance, decoder_window, encoder_instance, encoder_window, m, overlap,effect_type,effect_para);
}

 
 IMAGELIB_ERROR_CODE image_zoom_impl(IMAGELIB_SESSION* _session,IMAGE_INSTANCE *decoder_instance,WINDOW_BOX * decoder_window_p, IMAGE_INSTANCE *encoder_instance, WINDOW_BOX *encoder_window_p, Mapping *m,IMAGELIB_EFFECT effect_type,INT32 effect_para)
{
    WINDOW_BOX decoder_window, encoder_window, t;
    if(decoder_instance == NULL || encoder_instance == NULL)
		return IMAGELIB_RAM_EXECPTION;


    if (m->sx<=0. || m->sy<=0.) 
			return IMAGELIB_OTHER_ERROR;


    decoder_window = *decoder_window_p;
    encoder_window = *encoder_window_p;
    window_box_set_size(&decoder_window);
    window_box_set_size(&encoder_window);

    /*
     * find scale of filter in a space (source space)
     * when minifying, ascale=1/scale, but when magnifying, ascale=1
     */



    /* clip source and dest windows against their respective pictures */
    window_box_intersect(&decoder_window, (WINDOW_BOX *)image_get_window(decoder_instance, &t), &decoder_window);
    if (image_get_window(encoder_instance, &t)->x0 != IMAGE_UNDEFINED)
			window_box_intersect(&encoder_window, &t, &encoder_window);


    image_set_window(encoder_instance, (void *) &encoder_window);

    /* compute offsets for MAP (these will be .5 if zoom() routine was called)*/
    m->ux = encoder_window.x0-m->sx*(decoder_window.x0-.5)-m->tx;
    m->uy = encoder_window.y0-m->sy*(decoder_window.y0-.5)-m->ty;

    if (decoder_window.nx<=0 || decoder_window.ny<=0 || encoder_window.nx<=0 || encoder_window.ny<=0) 
    	return IMAGELIB_OTHER_ERROR;

    return image_zoom_unfiltered(_session,decoder_instance, &decoder_window, encoder_instance, &encoder_window, m,effect_type,effect_para);

}



IMAGELIB_ERROR_CODE image_zoom_entrance(IMAGELIB_SESSION* _session,IMAGE_INSTANCE *decoder_instance, WINDOW_BOX *decoder_window,IMAGE_INSTANCE * encoder_instance,WINDOW_BOX * encoder_window,IMAGELIB_EFFECT effect_type,INT32 effect_para)
{
    Mapping map;
    INT32 dest_w = encoder_window->nx;
    INT32 dest_h = encoder_window->ny;

    if (encoder_window->x0==IMAGE_UNDEFINED) 
    {
	return IMAGELIB_OTHER_ERROR; // some error
    }
    window_box_set_size(decoder_window);
    window_box_set_size(encoder_window);
    if (decoder_window->nx<=0 || decoder_window->ny<=0) 
		return IMAGELIB_OTHER_ERROR;
    map.sx = (double)encoder_window->nx/decoder_window->nx;
    map.sy = (double)encoder_window->ny/decoder_window->ny;


    if (map.sx>map.sy) 
     {	
	    map.sx = map.sy;
	    //encoder_window->nx = (INT32) ceil(decoder_window->nx*map.sx);
	    encoder_window->nx = (INT32) floor(decoder_window->nx*map.sx);
     }
     else 
     {
	    map.sy = map.sx;
	    //encoder_window->ny = (INT32) ceil(decoder_window->ny*map.sy);
	    encoder_window->ny = (INT32) floor(decoder_window->ny*map.sy);
     }
 
#if 0

     if (map.sx>1.) 
    {
		//    map.sx = (int) floor(map.sx);
			encoder_window->nx = (int) ceil(a->nx*map.sx);
    }
     if (map.sy>1.) 
     {
		//    map.sy = (int) floor(map.sy);
			encoder_window->ny = (int) ceil(a->ny*map.sy);
     }

#endif
    window_box_set_max(encoder_window);
    map.tx = encoder_window->x0-.5 - map.sx*(decoder_window->x0-.5);
    map.ty = encoder_window->y0-.5 - map.sy*(decoder_window->y0-.5);
   return image_zoom_impl(_session,decoder_instance, decoder_window, encoder_instance, encoder_window, &map,effect_type,effect_para);
   
}

static void make_map_table(double scale,double tran,double asupp, INT32 a0, INT32 b0, INT32 bn, INT32 overlap,INT32 * map)
{
    INT32 split, b, i0;
    double z, u;

    /* find fixed point of mapping; let split=b-b0 at the point where a=b */

    if (overlap)
     {			/* source and dest windows overlap */
			if (scale==1.)			/* no scale change, translation only */
	    /* if moving left then scan to right, and vice versa */
	    split = a0<b0 ? 0 : bn;
			else 
			{				/* magnify or minify */

	    /* THE MAGIC SPLIT FORMULA: */

	    if (scale>1.) 
	   {		
	
		split = (INT32) floor((tran+scale*asupp-.5)/(1.-scale)-b0+1.);
	    }
	    else 
	   {			
		
		split = (INT32) ceil((tran+scale*asupp)/(1.-scale)-b0);

		u = b0-scale*(a0-.5)-tran;	/* recalculate offset */
		z = MAP(split-1, scale, u);	/* cen at b=split-1 */
		z = z-asupp+.5;
		i0 = (INT32) z;				/* i0 at b=split-1 */
		if (a0+i0>=b0+split)		/* feedback at b=split-1? */
		    split--;			/* correct for roundoff */
	    }
	    if (split<0) 
			split = 0;
	    else if (split>bn) 
			split = bn;

	}

	if (scale>=1.) 
	{		/* magnify: scan in toward split */
	    for (b=0;    b<split;  b++) *map++ = b;
	    for (b=bn-1; b>=split; b--) *map++ = b;
	}
	else 
	{				/* minify: scan out away from split */
	    for (b=split-1; b>=0;  b--) *map++ = b;
	    for (b=split;   b<bn;  b++) *map++ = b;
	}
  }
    else				/* no overlap; either order will work */
	for (b=0; b<bn; b++) *map++ = b;	/* we opt for forward order */
}


