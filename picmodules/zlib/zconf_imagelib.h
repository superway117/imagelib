/*	------------------------------------------------------------------------
	Copyright (C) COMNEON electronic technology GmbH & Co. OHG.
	All rights reserved.
	------------------------------------------------------------------------
	This document contains proprietary information belonging to COMNEON.
	Passing on and copying of this document, use and communication of its
	contents is not permitted without prior written authorisation.
	------------------------------------------------------------------------ */

#if !defined(zconf_APOXI_h)
#define zconf_APOXI_h

#include "portab.h"
/* This configuration file defines the settings for using zlib within the APOXI
 * framework.
 */

//#include <Kernel/Types.h>

/* Adaptations for the APOXI framework on target platforms:
 * - Zlib uses the define FAR for declaring huge pointers. We define FAR to 
 *   ZLIB_HUGE which is defined to HUGE for support of 16-bit target compilers.
 * - Zlib must not use the standard C memory functions due to the handling of 
 *   huge pointers (see above). For this reason zlib has to build its own 
 *   implementations of these functions (see zutil.h and zutil.c) which use the
 *   correct huge pointer types. To accomplish this we have to assure that 
 *   NO_MEMCPY is defined. Define APOXI_ZLIB_USE_STACK_MEM_FUNCS triggers the 
 *   usage of the highly optimized APOXI stack memory functions.
 *
 * Usage hint for clients of zlib:
 * - APOXI clients of zlib are expected to use the define ZLIB_HUGE for 
 *   declaring huge pointers that are used to communicate with zlib.
 */
//#if defined(APOXI_RTOS_NUC16) || defined(APOXI_RTOS_OSE166)
#ifndef WIN32
#ifdef _OSE_TYPES_H
	#if !defined(ZLIB_HUGE)
		#define ZLIB_HUGE huge
	#endif
#else
	#if !defined(ZLIB_HUGE)
		#define ZLIB_HUGE
	#endif
	#endif

	#if !defined(NO_MEMCPY)
		#define NO_MEMCPY
	#endif

	#if !defined(MAXSEG_64K)
		#define MAXSEG_64K
	#endif

	//#if !defined(APOXI_ZLIB_USE_STACK_MEM_FUNCS)
	//	#define APOXI_ZLIB_USE_STACK_MEM_FUNCS
	//#endif
//#elif defined(APOXI_RTOS_NUC32) || defined(APOXI_RTOS_WIN32)
#elif defined(WIN32)
	#if !defined(ZLIB_HUGE)
		#define ZLIB_HUGE
	#endif
#else
	#error APOXI_RTOS not defined or zlib is not tested for that operating system!
#endif

#if defined(FAR)
	#undef FAR
#endif
#define FAR ZLIB_HUGE
#endif	// zconf_APOXI_h


