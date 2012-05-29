#include <image_headfile.h>

#include <png.h>



/*------------------------------------------------------------*/
struct tagPicPng
{
    IMAGE_FILE_HANDLER filehandle;
    char* userdata;
    char *userdata_ptr;
    INT32 read_not_write;
    png_uint_32 width;
    png_uint_32 height;
    png_structp png_ptr;
    png_infop info_ptr;
    INT32 bit_depth;
    INT32 color_type;
    INT32 pChannels;
    INT32 interlace_type;
    UINT8 bg_r;
    UINT8 bg_g;
    UINT8 bg_b;
    png_bytep row;
    INT32 scanline;
//  jmp_buf  jmpbuf;
};
typedef struct tagPicPng PicPng;



static void pngwrap_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
    if (png_ptr);
    {

        longjmp(png_ptr->jmpbuf,1);
    }
}

static png_voidp pngwrap_malloc(png_structp png_struct, png_size_t size)
{
    png_voidp pNew;
    pNew = IMAGE_MALLOC(size);
	if(pNew == NULL)
		printf("ram error");
    return pNew;
}	// PngLibraryMallocFunction


static void pngwrap_free(png_structp png_struct, png_voidp ptr)
{

    IMAGE_FREE(ptr);

}
static void pngwrap_read(png_structp png_struct, png_bytep buffer, png_size_t length)
{
    png_size_t check;
    PicPng *png  = (PicPng*)png_get_io_ptr(png_struct);
    if (png->filehandle > 0)
    {
        check = (png_size_t)IMAGE_FILE_READ(buffer,(png_size_t) length,1,png->filehandle);
    }
    else  if (png->userdata!= NULL)
    {
        memcpy(buffer,png->userdata_ptr,length);
        png->userdata_ptr+=length;
    }
}
static void pngwrap_write(png_structp png_struct, png_bytep buffer, png_size_t length)
{
    png_size_t check;
    PicPng *png  = (PicPng*)png_get_io_ptr(png_struct);
    if (png->filehandle > 0)
    {
        check = (png_size_t)IMAGE_FILE_WRITE(buffer,(png_size_t) length,1,png->filehandle);
    }

}


static BOOL pngwrap_get_bgcolor(PicPng *png)
{
    png_color_16p pBackground;


    /* setjmp() must be called in every function that calls a PNG-reading
     * libpng function */

    if (setjmp(png->png_ptr->jmpbuf))
    {
        png_destroy_read_struct(&png->png_ptr, &png->info_ptr, NULL);
        return FALSE;
    }
    else
    {
        if (!png_get_valid(png->png_ptr, png->info_ptr, PNG_INFO_bKGD))
            return TRUE;	//no bKGD chunk,

        /* it is not obvious from the libpng documentation, but this function
         * takes a pointer to a pointer, and it always returns valid red, green
         * and blue values, regardless of color_type: */

        png_get_bKGD(png->png_ptr, png->info_ptr, &pBackground);


        /* however, it always returns the raw bKGD data, regardless of any
         * bit-depth transformations, so check depth and adjust if necessary */

        if (png->color_type == PNG_COLOR_TYPE_GRAY && png->bit_depth < 8)
        {
            if (png->bit_depth == 1)
                png->bg_r= png->bg_g = png->bg_b = pBackground->gray? 255 : 0;
            else if (png->bit_depth == 2)
                png->bg_r= png->bg_g = png->bg_b = (255/3) * pBackground->gray;
            else /* bit_depth == 4 */
                png->bg_r= png->bg_g = png->bg_b = (255/15) * pBackground->gray;
        }
        else
        {
            png->bg_r   = (UINT8)pBackground->red;
            png->bg_g = (UINT8)pBackground->green;
            png->bg_b  = (UINT8)pBackground->blue;
        }
        return TRUE;
    }

}


static IMAGELIB_ERROR_CODE pngwrap_read_init(PicPng *png)
{


    UINT16 rowbytes = 0;
    if (png == NULL)
        return IMAGELIB_RAM_EXECPTION;
    if (setjmp(png->png_ptr->jmpbuf))
        return IMAGELIB_RAM_EXECPTION;
    else
    {
        png_read_info(png->png_ptr, png->info_ptr);
        png_get_IHDR(png->png_ptr, png->info_ptr,
                     &png->width, &png->height, &png->bit_depth,
                     &png->color_type, &png->interlace_type, NULL, NULL);
        png->pChannels = (INT32)png_get_channels(png->png_ptr, png->info_ptr);
        if (png->color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png->png_ptr);
        if (png->color_type == PNG_COLOR_TYPE_GRAY && png->bit_depth < 8)
            png_set_expand(png->png_ptr);
        if (png_get_valid(png->png_ptr,png->info_ptr, PNG_INFO_tRNS))
            png_set_expand(png->png_ptr);
        if (png->bit_depth == 16)
            png_set_strip_16(png->png_ptr);
        if (png->color_type == PNG_COLOR_TYPE_GRAY || png->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png->png_ptr);

        //	png_set_interlace_handling(png->png_ptr);
        png_read_update_info(png->png_ptr, png->info_ptr);
        png_get_IHDR(png->png_ptr, png->info_ptr,
                     &png->width, &png->height, &png->bit_depth,
                     &png->color_type, &png->interlace_type, NULL, NULL);
        pngwrap_get_bgcolor(png);
        rowbytes = (UINT16)png_get_rowbytes(png->png_ptr, png->info_ptr);

        png->row = (png_bytep)IMAGE_MALLOC(rowbytes);
	 	if(png->row == NULL)
	 		return IMAGELIB_RAM_NOT_ENOUGH;

        return IMAGELIB_SUCCESS;
    }




}

