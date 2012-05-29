#include <image_headfile.h>

#include <lcd_api.h>


#define NATIVE_BITMAP_MAX_MEMORY_SIZE_SUPPORT 80960

#define NATIVE_BITAMP_READ(_buf,_len,_count,_bmp)         \
  (_bmp)->read ?                   \
    (_bmp)->read(_buf,_len,_count,_bmp) : \
    IMAGE_FILE_READ(_buf,_len,_count,(_bmp->filehandle))



#define NATIVE_BITAMP_SEEK(_bmp,_offset,_origin )        \
  (_bmp)->seek ?                   \
    (_bmp)->seek(_bmp,_offset,_origin ) : \
    IMAGE_FILE_SEEK((_bmp->filehandle),_origin,_offset)


    
typedef int (*NativeBitmap_ReadFunc) (char* buf,INT32 len,INT32 count,void* bmp);
typedef int (*NativeBitmap_SeekFunc) (void* bmp,INT32 offset,INT32 origin);



/*------------------------------------------------------------*/
struct tagPicNativeBitmap
{
    IMAGE_FILE_HANDLER filehandle;
    
    int read_not_write;
    UINT8 bit_depth;
    INT32 scanline;

    char* 	row;
    NativeBitmap_ReadFunc read;
    NativeBitmap_SeekFunc seek;

    huge UINT16 *userdata_ptr;
    DISP_BITMAP* bitmap;
};
typedef struct tagPicNativeBitmap PicNativeBitmap;


static INT32 native_bitmap_read_imagedata( void* buf, INT32 len ,INT32 count,PicNativeBitmap* bmp)
{
    INT32 size = len*count;

    return size;
}
static INT32 native_bitmap_seek_imagedata(PicNativeBitmap* bmp, INT32 offset, INT32 origin)
{
  
    return 0;
}




static BOOL native_bitmap_read_header(PicNativeBitmap *native_bmp)
{
    if(native_bmp == NULL || native_bmp->read_not_write!=1)
		return FALSE;
    if(native_bmp->filehandle == IMAGE_INVAILD_FILE_HANDLER)
    {
    	IMAGE_FILE_READ(((void*)&native_bmp->bitmap->height),(sizeof(UINT16)),1,native_bmp->filehandle);
	IMAGE_FILE_READ(((void*)&native_bmp->bitmap->width),(sizeof(UINT16)),1,native_bmp->filehandle);
	IMAGE_FILE_READ(((void*)&native_bmp->bitmap->bits),(sizeof(UINT8)),1,native_bmp->filehandle);
    }
    return TRUE;
}

