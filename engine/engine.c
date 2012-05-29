
#include <image_headfile.h>


INT32 image_instance_number = -1;

void image_init(void)
{
    /* count the pic device types to set npic */
    for (image_instance_number=0; image_instance_number<IMAGE_INSTANCE_LISTMAX && image_instance_list[image_instance_number]; image_instance_number++);
}
IMAGELIB_ERROR_CODE image_get_info_by_filename(const char *file,IMAGELIB_INFO* info)
{
    int i;
    IMAGE_INSTANCE *pic;
    char* dev=image_file_dev(file);
    IMAGELIB_ERROR_CODE rt_code = IMAGELIB_UNKNOW_FORMAT;
    if (image_instance_number<0) image_init();
    if (!dev)
    {
        return IMAGELIB_UNKNOW_FORMAT;
    }
    for (i=0; i<image_instance_number && !STR_EQUAL(dev, image_instance_list[i]->dev); i++)
        ;
    if (i>=image_instance_number)
    {

        return IMAGELIB_UNKNOW_FORMAT;
    }
    pic = image_instance_list[i];
    return (*pic->procs->get_info_by_filename)(file, info);

}


IMAGELIB_ERROR_CODE image_get_info_by_imagedata(const char *dev, const char *image_data, UINT32 data_size,IMAGELIB_INFO* info)
{
    INT32 i;
    IMAGE_INSTANCE *pic;
    IMAGELIB_ERROR_CODE rt_code = IMAGELIB_SUCCESS;
    if (image_instance_number<0) image_init();
    if (!dev)
    {			/* probably comes from pic_file_dev */

        return IMAGELIB_UNKNOW_FORMAT;
    }
    for (i=0; i<image_instance_number && !STR_EQUAL(dev, image_instance_list[i]->dev); i++)
        ;
    if (i>=image_instance_number)
    {
        return IMAGELIB_UNKNOW_FORMAT;
    }
    pic = image_instance_list[i];
    return (*pic->procs->get_info_by_imagedata)(image_data,data_size, info);

}

IMAGELIB_ERROR_CODE image_open_by_filename(const char *file, const char *mode,const void* open_para,IMAGE_INSTANCE** _instance_rt)
{
    return image_open_dev(image_file_dev(file), file, mode,open_para,_instance_rt);
}

IMAGELIB_ERROR_CODE image_open_dev_by_imagedata(const char *dev, const char *image_data, UINT32 data_size,IMAGE_INSTANCE** _instance_rt)
{
    int i;
    void *data;
    IMAGE_INSTANCE *p, *q;
    IMAGELIB_ERROR_CODE error_code;

    if (image_instance_number<0) image_init();
    if (!dev)
    {

        return IMAGELIB_UNKNOW_FORMAT;
    }
    for (i=0; i<image_instance_number && !STR_EQUAL(dev, image_instance_list[i]->dev); i++)
        ;
    if (i>=image_instance_number)
    {

        return IMAGELIB_UNKNOW_FORMAT;
    }
    q = image_instance_list[i];
    error_code = (*q->procs->open_by_imagedata)(image_data, data_size,&data);
    if (error_code !=IMAGELIB_SUCCESS)
        return error_code;

    /* copy the IMAGE_INSTANCE structure before modifying it */
    IMAGE_ALLOC(p, IMAGE_INSTANCE, 1);
    *p = *q;
    p->data = data;
    *_instance_rt = p;
    return error_code;
}
IMAGELIB_ERROR_CODE image_open_dev(const char *dev, const char *name, const char *mode,const void* open_para,IMAGE_INSTANCE** _instance_rt)
{
    int i;
    void *data;
    IMAGELIB_ERROR_CODE error_code;
    IMAGE_INSTANCE *p, *q;

    if (image_instance_number<0) image_init();
    if (!dev)
    {

        return IMAGELIB_UNKNOW_FORMAT;
    }
    for (i=0; i<image_instance_number && !STR_EQUAL(dev, image_instance_list[i]->dev); i++)
        ;
    if (i>=image_instance_number) {

        return IMAGELIB_UNKNOW_FORMAT;
    }
    q = image_instance_list[i];
    error_code = (*q->procs->open_by_filename)((char *)name, mode,open_para,&data);
    if (error_code !=IMAGELIB_SUCCESS)
        return error_code;

    /* copy the IMAGE_INSTANCE structure before modifying it */
    IMAGE_ALLOC(p, IMAGE_INSTANCE, 1);
    *p = *q;
    p->data = data;
    *_instance_rt = p;
    return error_code;
}

