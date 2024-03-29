#include <image_headfile.h>
#include <setjmp.h>
#include <string.h>
#include <jpeglib.h>

#define INPUT_BUF_SIZE  4096        /* choose an efficiently fread'able size */

/* Expanded data source object for stdio input */
typedef struct {
    struct jpeg_source_mgr 	pub;        /* public fields */
    IMAGE_FILE_HANDLER 	filehandle;
    void* 					userdata;
    UINT32 					userdata_size;
    void *					userdata_ptr;
    JOCTET * 			  	buffer;                /* start of buffer */
    BOOL				   	start_of_file;
}JPG_SOURCE_MGR;


typedef struct {
    struct jpeg_error_mgr pub;    /* "public" fields */
    jmp_buf setjmp_buffer;    /* for return to caller */
}JPG_ERR_MGR;

typedef struct tagJpg {
    JPG_ERR_MGR		error;
  //  JPG_SOURCE_MGR      src;
    int read_not_write;
    struct jpeg_decompress_struct decode_info;
    struct jpeg_compress_struct encode_info;
    INT32 format;
    INT32 scanline;
    INT32 row_stride;
    JSAMPARRAY row;
}Jpg;


static INT32 jpg_read_imagedata (JPG_SOURCE_MGR* src,char* buf,INT32 len,INT32 count)
{
    INT32 size = len*count;

    char* ptr = NULL;
    if (src == NULL || buf == NULL)
        return -1;
    ptr = src->userdata_ptr;
    memcpy(buf, ptr, size);
    src->userdata_ptr= ptr + size;
    return size;
}

static INT32 jpg_seek_imagedata(JPG_SOURCE_MGR* src,INT32 offset,INT32 origin)
{

    if (src == NULL || src->userdata == NULL)
        return -1;

    if (IMAGE_FILE_ORIGIN_SET == origin)
    {
        if (offset<0)
            return -1;
        else if ((UINT32)(offset +1) > src->userdata_size)
            return -1;
        src->userdata_ptr = (char*)src->userdata + offset;
    }
    else if (IMAGE_FILE_ORIGIN_CUR == origin)
    {
        if (((char*)src->userdata_ptr - (char*)src->userdata + offset)  <0)
            return -1;
        else if ((UINT32)((char*)src->userdata_ptr - (char*)src->userdata + offset +1)  >= src->userdata_size)
            return -1;
        src->userdata_ptr = (char*)src->userdata_ptr + offset;
    }
    return 0;
}

#define JPG_READ(_src,_buf,_len,_count)         \
  (_src)->userdata ?                   \
    jpg_read_imagedata(_src,_buf,_len,_count) : \
    IMAGE_FILE_READ(_buf,_len,_count,(_src->filehandle))



#define JPG_SEEK(_src,_offset,_origin )        \
  (_src)->userdata ?                   \
    jpg_seek_imagedata(_src,_offset,_origin ) : \
    IMAGE_FILE_SEEK((_src->filehandle),_offset,_origin)


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

static void init_source (j_decompress_ptr decode_info)
{
    JPG_SOURCE_MGR* src = (JPG_SOURCE_MGR*) decode_info->src;

    /* We reset the empty-input-file flag for each image,
     * but we don't clear the input buffer.
     * This is correct behavior for reading a series of images from one source.
     */
    src->start_of_file = TRUE;
}