static IMAGELIB_ERROR_CODE native_bitmap_open_by_filename(const char *pathname, const char *mode,const void* open_para,void** data_rt)
{
    PicNativeBitmap *bmp = NULL;
    IMAGELIB_ERROR_CODE error_code = IMAGELIB_SUCCESS;
    IMAGE_FILE_HANDLER hdl=0;
    if('r' == mode[0])
		hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_READ_MODE);
    else if('w' == mode[0])
		hdl = IMAGE_FILE_OPEN(pathname, IMAGE_FILE_OPEN_CREATE_WRITE_MODE);
    if (hdl>0) 
    {
    	
        IMAGE_ALLOC(bmp, PicNativeBitmap, 1);
	if(bmp==NULL)
	{
			IMAGE_FILE_CLOSE(bmp->filehandle);
			return IMAGELIB_RAM_NOT_ENOUGH;
        }
        memset(bmp, 0, sizeof(PicNativeBitmap));
        bmp->filehandle = hdl;
        if ('r' == mode[0])
        {
            bmp->read_not_write = 1;
        }
	
        
        if (bmp->read_not_write)
        {
            if (!native_bitmap_read_header(bmp))
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
    bmp->userdata_ptr= NULL;
    bmp->read = NULL;
    bmp->seek = NULL;
    bmp->bitmap=IMAGE_MALLOC(sizeof(DISP_BITMAP));
    if(bmp->bitmap == NULL)
    {
    	IMAGE_FREE(bmp);
    	IMAGE_FILE_CLOSE(bmp->filehandle);
    	error_code = IMAGELIB_RAM_NOT_ENOUGH;
    }
    bmp->bitmap->p_bitmap=NULL;
    bmp->bitmap->bits = 16;
    bmp->row = NULL;
    return error_code;
}
static IMAGELIB_ERROR_CODE native_bitmap_open_by_imagedata(const char *buf,UINT32 buf_size,void** data_rt)
{
    PicNativeBitmap *bmp = NULL;
    IMAGELIB_ERROR_CODE error_code = IMAGELIB_SUCCESS;
    IMAGE_ALLOC(bmp, PicNativeBitmap, 1);
    if(bmp==NULL)
	 return IMAGELIB_RAM_NOT_ENOUGH;
    memset(bmp, 0, sizeof(PicNativeBitmap));
    bmp->filehandle = IMAGE_INVAILD_FILE_HANDLER;
    if(buf != NULL)
    {
    		bmp->bitmap = (DISP_BITMAP*)buf;
   		bmp->read_not_write = 1;
		bmp->bit_depth= bmp->bitmap->bits;	
		bmp->userdata_ptr =(huge UINT16 *) bmp->bitmap->p_bitmap;
    }
    else
    {
   	 bmp->read_not_write = 0;
	bmp->bit_depth= 16;	
	bmp->bitmap=IMAGE_MALLOC(sizeof(DISP_BITMAP));
	if(bmp->bitmap == NULL)
	{
	    	IMAGE_FREE(bmp);
	    	error_code = IMAGELIB_RAM_NOT_ENOUGH;
	}
	bmp->userdata_ptr  = NULL;
	bmp->bitmap->p_bitmap=NULL;
	bmp->bitmap->bits = 16;
    }		
    bmp->read = native_bitmap_read_imagedata;
    bmp->seek = native_bitmap_seek_imagedata;
    bmp->row = NULL;
   
    *data_rt = bmp;
   return error_code;
   
}



/*------------------------------------------------------------*/
static IMAGELIB_ERROR_CODE native_bitmap_close(void *p,BOOL force)
{
    PicNativeBitmap *bmp = (PicNativeBitmap *) p;
    if(bmp == NULL)
		return IMAGELIB_RAM_EXECPTION;
    if(bmp->row!=NULL)
	IMAGE_FREE(bmp->row);
    if (bmp->filehandle != IMAGE_INVAILD_FILE_HANDLER)
    {
        IMAGE_FILE_CLOSE(bmp->filehandle);
        bmp->filehandle = IMAGE_INVAILD_FILE_HANDLER;
    }
    if(force)
    {
    	IMAGE_FREE(bmp->bitmap->p_bitmap);
    	IMAGE_FREE(bmp->bitmap);
	
    }
    IMAGE_FREE(bmp);
    return IMAGELIB_SUCCESS;
}

static IMAGELIB_ERROR_CODE native_bitmap_get_info_by_filename(const char *pathname, IMAGELIB_INFO* info)
{
	
	return IMAGELIB_OTHER_ERROR;
}

static IMAGELIB_ERROR_CODE native_bitmap_get_info_by_imagedata(const char *buf,UINT32 buf_size,IMAGELIB_INFO* info)
{
	return IMAGELIB_OTHER_ERROR;
}

static INT32 native_bitmap_get_width(void *p)
{
	PicNativeBitmap *bmp = (PicNativeBitmap *) p;
	if(bmp == NULL)
		return 0;
  	return bmp->bitmap->width;  
}

static INT32 native_bitmap_get_height(void *p)
{
	PicNativeBitmap *bmp = (PicNativeBitmap *) p;
	if(bmp == NULL)
		return 0;
  	return bmp->bitmap->height;  
}

static IMAGELIB_IMAGE_TYPE native_get_image_type(void *p)
{
    return IMAGELIB_NATIVE_BMP_TYPE;
}

static void native_bitmap_set_nchan(void *vp, INT32 nchan)
{
 
}

/*------------------------------------------------------------*/
static void native_bitmap_set_box(void *vp, INT32 ox, INT32 oy, INT32 dx, INT32 dy)
{
    PicNativeBitmap *bmp = (PicNativeBitmap *) vp;
    if(bmp == NULL)
		return;
    if (! bmp->read_not_write) 
    {
        bmp->bitmap->width = (UINT16)(ox + dx);
        bmp->bitmap->height =(UINT16)(oy + dy);
    }
}


static void native_bitmap_seek_row_write(PicNativeBitmap *bmp, INT32 y)
{
	if(bmp == NULL)
		return;
	if(bmp->filehandle == IMAGE_INVAILD_FILE_HANDLER)
	{
	#if 0
	 	if(bmp->userdata_ptr != NULL)
	 	{
	  		bmp->userdata_ptr =(huge UINT16 *)(bmp->bitmap->p_bitmap+ bmp->bitmap->width * y*2);//bmp->row_len*y ;

		}
	#else
		if(bmp->userdata_ptr != NULL)
	 	{
	  		bmp->userdata_ptr =(huge UINT16 *)(bmp->bitmap->p_bitmap+ bmp->bitmap->width * y*2);//bmp->row_len*y ;
		}
	#endif
	}
	else
	{
		UINT16 row_len = bmp->bitmap->width*2;
		while (bmp->scanline < y) 
		{
		    	 IMAGE_FILE_WRITE(bmp->row,row_len,1,bmp->filehandle);
		        bmp->scanline++;
		}
	}
	bmp->scanline = y;

}
static IMAGELIB_ERROR_CODE native_bitmap_write_init(PicNativeBitmap *bmp)
{
	if(bmp == NULL)
		return IMAGELIB_RAM_EXECPTION;
	if(bmp->scanline )
		return IMAGELIB_SUCCESS;
	if(bmp->filehandle == IMAGE_INVAILD_FILE_HANDLER)
	{
		if(bmp->bitmap->p_bitmap == NULL)
		{	
			if(bmp->bitmap->width == 0||bmp->bitmap->height== 0)
				return IMAGELIB_RAM_EXECPTION;
			if((bmp->bitmap->width*bmp->bitmap->height*2) > NATIVE_BITMAP_MAX_MEMORY_SIZE_SUPPORT)
				return IMAGELIB_RAM_NOT_ENOUGH;
			bmp->bitmap->p_bitmap = (huge UINT8*)IMAGE_MALLOC(bmp->bitmap->width*bmp->bitmap->height*2);
			if(bmp->bitmap->p_bitmap == NULL)
			{
				return IMAGELIB_RAM_NOT_ENOUGH;
			}
			bmp->userdata_ptr=(huge UINT16*)bmp->bitmap->p_bitmap;
			return IMAGELIB_SUCCESS;
		
		}
	}
	else
	{
	//	UINT32 null_point = 0;
		if(bmp->row == NULL)
		{
			bmp->row = IMAGE_MALLOC(bmp->bitmap->width*2);
			if(bmp->row == NULL)
			{
				return IMAGELIB_RAM_NOT_ENOUGH;
			}
		}
	//	IMAGE_FILE_WRITE(&null_point,sizeof(void*),1,bmp->filehandle);
		IMAGE_FILE_WRITE(&bmp->bitmap->height,sizeof(UINT16),1,bmp->filehandle);
		IMAGE_FILE_WRITE(&bmp->bitmap->width,sizeof(UINT16),1,bmp->filehandle);
		IMAGE_FILE_WRITE(&bmp->bitmap->bits,sizeof(UINT8),1,bmp->filehandle);
	}
	return IMAGELIB_SUCCESS;
}



static IMAGELIB_ERROR_CODE native_bitmap_write_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, const PIXEL_RGBA *buf)
{
    PicNativeBitmap *bmp = (PicNativeBitmap *) vp;
    IMAGELIB_ERROR_CODE error;
    if(bmp == NULL)
		return IMAGELIB_RAM_EXECPTION;
    if(bmp->read_not_write)
		return IMAGELIB_SUCCESS;
    error =native_bitmap_write_init(bmp);
    if(error !=IMAGELIB_SUCCESS)
		return error;

    {
        INT32 i, j;
        native_bitmap_seek_row_write(bmp, y);
	 if(bmp->filehandle == IMAGE_INVAILD_FILE_HANDLER)
	 {
        for (i = 0, j = x0; i < nx; i++) 
	{
		bmp->userdata_ptr [j++] = RGB16(buf[i].r,buf[i].g,buf[i].b);
	        }
	 }
	 else
	 {
	        UINT16 *row = (UINT16 *) bmp->row;
	        for (i = 0, j = x0; i < nx; i++) 
		{
	            row[j++] = RGB16(buf[i].r,buf[i].g,buf[i].b);
				if(j>0 && row[j-1] != 0xffff)
					printf("%d:%d",j-1,row[j-1]);
	        }	
	 	IMAGE_FILE_WRITE((void*)bmp->row,bmp->bitmap->width*2,1,bmp->filehandle);
		bmp->scanline++;
        }
    }

   return IMAGELIB_SUCCESS;
}

