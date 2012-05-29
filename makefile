# ------------------------------------------------------------------------ #
# RedArrow Image library                                                         #
# ------------------------------------------------------------------------ #
BUILD_LIB=redarrow_imagelib

# ------------------------------------------------------------------------ #
# Include project related information
# ------------------------------------------------------------------------ #
include ../${TARGETSYSTEM}/${PLATFORM}/project.cfg

# ------------------------------------------------------------------------ #
# Include tool(s) related information
# ------------------------------------------------------------------------ #
include ./tools.mk

# ------------------------------------------------------------------------ #
# Include global makeoptions file
# ------------------------------------------------------------------------ #
include ./makeoptions.mk


# ------------------------------------------------------------------------ #
# List of source SDL files
# ------------------------------------------------------------------------ #
${BUILD_LIB}_SDL_C_FILES = 

${BUILD_LIB}_SDL_H_FILES = 

    
${BUILD_LIB}_IFC_FILES = 

# ------------------------------------------------------------------------ #
# List of objects to be compiled
# ------------------------------------------------------------------------ #
${BUILD_LIB}_FILES =
# ------------------------------------------------------------------------ #
# List of objects to be compiled in thumb mode
# ------------------------------------------------------------------------ #

${BUILD_LIB}_THUMB_FILES = \
	zutil.c \
  uncompr.c \
  trees.c \
  inftrees.c \
  inflate.c \
  inffast.c \
  infback.c \
  gzio.c \
  deflate.c \
  crc32.c \
  compress.c \
  adler32.c \
  png_wrap.c \
  native_bitmap.c \
  pngwutil.c \
  pngwtran.c \
  pngwrite.c \
  pngwio.c \
  pngvcrd.c \
  pngtrans.c \
  pngset.c \
  pngrutil.c \
  pngrtran.c \
  pngrio.c \
  pngread.c \
  pngpread.c \
  pngmem.c \
  PngHugeMem.c \
  pngget.c \
  pnggccrd.c \
  pngerror.c \
  png.c \
  lcd_bitmap.c \
  jpg.c \
  quantize.c \
  qprintf.c \
  gif_hash.c \
  gif_font.c \
  gif_err.c \
  gifalloc.c \
  egif_lib.c \
  dgif_lib.c \
  dev2gif.c \
  gif.c \
  bmp.c \
  jutils.c \
  jquant2.c \
  jquant1.c \
  jmemnobs.c \
  jmemmgr.c \
  jidctred.c \
  jidctfst.c \
  jfdctint.c \
  jfdctfst.c \
  jerror.c \
  jdsample.c \
  jdpostct.c \
  jdphuff.c \
  jdmerge.c \
  jdmaster.c \
  jdmarker.c \
  jdmainct.c \
  jdinput.c \
  jdhuff.c \
  jddctmgr.c \
  jdcolor.c \
  jdcoefct.c \
  jdapistd.c \
  jdapimin.c \
  jcsample.c \
  jcprepct.c \
  jcphuff.c \
  jcparam.c \
  jcomapi.c \
  jcmaster.c \
  jcmarker.c \
  jcmainct.c \
  jcinit.c \
  jchuff.c \
  jcdctmgr.c \
  jccolor.c \
  jccoefct.c \
  jcapistd.c \
  jcapimin.c \
  image_main.c \
  zoom.c \
  window.c \
  image_register.c \
  image_effect.c \
  image_dev.c \
  engine.c \
  image_ipc.c



# ------------------------------------------------------------------------ #
# Additional library specific flags
# ------------------------------------------------------------------------ #
CPP_DEFS +=

CC_DEFS += 
ASM_DEFS += 

# ------------------------------------------------------------------------ #
# MAKE path and compiler include path settings
# ------------------------------------------------------------------------ #
${BUILD_LIB}_INC = \
		${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib/adapter \
    ${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib/engine \
    ${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib/image \
    ${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib/picmodules \
    ${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib/picmodules/zlib \
    ${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib/picmodules/giflib \
    ${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib/picmodules/libpng	\
    ${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib/picmodules/jpeg-6b	\
    ${GLOBALPATH}/red_arrow/Source/Add_Ons/ImageLib \
    ${GLOBALPATH}/uta_inc/uta	\
    ${GLOBALPATH}/red_arrow/Include/inc_add_ons/inc_multimedia \
    ${GLOBALPATH}/red_arrow/Include/inc_ui \
    ${GLOBALPATH}/red_arrow/Include/inc_api \
    ${GLOBALPATH}/platform-src/platform/text \
    ${GLOBALPATH}/stack-interface/globals \
    ${GLOBALPATH}/stack-interface/sdl \
    ${GLOBALPATH}/ms-mi-src/text \
    ${GLOBALPATH}/ms-src/text \
    ${GLOBALPATH}/dwddrvif/inc \
    ${GLOBALPATH}/sdl-build/HW/sdl/text \
    ${GLOBALPATH}/sdl-build/HW/text \
    ${GLOBALPATH}/platform-src/tic/text \
    ${GLOBALPATH}/os-src/${PLATFORMOS}/text \
    ${GLOBALPATH}/dwddrv/SW_PLATFORM/src \
    ${GLOBALPATH}/ms-interface-src/h \
    ${GLOBALPATH}/platform-src/xd/text \
    ${GLOBALPATH}/platform-src/tic/text \
    ${GLOBALPATH}/platform-src/sio/text

# ------------------------------------------------------------------------ #
# Include compiler related information
# ------------------------------------------------------------------------ #
include ./${COMPILER}.mk

vpath %.h                ${${BUILD_LIB}_INC}
vpath %.c                ${${BUILD_LIB}_INC}
vpath %.asm              ${${BUILD_LIB}_INC}
vpath %.${DEPFILEEXT}    ${DEPDIR}/${BUILD_LIB}
vpath %.${OBJFILEEXT}    ${OBJDIR}/${BUILD_LIB}

# ------------------------------------------------------------------------ #
# Include rules to build the targets
# ------------------------------------------------------------------------ #
include ./target_rules.mk
