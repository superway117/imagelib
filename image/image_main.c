
#if defined(WIN32)
#include <process.h>
#include <windows.h>
#endif
#include <math.h>
#include <stdlib.h>

#include <image_headfile.h>


#include "zoom.h"

extern IMAGELIB_ERROR_CODE image_zoom_entrance(IMAGELIB_SESSION* _session,IMAGE_INSTANCE *_decoder_instance, WINDOW_BOX *decoder_window,IMAGE_INSTANCE * _encoder_instance,WINDOW_BOX * encoder_window,IMAGELIB_EFFECT effect_type,INT32 effect_para);

//extern IMAGELIB_ERROR_CODE zoom_continuous1(IMAGE_INSTANCE *_decoder_instance,WINDOW_BOX * awin, IMAGE_INSTANCE *_encoder_instance, WINDOW_BOX *bwin, Mapping *m,IMAGELIB_EFFECT effect_type,INT32 effect_para);

extern BOOL imagelib_postworkmessage(IMAGELIB_MSG_PARA_LIST** para_list);



#define DEFAULT_LCD_BITMAP_FILE "-.lcd_bitmap"

static BOOL imagelib_check_window(const IMAGE_INSTANCE *p,WINDOW_BOX * win)
{
    INT32 x0,y0,dx, dy;
    if(p == NULL)
	 return FALSE;
    image_get_box(p, &x0, &y0, &dx, &dy);
    if(win->y0 != IMAGE_UNDEFINED && win->y0>=dy)
	return FALSE;
    if(win->x0 != IMAGE_UNDEFINED  && win->x0 %3 !=0 )
    {
    	INT32 pad = 3 - win->x0 %3;
	win->x0 += pad;
	win->nx += pad;
	if(win->x0>=dx)
		return FALSE;
    }
    if(win->nx == 0 || win->nx + win->x0> dx)
    {
		win->nx  = dx -win->x0;
    }
    if(win->ny == 0 || win->ny + win->y0> dy)
    {
		win->ny  = dy -win->y0;
    }
    return TRUE;
    
}

static IMAGELIB_ERROR_CODE imagelib_main_impl(IMAGELIB_SESSION* _session,IMAGE_INSTANCE* _decoder_instance,IMAGE_INSTANCE* _encoder_instance,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height, IMAGELIB_EFFECT_PARA _effect_para)
{
    IMAGE_INSTANCE *decoder_instance = (IMAGE_INSTANCE *)_decoder_instance;
    IMAGE_INSTANCE *encoder_instance=(IMAGE_INSTANCE *)_encoder_instance;
    WINDOW_BOX decoder_window;		/* src window */
    WINDOW_BOX encoder_window;		/* dst window */
     
    if(_session == NULL || decoder_instance == NULL || encoder_instance== NULL)
		return IMAGELIB_RAM_EXECPTION;

 
   decoder_window.x0 = _src_rect.x;
   decoder_window.y0 = _src_rect.y;
   decoder_window.x1=IMAGE_UNDEFINED;
   decoder_window.y1=IMAGE_UNDEFINED;
   decoder_window.nx = _src_rect.width;
   decoder_window.ny = _src_rect.height;
   
   if(!imagelib_check_window(decoder_instance,(WINDOW_BOX *)&decoder_window))
   	return IMAGELIB_REGION_ERROR;
   	
   if(_src_rect.width== 0 || _src_rect.height==0)
   	image_get_window(decoder_instance, (WINDOW_BOX *)&decoder_window);
   if (decoder_window.x1==IMAGE_UNDEFINED) 
   	window_box_set_max(&decoder_window);

    encoder_window.x0 = 0;
    encoder_window.y0 = 0;
    encoder_window.x1=IMAGE_UNDEFINED;
    encoder_window.y1=IMAGE_UNDEFINED;
    encoder_window.nx = _dest_width;
    encoder_window.ny = _dest_height;
    if(_dest_width == 0 || _dest_height ==0)
    {
    		encoder_window.nx = decoder_window.nx;
		encoder_window.ny = decoder_window.ny;
    }
     if (encoder_window.x1==IMAGE_UNDEFINED) 
	 	window_box_set_max(&encoder_window);


  return 	image_zoom_entrance(_session,decoder_instance, &decoder_window, encoder_instance, &encoder_window,_effect_para.effect_type,_effect_para.effect_type_para);
}



