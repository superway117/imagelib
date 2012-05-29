#include <image_headfile.h>



#define BMP_READ(_buf,_len,_count,_bmp)         \
  (_bmp)->read ?                   \
    (_bmp)->read(_buf,_len,_count,_bmp) : \
    IMAGE_FILE_READ(_buf,_len,_count,(_bmp->filehandle))



#define BMP_SEEK(_bmp,_offset,_origin )        \
  (_bmp)->seek ?                   \
    (_bmp)->seek(_bmp,_offset,_origin ) : \
    IMAGE_FILE_SEEK((_bmp->filehandle),_offset,_origin)



typedef INT32 (*ReadFunc) (char* buf,INT32 len,INT32 count,void* bmp);
typedef INT32 (*SeekFunc) (void* bmp,INT32 offset,INT32 origin);



/* 2-byte, 4-byte unsigned */
typedef UINT16 WORD;
typedef UINT32 DWORD;
/* 4-byte integer */
//typedef int LONG;
/* constants for the biCompression field */
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#if defined(RED_ARROW) && defined(ARM_TARGET)
typedef struct tagRGBQUAD {
    UINT8    rgbBlue;
    UINT8    rgbGreen;
    UINT8    rgbRed;
    UINT8    rgbReserved;
} RGBQUAD;

__packed struct tag_bfi {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} ;
typedef struct tag_bfi  BITMAPFILEHEADER;
__packed struct tag_bih {
    DWORD biSize;
    INT32 biWidth;
    INT32 biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    INT32 biXPelsPerMeter;
    INT32 biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} ;
typedef struct tag_bih  BITMAPINFOHEADER;
#else
#pragma pack(push)
#pragma pack(1)

typedef struct tagRGBQUAD {
    UINT8    rgbBlue;
    UINT8    rgbGreen;
    UINT8    rgbRed;
    UINT8    rgbReserved;
} RGBQUAD;