/*------------------------------------------------------------*/
static int native_bitmap_get_nchan(void *vp)
{
    return 3;
}


static void native_bitmap_get_box(void *vp, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy)
{
    PicNativeBitmap *bmp = (PicNativeBitmap *) vp;
    if(bmp == NULL)
   	return;
    if (bmp->read_not_write || bmp->scanline) 
    {
        *ox = 0;
        *oy = 0;
        *dx = bmp->bitmap->width;
        *dy = bmp->bitmap->height;
    } 
    else 
    {
        *ox = PIXEL_UNDEFINED;
        *oy = PIXEL_UNDEFINED;
        *dx = PIXEL_UNDEFINED;
        *dy = PIXEL_UNDEFINED;
    }
}

static void native_bitmap_seek_row_read(PicNativeBitmap *bmp, INT32 y)
{
   if(bmp == NULL)
   	return;
   if(bmp->filehandle == IMAGE_INVAILD_FILE_HANDLER)
   {
     	bmp->userdata_ptr =(huge UINT16*)(bmp->bitmap->p_bitmap+ bmp->bitmap->width * y*2);//bmp->row_len*y ;
	bmp->scanline = y;
   }
   else
   {
   	UINT32 row_len = bmp->bitmap->width *2;
   	 while (bmp->scanline <= y)
        {
            INT32 count = IMAGE_FILE_WRITE(bmp->row, row_len, 1, bmp->filehandle);

            bmp->scanline++;
        }
   }
}
static BOOL native_bitmap_read_init(PicNativeBitmap *bmp)
{
	if(bmp == NULL)
		return FALSE;
	
	if(bmp->filehandle != IMAGE_INVAILD_FILE_HANDLER)
	{
		if(bmp->row == NULL)
			bmp->row = IMAGE_MALLOC(bmp->bitmap->width*2);
		
	}
	return TRUE;
}