IMAGELIB_ERROR_CODE imagelib_draw_frame_sync(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height,IMAGELIB_EFFECT_PARA _effect_para,
							    IMAGELIB_DRAW_PARA _draw_para)
{
	IMAGE_INSTANCE *decoder_instance,*encoder_instance=NULL;
	

	if(_session == NULL || _session->instance_p == NULL)
	{
		return IMAGELIB_RAM_EXECPTION;
	}
	decoder_instance = _session->instance_p;
	
	_session->error_code = image_open_by_filename(DEFAULT_LCD_BITMAP_FILE, "wb",&_draw_para,&encoder_instance);
	 if(_session->error_code != IMAGELIB_SUCCESS || encoder_instance==NULL)
		return _session->error_code;
	_session->state = IMAGELIB_DECODING;
       _session->error_code = imagelib_main_impl(_session,decoder_instance,encoder_instance,_src_rect,_dest_width,_dest_height,_effect_para);
	image_close(encoder_instance,TRUE);
	
	_session->state = IMAGELIB_IDLE;
	return _session->error_code;	
}


IMAGELIB_ERROR_CODE imagelib_save_frame_to_file_sync(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    const WCHAR* _dest_pathname,INT32 _dest_width,INT32 _dest_height,
							    IMAGELIB_EFFECT_PARA _effect_para )
{
	IMAGE_INSTANCE *decoder_instance,*encoder_instance=NULL;
	
	char utf_pathname[UTA_MAX_PATHNAME_LEN*2+1];
	if(_session == NULL || _session->instance_p == NULL)
	{
		return IMAGELIB_RAM_EXECPTION;
	}
	 decoder_instance = _session->instance_p;
	 
	UnicodeToUtf8(utf_pathname,_dest_pathname);
	_session->error_code=image_open_by_filename(utf_pathname, "wb",NULL,&encoder_instance);
	  if(_session->error_code != IMAGELIB_SUCCESS || encoder_instance==NULL)
		return _session->error_code;
	_session->state = IMAGELIB_DECODING;
	_session->error_code = imagelib_main_impl(_session,decoder_instance,encoder_instance,_src_rect,_dest_width,_dest_height,_effect_para);
  	_session->state = IMAGELIB_IDLE;
	image_close(encoder_instance,TRUE);
     	return _session->error_code;	
}


IMAGELIB_ERROR_CODE imagelib_get_frame_data_sync(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height, IMAGELIB_EFFECT_PARA _effect_para,
							    DISP_BITMAP** _bitmap_rt)
{
	IMAGE_INSTANCE *decoder_instance,*encoder_instance=NULL;


	if(_session == NULL || _session->instance_p == NULL)
	{
		return IMAGELIB_RAM_EXECPTION;
	}
	 decoder_instance = _session->instance_p;
	 
	_session->error_code= image_open_dev_by_imagedata("rab",NULL, 0,&encoder_instance);

	 if(_session->error_code != IMAGELIB_SUCCESS || encoder_instance==NULL)
		return _session->error_code;
	
	 _session->state = IMAGELIB_DECODING;
	_session->error_code = imagelib_main_impl(_session,decoder_instance,encoder_instance,_src_rect,_dest_width,_dest_height,_effect_para);
  	_session->state = IMAGELIB_IDLE;
	if(_session->error_code != IMAGELIB_SUCCESS)
	{
		image_close(encoder_instance,TRUE);
		*_bitmap_rt = NULL;
	}
	else
	{
		image_close(encoder_instance,FALSE);
		*_bitmap_rt = (DISP_BITMAP*)image_get_result(encoder_instance);
	}
	
     	return _session->error_code;	
}

 

 
 
 
static void imagelib_dispose_frame_ending(DECODE_FRAME_ENDING_CB cb,IMAGELIB_SESSION* session,void* para,IMAGELIB_ERROR_CODE error_code )
{
	if(cb != NULL)
	{
		cb(session,para,error_code);
	}

}

