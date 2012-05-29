#include <image_headfile.h>


#include "portab.h"
#include "lcd_api.h"

//lcd bitmap only support encode

/*------------------------------------------------------------*/
struct tagPicLCDBitmap
{
    INT32 width;
    INT32 height;
    UINT32 scanline;
    IMAGELIB_DRAW_PARA draw_para;
};
typedef struct tagPicLCDBitmap PicLCDBitmap;



static IMAGELIB_ERROR_CODE LCD_bitmap_open(char *filename, const char *mode,const void* open_para,void** data_rt)
{
    PicLCDBitmap *lcd_bmp = NULL;
    IMAGE_ALLOC(lcd_bmp, PicLCDBitmap, 1);
    if(lcd_bmp == NULL)
	 	return IMAGELIB_RAM_NOT_ENOUGH;
    memset(lcd_bmp, 0, sizeof(PicLCDBitmap));
    lcd_bmp->width = PIXEL_UNDEFINED;
    lcd_bmp->height = PIXEL_UNDEFINED;
    if (open_para!=NULL)
        memcpy((void*)&lcd_bmp->draw_para,open_para,sizeof(IMAGELIB_DRAW_PARA));
    *data_rt = lcd_bmp;
    return IMAGELIB_SUCCESS;
}


static IMAGELIB_ERROR_CODE LCD_bitmap_close(void *p,BOOL force)
{
    PicLCDBitmap *lcd_bmp = (PicLCDBitmap *) p;
    if (lcd_bmp == NULL)
        return IMAGELIB_RAM_EXECPTION;
    IMAGE_FREE(lcd_bmp);
    return IMAGELIB_SUCCESS;

}

static INT32 LCD_get_width(void *p)
{
    PicLCDBitmap *lcd_bmp = (PicLCDBitmap *) p;
    if (lcd_bmp == NULL)
        return 0;
    return lcd_bmp->width;
}

static INT32 LCD_get_height(void *p)
{
    PicLCDBitmap *lcd_bmp = (PicLCDBitmap *) p;
    if (lcd_bmp == NULL)
        return 0;
    return lcd_bmp->height;
}

static IMAGELIB_IMAGE_TYPE LCD_get_image_type(void *p)
{
    return IMAGELIB_LCD_BMP_TYPE;
}


static void LCD_bitmap_set_nchan(void *vp, INT32 nchan)
{

}



static void LCD_bitmap_set_box(void *vp, INT32 ox, INT32 oy, INT32 dx, INT32 dy)
{
    PicLCDBitmap *lcd_bmp = (PicLCDBitmap *) vp;
    IMAGELIB_ROTATE_ANGLE angle;
    if (lcd_bmp == NULL)
        return;
    angle = lcd_bmp->draw_para.m_angle;

    lcd_bmp->width = ox + dx;
    lcd_bmp->height = oy + dy;
    if (angle== IMAGELIB_ROTATE_0 || angle == IMAGELIB_ROTATE_180)
    {
        if (lcd_bmp->draw_para.m_rect.width>lcd_bmp->width)
        {
            lcd_bmp->draw_para.m_rect.x += (lcd_bmp->draw_para.m_rect.width-lcd_bmp->width)/2;
            lcd_bmp->draw_para.m_rect.width = lcd_bmp->width;
        }
        if (lcd_bmp->draw_para.m_rect.height>lcd_bmp->height)
        {
            lcd_bmp->draw_para.m_rect.y += (lcd_bmp->draw_para.m_rect.height-lcd_bmp->height)/2;
            lcd_bmp->draw_para.m_rect.height = lcd_bmp->height;
        }
    }
    else
    {
        if (lcd_bmp->draw_para.m_rect.width>lcd_bmp->height)
        {
            lcd_bmp->draw_para.m_rect.x += (lcd_bmp->draw_para.m_rect.width-lcd_bmp->height)/2;
            lcd_bmp->draw_para.m_rect.width = lcd_bmp->height;
        }
        if (lcd_bmp->draw_para.m_rect.height>lcd_bmp->width)
        {
            lcd_bmp->draw_para.m_rect.y += (lcd_bmp->draw_para.m_rect.height-lcd_bmp->width)/2;
            lcd_bmp->draw_para.m_rect.height = lcd_bmp->width;
        }
    }


}



static void LCD_bitmap_seek_row_write(PicLCDBitmap *bmp, INT32 y)
{
#if 0
    while (bmp->scanline < y) {

        bmp->scanline++;
    }
#endif
}


