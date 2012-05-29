/*	------------------------------------------------------------------------
	Copyright (C) COMNEON electronic technology GmbH & Co. OHG.
	All rights reserved.
	------------------------------------------------------------------------
	This document contains proprietary information belonging to COMNEON.
	Passing on and copying of this document, use and communication of its
	contents is not permitted without prior written authorisation.
	------------------------------------------------------------------------ */

#if !defined(Apoxi_PngHugeMem_h)
#define Apoxi_PngHugeMem_h

#ifdef __cplusplus
	extern "C" {
#endif

#include "png.h"

int png_memcmp(const png_voidp s1, const png_voidp s2, png_size_t len);
png_voidp png_memcpy(png_voidp dest, const png_voidp source, png_size_t len);
png_voidp png_memset(png_voidp dest, int value, png_size_t len);

#ifdef __cplusplus
	}
#endif

#endif	// Apoxi_PngHugeMem_h