typedef struct tag_bfi {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
typedef struct tag_bih {
    DWORD biSize;
    INT32 biWidth;
    INT32 biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    INT32 biXPelsPerMeter;
    INT32 biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop)
#endif


/*------------------------------------------------------------*/
struct tagPicBmp
{
    IMAGE_FILE_HANDLER filehandle;
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER ih;
    DWORD palette[256];
    INT32 read_not_write;
    INT32 width;
    INT32 height;
    INT32 scanline;
    void *row;
    UINT32 row_len;
    ReadFunc read;
    SeekFunc seek;

    void* userdata;
    UINT32 userdata_size;
    void *userdata_ptr;
};
typedef struct tagPicBmp PicBmp;

//static INT32 bmp_read_imagedata( void* buf, INT32 len ,INT32 count,PicBmp* bmp)
static INT32 bmp_read_imagedata (char* buf,INT32 len,INT32 count,void* bmp)
{
    INT32 size = len*count;
    PicBmp* p_bmp = (PicBmp*)bmp;
    char* ptr = NULL;
    if (p_bmp == NULL || buf == NULL)
        return -1;
    ptr = p_bmp->userdata_ptr;
    memcpy(buf, ptr, size);
    p_bmp->userdata_ptr= ptr + size;
    return size;
}
//static INT32 bmp_seek_imagedata(PicBmp* bmp, INT32 offset, INT32 origin)
static INT32 bmp_seek_imagedata(void* bmp,INT32 offset,INT32 origin)
{
    PicBmp* p_bmp = (PicBmp*)bmp;

    if (p_bmp == NULL || p_bmp->userdata == NULL)
        return -1;

    if (IMAGE_FILE_ORIGIN_SET == origin)
    {
        if (offset<0)
            return -1;
        else if ((UINT32)(offset +1) > p_bmp->userdata_size)
            return -1;
        p_bmp->userdata_ptr = (char*)p_bmp->userdata + offset;
    }
    else if (IMAGE_FILE_ORIGIN_CUR == origin)
    {
        if (((char*)p_bmp->userdata_ptr - (char*)p_bmp->userdata + offset)  <0)
            return -1;
        else if ((UINT32)((char*)p_bmp->userdata_ptr - (char*)p_bmp->userdata + offset +1)  >= p_bmp->userdata_size)
            return -1;
        p_bmp->userdata_ptr = (char*)p_bmp->userdata_ptr + offset;
    }
    return 0;
}

static BOOL bmp_read_header(PicBmp *bmp)
{
    // read BITMAPFILEHEADER
    //FILE_READ_MEMBER(fh);

    BMP_READ(((void*)&bmp->fh),(sizeof(bmp->fh)),1,bmp);
    if (bmp->fh.bfType != ('B' | ('M' << 8)))
    {

        return FALSE;
    }

    // read BITMAPINFOHEADER
    BMP_READ((void*)&bmp->ih.biSize,sizeof(bmp->ih.biSize),1,bmp);
    BMP_READ((void*)(((unsigned char *) &bmp->ih) + sizeof(bmp->ih.biSize)),
             sizeof(bmp->ih) - sizeof(bmp->ih.biSize), 1, bmp);

    if (bmp->ih.biCompression != BI_RGB)
    {

        return FALSE;
    }
    if (bmp->ih.biSize > sizeof(bmp->ih))
    {
        // skip remaining part of info header we didn't read
        BMP_SEEK(bmp, bmp->ih.biSize - sizeof(bmp->ih), IMAGE_FILE_ORIGIN_CUR);
    }
    bmp->width = bmp->ih.biWidth;
    bmp->height = bmp->ih.biHeight;
    if (bmp->height < 0)
    {
        bmp->height = -bmp->height;
    }

    // now ready to read image data, so allocate row buffer
    switch (bmp->ih.biBitCount)
    {
    case 1:
        bmp->row_len = (bmp->width + 7) >> 3;
        break;
    case 2:
        bmp->row_len = (bmp->width + 3) >> 2;
        break;
    case 4:
        bmp->row_len = (bmp->width + 1) >> 1;
        break;
    case 8:
        bmp->row_len = bmp->width;
        break;
    case 16:
        bmp->row_len = 2*bmp->width;
        break;
    case 24:
        bmp->row_len = 3*bmp->width;
        break;
    case 32:
        bmp->row_len = 4*bmp->width;
        break;

    default:
        return FALSE;
        break;
    }
    bmp->row_len = (bmp->row_len + 3) & ~3;

    // read palette for indexed formats, which comes after info header
    if (bmp->ih.biBitCount < 16)
    {
        int count = 1L << bmp->ih.biBitCount;
        int i;
        BMP_READ((void*)&bmp->palette[0], sizeof(RGBQUAD), count, bmp);

        for (i = 0; i < count; i++)
        {
            bmp->palette[i] |= (255 << 24);
        }
    }

    // skip to bits
    {

        BMP_SEEK(bmp, bmp->fh.bfOffBits, IMAGE_FILE_ORIGIN_SET);

    }


    bmp->row = IMAGE_MALLOC((UINT16)bmp->row_len);
    if(bmp->row == NULL)
	return FALSE;

    return TRUE;
}

static IMAGELIB_ERROR_CODE bmp_open_by_filename(const char *pathname, const char *mode,const void* open_para,void** data_rt)
{
    PicBmp *bmp = NULL;
    IMAGELIB_ERROR_CODE error_code = IMAGELIB_SUCCESS;
    IMAGE_FILE_HANDLER hdl=0;
    if ('r' == mode[0])
        hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_READ_MODE);
    else if ('w' == mode[0])
        hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_CREATE_WRITE_MODE);
    if (hdl>0)
    {

        IMAGE_ALLOC(bmp, PicBmp, 1);
	 	if(bmp==NULL)
	 	return IMAGELIB_RAM_NOT_ENOUGH;
        memset(bmp, 0, sizeof(PicBmp));
        bmp->filehandle = hdl;
        if ('r' == mode[0])
        {
            bmp->read_not_write = 1;
        }
        bmp->userdata= NULL;
        bmp->userdata_ptr= NULL;
        bmp->read = NULL;
        bmp->seek = NULL;

        if (bmp->read_not_write)
        {
            if (!bmp_read_header(bmp))
            {
                IMAGE_FILE_CLOSE(bmp->filehandle);
                IMAGE_FREE(bmp);
                error_code = IMAGELIB_UNKNOW_FORMAT;
                *data_rt = NULL;
                return error_code;
            }
        }
        *data_rt = bmp;

    }
    else
    {
        error_code = IMAGELIB_OPENFILE_ERROR;
        *data_rt = NULL;
    }

    return error_code;
}

