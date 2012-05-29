
#ifndef IMAGELIB_COMMON_H
#define IMAGELIB_COMMON_H

#include <image_lib.h>
#include <engine.h>

typedef enum
{
     IMAGELIB_NULL_MSG,
     IMAGELIB_SAVEFRAMEDATA_MSG,
     IMAGELIB_DRAWFRAMEDATA_MSG,
     IMAGELIB_GETFRAMEDATA_MSG,
}IMAGELIB_MSG;

typedef struct
{
	IMAGELIB_MSG   msg_id;
	IMAGELIB_SESSION* session;
	IMAGE_INSTANCE* src_instance;
	IMAGE_INSTANCE* dest_instance;
	IMAGELIB_RECT src_rect;
	INT32 dest_width;
	INT32 dest_height;
	IMAGELIB_EFFECT_PARA effect_para;
	DECODE_FRAME_ENDING_CB	ending_cb;
	
}IMAGELIB_MSG_PARA_LIST;

#endif
