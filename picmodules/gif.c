#include <image_headfile.h>

#include <string.h>


#include "gif_lib.h"

#define GIF_MAX_MEMORY_SIZE_SUPPORT 8192//40960

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif


#define GIF_TRANSPARENT		0x01
#define GIF_USER_INPUT			0x02
#define GIF_DISPOSE_MASK		0x07
#define GIF_DISPOSE_SHIFT		2

#define GIF_NOT_TRANSPARENT	-1

#define GIF_DISPOSE_NONE	0		// No disposal specified. The decoder is
// not required to take any action.
#define GIF_DISPOSE_LEAVE	1		// Do not dispose. The graphic is to be left
// in place.
#define GIF_DISPOSE_BACKGND	2		// Restore to background color. The area used by the
// graphic must be restored to the background color.

#define GIF_DISPOSE_RESTORE	3		// Restore to previous. The decoder is required to
// restore the area overwritten by the graphic with
// what was there prior to rendering the graphic.

// Netscape 2.0 looping extension block
GifByteType  szNetscape20ext[] = "\x0bNETSCAPE2.0";

#define NSEXT_LOOP      0x01        // Loop Count field code


const INT32 InterlacedOffset[] = { 0, 4, 2, 1 }; /* The way Interlaced image should. */
const INT32 InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */


static INT32 gif_next_pic_impl(void *vp);

static INT32 gif_readFunc(GifFileType* GifFile, GifByteType* buf, int count)
{
    char* ptr = GifFile->UserData;
    memcpy(buf, ptr, count);
    GifFile->UserData = ptr + count;
    return count;
}

struct tagGif
{
    char *name;
    GifFileType* m_pGifFile;

    GifPixelType* 	m_current_frame_buf;
    GifPixelType* 	m_next_frame_buf;

    INT32 m_rgbTransparent;
    INT32 m_rgbBackgnd;

    UINT32 m_iImageNum;
    UINT32 m_uLoopCount;               // Netscape 2.0 loop count
    UINT8 read_not_write;
 //   INT32  num_channels;
};
typedef struct tagGif Gif;

static IMAGELIB_ERROR_CODE gif_decode_open(void *p)
{
    Gif *gif = (Gif *) p;
    UINT32 Size;
    IMAGELIB_ERROR_CODE error_code = IMAGELIB_SUCCESS;
    if(gif == NULL)
	 return IMAGELIB_RAM_EXECPTION;

    if (gif->m_pGifFile)
    {
        const int cxScreen =  gif->m_pGifFile->SWidth;
        const int cyScreen = gif->m_pGifFile->SHeight;
        Size = cxScreen*cyScreen*sizeof(GifPixelType);
	 if(Size > GIF_MAX_MEMORY_SIZE_SUPPORT)
	 {
	 	DGifCloseFile(gif->m_pGifFile);
    		IMAGE_FREE(gif);
		error_code = IMAGELIB_RAM_NOT_ENOUGH;
	 	return error_code;
	 }
	 gif->m_current_frame_buf = (GifPixelType*)IMAGE_MALLOC(Size);
        gif->m_next_frame_buf = (GifPixelType*)IMAGE_MALLOC(Size);
	 if(gif->m_current_frame_buf ==NULL || gif->m_next_frame_buf==NULL)
	 {
	 	DGifCloseFile(gif->m_pGifFile);
	 	IMAGE_FREE(gif->m_current_frame_buf );
		IMAGE_FREE(gif->m_next_frame_buf);
	 	return IMAGELIB_RAM_NOT_ENOUGH;
	 }
	


        memset(gif->m_next_frame_buf, gif->m_pGifFile->SBackGroundColor,Size);


        if (gif->m_pGifFile->SColorMap)
        {

            gif->m_rgbTransparent=gif->m_rgbBackgnd = gif->m_pGifFile->SBackGroundColor;

        }

        gif->m_iImageNum = 0;
        gif->m_uLoopCount = 0U;
     //   gif->num_channels = 3;
        gif_next_pic_impl(gif);


    }
    else
    {
    	IMAGE_FREE(gif);
    	error_code = IMAGELIB_OPENFILE_ERROR;
    }
		
    return error_code;
}