static IMAGELIB_ERROR_CODE LCD_bitmap_write_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, const PIXEL_RGBA *buf)
{
    PicLCDBitmap *lcd_bmp = (PicLCDBitmap *) vp;
    IMAGELIB_ROTATE_ANGLE angle = lcd_bmp->draw_para.m_angle;
    INT32 i,src_index;
    INT32 draw_rect_x,draw_rect_y,draw_rect_width,draw_rect_height,draw_x,draw_y;

    if (lcd_bmp == NULL)
        return IMAGELIB_RAM_EXECPTION;
    angle = lcd_bmp->draw_para.m_angle;

    draw_rect_x = lcd_bmp->draw_para.m_rect.x;
    draw_rect_y = lcd_bmp->draw_para.m_rect.y;
    draw_rect_width = lcd_bmp->draw_para.m_rect.width;
    draw_rect_height = lcd_bmp->draw_para.m_rect.height;

    if (angle == IMAGELIB_ROTATE_0)		//ok
    {
        if (y<lcd_bmp->draw_para.m_src_y)
            return IMAGELIB_SUCCESS;
        nx-=lcd_bmp->draw_para.m_src_x;
        if (nx >draw_rect_width)			//no use x0, it need redo
            nx = draw_rect_width;

        draw_y= draw_rect_y+y-lcd_bmp->draw_para.m_src_y;
        if (draw_y>=0 && draw_y<=draw_rect_y+draw_rect_height)
        {
            for (i = 0,src_index=lcd_bmp->draw_para.m_src_x; i < nx; i++,src_index++)
            {
                draw_x= draw_rect_x+i;
                if (draw_x < 0 || draw_x>draw_rect_x+draw_rect_width)
                    continue;
                disp_DrawPixelWithColor_1((UINT16)draw_x,(UINT16)draw_y,RGB16(buf[src_index].r,buf[src_index].g,buf[src_index].b));
            }
        }
    }
    else if (angle == IMAGELIB_ROTATE_90)
    {
        if (nx >draw_rect_height)			//no use x0, it need redo
            nx = draw_rect_height;
        draw_x= draw_rect_x+(lcd_bmp->height-y-1);
        if (draw_x >= 0 && draw_x<=draw_rect_x+draw_rect_width)
        {
            for (i = 0; i < nx; i++)
            {
                draw_y= draw_rect_y+i;

                if (draw_y<0 || (draw_y>draw_rect_y+draw_rect_height))
                    continue;

                disp_DrawPixelWithColor_1((UINT16)draw_x,(UINT16)draw_y,RGB16(buf[i].r,buf[i].g,buf[i].b));

            }
        }
    }
    else if (angle == IMAGELIB_ROTATE_180)
    {
        if (nx >draw_rect_width)			//no use x0, it need redo
            nx = draw_rect_width;
        draw_y= draw_rect_y+(lcd_bmp->height-y-1);
        if (draw_y>=0 && draw_y<=draw_rect_y+draw_rect_height)
        {
            for (i = 0; i < nx; i++)
            {
                draw_x= draw_rect_x+(lcd_bmp->width-i-1);

                if (draw_x < 0 || draw_x>draw_rect_x+draw_rect_width)
                    continue;

                disp_DrawPixelWithColor_1((UINT16)draw_x,(UINT16)draw_y,RGB16(buf[i].r,buf[i].g,buf[i].b));

            }
        }
    }

    else if (angle == IMAGELIB_ROTATE_270)		//ok
    {
        if (nx >draw_rect_height)			//no use x0, it need redo
            nx = draw_rect_height;
        draw_x= draw_rect_x+y;
        if (draw_x >= 0 && draw_x<=draw_rect_x+draw_rect_width)
        {
            for (i = 0; i < nx; i++)
            {

                draw_y= draw_rect_y + i;

                if (draw_y<0 || (draw_y>draw_rect_y+draw_rect_height))
                    continue;
                disp_DrawPixelWithColor_1((UINT16)draw_x,(UINT16)draw_y,RGB16(buf[i].r,buf[i].g,buf[i].b));

            }
        }
    }

    lcd_bmp->scanline++;

    return IMAGELIB_SUCCESS;
}


static UINT32 LCD_bitmap_get_nchan(void *vp)
{
    return 3;
}


static void LCD_bitmap_get_box(void *vp, INT32 *ox, INT32 *oy, INT32 *dx, INT32 *dy)
{
    PicLCDBitmap *lcd_bmp = (PicLCDBitmap *) vp;
    if (lcd_bmp == NULL)
        return;
    if (lcd_bmp->scanline)
    {
        *ox = 0;
        *oy = 0;
        *dx = lcd_bmp->width;
        *dy = lcd_bmp->height;
    }
    else
    {

        *ox = PIXEL_UNDEFINED;
        *oy = PIXEL_UNDEFINED;
        *dx = PIXEL_UNDEFINED;
        *dy = PIXEL_UNDEFINED;
    }
}


static IMAGELIB_ERROR_CODE LCD_bitmap_read_row_rgba(void *vp, INT32 y, INT32 x0, INT32 nx, PIXEL_RGBA *buf)
{
    return IMAGELIB_OTHER_ERROR;
}

/*------------------------------------------------------------*/
static INT32 LCD_bitmap_next_pic(void *vp)
{
    return 0;
}
static void* LCD_bitmap_get_result(void *vp)
{
    return NULL;
}
/*------------------------------------------------------------*/
static IMAGE_PROCS lcd_bitmap_procs = {
    NULL,
    NULL,
    LCD_bitmap_open,
    NULL,
    LCD_bitmap_close,

    LCD_get_width,
    LCD_get_height,
    LCD_get_image_type,


    LCD_bitmap_set_nchan,
    LCD_bitmap_set_box,


    LCD_bitmap_write_row_rgba,

    LCD_bitmap_get_nchan,
    LCD_bitmap_get_box,


    LCD_bitmap_read_row_rgba,
    LCD_bitmap_next_pic,
    LCD_bitmap_get_result
};

/*------------------------------------------------------------*/
IMAGE_INSTANCE lcd_bitmap_instance =
{
    "lcd_bitmap", &lcd_bitmap_procs
};