static IMAGELIB_ERROR_CODE pngwrap_openpng(PicPng *png)
{

    if (png == NULL)
        return IMAGELIB_RAM_EXECPTION;
    
//Try
    {
        png->scanline = 0;
        png->png_ptr = (png->read_not_write ?
                        png_create_read_struct_2 : png_create_write_struct_2)
                       (PNG_LIBPNG_VER_STRING, NULL, pngwrap_cexcept_error, NULL,png,pngwrap_malloc,pngwrap_free);
        if ( png->read_not_write ==1)
            png_set_read_fn(png->png_ptr, png, pngwrap_read);
        else
            png_set_write_fn(png->png_ptr, png, pngwrap_write,NULL);//(png_structp png_ptr, png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn)

        assert(png->png_ptr);
        png->info_ptr = png_create_info_struct(png->png_ptr);
        assert(png->info_ptr);
        if (png->read_not_write)
        {
            return (pngwrap_read_init(png));
        }
        else
        {
            png_set_compression_mem_level(png->png_ptr,4);
            png_set_compression_window_bits(png->png_ptr,8);
        }
        return IMAGELIB_SUCCESS;
    }

}


static IMAGELIB_ERROR_CODE pngwrap_open_by_imagedata(const char *buf,UINT32 buf_size,void** data_rt)
{
    PicPng *png = NULL;
    IMAGELIB_ERROR_CODE error_code = IMAGELIB_SUCCESS;

    if (buf)
    {
        IMAGE_ALLOC(png, PicPng, 1);
	 if(png == NULL)
	 	return IMAGELIB_RAM_NOT_ENOUGH;
        memset(png, 0, sizeof(PicPng));
        png->filehandle = 0;
        png->userdata= (char *)buf;
        png->userdata_ptr = png->userdata;
        png->read_not_write = 1;
	 error_code = pngwrap_openpng(png);
        if (IMAGELIB_SUCCESS==error_code)
            *data_rt = png;
        else
        {
            if (png->read_not_write)
                png_destroy_read_struct(&png->png_ptr, &png->info_ptr, NULL);
            else
                png_destroy_write_struct(&png->png_ptr, &png->info_ptr);
            if (png->row)
                IMAGE_FREE(png->row);
            IMAGE_FREE(png);
            *data_rt = NULL;
           
        }

    }
    else
    {
        *data_rt = NULL;
        error_code= IMAGELIB_RAM_EXECPTION;
    }

    return error_code;
}


/*------------------------------------------------------------*/

static IMAGELIB_ERROR_CODE pngwrap_open_by_filename(const char *pathname, const char *mode,const void* open_para,void** data_rt)
{
    PicPng *png = NULL;
    IMAGELIB_ERROR_CODE error_code = IMAGELIB_SUCCESS;
    IMAGE_FILE_HANDLER hdl = 0;

    if ('r' == mode[0])
        hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_READ_MODE);
    else if ('w' == mode[0])
        hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_CREATE_WRITE_MODE);

    if (hdl>0)
    {
        IMAGE_ALLOC(png, PicPng, 1);
	 	if(png == NULL)
	 		return IMAGELIB_RAM_NOT_ENOUGH;
        memset(png, 0, sizeof(PicPng));
        png->filehandle = hdl;
        png->userdata= NULL;
        if ('r' == mode[0])
            png->read_not_write = 1;
        else
            png->read_not_write = 0;
	 	error_code = pngwrap_openpng(png);
        if (IMAGELIB_SUCCESS==error_code)
            *data_rt = png;
        else
        {
            if (png->read_not_write)
                png_destroy_read_struct(&png->png_ptr, &png->info_ptr, NULL);
            else
                png_destroy_write_struct(&png->png_ptr, &png->info_ptr);
            if (png->row)
                IMAGE_FREE(png->row);
            if (png->filehandle>0)
                IMAGE_FILE_CLOSE(png->filehandle);
            IMAGE_FREE(png);
            *data_rt = NULL;
        }
    }
    else
    {
        error_code = IMAGELIB_OPENFILE_ERROR;
        *data_rt = NULL;
    }

    return error_code;
}