static IMAGELIB_ERROR_CODE gif_open_by_filename(const char *filename, const char *mode,const void* open_para,void** data_rt)
{
    Gif *gif = NULL;
    IMAGELIB_ERROR_CODE error_code;
    IMAGE_ALLOC(gif, Gif, 1);
	if(gif == NULL)
		return IMAGELIB_RAM_NOT_ENOUGH;
 //   if('r' == mode[0])
		gif->m_pGifFile = DGifOpenFileName(filename);
  //  else if('w' == mode[0])
//		gif->m_pGifFile = EGifOpenFileName(filename,FALSE);
    if(gif->m_pGifFile == NULL)
    	{
    		IMAGE_FREE(gif);
		return IMAGELIB_OPENFILE_ERROR;
    	}
    error_code =  gif_decode_open(gif);
    if(error_code == IMAGELIB_SUCCESS)
	*data_rt=gif;
    return error_code;
}

static IMAGELIB_ERROR_CODE gif_open_by_imagedata(const char *buf,UINT32 buf_size,void** data_rt)
{
    Gif *gif = NULL;
    IMAGELIB_ERROR_CODE error_code;
    IMAGE_ALLOC(gif, Gif, 1);
	if(gif == NULL)
		return IMAGELIB_RAM_NOT_ENOUGH;
    gif->m_pGifFile = DGifOpen((void*)buf,gif_readFunc);
     if(gif->m_pGifFile == NULL)
     {
     		IMAGE_FREE(gif);
		return IMAGELIB_OPENFILE_ERROR;
     }
    error_code =  gif_decode_open(gif);
     if(error_code == IMAGELIB_SUCCESS)
	*data_rt=gif;
    return error_code;
}


static IMAGELIB_ERROR_CODE gif_close(void *p,BOOL force)
{
    Gif *gif = (Gif *) p;
    if(gif == NULL)
		return IMAGELIB_RAM_EXECPTION;
	
    if(gif->m_next_frame_buf)
    	IMAGE_FREE(gif->m_next_frame_buf);
    if(gif->m_current_frame_buf)
    	IMAGE_FREE(gif->m_current_frame_buf);
    

    DGifCloseFile(gif->m_pGifFile);
    IMAGE_FREE(gif);
	return IMAGELIB_SUCCESS;

}

static IMAGELIB_ERROR_CODE gif_get_info_by_filename(const char *pathname, IMAGELIB_INFO* info)
{
	GifFileType* gif_file=NULL;


    	gif_file = DGifOpenFileName(pathname);
	if(gif_file == NULL)
		return IMAGELIB_OPENFILE_ERROR;
	info->width = gif_file->SWidth;
	info->height = gif_file->SHeight;
	info->bits_per_pixel = 8;
	info->image_type = IMAGELIB_GIF_TYPE;
	
	DGifCloseFile(gif_file);
	
	return IMAGELIB_SUCCESS;
}

static IMAGELIB_ERROR_CODE gif_get_info_by_imagedata(const char *buf,UINT32 buf_size,IMAGELIB_INFO* info)
{
	GifFileType* gif_file=NULL;

	
       gif_file = DGifOpen((void*)buf,gif_readFunc);
	if(gif_file == NULL)
		return IMAGELIB_OPENFILE_ERROR;

	info->width = gif_file->SWidth;
	info->height = gif_file->SHeight;
	info->bits_per_pixel = 8;
	info->image_type = IMAGELIB_GIF_TYPE;

	DGifCloseFile(gif_file);
		
	return IMAGELIB_SUCCESS;
}

static INT32 gif_get_width(void *p)
{
    Gif *gif = (Gif *) p;
    if(gif == NULL)
		return 0;
    return gif->m_pGifFile->SWidth;

}

static INT32 gif_get_height(void *p)
{
    Gif *gif = (Gif *) p;
    if(gif == NULL)
		return 0;
    return gif->m_pGifFile->SHeight;
}

static IMAGELIB_IMAGE_TYPE gif_get_image_type(void *p)
{
    return IMAGELIB_GIF_TYPE;
}
static void gif_set_nchan(void *vp, INT32 nchan)
{

}

static void gif_set_box(void *vp, INT32 ox,INT32 oy, INT32 dx, INT32 dy)
{
    Gif *gif = (Gif *) vp;
    if (! gif->read_not_write)
    {
        gif->m_pGifFile->SWidth = ox + dx;
        gif->m_pGifFile->SHeight = oy + dy;
    }
}

static IMAGELIB_ERROR_CODE gif_write_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, const PIXEL_RGBA *buf)
{
	return IMAGELIB_OTHER_ERROR;
}

static INT32 gif_get_nchan(void *vp)
{
    return 3;
}

static void gif_get_box(void *vp, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy)
{
    Gif *gif = (Gif *) vp;

    *ox = 0;
    *oy = 0;
    *dx = gif->m_pGifFile->SWidth;
    *dy = gif->m_pGifFile->SHeight;
}


