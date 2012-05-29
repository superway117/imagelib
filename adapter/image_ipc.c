#ifdef WIN32
#include <windows.h>
#endif

#include "portab.h"
#include <image_types.h>
#include <image_common.h>


#ifndef WIN32

extern UtaOsQueue                                       g_QImgLib;

void imagelib_thread(UINT32 argc,void* argv)
{
	UINT32 msg;
	UtaInt status;

	while(1)
	{
        status = UtaOsQueueReceive(&g_QImgLib, &msg, UTA_TRUE);

		if(status == UTA_SUCCESS)
			imagelib_thread_main((IMAGELIB_MSG_PARA_LIST*)msg);

	}		
}

BOOL imagelib_postworkmessage(IMAGELIB_MSG_PARA_LIST** para_list)
{
	UtaInt status;
	
	if(para_list == NULL || *para_list == NULL)
		return FALSE;

    status = UtaOsQueueSend(&g_QImgLib, ( void *)(para_list), UTA_FALSE);

	return (status == UTA_SUCCESS);
}

#else

#include <process.h>

extern UINT32 imagelib_thread_main(IMAGELIB_MSG_PARA_LIST* msg);

#define     WM_IMAGELIB_MESSAGE  0x8183

static UINT32 s_thread_id = 0xFF;
static void* s_thread_handle = NULL;

extern  UINT32 imagelib_thread_main(IMAGELIB_MSG_PARA_LIST* msg);
static UINT32 imagelib_thread(void* para);

void imagelib_start_thread()
{
	s_thread_handle = CreateThread(NULL,   //security attribute
                            0,     //64K stack
                            (LPTHREAD_START_ROUTINE)imagelib_thread,
                            NULL,          //parameters
                            0,
                            (LPDWORD)(&s_thread_id)); 
}

BOOL imagelib_postworkmessage(IMAGELIB_MSG_PARA_LIST** para_list)
{

	if(para_list == NULL || *para_list == NULL)
		return FALSE;
	
	return PostThreadMessage(s_thread_id,WM_IMAGELIB_MESSAGE,0,(UINT32)(*para_list));
}

static UINT32 imagelib_thread(void* para)
{
	MSG msg;

	while(1)
	{
		GetMessage(&msg,NULL,WM_IMAGELIB_MESSAGE, WM_IMAGELIB_MESSAGE);
	    
		imagelib_thread_main((IMAGELIB_MSG_PARA_LIST*)msg.lParam);

	}
	ExitThread(0);
	return 0;
		
}

#endif