UINT32 imagelib_thread_main(IMAGELIB_MSG_PARA_LIST* msg)
{
	IMAGELIB_MSG_PARA_LIST* para_list = (IMAGELIB_MSG_PARA_LIST*)msg;
	
	IMAGELIB_SESSION* session = NULL;
	if(para_list == NULL || para_list->session==NULL)
		return 0;
	session = para_list->session;
	wait_sem(session->decode_sem);
	if(para_list->msg_id == IMAGELIB_SAVEFRAMEDATA_MSG ||para_list->msg_id == IMAGELIB_DRAWFRAMEDATA_MSG )
	{
		 session->error_code = imagelib_main_impl(session,para_list->src_instance,para_list->dest_instance,para_list->src_rect,
		 						para_list->dest_width,para_list->dest_height,para_list->effect_para);
	
		if (para_list->dest_instance != NULL) 
		{
			image_close(para_list->dest_instance,TRUE);		
		}
	
		if( session->state == IMAGELIB_DECODING)
		{
			session->state = IMAGELIB_IDLE;
			imagelib_dispose_frame_ending(para_list->ending_cb,session,NULL,para_list->session->error_code);
		}
	}

	else if(para_list->msg_id == IMAGELIB_GETFRAMEDATA_MSG)
	{
		DISP_BITMAP* bitmap=NULL;
		session->error_code =imagelib_main_impl(session,para_list->src_instance,para_list->dest_instance,para_list->src_rect,
		 						para_list->dest_width,para_list->dest_height,para_list->effect_para);
		if (para_list->dest_instance != NULL)  
		{
			if(session->error_code != IMAGELIB_SUCCESS)
			{
				image_close(para_list->dest_instance,TRUE);
				bitmap = NULL;
			}
			else
			{
				
				bitmap =(DISP_BITMAP*)image_get_result(para_list->dest_instance);
				image_close(para_list->dest_instance,FALSE);
				
			}
		}
		if( session->state == IMAGELIB_DECODING)
		{
			para_list->session->state = IMAGELIB_IDLE;
			imagelib_dispose_frame_ending(para_list->ending_cb,session,bitmap,para_list->session->error_code);
		}
		
	}
	if(para_list->session->state == IMAGELIB_CLOSED)
	{
		if(session->pathname)
			IMAGE_FREE(session->pathname);
		image_close((IMAGE_INSTANCE*)session->instance_p,TRUE);
		session->instance_p = NULL;
		if(para_list->session->decode_sem != NULL)
		{
			signal_sem(para_list->session->decode_sem);
			kill_sem(para_list->session->decode_sem);
			para_list->session->decode_sem = NULL;
		}
		IMAGE_FREE(para_list->session);
	}
	else
		signal_sem(session->decode_sem);
	IMAGE_FREE(para_list);
	return 0;
}

 

static IMAGELIB_MSG_PARA_LIST*  imagelib_copy_msg_paralist(IMAGELIB_MSG _msg_id,IMAGELIB_SESSION*_session,IMAGELIB_INSTANCE_P _src_instance,IMAGELIB_INSTANCE_P _dest_instance,IMAGELIB_RECT _src_rect,
				INT32 _dest_width,INT32 _dest_height,IMAGELIB_EFFECT_PARA _effect_para,DECODE_FRAME_ENDING_CB cb )
{
	IMAGELIB_MSG_PARA_LIST* para_list = IMAGE_MALLOC(sizeof(IMAGELIB_MSG_PARA_LIST));
	if(para_list==NULL)
		return NULL;
	para_list->msg_id = _msg_id;
	para_list->session = _session;
	 para_list->src_instance = (IMAGE_INSTANCE*)_src_instance;
	 para_list->dest_instance = (IMAGE_INSTANCE*)_dest_instance;
	 memcpy(&para_list->src_rect, &_src_rect, sizeof(IMAGELIB_RECT));
	 para_list->dest_width = _dest_width;
	 para_list->dest_height = _dest_height;
	 memcpy(&para_list->effect_para, &_effect_para, sizeof(IMAGELIB_EFFECT_PARA));
	 para_list->ending_cb = cb;
	 return para_list;
	 
}


IMAGELIB_ERROR_CODE imagelib_draw_frame_asyn(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height,IMAGELIB_EFFECT_PARA _effect_para ,
							    IMAGELIB_DRAW_PARA _draw_para,DECODE_FRAME_ENDING_CB cb )
{
	IMAGE_INSTANCE *decoder_instance,*encoder_instance=NULL;
	IMAGELIB_MSG_PARA_LIST* para_list = NULL;

	if(_session == NULL || _session->instance_p == NULL)
	{
		return IMAGELIB_RAM_EXECPTION;
	}
	wait_sem(_session->decode_sem);
	if(_session ->state == IMAGELIB_DECODING)
	{
		signal_sem(_session->decode_sem);
		return IMAGELIB_CURRENT_SESSION_BUSY;
	}
        decoder_instance = _session->instance_p;
	_session->error_code = image_open_by_filename(DEFAULT_LCD_BITMAP_FILE, "wb",&_draw_para,&encoder_instance);	  
	 if(_session->error_code != IMAGELIB_SUCCESS || encoder_instance==NULL)
	 {
	 	signal_sem(_session->decode_sem);
		return _session->error_code;
	 }
	 _session ->state = IMAGELIB_DECODING;
	 para_list = imagelib_copy_msg_paralist(IMAGELIB_DRAWFRAMEDATA_MSG,_session,decoder_instance,encoder_instance,_src_rect,_dest_width,_dest_height,_effect_para,cb);
	imagelib_postworkmessage(&para_list);
	signal_sem(_session->decode_sem);
     	return _session->error_code;	
}