static IMAGELIB_ERROR_CODE gif_read_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA *buf)
{
    Gif *gif = (Gif *) vp;
    GifPixelType* GifRow;

    ColorMapObject* ColorMap;

    GifColorType *ColorMapEntry;
    INT32 i;
    UINT32 row_size =0;
    if(gif == NULL)
		return IMAGELIB_RAM_EXECPTION;
	
     row_size = gif->m_pGifFile->SWidth * sizeof(GifPixelType);

    ColorMap = (gif->m_pGifFile->Image.ColorMap
                ? gif->m_pGifFile->Image.ColorMap
                : gif->m_pGifFile->SColorMap);


    GifRow = &gif->m_current_frame_buf[y*row_size+x0];

    for (i = 0; i < nx; i++)
    {
        ColorMapEntry = &ColorMap->Colors[GifRow[i]];

        buf[i].r = ColorMapEntry->Red;
        buf[i].g = ColorMapEntry->Green;
        buf[i].b = ColorMapEntry->Blue;
    }
   return IMAGELIB_SUCCESS;
}

//#define GIF_ROW_PAD(x)		(((x) + 3) & ~3)
#define MAKEWORD(low, high) ((UINT16)(((UINT8)(low)) | \
				       ((UINT16)((UINT8)(high))) << 8))
#define MAKELONG(low, high) ((UINT32)(((UINT16)(low)) | \
				       ((UINT32)((UINT16)(high))) << 16))

static INT32 gif_copy_frame_line(GifPixelType * dest, GifPixelType *src,UINT32 width,INT32 transparent)
{
    UINT32 i;
    for (i=0;i<width;i++)
    {
        GifPixelType b = *src++;
        if (b != transparent)
            *dest = b;
        dest++;
    }
    return 1;
}

// Fix pixels in 24-bit GIF buffer
static void gif_fill_frame_line(GifPixelType* pDst, const INT32 rgb, INT32 width)
{
    if (width)
    {
        do
        {

            *pDst++ = (GifPixelType)rgb;
        }
        while (--width);
    }
}