/*------------------------------------------------------------*/
static IMAGELIB_ERROR_CODE pngwrap_close(void *p,BOOL force)
{
    PicPng *png = (PicPng *) p;
    if (png == NULL)
    {
        return IMAGELIB_RAM_EXECPTION;
    }
    if (png->read_not_write)
        png_destroy_read_struct(&png->png_ptr, &png->info_ptr, NULL);
    else
        png_destroy_write_struct(&png->png_ptr, &png->info_ptr);


    if (png->filehandle>0)
        IMAGE_FILE_CLOSE(png->filehandle);
    if (png->row)
        IMAGE_FREE(png->row);
    if (png)
        IMAGE_FREE(png);
    return IMAGELIB_SUCCESS;
}

static IMAGELIB_ERROR_CODE pngwrap_get_info_by_filename(const char *pathname, IMAGELIB_INFO* info)
{
    PicPng* png = NULL;
    IMAGELIB_ERROR_CODE error_code  = pngwrap_open_by_filename(pathname,"rb",NULL,&png);

    if (error_code == IMAGELIB_SUCCESS)
    {

        info->width = png->width;
        info->height = png->height;
        info->bits_per_pixel = png->info_ptr->pixel_depth;
        info->image_type = IMAGELIB_PNG_TYPE;
        error_code = pngwrap_close(png,TRUE);

    }
    return error_code;
}
static IMAGELIB_ERROR_CODE pngwrap_get_info_by_imagedata(const char *buf,UINT32 buf_size,IMAGELIB_INFO* info)
{
    PicPng* png = NULL;
    IMAGELIB_ERROR_CODE error_code  = pngwrap_open_by_imagedata(buf,buf_size,&png);

    if (error_code == IMAGELIB_SUCCESS)
    {
        info->width = png->width;
        info->height = png->height;
        //	info->bits_per_pixel = png->bit_depth;
        info->bits_per_pixel = png->info_ptr->pixel_depth;
        info->image_type = IMAGELIB_PNG_TYPE;
        error_code=pngwrap_close(png,TRUE);

    }

    return error_code;
}


static INT32 pngwrap_get_width(void *p)
{
    PicPng *png = (PicPng *) p;
    if (png == NULL)
        return 0;
    return png->width;
}

static INT32 pngwrap_get_height(void *p)
{
    PicPng *png = (PicPng *) p;
    if (png == NULL)
        return 0;
    return png->height;
}

static IMAGELIB_IMAGE_TYPE pngwrap_get_image_type(void *p)
{
    return IMAGELIB_PNG_TYPE;
}
/*------------------------------------------------------------*/
static void pngwrap_set_nchan(void *vp, INT32 nchan)
{

}

/*------------------------------------------------------------*/
static void pngwrap_set_box(void *vp, INT32 ox, INT32 oy, INT32 dx, INT32 dy)
{
    PicPng *png = (PicPng *) vp;
    if (png == NULL)
        return;
    if (! png->read_not_write)
    {
        png->width = ox + dx;
        png->height = oy + dy;
    }
}



/*------------------------------------------------------------*/
#define WRITE_INIT(png) ((png)->scanline || pngwrap_write_init(png))
static INT32 pngwrap_write_init(PicPng *png)
{
    png->bit_depth = 8;

    png->color_type = PNG_COLOR_TYPE_RGB;

    png_set_IHDR(png->png_ptr, png->info_ptr, png->width, png->height,
                 png->bit_depth, png->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png->png_ptr, png->info_ptr);
    png->row = (png_bytep)IMAGE_MALLOC((UINT16)png_get_rowbytes(png->png_ptr, png->info_ptr));
    assert(png->row);

    return 1;
}

/*------------------------------------------------------------*/
static void pngwrap_seek_row_write(PicPng *png, int y)
{
    memset(png->row, 0, png_get_rowbytes(png->png_ptr, png->info_ptr));
    while (png->scanline < y)
    {
        png_write_rows(png->png_ptr, &png->row, 1);
        png->scanline++;
    }
}



/*------------------------------------------------------------*/
static IMAGELIB_ERROR_CODE pngwrap_write_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, const PIXEL_RGBA *buf)
{
    PicPng *png = (PicPng *) vp;
    if (png == NULL)
        return IMAGELIB_RAM_EXECPTION;

    if (! png->read_not_write &&  WRITE_INIT(png))
    {
        INT32 i, j;

        pngwrap_seek_row_write(png, y);
        for (i = 0, j = x0; i < nx; i++, j += 3)
        {
            png->row[j+0] = buf[i].r;
            png->row[j+1] = buf[i].g;
            png->row[j+2] = buf[i].b;
        }
        png_write_row(png->png_ptr, png->row);
        png->scanline++;

    }
    return IMAGELIB_SUCCESS;
}

