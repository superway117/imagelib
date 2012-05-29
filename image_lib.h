#ifndef _IMAGE_LIB_H
#define _IMAGE_LIB_H

#include "portab.h"
#include "lcd_api.h"
/**
   The enumeration defines all error code.
 */
typedef enum
{
    IMAGELIB_SUCCESS,
    IMAGELIB_UNKNOW_FORMAT,		
    IMAGELIB_OPENFILE_ERROR,
    IMAGELIB_RAM_NOT_ENOUGH,
    IMAGELIB_RAM_EXECPTION,
    IMAGELIB_CURRENT_SESSION_BUSY,
    IMAGELIB_EXTERNAL_STOP,
    IMAGELIB_REGION_ERROR,
    IMAGELIB_OTHER_ERROR,

}IMAGELIB_ERROR_CODE;

/**
   The enumeration defines all states of IMAGELIB_SESSION.
 */
typedef enum
{

    IMAGELIB_OPENED,		
    IMAGELIB_DECODING,
    IMAGELIB_IDLE,
    IMAGELIB_CLOSED
}IMAGELIB_STATE;

#define IMAGELIB_INSTANCE_P void*

/**
   The structure represents a session used for invoker to control the procedure of image process.\n
   To dispose an image, a session must be open first using function imagelib_start_decode_by_file or imagelib_start_decode_by_imagedata.
 */
typedef struct 
{	
  WCHAR*   			   pathname;
  IMAGELIB_INSTANCE_P instance_p;	
  IMAGELIB_STATE           state;
  IMAGELIB_ERROR_CODE error_code;
  SEMAPHORE *		   decode_sem;
  
} IMAGELIB_SESSION;
/**
   The prototype defines a callback prototype invoked when a image frame is disposed over using asynchronous mode.
 */
typedef void (*DECODE_FRAME_ENDING_CB)(IMAGELIB_SESSION* session,void* return_data,IMAGELIB_ERROR_CODE error_code);

/**
   The structure represents a rectangle.
 */

typedef struct 
{	
  INT32 x;	
  INT32 y;
  INT32 width;
  INT32 height;
} IMAGELIB_RECT;

/**
   The structure represents the information of an image.
 */
 typedef enum
{
	IMAGELIB_GIF_TYPE,
	IMAGELIB_JPEG_TYPE,
	IMAGELIB_BMP_TYPE,
	IMAGELIB_PNG_TYPE,
	IMAGELIB_WBMP_TYPE,
	IMAGELIB_NATIVE_BMP_TYPE,
	IMAGELIB_LCD_BMP_TYPE,
	IMAGELIB_INVAILD_TYPE
		
}IMAGELIB_IMAGE_TYPE;
   
typedef struct
{
    INT32 width;
    INT32 height;
    INT16	bits_per_pixel;
    IMAGELIB_IMAGE_TYPE image_type;
}IMAGELIB_INFO;

/**
   The enumeration defines all effect types.
 */
typedef enum
{
    IMAGELIB_NONE_EFFECT,
    IMAGELIB_EMBOSS_EFFECT,
    IMAGELIB_BRIGHT_EFFECT,
    IMAGELIB_BLACKWHITE_EFFECT,
    IMAGELIB_NEGATIVE_EFFECT,
    IMAGELIB_GRAY_EFFECT,
}IMAGELIB_EFFECT;

/**
   The structure represents effect parameters.
 */
typedef struct
{
    IMAGELIB_EFFECT effect_type;
    INT32 effect_type_para;
}IMAGELIB_EFFECT_PARA;

/**
   The enumeration defines all rotate types.
 */
typedef enum
{
    IMAGELIB_ROTATE_0,
    IMAGELIB_ROTATE_90,
    IMAGELIB_ROTATE_180,
    IMAGELIB_ROTATE_270,
}IMAGELIB_ROTATE_ANGLE;

/**
   The structure represents draw parameters.
 */
typedef struct
{
    IMAGELIB_RECT 			m_rect;
    IMAGELIB_ROTATE_ANGLE	m_angle;
    INT32						m_src_x;		//source x
    INT32						m_src_y;		//source x
}IMAGELIB_DRAW_PARA;

/**
   The function open a os task/thread, the task/thread is blocked if there is no image process request.\n
   The function is invoked on the beginning of the system start.
 */