static BOOL fill_input_buffer (j_decompress_ptr decode_info)
{
    JPG_SOURCE_MGR* src = (JPG_SOURCE_MGR*) decode_info->src;
    size_t nbytes;

    nbytes = JPG_READ (src, src->buffer, 1, INPUT_BUF_SIZE);

    if (nbytes <= 0)
    {
        /* Insert a fake EOI marker */
        src->buffer[0] = (JOCTET) 0xFF;
        src->buffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file = FALSE;

    return TRUE;
}

static void skip_input_data (j_decompress_ptr decode_info, long num_bytes)
{
    JPG_SOURCE_MGR* src = (JPG_SOURCE_MGR*) decode_info->src;

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0)
    {
        while (num_bytes > (long) src->pub.bytes_in_buffer)
        {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            (void) fill_input_buffer(decode_info);
            /* note we assume that fill_input_buffer will never return FALSE,
             * so suspension need not be handled.
             */
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}
/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

static void term_source (j_decompress_ptr decode_info)
{
    /* no work necessary here */
}
#if 0
static IMAGELIB_ERROR_CODE jpg_open_by_filename(const char *pathname, const char *mode,const void* open_para,void** data_rt)
{
    Jpg *jpg = NULL;
    IMAGE_FILE_HANDLER hdl = 0;

    if ('r' == mode[0])
        hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_READ_MODE);
    else if ('w' == mode[0])
        hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_CREATE_WRITE_MODE);

    if (hdl>0)
    {
        IMAGE_ALLOC(jpg, Jpg, 1);
        if (jpg == NULL)
            return IMAGELIB_RAM_NOT_ENOUGH;
        memset(jpg, 0, sizeof(Jpg));
        jpg->filehandle = hdl;

        if ('r' == mode[0])
        {
            jpg->read_not_write = 1;
            jpg->in.err = jpeg_std_error(&jpg->jerr);
            jpeg_create_decompress(&jpg->in);
            jpeg_stdio_src(&jpg->in, jpg->filehandle);
            jpeg_read_header(&jpg->in, TRUE);
            jpeg_start_decompress(&jpg->in);
            jpg->row_stride = jpg->in.output_width * jpg->in.output_components;
            jpg->row = (*jpg->in.mem->alloc_sarray)((j_common_ptr) &jpg->in,
                                                    JPOOL_IMAGE, jpg->row_stride, 1);
        }
        else
        {
            jpg->read_not_write = 0;
            jpg->encode_info.err = jpeg_std_error(&jpg->jerr);
            jpeg_create_compress(&jpg->encode_info);
            jpeg_stdio_dest(&jpg->encode_info, jpg->filehandle);
            jpg->encode_info.in_color_space = JCS_RGB;
        }
        *data_rt = jpg;
        return IMAGELIB_SUCCESS;
    }
    *data_rt=NULL;
    return IMAGELIB_OPENFILE_ERROR;

}
static void jpg_close(void *p,BOOL force)
{
    Jpg *jpg = (Jpg *) p;

    if (jpg->read_not_write)
    {
        jpeg_finish_decompress(&jpg->in);
        jpeg_destroy_decompress(&jpg->in);
    }
    else
    {
        jpeg_finish_compress(&jpg->encode_info);
        jpeg_destroy_compress(&jpg->encode_info);
    }
    if (jpg->filehandle > 0)
    {
        IMAGE_FILE_CLOSE(jpg->filehandle);
        jpg->filehandle = 0;
    }
    IMAGE_FREE(jpg);
}
#else
static void jpg_set_data_src (j_decompress_ptr cinfo, IMAGE_FILE_HANDLER hdl)
{
  JPG_SOURCE_MGR* src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) 
  {        /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  sizeof(JPG_SOURCE_MGR));
    src = (JPG_SOURCE_MGR*) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  INPUT_BUF_SIZE * sizeof(JOCTET));
  }


  src = (JPG_SOURCE_MGR*) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->filehandle= hdl;
  src->userdata=NULL;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}
static void jpg_set_data_src_imagedata (j_decompress_ptr cinfo, void* data,UINT32 buf_size)
{
  JPG_SOURCE_MGR* src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) 
  {        /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  sizeof(JPG_SOURCE_MGR));
    src = (JPG_SOURCE_MGR*) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  INPUT_BUF_SIZE * sizeof(JOCTET));
  }


  src = (JPG_SOURCE_MGR*) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->filehandle= 0;
  src->userdata=data;
  src->userdata_ptr=data;
  src->userdata_size=buf_size;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}

