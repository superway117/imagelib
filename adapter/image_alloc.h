

#ifndef IMAGELIB_ALLOC_H
#define IMAGELIB_ALLOC_H

#if defined(RED_ARROW)

#include "portab.h"


//#define IMAGE_MALLOC(size) ((size)<(16*1024))? MMI_MALLOC((UINT16)(size)):MMI_HMALLOC((UINT32)(size))
#define IMAGE_MALLOC(size) ((size)<(16*1024))? MMI_MALLOC((UINT16)(size)):MMI_HMALLOC((UINT32)(size))

#define IMAGE_FREE(p) ((isHugeBlock(p)) ==1)?MMI_HFREE(p):MMI_FREE(p)

#define IMAGE_ALLOC(ptr, type, n) (((n)*sizeof(type)<(16*1024))? (ptr = (type *)MMI_MALLOC((UINT16)((n)*sizeof(type)))):(ptr = (type *)MMI_HMALLOC((UINT32)((n)*sizeof(type)))) )

#define IMAGE_CALLOC(count,size) ((((count*size)<(16*1024))? MMI_CALLOC((unsigned short)(count*size)):MMI_HCALLOC((unsigned short)(count*size))))

#define IMAGE_ALLOC_ZERO(ptr, type, n) (((n)*sizeof(type)<(16*1024))? (ptr =MMI_CALLOC((UINT16)((n)*sizeof(type)))):(ptr =MMI_HCALLOC((UINT32)((n)*sizeof(type)))))

#define IMAGE_REALLOC(ptr,size) ((((size)<(16*1024))? GSM_REALLOC(ptr,(unsigned short)(size)):GSM_HREALLOC(ptr,(UINT32)(size))))
#else

#include <stdio.h>
#define IMAGE_MALLOC(size)  malloc(size)

#define IMAGE_FREE(p) free(p)

#define IMAGE_ALLOC(ptr, type, n) (ptr = (type *)malloc((n)*sizeof(type)))

#define IMAGE_CALLOC(count,size) calloc(count,size)

#define IMAGE_ALLOC_ZERO(ptr, type, n) (ptr = (type *)calloc((n),sizeof(type)))

#define IMAGE_REALLOC(ptr,size) (realloc(ptr,size))
#endif

#endif
