
#include <string.h>
#if defined(WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <image_headfile.h>


typedef struct 
{
    char *dev;			/* device name */
    char *suffix;		/* file suffix */
} DEV_INFO;

static  DEV_INFO supported_dev_list[] = 
{
 /*  DEV      SUFFIX	*/
    "lcd_bitmap",    "lcd_bitmap",  
    "rab",    "rab",
    "bmp",    "bmp", 
    "jpg",    "jpg",
    "jpg",    "jpeg",
    "jpg",    "pjpg",	
    "jpg",    "pjpeg",	
    "png",    "png",
    "gif",    "gif", 
    "tif",    "tif", 
    "tif",    "tiff", 
    "pnm",    "pbm",  
    "pnm",    "pgm", 
    "pnm",    "ppm", 
    "pnm",    "pnm", 
    "iris",   "iris",
    "rle",    "rle", 
    "rle",    "Z",
    "rle",    "z", 
    
};
#define NUM_OF_DEVLIST (sizeof supported_dev_list / sizeof supported_dev_list[0])


char *image_file_dev(char* file)
{
    char *suffix;
    char* suffix_p;
    DEV_INFO *d;
    
    suffix = strrchr(file, '.');
    if (suffix) 
    {
   	 suffix++;
    }
    else 
    {
	suffix = strrchr(file, '/');
	suffix = suffix ? suffix+1 : file;
    }
    suffix_p = IMAGE_MALLOC(strlen(suffix)+1);
    strcpy(suffix_p,suffix);
    strlwr(suffix_p);	
    for (d=supported_dev_list; d<supported_dev_list+NUM_OF_DEVLIST; d++)
    {
	if (STR_EQUAL(d->suffix, suffix_p)) 
	{
		IMAGE_FREE(suffix_p);
		return d->dev;
	}
    }
    /*  failure */
    IMAGE_FREE(suffix_p);
    return 0;
}


void	exit_fake(INT32 exit_code)
{

}