static IMAGELIB_ERROR_CODE jpg_open_by_filename(const char *pathname, const char *mode,const void* open_para,void** data_rt)
{
    Jpg *jpg = NULL;
    IMAGE_FILE_HANDLER hdl = 0;

    if ('r' == mode[0])
        hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_READ_MODE);
    else if ('w' == mode[0])
        hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_CREATE_WRITE_MODE);
    if (hdl>0)
    {
        IMAGE_ALLOC(jpg, Jpg, 1);
        if (jpg == NULL)
            return IMAGELIB_RAM_NOT_ENOUGH;
        memset(jpg, 0, sizeof(Jpg));

        if ('r' == mode[0])
        {
            jpg->read_not_write = 1;
	     jpg->decode_info.err = jpeg_std_error (&jpg->error.pub);
            jpeg_create_decompress(&jpg->decode_info);
	     jpg_set_data_src(&jpg->decode_info,hdl);
	     //jpg_set_data_src(jpg,hdl);
            //      jpeg_stdio_src(&jpg->decode_info, jpg->filehandle);
            jpeg_read_header(&jpg->decode_info, TRUE);

            jpg->decode_info.out_color_space = JCS_RGB;
            jpg->decode_info.quantize_colors = FALSE;
            jpeg_start_decompress(&jpg->decode_info);
		#if 1
            jpg->row_stride = jpg->decode_info.output_width * jpg->decode_info.output_components;
            jpg->row = (*jpg->decode_info.mem->alloc_sarray)((j_common_ptr) &jpg->decode_info,
                       JPOOL_IMAGE, jpg->row_stride, 1);
		#endif
        }
        else
        {
            jpg->read_not_write = 0;
            //     jpg->encode_info.err = jpeg_std_error(&jpg->jerr);
            jpeg_create_compress(&jpg->encode_info);
//	      jpeg_stdio_dest(&jpg->encode_info, jpg->filehandle);
            jpg->encode_info.in_color_space = JCS_RGB;
        }
        *data_rt = jpg;
        return IMAGELIB_SUCCESS;
    }
     return IMAGELIB_OPENFILE_ERROR;


}
static IMAGELIB_ERROR_CODE jpg_open_by_imagedata(const char *buf,UINT32 buf_size,void** data_rt)
{
    Jpg *jpg = NULL;
    IMAGELIB_ERROR_CODE error_code = IMAGELIB_SUCCESS;
    if (jpg== NULL)
    {
        *data_rt = NULL;
        return IMAGELIB_RAM_EXECPTION;
    }
    IMAGE_ALLOC(jpg, Jpg, 1);
    if(jpg==NULL)
	 	return IMAGELIB_RAM_NOT_ENOUGH;
    memset(jpg, 0, sizeof(Jpg));

    jpg->read_not_write = 1;
    jpg->decode_info.err = jpeg_std_error (&jpg->error.pub);
    jpeg_create_decompress(&jpg->decode_info);
    jpg_set_data_src_imagedata(&jpg->decode_info,buf,buf_size);
	     //jpg_set_data_src(jpg,hdl);
            //      jpeg_stdio_src(&jpg->decode_info, jpg->filehandle);
    jpeg_read_header(&jpg->decode_info, TRUE);

    jpg->decode_info.out_color_space = JCS_RGB;
    jpg->decode_info.quantize_colors = FALSE;
    jpeg_start_decompress(&jpg->decode_info);
    jpg->row_stride = jpg->decode_info.output_width * jpg->decode_info.output_components;
    jpg->row = (*jpg->decode_info.mem->alloc_sarray)((j_common_ptr) &jpg->decode_info,
                       JPOOL_IMAGE, jpg->row_stride, 1);

     return IMAGELIB_OPENFILE_ERROR;


}
static IMAGELIB_ERROR_CODE jpg_close(void *vp,BOOL force)
{
    Jpg *jpg = (Jpg *) vp;
    JPG_SOURCE_MGR* src = NULL;
    if (jpg == NULL)
    {
        return IMAGELIB_RAM_EXECPTION;
    }
    
  
 //   IMAGE_FREE(jpg->row);
 //   jpg->row=NULL;
    src = jpg->decode_info.src;
    

    if (src->filehandle > 0)
    {
        IMAGE_FILE_CLOSE(src->filehandle);
        src->filehandle = 0;
    }
  //  IMAGE_FREE(src->buffer); 
//	src->buffer=NULL;
	
//    IMAGE_FREE(src);
//    jpg->decode_info.src=NULL;
    if (jpg->read_not_write) 
   {
    	jpeg_finish_decompress(&jpg->decode_info);
    	jpeg_destroy_decompress(&jpg->decode_info);
   } 
   else 
   {
    	jpeg_finish_compress(&jpg->encode_info);
    	jpeg_destroy_compress(&jpg->encode_info);
   }
    IMAGE_FREE(jpg);
    return IMAGELIB_SUCCESS;
}
#endif
static IMAGELIB_ERROR_CODE jpg_get_info_by_filename(const char *pathname, IMAGELIB_INFO* info)
{
    // Jpg *jpg =NULL;
    // IMAGELIB_ERROR_CODE error_code = jpg_open_by_filename(pathname,buf_size,&jpg );
    return IMAGELIB_UNKNOW_FORMAT;
}