static IMAGELIB_ERROR_CODE bmp_open_by_imagedata(const char *buf,UINT32 buf_size,void** data_rt)
{
    PicBmp *bmp = NULL;
    IMAGELIB_ERROR_CODE error_code = IMAGELIB_SUCCESS;
    if (buf == NULL)
    {
        *data_rt = NULL;
        return IMAGELIB_RAM_EXECPTION;
    }
    IMAGE_ALLOC(bmp, PicBmp, 1);
    if(bmp==NULL)
	 	return IMAGELIB_RAM_NOT_ENOUGH;
    memset(bmp, 0, sizeof(PicBmp));
    bmp->filehandle = 0;
    // bmp->name = NULL;

    bmp->read_not_write = 1;


    bmp->userdata= (void*)buf;
    bmp->userdata_ptr= (void*)buf;
    bmp->read = bmp_read_imagedata;
    bmp->seek = bmp_seek_imagedata;

    bmp->userdata_size = buf_size;

    if (!bmp_read_header(bmp))
    {
        IMAGE_FREE(bmp);
        error_code = IMAGELIB_UNKNOW_FORMAT;
        *data_rt = NULL;
        return error_code;
    }
    *data_rt = bmp;
    return error_code;
}



/*------------------------------------------------------------*/
static IMAGELIB_ERROR_CODE bmp_close(void *p,BOOL force)
{
    PicBmp *bmp = (PicBmp *) p;
    if (bmp == NULL)
    {
        return IMAGELIB_RAM_EXECPTION;
    }

    IMAGE_FREE(bmp->row);
    bmp->row=NULL;

    if (bmp->filehandle > 0)
    {
        IMAGE_FILE_CLOSE(bmp->filehandle);
        bmp->filehandle = 0;
    }
    IMAGE_FREE(bmp);
    return IMAGELIB_SUCCESS;
}

static IMAGELIB_ERROR_CODE bmp_get_info_by_filename(const char *pathname, IMAGELIB_INFO* info)
{
    PicBmp *bmp = NULL;
    IMAGELIB_ERROR_CODE error_code = bmp_open_by_filename(pathname,"rb",NULL,&bmp);

    if (error_code == IMAGELIB_SUCCESS)
    {
        info->width = bmp->width;
        info->height = bmp->height;
        info->bits_per_pixel = bmp->ih.biBitCount;
        info->image_type = IMAGELIB_BMP_TYPE;
        error_code = bmp_close(bmp,TRUE);

    }
    return error_code;
}

static IMAGELIB_ERROR_CODE bmp_get_info_by_imagedata(const char *buf,UINT32 buf_size,IMAGELIB_INFO* info)
{
    PicBmp *bmp =NULL;
    IMAGELIB_ERROR_CODE error_code = bmp_open_by_imagedata(buf,buf_size,&bmp);

    if (error_code == IMAGELIB_SUCCESS)
    {
        info->width = bmp->width;
        info->height = bmp->height;
        info->bits_per_pixel = bmp->ih.biBitCount;
        info->image_type = IMAGELIB_BMP_TYPE;
        error_code=bmp_close(bmp,TRUE);

    }

    return error_code;
}

static INT32 bmp_get_width(void *p)
{
    PicBmp *bmp = (PicBmp *) p;
    if (bmp  == NULL)
        return 0;
    return bmp->width;
}

