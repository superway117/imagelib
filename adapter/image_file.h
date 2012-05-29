
#ifndef IMAGELIB_FILE_H
#define IMAGELIB_FILE_H

#include <stdio.h>
#if defined(RED_ARROW)

#include "uta_fs.h"

extern INT16 UnicodeToUtf8(CHAR *p_utf8, const WCHAR *p_unicode);

#define IMAGE_FILE_HANDLER UtaIoHdl

#define IMAGE_INVAILD_FILE_HANDLER 0
#define IMAGE_FILE_OPEN UtaFsOpenFile

#define IMAGE_FILE_OPEN_CREATE_MODE UTA_FS_ACCESS_MODE_CREATE

#define IMAGE_FILE_OPEN_CREATE_WRITE_MODE UTA_FS_ACCESS_MODE_CREATE|UTA_FS_ACCESS_MODE_WRITE

#define IMAGE_FILE_OPEN_WRITE_MODE UTA_FS_ACCESS_MODE_WRITE

#define IMAGE_FILE_OPEN_READ_MODE UTA_FS_ACCESS_MODE_READ

#define IMAGE_FILE_OPEN_READ_WRITE_MODE UTA_FS_ACCESS_MODE_WRITE|UTA_FS_ACCESS_MODE_READ

#define IMAGE_FILE_CLOSE UtaFsCloseFile

#define IMAGE_FILE_READ(_buf,_size,_count,_filehandler)  (UtaFsReadFile(_filehandler,_buf,_size*_count))

#define IMAGE_FILE_WRITE(_buf,_size,_count,_filehandler)  (UtaFsWriteFile(_filehandler,_buf,_size*_count))

#define IMAGE_FILE_ORIGIN_SET UTA_FS_SEEK_ORIGIN_SET	//Beginning of file

#define IMAGE_FILE_ORIGIN_CUR UTA_FS_SEEK_ORIGIN_CUR	//Current position of the file pointer

#define IMAGE_FILE_ORIGIN_END UTA_FS_SEEK_ORIGIN_END	//End of file

#define IMAGE_FILE_SEEK(_filehandler,_offset,_origin)  (UtaFsSeekFile(_filehandler,_origin,_offset))



#else


#if 0
#define IMAGE_FILE_FILENAME_STRING_TYPE	char

#define IMAGE_FILE_FILENAME_CONVERT(_name,_converted_name) 
#endif

#define IMAGE_FILE_HANDLER FILE*

#define IMAGE_INVAILD_FILE_HANDLER NULL
#define IMAGE_FILE_OPEN fopen

#define IMAGE_FILE_OPEN_CREATE_WRITE_MODE "wb"

#define IMAGE_FILE_OPEN_WRITE_MODE "wb"

#define IMAGE_FILE_OPEN_READ_MODE "rb"

#define IMAGE_FILE_OPEN_READ_WRITE_MODE "w+"

#define IMAGE_FILE_CLOSE fclose

#define IMAGE_FILE_READ(_buf,_size,_count,_filehandler)  (fread(_buf,_size,_count,_filehandler))

#define IMAGE_FILE_WRITE(_buf,_size,_count,_filehandler)  (fwrite(_buf,_size,_count,_filehandler))

#define IMAGE_FILE_ORIGIN_SET SEEK_SET		//Beginning of file

#define IMAGE_FILE_ORIGIN_CUR SEEK_CUR	//Current position of the file pointer

#define IMAGE_FILE_ORIGIN_END SEEK_END	//End of file

#define IMAGE_FILE_SEEK(_filehandler,_offset,_origin)  (fseek(_filehandler,_offset,_origin))



#endif


#endif