static IMAGELIB_ERROR_CODE jpg_get_info_by_imagedata(const char *buf,UINT32 buf_size,IMAGELIB_INFO* info)
{
    return IMAGELIB_UNKNOW_FORMAT;
}

static INT32 jpg_get_width(void *p)
{
    Jpg *jpg = (Jpg *) p;
    if (jpg->read_not_write)
    {

        return  jpg->decode_info.image_width;
    }
    return 0;

}

static INT32 jpg_get_height(void *p)
{
    Jpg *jpg = (Jpg *) p;
    if (jpg->read_not_write)
    {

        return  jpg->decode_info.image_height;
    }
    return 0;
}

static IMAGELIB_IMAGE_TYPE jpg_get_image_type(void *p)
{
    return IMAGELIB_JPEG_TYPE;
}

static void jpg_set_nchan(void *vp, INT32 nchan)
{

}

static void jpg_set_box(void *vp, INT32 ox, INT32 oy, INT32 dx, INT32 dy)
{
    Jpg *jpg = (Jpg *) vp;
    if (! jpg->read_not_write)
    {
        jpg->encode_info.image_width = ox + dx;
        jpg->encode_info.image_height = oy + dy;
    }
}



//#define WRITE_INIT(jpg_) ((jpg_)->scanline || write_init(jpg_))
static IMAGELIB_ERROR_CODE write_init(Jpg *jpg)
{
    return 1;
}

static void seek_row_write(Jpg *jpg, INT32 y)
{

}


static void jpg_write_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, const PIXEL_RGBA *buf)
{

}

static int jpg_get_nchan(void *vp)
{
    return 3;
}

static void jpg_get_box(void *vp, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy)
{
    Jpg *jpg = (Jpg *) vp;


    if (jpg->read_not_write)
    {
	  *ox = 0;
        *oy = 0;
        *dx = jpg->decode_info.image_width;
        *dy = jpg->decode_info.image_height;
    }
    else if (jpg->scanline)
    {
        *ox = 0;
        *oy = 0;
        *dx = jpg->encode_info.image_width;
        *dy = jpg->encode_info.image_height;
    }
    else
    {
        *ox = PIXEL_UNDEFINED;
        *oy = PIXEL_UNDEFINED;
        *dx = PIXEL_UNDEFINED;
        *dy = PIXEL_UNDEFINED;
    }
}


static void  jpg_seek_row_read(Jpg *jpg, int y)
{
    while (jpg->scanline <= y)
    {
        jpeg_read_scanlines(&jpg->decode_info, jpg->row, 1);
        jpg->scanline++;
    }
}
static IMAGELIB_ERROR_CODE jpg_read_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA *buf)
{
    Jpg *jpg = (Jpg *) vp;

    if (jpg->read_not_write && 3 == jpg->decode_info.output_components)
    {
        INT32 i;

        jpg_seek_row_read(jpg, y);
        if (1 == jpg->decode_info.output_components)
            for (i = 0; i < nx; i++)
            {
                const INT32 luminance = jpg->row[0][x0+i];
                buf[i].r = luminance;
                buf[i].g = luminance;
                buf[i].b = luminance;
            }
        else
            for (i = 0; i < nx; i++)
            {
                buf[i].r = jpg->row[0][(x0+i)*3+0];
                buf[i].g = jpg->row[0][(x0+i)*3+1];
                buf[i].b = jpg->row[0][(x0+i)*3+2];
            }
    }
    return IMAGELIB_SUCCESS;
}

static INT32 jpg_next_pic(void *vp)
{
    return 0;
}

static void* jpg_get_result(void *vp)
{
    return NULL;

}

static IMAGE_PROCS jpg_procs =
{
    jpg_get_info_by_filename,
    jpg_get_info_by_imagedata,
    jpg_open_by_filename,
    NULL,
    jpg_close,

    jpg_get_width,
    jpg_get_height,
    jpg_get_image_type,

    jpg_set_nchan,
    jpg_set_box,

    jpg_write_row_rgba,

    jpg_get_nchan,
    jpg_get_box,

    jpg_read_row_rgba,
    jpg_next_pic,
    jpg_get_result,

};


IMAGE_INSTANCE jpg_instance =
{
    "jpg", &jpg_procs
};