static INT32 bmp_get_height(void *p)
{
    PicBmp *bmp = (PicBmp *) p;
    if (bmp  == NULL)
        return 0;
    return bmp->height;
}
static IMAGELIB_IMAGE_TYPE bmp_get_image_type(void *p)
{
    return IMAGELIB_BMP_TYPE;
}
/*------------------------------------------------------------*/
static void bmp_set_nchan(void *vp, INT32 nchan)
{

}

/*------------------------------------------------------------*/
static void bmp_set_box(void *vp, INT32 ox, INT32 oy, INT32 dx, INT32 dy)
{
    PicBmp *bmp = (PicBmp *) vp;
    if (bmp == NULL)
        return;
    if (! bmp->read_not_write)
    {
        bmp->width = ox + dx;
        bmp->height = oy + dy;
    }
}


/*------------------------------------------------------------*/
#define WRITE_INIT(picbmp_) ((picbmp_)->scanline || bmp_write_init(picbmp_))
static IMAGELIB_ERROR_CODE bmp_write_init(PicBmp *bmp)
{
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER ih;
    if(bmp->scanline >0)
	return IMAGELIB_SUCCESS;
    fh.bfType = 'B' | ('M' << 8);
    fh.bfSize = (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmp->height*bmp->row_len);
    fh.bfReserved1 = fh.bfReserved2=0;
    fh.bfOffBits =sizeof(bmp->ih) + sizeof(bmp->fh);
    memset(&ih,0,sizeof(BITMAPINFOHEADER));
    ih.biSize = sizeof(ih);
    ih.biWidth = bmp->width;
    ih.biHeight = -bmp->height;
    ih.biPlanes=1;
    ih.biBitCount=24;
    ih.biCompression = BI_RGB;


    bmp->row_len = (3*bmp->width + 3) & ~3;


    // write BITMAPFILEHEADER

    memcpy(&bmp->fh, &fh, sizeof(bmp->fh));

    IMAGE_FILE_WRITE(&bmp->fh,sizeof(bmp->fh),1,bmp->filehandle);



    // write BITMAPINFOHEADER
    memcpy(&bmp->ih, &ih, sizeof(bmp->ih));

    IMAGE_FILE_WRITE(&bmp->ih,sizeof(bmp->ih),1,bmp->filehandle);


    // no palette mode support, so don't write palette

    // now ready to write pixel data
    bmp->row = IMAGE_MALLOC(bmp->row_len);
    if(bmp->row == NULL)
		return IMAGELIB_RAM_NOT_ENOUGH;
    return IMAGELIB_SUCCESS;
}



static void bmp_seek_row_write(PicBmp *bmp, INT32 y)
{
    memset(bmp->row, 0, bmp->row_len);
    while (bmp->scanline < y)
    {
        IMAGE_FILE_WRITE(bmp->row,bmp->row_len,1,bmp->filehandle);
        bmp->scanline++;
    }
}

static IMAGELIB_ERROR_CODE bmp_write_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, const PIXEL_RGBA *buf)
{
    PicBmp *bmp = (PicBmp *) vp;
    IMAGELIB_ERROR_CODE error_code;
    if (bmp == NULL)
        return IMAGELIB_RAM_EXECPTION;
    error_code = bmp_write_init(bmp);
    if(error_code != IMAGELIB_SUCCESS)
	return error_code;
    if (! bmp->read_not_write)
    {
        INT32 i, j;
        COLOR *row = (COLOR *) bmp->row;

        bmp_seek_row_write(bmp, y);
        for (i = 0, j = x0; i < nx; i++, j += 3)
        {
            row[j+0] = buf[i].b;
            row[j+1] = buf[i].g;
            row[j+2] = buf[i].r;
        }
        IMAGE_FILE_WRITE(bmp->row,bmp->row_len,1,bmp->filehandle);
        bmp->scanline++;

    }
    return IMAGELIB_SUCCESS;
}


static INT32 bmp_get_nchan(void *vp)
{
    return 3;
}


static void bmp_get_box(void *vp, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy)
{
    PicBmp *bmp = (PicBmp *) vp;
    if (bmp == NULL)
        return;
    if (bmp->read_not_write || bmp->scanline)
    {
        *ox = 0;
        *oy = 0;
        *dx = bmp->width;
        *dy = bmp->height;
    }
    else
    {
        *ox = PIXEL_UNDEFINED;
        *oy = PIXEL_UNDEFINED;
        *dx = PIXEL_UNDEFINED;
        *dy = PIXEL_UNDEFINED;
    }
}