static IMAGELIB_ERROR_CODE native_bitmap_read_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA *buf)
{

    PicNativeBitmap *bmp = (PicNativeBitmap *) vp;
    if(bmp == NULL)
   	return IMAGELIB_RAM_EXECPTION;
    if (bmp->read_not_write && native_bitmap_read_init(bmp)) 
     {
        INT32 i;
        INT32 j;
        
        native_bitmap_seek_row_read(bmp, y);
	 if(bmp->filehandle == IMAGE_INVAILD_FILE_HANDLER)
	 {	
	 	 UINT16 huge* row = (UINT16 huge*) bmp->userdata_ptr;
	        switch (bmp->bit_depth)
	        {
	        case 16:
	            for (i = 0, j = x0; i < nx; i++, j ++)
	            {
	       
	                buf[i].b = BLUE_OF_RGB16(row[j])<<3;
			  buf[i].g = GREEN_OF_RGB16(row[j])<<2;
	                buf[i].r = RED_OF_RGB16(row[j])<<3;	  
	                buf[i].a = 255;
	            }
	            break;
	        }
	  }
	  else
	  {
	  	UINT16 * row = (UINT16 *) bmp->row;
	  	switch (bmp->bit_depth)
	        {
	        case 16:
		     IMAGE_FILE_READ(((void*)row),(bmp->bitmap->width*2),1,bmp->filehandle);
	            for (i = 0, j = x0; i < nx; i++, j ++)
	            {
	       
	                buf[i].b = BLUE_OF_RGB16(row[j])<<3;
			  buf[i].g = GREEN_OF_RGB16(row[j])<<2;
	                buf[i].r = RED_OF_RGB16(row[j])<<3;	  
	                buf[i].a = 255;
	            }
	            break;
	        }
        }
    }
    return IMAGELIB_SUCCESS;
}

/*------------------------------------------------------------*/
static INT32 native_bitmap_next_pic(void *vp)
{
    return 0;
}

static void* native_bitmap_get_result(void *vp)
{
	PicNativeBitmap *bmp = (PicNativeBitmap *) vp;
       if(bmp != NULL)
	   return bmp->bitmap;
	return NULL;
	
}


/*------------------------------------------------------------*/
static IMAGE_PROCS native_bitmap_procs = {
    native_bitmap_get_info_by_filename,
    native_bitmap_get_info_by_imagedata,
    native_bitmap_open_by_filename,
    native_bitmap_open_by_imagedata,
    native_bitmap_close,

    native_bitmap_get_width,
    native_bitmap_get_height,
    native_get_image_type,
    


    native_bitmap_set_nchan,
    native_bitmap_set_box,

    native_bitmap_write_row_rgba,

    native_bitmap_get_nchan,
    native_bitmap_get_box,
 
    native_bitmap_read_row_rgba,
    native_bitmap_next_pic,
    native_bitmap_get_result
};

/*------------------------------------------------------------*/
IMAGE_INSTANCE native_bitmap_instance =
{
    "rab", &native_bitmap_procs
};

