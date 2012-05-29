#ifndef IMAGELIB_ENGINE_H
#define IMAGELIB_ENGINE_H


#include "pixel.h"
#include "window.h"
#include <image_types.h>
#include <image_lib.h>

typedef IMAGELIB_ERROR_CODE (open_proc_by_filename)( char *pathname, const char *mode,const void* open_para,void** data_rt);

typedef IMAGELIB_ERROR_CODE (open_proc_by_imagedata)( const char *imagedata, UINT32 data_size,void** data_rt);

typedef IMAGELIB_ERROR_CODE (*get_info_by_filename)(const char *pathname, IMAGELIB_INFO* info);

typedef IMAGELIB_ERROR_CODE (*get_info_by_imagedata)(const char *buf,UINT32 buf_size,IMAGELIB_INFO* info);

typedef struct {		/* PICTURE PROCEDURE POINTERS */
  get_info_by_filename get_info_by_filename;
  get_info_by_imagedata get_info_by_imagedata;
  
  open_proc_by_filename *open_by_filename;
  open_proc_by_imagedata* open_by_imagedata;
  IMAGELIB_ERROR_CODE (*close)(void *p,BOOL force);
  
  INT32 (*get_width)(void *p);
  INT32 (*get_height)(void *p);
  IMAGELIB_IMAGE_TYPE(*get_image_type)(void *p);
  
  void (*set_nchan)(void *p, INT32 nchan);
  void (*set_box)(void *p, INT32 ox, INT32 oy, INT32 dx, INT32 dy);
 
  IMAGELIB_ERROR_CODE (*write_row_rgba)(void *p, INT32 y, INT32 x0, INT32 nx,
			 const PIXEL_RGBA *buf);

  INT32 (*get_nchan)(void *p);
  void (*get_box)(void *p, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy);

  IMAGELIB_ERROR_CODE (*read_row_rgba)(void *p, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA *buf);

  INT32 (*next_pic)(void *p);

  void* (*get_result)(void *p);

}IMAGE_PROCS;

typedef struct {	/* PICTURE INFO */
  char *dev;		/* device/filetype name */
  IMAGE_PROCS *procs;	/* structure of generic procedure pointers */
  void *data;		/* device-dependent data (usually ptr to structure) */
} IMAGE_INSTANCE;

#define IMAGE_INSTANCE_LISTMAX 10
extern IMAGE_INSTANCE *image_instance_list[IMAGE_INSTANCE_LISTMAX];	
extern INT32 image_instance_number;			

#define IMAGE_UNDEFINED PIXEL_UNDEFINED	

extern IMAGELIB_ERROR_CODE image_get_info_by_filename(const char *file,IMAGELIB_INFO* info);
extern IMAGELIB_ERROR_CODE image_get_info_by_imagedata(const char *dev, const char *image_data, UINT32 data_size,IMAGELIB_INFO* info);

extern IMAGELIB_ERROR_CODE image_open_by_filename(const char *file, const char *mode,const void* open_para,IMAGE_INSTANCE** _instance_rt);
extern IMAGELIB_ERROR_CODE image_open_dev_by_imagedata(const char *dev, const char *image_data, UINT32 data_size,IMAGE_INSTANCE** _instance_rt);
extern IMAGELIB_ERROR_CODE image_open_dev(const char *dev, const char *name, const char *mode,const void* open_para,IMAGE_INSTANCE** _instance_rt);
extern IMAGELIB_ERROR_CODE image_close(IMAGE_INSTANCE *p,BOOL force);

extern IMAGELIB_ERROR_CODE image_read_row_rgba(IMAGE_INSTANCE *p, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA * buf);
extern IMAGELIB_ERROR_CODE image_write_row_rgba(IMAGE_INSTANCE *p, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA * buf);


extern INT32  image_get_width(IMAGE_INSTANCE *p);
extern  INT32  image_get_height(IMAGE_INSTANCE *p);
extern IMAGELIB_IMAGE_TYPE  image_get_image_type(IMAGE_INSTANCE *p);

extern void image_set_box(IMAGE_INSTANCE *p, INT32 ox, INT32 oy, INT32 dx, INT32 dy);
extern void image_get_box(const IMAGE_INSTANCE *p, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy);

#if 0	
extern void image_set_nchan(IMAGE_INSTANCE *p, INT32 nchan);
extern INT32 image_get_nchan(IMAGE_INSTANCE *p);
#endif
	
extern INT32 image_next_pic(IMAGE_INSTANCE *p);
extern void* image_get_result(IMAGE_INSTANCE *p);

extern void image_init(void);

//#define image_get_dev(p) (p)->dev

extern void	image_set_window(IMAGE_INSTANCE *p, const WINDOW *win);
extern WINDOW *image_get_window(/* p, win */);

char	*image_file_dev(/* file */);

#define exit  exit_fake
void	exit_fake(INT32 exit_code);


#endif
