#ifndef IMAGELIB_TYPES_H
#define IMAGELIB_TYPES_H

#if defined(RED_ARROW)
#include "portab.h"
#else
#include <stdio.h>
#endif


#define assert(p) p


#define STR_EQUAL(a, b)	(strcmp((a), (b)) == 0)
#define MIN(a, b)	((a)<(b) ? (a) : (b))
#define MAX(a, b)	((a)>(b) ? (a) : (b))
#define ABS(a)		((a)>=0 ? (a) : -(a))
#define SWAP(a, b, t)	{t = a; a = b; b = t;}
#define LERP(t, a, b)	((a)+(t)*((b)-(a)))



/* note: the following are machine dependent! (ifdef them if possible) */
#if defined(WIN32)
#define bzero(ptr_, size_) memset(ptr_, 0, size_)
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#else
#define bzero(ptr_, size_) memset(ptr_, 0, size_)
#endif

#endif
