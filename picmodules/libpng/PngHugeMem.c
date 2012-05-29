/*	------------------------------------------------------------------------
	Copyright (C) COMNEON electronic technology GmbH & Co. OHG.
	All rights reserved.
	------------------------------------------------------------------------
	This document contains proprietary information belonging to COMNEON.
	Passing on and copying of this document, use and communication of its
	contents is not permitted without prior written authorisation.
	------------------------------------------------------------------------ */

#include "PngHugeMem.h"

/* This file provides three different implementations of memory functions using
 * huge pointers, from non optimized to highly optimized versions. Tests showed
 * that the highly optimized version is slightly more performant than the non 
 * optimized version, however, the performance boost is not very significant 
 * and on different target platforms it might outperform differently. To decide
 * for the most performant version for a distinct target platform, target 
 * platform performance tests have to be run with each version.
 */

/* Four defines control which kind of memory functions to build: 
 * - Define _PNGHUGEMEM_NON_OPTIMIZED for non optimized memory functions.
 * - Define _PNGHUGEMEM_SIMPLE_OPTIMIZED_TASKING for simple optimization using 
 *   the Tasking memory functions.
 * - Define _PNGHUGEMEM_HIGHLY_OPTIMIZED_TASKING for high optimization using 
 *   the Tasking memory functions.
 * - Define _PNGHUGEMEM_OPTIMIZED_APOXI_STACK for optimization using the Apoxi 
 *   stack memory functions. This should be the fastest method!
 */
//#ifdef WIN32
 #define _PNGHUGEMEM_NON_OPTIMIZED 
//#endif
/* #define _PNGHUGEMEM_SIMPLE_OPTIMIZED_TASKING */

/* #define _PNGHUGEMEM_HIGHLY_OPTIMIZED_TASKING */

/* #define _PNGHUGEMEM_OPTIMIZED_APOXI_STACK */

#if defined(_PNGHUGEMEM_NON_OPTIMIZED)
/* No optimization. */

int png_memcmp(const png_voidp s1, const png_voidp s2, png_size_t len)
{
	png_const_charp cs1 = s1;
	png_const_charp cs2 = s2;
	
	while (len-- != 0) {
		if (*cs1++ != *cs2++) {
			if (*(cs1 - 1) > *(cs2 - 1)) 
				return 1;
			else 
				return -1;
		}
	}
	
	return 0;
}

png_voidp png_memcpy(png_voidp dest, const png_voidp source, png_size_t len)
{
	png_charp d = dest;
	png_const_charp s = source;
	
	while (len-- != 0) {
		*d++ = *s++;
	}
	
	return dest;
}

png_voidp png_memset(png_voidp dest, int value, png_size_t len)
{
	png_charp d = dest;
	
	while (len-- != 0) {
		*d++ = value;
	}
	
	return dest;
}

#elif defined(_PNGHUGEMEM_SIMPLE_OPTIMIZED_TASKING)
/* Simple optimization using the Tasking memory functions. */

#include <string.h>

int png_memcmp(const png_voidp s1, const png_voidp s2, png_size_t len)
{
	png_const_charp cs1 = s1;
	png_const_charp cs2 = s2;
	
	while (len-- != 0) {
		if (*cs1++ != *cs2++) {
			if (*(cs1 - 1) > *(cs2 - 1)) 
				return 1;
			else 
				return -1;
		}
	}
	
	return 0;
}

png_voidp png_memcpy(png_voidp dest, const png_voidp source, png_size_t len)
{
	memcphhb(dest, source, len);
	return dest;
}

png_voidp png_memset(png_voidp dest, int value, png_size_t len)
{
	png_charp d = dest;
	
	while (len-- != 0) {
		*d++ = value;
	}
	
	return dest;
}

#elif defined(_PNGHUGEMEM_HIGHLY_OPTIMIZED_TASKING)
/* High optimization using the Tasking memory functions. */

#include <string.h>

int png_memcmp(const png_voidp s1, const png_voidp s2, png_size_t len)
{
	png_const_charp cs1 = s1;
	png_const_charp cs2 = s2;
	
	while (len-- != 0) {
		if (*cs1++ != *cs2++) {
			if (*(cs1 - 1) > *(cs2 - 1)) 
				return 1;
			else 
				return -1;
		}
	}
	
	return 0;
}

png_voidp png_memcpy(png_voidp dest, const png_voidp source, png_size_t len)
{
	png_charp d = dest;
	png_const_charp s = source;
	
	if (!((int)d & 0x1) && !((int)s & 0x1)) {
		/* Both buffers are aligned on word boundaries. */
		
		/* Copy all complete words. */
		memcphhw(d, s, len >> 1);
		
		if (len & 0x1) {
			/* Copy the last byte. */
			*(d + (len - 1)) = *(s + (len - 1));
		}
	}
	else if (((int)d & 0x1) && ((int)s & 0x1)) {
		/* Both buffers are not aligned on word boundaries. */

		/* Copy the first byte. */
		*d = *s;

		/* Copy all complete words. */
		memcphhw(++d, ++s, --len >> 1);

		if (len & 0x1) {
			/* Copy the last byte. */
			*(d + (len - 1)) = *(s + (len - 1));
		}
	}
	else {
		/* Only one of the buffers is word aligned. */
		/* It's only possible to copy byte by byte. */
		memcphhb(d, s, len);
	}

	return dest;
}

png_voidp png_memset(png_voidp dest, int value, png_size_t len)
{
	png_charp d = dest;
	
	while (len-- != 0) {
		*d++ = value;
	}
	
	return dest;
}

#elif defined(_PNGHUGEMEM_OPTIMIZED_APOXI_STACK)
/* Optimization using the Apoxi stack memory functions. This should be the 
 * fastest method! */

int png_memcmp(const png_voidp s1, const png_voidp s2, png_size_t len)
{
	png_const_charp cs1 = s1;
	png_const_charp cs2 = s2;
	
	while (len-- != 0) {
		if (*cs1++ != *cs2++) {
			if (*(cs1 - 1) > *(cs2 - 1)) 
				return 1;
			else 
				return -1;
		}
	}
	
	return 0;
}

png_voidp png_memcpy(png_voidp dest, const png_voidp source, png_size_t len)
{
	return (png_voidp) hmemcpy(dest, source, len);
}

png_voidp png_memset(png_voidp dest, int value, png_size_t len)
{
	return (png_voidp) hmemset(dest, value, len);
}

#else
	#error No memory functions selected

#endif


