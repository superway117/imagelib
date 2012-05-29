
#include <image_headfile.h>

#define IMAGE_EFFECT_BIAS  128
#define IMAGE_MULTI_VALUE  1
#define IMAGE_BRIGHT_THRESHOLD 20
#define IMAGE_BRIGHT_JUMP_STEP 5


static void image_effect_rgba_emboss(PIXEL_RGBA *buf, INT32 number)
{
		INT32 i;
		UINT16 size = sizeof(PIXEL_RGBA)*number;
		PIXEL_RGBA *tmp_buf = IMAGE_MALLOC(size);
		if(tmp_buf == NULL)
			return;
		memcpy(tmp_buf,buf,size);
		buf[0].r=(tmp_buf[0].r)+IMAGE_EFFECT_BIAS;
		buf[0].g=(tmp_buf[0].g)+IMAGE_EFFECT_BIAS;
		buf[0].b=(tmp_buf[0].b)+IMAGE_EFFECT_BIAS;
		for(i = 1 ;i< number;i++)
		{

			if((tmp_buf[i].r)+IMAGE_EFFECT_BIAS>(tmp_buf[i-1].r))
				buf[i].r=(tmp_buf[i].r)-(tmp_buf[i-1].r)+IMAGE_EFFECT_BIAS;
			else
				buf[i].r=0;
			if((tmp_buf[i].g)+IMAGE_EFFECT_BIAS>(tmp_buf[i-1].g))
				buf[i].g=(tmp_buf[i].g)-(tmp_buf[i-1].g)+IMAGE_EFFECT_BIAS;
			else
				buf[i].g=0;
			if((tmp_buf[i].b)+IMAGE_EFFECT_BIAS>(tmp_buf[i-1].b))
				buf[i].b=(tmp_buf[i].b)-(tmp_buf[i-1].b)+IMAGE_EFFECT_BIAS;
			else
				buf[i].b=0;
			
		}
		IMAGE_FREE(tmp_buf);
}
static void image_effect_rgba_bright( PIXEL_RGBA *buf, INT32 number,UINT8 level)
{
		INT32 i;
		UINT16 bias = IMAGE_BRIGHT_THRESHOLD;
		if(level == 0)
			return;
		else
			bias+=(level*IMAGE_BRIGHT_JUMP_STEP);

		for(i = 0 ;i< number;i++)
		{
			buf[i].r=(buf[i].r)+bias>255?255:(buf[i].r)+bias;
	 		buf[i].g=(buf[i].g)+bias>255?255:(buf[i].g)+bias;
	 		buf[i].b=(buf[i].b)+bias>255?255:(buf[i].b)+bias;
		}
		
}
static void image_effect_rgba_negative(PIXEL_RGBA *buf, INT32 number)
{
	INT32 i;
	UINT16 size = sizeof(PIXEL_RGBA)*number;
	PIXEL_RGBA *tmp_buf = IMAGE_MALLOC(size);
	if(tmp_buf == NULL)
			return;
	memcpy(tmp_buf,buf,size);
	for(i = 1 ;i< number;i++)
	{
		buf[i].r=~(tmp_buf[i].r);
	 	buf[i].g=~(tmp_buf[i].g);
	 	buf[i].b=~(tmp_buf[i].b);
	}
	IMAGE_FREE(tmp_buf);

}
static void image_effect_rgba_blackwhite(PIXEL_RGBA *buf, INT32 number)
{
	INT32 i;
	for(i = 1 ;i< number;i++)
	{
		buf[i].r=buf[i].g=buf[i].b=(COLOR)(buf[i].r*0.3+buf[i].g*0.4+buf[i].b*0.3);
	}


}

static void image_effect_rgba_gray(PIXEL_RGBA *buf, INT32 number)
{
	INT32 i;
	UINT32 r,g,b;

	for(i = 0 ;i< number;i++)
	{
		r=buf[i].r;
	    	g=buf[i].g;
	    	b=buf[i].b;
		buf[i].r=buf[i].g=buf[i].b=(COLOR)( (r*38 + g*75 + b*15) >> 7);
	}
}
void image_effect_rgba(PIXEL_RGBA *buf, INT32 number,IMAGELIB_EFFECT effect_type,INT32 effect_para)
{
	switch(effect_type)
	{
		case IMAGELIB_EMBOSS_EFFECT:
			image_effect_rgba_emboss(buf,number);
			break;
		case IMAGELIB_BRIGHT_EFFECT:
			image_effect_rgba_bright(buf,number,(UINT8)effect_para);
			break;
		case IMAGELIB_NEGATIVE_EFFECT:
			image_effect_rgba_negative(buf,number);
			break;
		case IMAGELIB_BLACKWHITE_EFFECT:
			image_effect_rgba_blackwhite(buf,number);
			break;
		case IMAGELIB_GRAY_EFFECT:
			image_effect_rgba_gray(buf,number);
			break;
	}

}