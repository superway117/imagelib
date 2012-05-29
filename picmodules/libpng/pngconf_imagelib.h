/*	------------------------------------------------------------------------
	Copyright (C) COMNEON electronic technology GmbH & Co. OHG.
	All rights reserved.
	------------------------------------------------------------------------
	This document contains proprietary information belonging to COMNEON.
	Passing on and copying of this document, use and communication of its
	contents is not permitted without prior written authorisation.
	------------------------------------------------------------------------ */

#if !defined(pngconf_APOXI_h)
#define pngconf_APOXI_h

#include <setjmp.h>

/* This configuration file defines the settings for using libpng within the 
 * APOXI framework.
 */

/* On 16-bit target platforms the APOXI framework has to use huge pointers 
 * which are handled by the FAR define in libpng (see zconf_APOXI.h for further
 * details).
 */
 #define PNG_ABORT() 



 #ifndef PNG_SETJMP_NOT_SUPPORTED
 	#  define PNG_SETJMP_NOT_SUPPORTED
 #endif

 

#if !defined(PNG_USER_MEM_SUPPORTED)
	#define PNG_USER_MEM_SUPPORTED
#endif

#if !defined(PNG_STATIC)
	#define PNG_STATIC
#endif

#if !defined(PNG_NO_WRITE_SUPPORTED)
	#define PNG_NO_WRITE_SUPPORTED
#endif

#if !defined(PNG_NO_CONSOLE_IO)
	#define PNG_NO_CONSOLE_IO
#endif

#if !defined(PNG_WRITE_SUPPORTED)
	#define PNG_WRITE_SUPPORTED
#endif
#if !defined(PNG_NO_STDIO)
	#define PNG_NO_STDIO
#endif

#if !defined(PNG_NO_PROGRESSIVE_READ)
	#define PNG_NO_PROGRESSIVE_READ
#endif

#if !defined(PNG_NO_ASSEMBLER_CODE)
	#define PNG_NO_ASSEMBLER_CODE
#endif

#if !defined(PNG_NO_EASY_ACCESS)
	#define PNG_NO_EASY_ACCESS
#endif

#if !defined(PNG_NO_FLOATING_POINT_SUPPORTED)
	#define PNG_NO_FLOATING_POINT_SUPPORTED
#endif

#if !defined(PNG_NO_READ_TEXT)
	#define PNG_NO_READ_TEXT
#endif

#if !defined(PNG_NO_READ_UNKNOWN_CHUNKS)
	#define PNG_NO_READ_UNKNOWN_CHUNKS
#endif

#if !defined(PNG_NO_READ_USER_CHUNKS)
	#define PNG_NO_READ_USER_CHUNKS
#endif

#if !defined(PNG_NO_INFO_IMAGE)
	#define PNG_NO_INFO_IMAGE
#endif

#endif	// pngconf_APOXI_h