/*------------------------------------------------------------*/
static INT32 pngwrap_get_nchan(void *vp)
{

    return 3;
}

/*------------------------------------------------------------*/
static void pngwrap_get_box(void *vp, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy)
{
    PicPng *png = (PicPng *) vp;
    if (png==NULL)
        return;

    if (png->read_not_write || png->scanline)
    {
        *ox = 0;
        *oy = 0;
        *dx = png->width;
        *dy = png->height;
    }
    else
    {
        *ox = PIXEL_UNDEFINED;
        *oy = PIXEL_UNDEFINED;
        *dx = PIXEL_UNDEFINED;
        *dy = PIXEL_UNDEFINED;
    }
}



/*------------------------------------------------------------*/
static void pngwrap_seek_row_read(PicPng *png, INT32 y)
{
    if (setjmp(png->png_ptr->jmpbuf))
        return;
    if (png == NULL)
        return;
    while (png->scanline <= y)
    {
        //  png_read_rows(png->png_ptr, &png->row, NULL, 1);
        png_read_row(png->png_ptr, png->row, png_bytep_NULL);
        png->scanline++;
    }
}


/*------------------------------------------------------------*/
#define alpha_composite(composite, fg, alpha, bg) {               \
    UINT16 temp = ((UINT16)(fg)*(UINT16)(alpha) +                          \
                (UINT16)(bg)*(UINT16)(255 - (UINT16)(alpha)) + (UINT16)128);  \
    (composite) = (UINT8)((temp + (temp >> 8)) >> 8);               \
}


static IMAGELIB_ERROR_CODE pngwrap_read_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA *buf)
{
    PicPng *png = (PicPng *) vp;
    if (png == NULL)
        return IMAGELIB_RAM_EXECPTION;
    if (png->read_not_write)
    {
        INT32 i;
        INT32 j;
	 if (setjmp(png->png_ptr->jmpbuf))
    	 {
        	return  IMAGELIB_RAM_EXECPTION;
    	 }
        pngwrap_seek_row_read(png, y);

        if (PNG_COLOR_TYPE_RGB_ALPHA == png->color_type)
        {
            for (i = 0, j = x0; i < nx; i++)
            {
                buf[i].a = png->row[j+3];
                if (buf[i].a == 255)
                {
                    buf[i].r = png->row[j+0];
                    buf[i].g = png->row[j+1];
                    buf[i].b = png->row[j+2];
                }
                else if (buf[i].a == 0)
                {
                    buf[i].r = png->bg_r;
                    buf[i].g  = png->bg_g;
                    buf[i].b = png->bg_b;
                }
                else
                {
                    /* this macro (copied from png.h) composites the
                     * foreground and background values and puts the
                     * result into the first argument; there are no
                     * side effects with the first argument */
                    alpha_composite(buf[i].b, png->row[j+2], buf[i].a , png->bg_b);
                    alpha_composite(buf[i].g, png->row[j+1], buf[i].a , png->bg_g);
                    alpha_composite(buf[i].r, png->row[j+0], buf[i].a , png->bg_r);
                }
                j+=4;
            }
        }
        else
        {
            for (i = 0, j = x0; i < nx; i++)
            {
                buf[i].r = png->row[j+0];
                buf[i].g = png->row[j+1];
                buf[i].b = png->row[j+2];
                buf[i].a = 255;
                j+=3;

            }
        }
        return IMAGELIB_SUCCESS;

    }
    return IMAGELIB_OTHER_ERROR;

}

/*------------------------------------------------------------*/
static INT32 pngwrap_next_pic(void *vp)
{
    return 0;
}

static void* pngwrap_get_result(void *vp)
{
    return NULL;

}
/*------------------------------------------------------------*/
static IMAGE_PROCS pngwrap_procs =
{
    pngwrap_get_info_by_filename,
    pngwrap_get_info_by_imagedata,
    pngwrap_open_by_filename,
    pngwrap_open_by_imagedata,
    pngwrap_close,

    pngwrap_get_width,
    pngwrap_get_height,
    pngwrap_get_image_type,


    pngwrap_set_nchan,
    pngwrap_set_box,

    pngwrap_write_row_rgba,

    pngwrap_get_nchan,
    pngwrap_get_box,

    pngwrap_read_row_rgba,
    pngwrap_next_pic,
    pngwrap_get_result
};

/*------------------------------------------------------------*/
IMAGE_INSTANCE png_instance =
{
    "png", &pngwrap_procs
};

