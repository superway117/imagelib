
#include <engine.h>

extern IMAGE_INSTANCE jpg_instance;
extern IMAGE_INSTANCE png_instance;
extern IMAGE_INSTANCE gif_instance;
extern IMAGE_INSTANCE bmp_instance;
extern IMAGE_INSTANCE native_bitmap_instance;
extern IMAGE_INSTANCE lcd_bitmap_instance;


IMAGE_INSTANCE *image_instance_list[IMAGE_INSTANCE_LISTMAX] = {
&lcd_bitmap_instance,
&native_bitmap_instance,
  &jpg_instance,
  &png_instance,
    &gif_instance,
  &bmp_instance,
  
  0
};