IMAGELIB_ERROR_CODE imagelib_save_frame_to_file_asyn(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    const WCHAR* _dest_pathname,INT32 _dest_width,INT32 _dest_height,
							    IMAGELIB_EFFECT_PARA _effect_para,DECODE_FRAME_ENDING_CB cb)
{
	IMAGE_INSTANCE *decoder_instance,*encoder_instance=NULL;
	IMAGELIB_MSG_PARA_LIST* para_list = NULL;
	char utf_pathname[UTA_MAX_PATHNAME_LEN*2+1];
	if(_session == NULL || _session->instance_p == NULL)
	{
		return IMAGELIB_RAM_EXECPTION;
	}
	wait_sem(_session->decode_sem);
	if(_session ->state == IMAGELIB_DECODING)
	{
		signal_sem(_session->decode_sem);
		return IMAGELIB_CURRENT_SESSION_BUSY;
	}
	 decoder_instance = _session->instance_p;
	 
	UnicodeToUtf8(utf_pathname,_dest_pathname);
	_session->error_code=image_open_by_filename(utf_pathname, "wb",NULL,&encoder_instance);
	  if(_session->error_code != IMAGELIB_SUCCESS || encoder_instance==NULL)
	  {
	  	signal_sem(_session->decode_sem);
		return _session->error_code;
	  }
	_session ->state = IMAGELIB_DECODING;
	para_list = imagelib_copy_msg_paralist(IMAGELIB_SAVEFRAMEDATA_MSG,_session,decoder_instance,encoder_instance,_src_rect,_dest_width,_dest_height,_effect_para,cb);
	imagelib_postworkmessage(&para_list);
  	signal_sem(_session->decode_sem);
     	return _session->error_code;	
}


IMAGELIB_ERROR_CODE imagelib_get_frame_data_asyn(IMAGELIB_SESSION*_session,IMAGELIB_RECT _src_rect,
							    INT32 _dest_width,INT32 _dest_height, IMAGELIB_EFFECT_PARA _effect_para,DECODE_FRAME_ENDING_CB cb )
{
	IMAGE_INSTANCE *decoder_instance,*encoder_instance=NULL;
	IMAGELIB_MSG_PARA_LIST* para_list = NULL;

	if(_session == NULL || _session->instance_p == NULL)
	{
		return IMAGELIB_RAM_EXECPTION;
	}
	wait_sem(_session->decode_sem);
	if(_session ->state == IMAGELIB_DECODING)
	{
		signal_sem(_session->decode_sem);
		return IMAGELIB_CURRENT_SESSION_BUSY;
	}
	 decoder_instance = _session->instance_p;
	 
	_session->error_code= image_open_dev_by_imagedata("rab",NULL, 0,&encoder_instance);

	 if(_session->error_code != IMAGELIB_SUCCESS || encoder_instance==NULL)
	 {
	 	signal_sem(_session->decode_sem);
		return _session->error_code;
	 }
	 _session ->state = IMAGELIB_DECODING;
	 para_list = imagelib_copy_msg_paralist(IMAGELIB_GETFRAMEDATA_MSG,_session,decoder_instance,encoder_instance,_src_rect,_dest_width,_dest_height,_effect_para,cb);
	 if(para_list==NULL)
		 return IMAGELIB_RAM_EXECPTION;
	 imagelib_postworkmessage(&para_list);
	 signal_sem(_session->decode_sem);
       
     	return _session->error_code;	
}
 

IMAGELIB_ERROR_CODE imagelib_getinfo_by_filename(const WCHAR* _pathname,IMAGELIB_INFO* _info)
{
	char utf_pathname[UTA_MAX_PATHNAME_LEN*2];
	if(_pathname == NULL)
		return IMAGELIB_OTHER_ERROR;//return FALSE;
	UnicodeToUtf8(utf_pathname,_pathname);
	return image_get_info_by_filename(utf_pathname,_info);
	//return image_get_info_by_filename(utf_pathname,&_info->m_width,&_info->m_height);

	
}


IMAGELIB_ERROR_CODE imagelib_getinfo_by_imagedata(const char* dev,const char* buf,UINT32 buf_size,IMAGELIB_INFO* _info)
{
	//return image_get_info_by_imagedata(dev,buf,buf_size,&_info->m_width,&_info->m_height);
	return image_get_info_by_imagedata(dev,buf,buf_size,_info);
}