extern void imagelib_start_thread();
/**
   The function is used to get the information of an image file.
   @param	_pathname			Source image path.
   @param _info					Return information value.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_getinfo_by_filename(const WCHAR* _pathname,IMAGELIB_INFO* _info);

/**
   The function is used to get the information of an image data.
   @param _dev					Image device type, for example, for a gif data, the device type is "gif".
   @param	_buf					Image data.
   @param	_buf_size			Image data size.
   @param _info					Return information value.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_getinfo_by_imagedata(const CHAR* _dev,const CHAR* _buf,UINT32 _buf_size,IMAGELIB_INFO* _info);

/**
   The function is used to start a session to process image file. It must be invoked on the beginning of image process.
   @param	_pathname			Source image path.
   @param	_session				Return session handle.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_start_decode_by_file(const WCHAR* _pathname,IMAGELIB_SESSION** _session);

/**
   The function is used to start a session to process image data. It must be invoked on the beginning of image process.
   @param _dev					Image device type, for example, for a gif data, the device type is "gif".
   @param	_image_data		Image data.
   @param	_size					Image data size.
   @param	_session				Return session handle.	
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_start_decode_by_imagedata(const char* _dev,const char* _image_data,UINT32 _size,IMAGELIB_SESSION** _session);

/**
   The function is used to directly draw image frame to LCD.\n
   The function is a asynchronous function.
   @param _session				Session handle.
   @param _src_rect				The rectangle of source image.
   @param _dest_width			Output width.
   @param _dest_height		Output height.
   @param _effect_para		Effect parameter.
   @param	_draw_para		  Draw parameter.
   @param _cb						  Callback functon pointer invoked when the image frame is disposed over.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_draw_frame_asyn(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height,IMAGELIB_EFFECT_PARA _effect_para ,
							    IMAGELIB_DRAW_PARA _draw_para,DECODE_FRAME_ENDING_CB _cb );

/**
   The function is used to decode the image frame to DISP_BTIMAP foramt data.\n
   The function is a asynchronous function.
   @param _session				Session handle.
   @param _src_rect				The rectangle of source image.
   @param _dest_width			Output width.
   @param _dest_height		Output height.
   @param _effect_para		Effect parameter.
   @param _cb						  Callback functon pointer invoked when the image frame is disposed over.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_get_frame_data_asyn(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height,
							    IMAGELIB_EFFECT_PARA _effect_para ,DECODE_FRAME_ENDING_CB _cb);
/**
   The function is used to save the image frame to a image file(exclude gif foramt).\n
   The function is a asynchronous function.
   @param _session				Session handle.
   @param _src_rect				The rectangle of source image.
   @param _dest_pathname  Destination pathname
   @param _dest_width			Output width.
   @param _dest_height		Output height.
   @param _effect_para		Effect parameter.
   @param _cb						  Callback functon pointer invoked when the image frame is disposed over.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_save_frame_to_file_asyn(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    const WCHAR* _dest_pathname,INT32 _dest_width,INT32 _dest_height,
							    IMAGELIB_EFFECT_PARA _effect_para,DECODE_FRAME_ENDING_CB _cb );

/**
   The function is used to directly draw image frame to LCD.\n
   The function is a synchronous function.
   @param _session				Session handle.
   @param _src_rect				The rectangle of source image.
   @param _dest_width			Output width.
   @param _dest_height		Output height.
   @param _effect_para		Effect parameter.
   @param	_draw_para		  Draw parameter.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_draw_frame_sync(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height,IMAGELIB_EFFECT_PARA _effect_para,
							    IMAGELIB_DRAW_PARA _draw_para);

/**
   The function is used to decode the image frame to DISP_BTIMAP foramt data.\n
   The function is a synchronous function.
   @param _session				Session handle.
   @param _src_rect				The rectangle of source image.
   @param _dest_width			Output width.
   @param _dest_height		Output height.
   @param _effect_para		Effect parameter.
   @param	_bitmap_rt		Return value.
   @return Error code.
 */
extern  IMAGELIB_ERROR_CODE imagelib_get_frame_data_sync(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height, IMAGELIB_EFFECT_PARA _effect_para,DISP_BITMAP** _bitmap_rt);

/**
   The function is used to save the image frame to a image file(exclude gif foramt).\n
   The function is a synchronous function.
   @param _session				Session handle.
   @param _src_rect				The rectangle of source image.
   @param _dest_pathname  Destination pathname
   @param _dest_width			Output width.
   @param _dest_height		Output height.
   @param _effect_para		Effect parameter.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_save_frame_to_file_sync(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    const WCHAR* _dest_pathname,INT32 _dest_width,INT32 _dest_height,
							    IMAGELIB_EFFECT_PARA _effect_para );

/**
   The function is used to get next frame.
   @param _session				Session handle.
   @return Delay value. If the delay equal zero, it means there is no next frame.
 */
extern INT32 imagelib_get_next_frame(IMAGELIB_SESSION*_session);

/**
   The function is used to get width of image.
   @param _session				Session handle.
   @return Width value.
 */
extern UINT32 imagelib_get_width(IMAGELIB_SESSION*_session);

/**
   The function is used to get height of image.
   @param _session				Session handle.
   @return Height value.
 */
extern UINT32 imagelib_get_height(IMAGELIB_SESSION*_session);

/**
   The function is used to get image type.
   @param _session				Session handle.
   @return image type.
 */
extern IMAGELIB_IMAGE_TYPE imagelib_get_image_type(IMAGELIB_SESSION*_session);
/**
   The function is used to stop the session.
   @param _session				Session handle.
   @return Error code.
 */
extern IMAGELIB_ERROR_CODE imagelib_end_decode(IMAGELIB_SESSION*_session);

#endif