IMAGELIB_ERROR_CODE image_close(IMAGE_INSTANCE *p,BOOL force)
{
    IMAGELIB_ERROR_CODE error_code=IMAGELIB_SUCCESS;
    if (p != NULL)
    {
        error_code= (*(p)->procs->close)((p)->data,force);
        IMAGE_FREE(p);
    }
    else
        error_code=IMAGELIB_RAM_EXECPTION;
    return error_code;
}

IMAGELIB_ERROR_CODE image_read_row_rgba(IMAGE_INSTANCE *p, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA * buf)
{
    if (p == NULL)
        return IMAGELIB_RAM_EXECPTION;
    return (*(p)->procs->read_row_rgba)((p)->data, y, x0, nx, buf);
}

IMAGELIB_ERROR_CODE image_write_row_rgba(IMAGE_INSTANCE *p, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA * buf)
{
    if (p == NULL)
        return IMAGELIB_RAM_EXECPTION;
    return (*(p)->procs->write_row_rgba)((p)->data, y, x0, nx, buf);
}


INT32  image_get_width(IMAGE_INSTANCE *p)
{
    if (p == NULL)
        return 0;
    return (*(p)->procs->get_width)((p)->data);
}

INT32  image_get_height(IMAGE_INSTANCE *p)
{
    if (p == NULL)
        return 0;
    return (*(p)->procs->get_height)((p)->data);
}

IMAGELIB_IMAGE_TYPE  image_get_image_type(IMAGE_INSTANCE *p)
{
    if (p == NULL)
        return IMAGELIB_INVAILD_TYPE;
    return (*(p)->procs->get_image_type)((p)->data);
}

void image_set_box(IMAGE_INSTANCE *p, INT32 ox, INT32 oy, INT32 dx, INT32 dy)
{
    if (p == NULL)
        return;
    (*(p)->procs->set_box)((p)->data, ox, oy, dx, dy);
}

void image_get_box(const IMAGE_INSTANCE *p, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy)
{
    if (p == NULL)
        return;
    (*(p)->procs->get_box)((p)->data, ox, oy, dx, dy);
}
#if 0
void image_set_nchan(IMAGE_INSTANCE *p, INT32 nchan)
{
    if (p == NULL)
        return;
    (*(p)->procs->set_nchan)((p)->data, nchan);
}

INT32 image_get_nchan(IMAGE_INSTANCE *p)
{
    if (p == NULL)
        return 0;
    return (*(p)->procs->get_nchan)((p)->data);
}
#endif

INT32 image_next_pic(IMAGE_INSTANCE *p)
{
    if (p == NULL)
        return 0;
    return ((p)->procs->next_pic ? (*(p)->procs->next_pic)((p)->data) : 0);
}

void* image_get_result(IMAGE_INSTANCE *p)
{
    if (p == NULL)
        return NULL;
    return (*(p)->procs->get_result)((p)->data);

}


void image_set_window(IMAGE_INSTANCE *p, const WINDOW *win)
{
    if (p == NULL)
        return;
    image_set_box(p, win->x0, win->y0, win->x1-win->x0+1, win->y1-win->y0+1);
}


WINDOW *image_get_window(const IMAGE_INSTANCE *p, WINDOW *win)
{
    INT32 dx, dy;
    if (p == NULL)
        return NULL;
    if (!win) IMAGE_ALLOC(win, WINDOW, 1);
    image_get_box(p, &win->x0, &win->y0, &dx, &dy);
    win->x1 = win->x0+dx-1;
    win->y1 = win->y0+dy-1;
    return win;
}