IMAGELIB_ERROR_CODE imagelib_start_decode_by_file(const WCHAR* _pathname,IMAGELIB_SESSION** _session)
{
	IMAGE_INSTANCE *instance = NULL;
	IMAGELIB_SESSION*  session =NULL;
	IMAGELIB_ERROR_CODE error_code;
	char utf_pathname[UTA_MAX_PATHNAME_LEN*2+1];
	if(_pathname == NULL)
		return IMAGELIB_RAM_EXECPTION;
	UnicodeToUtf8(utf_pathname,_pathname);
	error_code =image_open_by_filename(utf_pathname, "rb",NULL,&instance);
	if(error_code != IMAGELIB_SUCCESS)
	{
		*_session = NULL;
		return error_code;
	}
	session = IMAGE_MALLOC(sizeof(IMAGELIB_SESSION));
	if(session == NULL)
	{
		*_session = NULL;
		return IMAGELIB_RAM_EXECPTION;
	}
	session->error_code = error_code;
	session->pathname = IMAGE_MALLOC((wstrlen(_pathname)+1)*2);
	if(session->pathname== NULL)
	{
		*_session = NULL;
		IMAGE_FREE(session);
		return IMAGELIB_RAM_EXECPTION;
	}
	session->decode_sem = NULL;
	wstrcpy(session->pathname,_pathname);
	session->instance_p = (IMAGELIB_INSTANCE_P)instance;
	session->state= IMAGELIB_OPENED;
	*_session=session;
	session->decode_sem = create_sem(1);
	return IMAGELIB_SUCCESS;
}



IMAGELIB_ERROR_CODE imagelib_start_decode_by_imagedata(const char* _dev,const char* _image_data,UINT32 _size,IMAGELIB_SESSION** _session)
{
	IMAGE_INSTANCE *instance = NULL;
	IMAGELIB_SESSION*  session =NULL;
	IMAGELIB_ERROR_CODE error_code;
	if(_image_data == NULL)
		return IMAGELIB_RAM_EXECPTION;
	
	error_code=image_open_dev_by_imagedata(_dev,_image_data, _size,&instance);
	if(error_code != IMAGELIB_SUCCESS)
	{
		*_session = NULL;
		return error_code;
	}
	session = IMAGE_MALLOC(sizeof(IMAGELIB_SESSION));
	if(session == NULL)
	{
		*_session = NULL;
		return IMAGELIB_RAM_EXECPTION;
	}
	session->decode_sem = NULL;
	session->pathname = NULL;
	session->instance_p = (IMAGELIB_INSTANCE_P)instance;
	session->state= IMAGELIB_OPENED;
	*_session = session;
	return IMAGELIB_SUCCESS;
}

INT32  imagelib_get_next_frame(IMAGELIB_SESSION*_session)
{
	return image_next_pic((IMAGE_INSTANCE*)_session->instance_p);

}

UINT32 imagelib_get_width(IMAGELIB_SESSION*_session)
{
	if(_session == NULL)
		return 0;
	return image_get_width((IMAGE_INSTANCE*)_session->instance_p);
}
UINT32 imagelib_get_height(IMAGELIB_SESSION*_session)
{
	if(_session == NULL)
		return 0;
	return image_get_height((IMAGE_INSTANCE*)_session->instance_p);
}
IMAGELIB_IMAGE_TYPE imagelib_get_image_type(IMAGELIB_SESSION*_session)
{
	if(_session == NULL)
		return IMAGELIB_INVAILD_TYPE;
	return image_get_image_type((IMAGE_INSTANCE*)_session->instance_p);
}
 
IMAGELIB_ERROR_CODE imagelib_end_decode(IMAGELIB_SESSION*_session)
{
	IMAGELIB_ERROR_CODE error_code= IMAGELIB_SUCCESS;
	if(_session == NULL)
		return IMAGELIB_RAM_EXECPTION;

	if(_session->state != IMAGELIB_DECODING)
	{
		error_code =  image_close((IMAGE_INSTANCE*)_session->instance_p,TRUE);
		_session->instance_p = NULL;
		if(_session->pathname)
			IMAGE_FREE(_session->pathname);
		if(_session->decode_sem != NULL)
		{
			kill_sem(_session->decode_sem);
			_session->decode_sem = NULL;
		}
		IMAGE_FREE(_session);
		
	}
	else
		_session->state = IMAGELIB_CLOSED;
		
	return error_code;
}