static INT32 gif_next_pic_impl(void *vp)
{
    Gif *gif = (Gif *) vp;
    UINT32 cxScreen;
    UINT32 cyScreen;
    UINT32 dwRowBytes;
    UINT32 dwScreen;


    GifRecordType RecordType;
    GifByteType *pExtension;
    UINT32 delay = 10;     // Default to 100 msec
    INT32 dispose = 0;
    int transparent=GIF_NOT_TRANSPARENT;
    GifPixelType *pLine = NULL;

    // Error if no gif file!
    if (gif == NULL || gif->m_pGifFile== NULL )
    {
        return -1;
    }

    cxScreen =  gif->m_pGifFile->SWidth;
    cyScreen = gif->m_pGifFile->SHeight;
    dwRowBytes = cxScreen * sizeof(GifPixelType);//GIF_ROW_PAD(cxScreen * 3);

#define XYOFFSET(x,y)	((y) * dwRowBytes + (x) * sizeof(GifPixelType))

    dwScreen = dwRowBytes * cyScreen;

    do {
        int  ExtCode;

        if (DGifGetRecordType(gif->m_pGifFile, &RecordType) == GIF_ERROR)
        {
            break;
        }
        switch (RecordType)
        {
        case IMAGE_DESC_RECORD_TYPE:
            if (DGifGetImageDesc(gif->m_pGifFile) != GIF_ERROR)
            {
                const GifWord left= gif->m_pGifFile->Image.Left;
                const GifWord top = gif->m_pGifFile->Image.Top;

                const GifWord Width = gif->m_pGifFile->Image.Width;
                const GifWord Height = gif->m_pGifFile->Image.Height;
                GifWord i,j;

                pLine = (GifPixelType*)IMAGE_MALLOC((UINT16)(Width * sizeof(GifPixelType)));
		  if(pLine == NULL)
		  	return 0;


                ++gif->m_iImageNum;
                memcpy(gif->m_current_frame_buf, gif->m_next_frame_buf, dwScreen);

                if (gif->m_pGifFile->Image.Interlace)
                {
                    /* Need to perform 4 passes on the images: */
                    for (i = 0; i < 4; i++)
                    {
                        for (j = top + InterlacedOffset[i]; j < top + Height;j += InterlacedJumps[i])
                        {
                            if (DGifGetLine(gif->m_pGifFile, pLine,Width) == GIF_ERROR)
                            {
                            	IMAGE_FREE(pLine);
					return 0;
                            //    exit(EXIT_FAILURE);
                            }
                            gif_copy_frame_line(gif->m_current_frame_buf+XYOFFSET(left,j ),pLine,Width,transparent);

                        }
                    }

                }
                else
                {
                    for (i = 0; i < Height; i++)
                    {

                        if (DGifGetLine(gif->m_pGifFile, pLine,Width) == GIF_ERROR)
                        {
				IMAGE_FREE(pLine);
				return 0;
                           // exit(EXIT_FAILURE);
                        }
                        gif_copy_frame_line(gif->m_current_frame_buf+XYOFFSET(left,(top+i)),pLine,Width,transparent);

                    }

                }
                if (dispose == GIF_DISPOSE_BACKGND)
                {

                    // Clear next image to background index
                    // Note: if transparent restore to transparent color (else use GIF background color)
                    const INT32 rgbFill = (transparent == GIF_NOT_TRANSPARENT) ? gif->m_rgbBackgnd : gif->m_rgbTransparent;
                    INT32 i=0;
                    for (i = 0; i < Height; ++i)
                        gif_fill_frame_line(gif->m_next_frame_buf + XYOFFSET(left,top+i), rgbFill, Width);
                }
                else if (dispose != GIF_DISPOSE_RESTORE)
                {
                    // Copy current -> next (Update)
                    memcpy(gif->m_next_frame_buf, gif->m_current_frame_buf, dwScreen);
                }
                dispose = 0;
		  IMAGE_FREE(pLine);
                if (delay)
                {
                    return delay * 10;
                }
            }
            break;
        case EXTENSION_RECORD_TYPE:
        {
            BOOL bNetscapeExt = FALSE;
            if (DGifGetExtension(gif->m_pGifFile, &ExtCode, &pExtension) == GIF_ERROR)
            {

                return -2;
            }


            switch (ExtCode)
            {
            case COMMENT_EXT_FUNC_CODE:

                break;
            case GRAPHICS_EXT_FUNC_CODE:
            {
                int flag = pExtension[1];
                delay  = MAKEWORD(pExtension[2], pExtension[3]);
                transparent = (flag & GIF_TRANSPARENT) ? pExtension[4] : GIF_NOT_TRANSPARENT;
                dispose = (flag >> GIF_DISPOSE_SHIFT) & GIF_DISPOSE_MASK;

        
                break;
            }
            case PLAINTEXT_EXT_FUNC_CODE:

                break;
            case APPLICATION_EXT_FUNC_CODE:
            {
         

                if (memcmp(pExtension, szNetscape20ext, szNetscape20ext[0]) == 0)
                {

                    bNetscapeExt = TRUE;
                }
                break;
            }
            default:

                break;
            }
      
            do
            {
                if (DGifGetExtensionNext(gif->m_pGifFile, &pExtension) == GIF_ERROR)
                {
    
                    return -3;
                }
#if 1
                // Process Netscape 2.0 extension (GIF looping)
                if (pExtension && bNetscapeExt)
                {
                    GifByteType bLength = pExtension[0];
                    int iSubCode = pExtension[1] & 0x07;
                    if (bLength == 3 && iSubCode == NSEXT_LOOP)
                    {
                        UINT uLoopCount = MAKEWORD(pExtension[2], pExtension[3]);
                        gif->m_uLoopCount = uLoopCount - 1;
    
                    }
                }
#endif
            }while (pExtension);
            break;
        }
        case TERMINATE_RECORD_TYPE:
            break;
        default:		     // Should be trapped by DGifGetRecordType
            break;
        }
    }
    while (RecordType != TERMINATE_RECORD_TYPE);
    return 0;
}

static INT32 gif_next_pic(void *vp)
{
    Gif *gif = (Gif *) vp;
    return gif_next_pic_impl(vp);
}


static void* gif_get_result(void *vp)
{
    return NULL;

}

static IMAGE_PROCS gif_procs = {
    gif_get_info_by_filename,
    gif_get_info_by_imagedata,
    gif_open_by_filename,
    gif_open_by_imagedata,
    gif_close,

    gif_get_width,
    gif_get_height,
    gif_get_image_type,
    
    gif_set_nchan,
    gif_set_box,

    gif_write_row_rgba,

    gif_get_nchan,
    gif_get_box,
  
    gif_read_row_rgba,
    
    gif_next_pic,
    gif_get_result
};

IMAGE_INSTANCE gif_instance =
{
    "gif", &gif_procs
};