static void bmp_seek_row_read(PicBmp *bmp, INT32 y)
{
    if (bmp == NULL)
        return;
    if (bmp->ih.biHeight > 0)
    {
        // bottommost scanline is first in file
        INT32 count;
        size_t offset =bmp->fh.bfOffBits + (bmp->height - (y+1))*bmp->row_len;
        BMP_SEEK(bmp, offset, IMAGE_FILE_ORIGIN_SET);
        count = BMP_READ(bmp->row, 1, bmp->row_len, bmp);

        bmp->scanline = y+1;
    }
    else
    {
        // topmost scanline is first
        while (bmp->scanline <= y)
        {
            int count = BMP_READ(bmp->row, 1, bmp->row_len, bmp);

            bmp->scanline++;
        }
    }
}




static IMAGELIB_ERROR_CODE bmp_read_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA *buf)
{
    static const COLOR clut16[32] = {
        0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115,
        123, 132, 140, 148, 156, 165, 173, 181, 189, 197, 206, 214,
        222, 230, 239, 247, 255
    };
    PicBmp *bmp = (PicBmp *) vp;
    if (bmp == NULL)
        return IMAGELIB_RAM_EXECPTION;

    if (bmp->read_not_write )
    {
        INT32 i;
        INT32 j;
        COLOR *row = (COLOR *) bmp->row;

        bmp_seek_row_read(bmp, y);
        switch (bmp->ih.biBitCount)
        {
        case 1:
        case 2:
        case 4:
        case 8:
            for (i = 0, j = x0; i < nx; i++, j++)
            {
                const DWORD pixel = bmp->palette[row[j]];
                buf[i].b = (COLOR) pixel & 0xFF;
                buf[i].g = (COLOR) (pixel >> 8) & 0xFF;
                buf[i].r = (COLOR) (pixel >> 16) & 0xFF;
                buf[i].a = (COLOR) (pixel >> 24) & 0xFF;
            }
            break;

        case 16:
            for (i = 0, j = x0; i < nx; i++, j += 2)
            {
                buf[i].b = clut16[row[j+0] & 31];
                buf[i].g = clut16[((row[j+1] & 3) << 3) + ((row[j+0] >> 5) & 7)];
                buf[i].r = clut16[(row[j+1] >> 2) & 31];
                buf[i].a = 255;
            }
            break;

        case 24:
            for (i = 0, j = x0; i < nx; i++, j += 3)
            {
                buf[i].r = row[j+2];
                buf[i].g = row[j+1];
                buf[i].b = row[j+0];
                buf[i].a = 255;
            }
            break;
        case 32:
            for (i = 0, j = x0; i < nx; i++, j += 4)
            {
                buf[i].r = row[j+2];
                buf[i].g = row[j+1];
                buf[i].b = row[j+0];
                buf[i].a = 255;
            }
            break;

        }
        return IMAGELIB_SUCCESS;
    }
    return IMAGELIB_OTHER_ERROR;
}


static INT32 bmp_next_pic(void *vp)
{
    return 0;
}

static void* bmp_get_result(void *vp)
{
    return 0;

}


/*------------------------------------------------------------*/
static IMAGE_PROCS bmp_procs = {
    bmp_get_info_by_filename,
    bmp_get_info_by_imagedata,
    bmp_open_by_filename,
    bmp_open_by_imagedata,
    bmp_close,

    bmp_get_width,
    bmp_get_height,
    bmp_get_image_type,

    bmp_set_nchan,
    bmp_set_box,

    bmp_write_row_rgba,

    bmp_get_nchan,
    bmp_get_box,

    bmp_read_row_rgba,

    bmp_next_pic,

    bmp_get_result
};

/*------------------------------------------------------------*/
IMAGE_INSTANCE bmp_instance =
{
    "bmp", &bmp_procs
};


