#include "Private.h"

//#define DB_BLIT

#ifdef F_USE_PICASSO96
#define COLOR_ORDER RGBFB_R8G8B8A8
#else
#define COLOR_ORDER RECTFMT_RGBA
#endif

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///bitmap_blit_palette
STATIC void bitmap_blit_palette(struct FeelinBitMapBlit *fbb, APTR source, uint16 source_x, uint16 source_y, uint16 target_x, uint16 target_y, uint16 target_w, uint16 target_h)
{
	uint32 target_mod = target_w * sizeof (uint32);
	uint32 *line_buffer = IFEELIN F_New(target_w * sizeof (uint32));

	if (line_buffer != NULL)
	{
		uint32 target_x2 = target_x + target_w;
		uint32 target_y2 = target_y + target_h;

		for ( ; target_y < target_y2 ; target_y++, source_y++)
		{
			uint8 *src = (APTR)((uint32)(source) + source_x * sizeof (uint8) + source_y * fbb->Modulo);
			uint32 *buf = line_buffer;
			uint32 j;

			for (j = target_x ; j < target_x2 ; j++)
			{
				*buf++ = fbb->ColorArray[*src++];
			}


			#ifdef F_USE_PICASSO96
			struct RenderInfo rInfo;
			rInfo.Memory      = line_buffer;
			rInfo.BytesPerRow = target_mod;
			rInfo.RGBFormat   = COLOR_ORDER;

			IP96_ p96WritePixelArray
			(
				&rInfo,

				0, 0,

				fbb->RPort,

				target_x, target_y, target_w, 1
			);
			#else
			ICYBERGFX WritePixelArray
			(
				line_buffer,
				
				0,0, target_mod,
				
				fbb->RPort,
				
				target_x,target_y, target_w,1,
				
				COLOR_ORDER
			);
			#endif

		}

		IFEELIN F_Dispose(line_buffer);
	}
}
//+
///bitmap_blit_palette_alpha
STATIC void bitmap_blit_palette_alpha(struct FeelinBitMapBlit *fbb, APTR source, uint16 source_x, uint16 source_y, uint16 target_x, uint16 target_y, uint16 target_w, uint16 target_h)
{
	uint32 target_mod = target_w * sizeof (uint32);
	uint32 *line_buffer = IFEELIN F_New(target_w * sizeof (uint32));

	if (line_buffer != NULL)
	{
		uint32 target_x2 = target_x + target_w;
		uint32 target_y2 = target_y + target_h;

		for ( ; target_y < target_y2 ; target_y++, source_y++)
		{
			uint8 *src = (APTR)((uint32)(source) + source_x * sizeof (uint8) + source_y * fbb->Modulo);
			uint32 *buf = line_buffer;
			uint32 j;

			if ((0xFF & fbb->background) != 0)
			{
				uint32 *k = buf;
				uint32 *k2 = (uint32 *) ((uint32)(buf) + (target_w * sizeof (uint32)));

				while (k < k2)
				{
					*k++ = fbb->background;
				}
			}
			else
			{
				#ifdef F_USE_PICASSO96
				struct RenderInfo rInfo;

				rInfo.Memory      = line_buffer;
				rInfo.BytesPerRow = target_mod;
				rInfo.RGBFormat   = COLOR_ORDER;

				IP96_ p96ReadPixelArray
				(
					&rInfo,

					0, 0,
					fbb->RPort, target_x, target_y, target_w, 1
				);
				#else
				ICYBERGFX ReadPixelArray
				(
					line_buffer,
					
					0,0, target_mod,
					
					fbb->RPort,
					
					target_x,target_y, target_w,1,
					
					COLOR_ORDER
				);
				#endif
			}

			for (j = target_x ; j < target_x2 ; j++, src++)
			{
				uint32 rc;
				uint32 s_rgba = *buf;
				
				#if BIG_ENDIAN_MACHINE
				uint32 d_rgba = fbb->ColorArray[*src];
				#else
				uint32 d_rgba = AROS_BE2LONG(fbb->ColorArray[*src]);
				#endif

				int32 a = F_PIXEL_A(d_rgba);
				int32 s,d;

				s = F_PIXEL_R(s_rgba);
				d = F_PIXEL_R(d_rgba);

				rc = s + (((d - s) * a) >> 8);
				rc <<= 8;

				s = F_PIXEL_G(s_rgba);
				d = F_PIXEL_G(d_rgba);

				rc |= s + (((d - s) * a) >> 8);
				rc <<= 8;

				s = F_PIXEL_B(s_rgba);
				d = F_PIXEL_B(d_rgba);

				rc |= s + (((d - s) * a) >> 8);
				rc <<= 8;

				*buf++ = rc;
			}

			#ifdef F_USE_PICASSO96
			struct RenderInfo rInfo;
			rInfo.Memory      = line_buffer;
			rInfo.BytesPerRow = target_mod;
			rInfo.RGBFormat   = COLOR_ORDER;

			IP96_ p96WritePixelArray
			(
				&rInfo,

				0, 0,

				fbb->RPort,

				target_x, target_y, target_w, 1
			);
			#else
			ICYBERGFX WritePixelArray
			(
				line_buffer,

				0,0, target_mod,

				fbb->RPort,

				target_x,target_y, target_w,1,

				COLOR_ORDER
			);
			#endif

		}

		IFEELIN F_Dispose(line_buffer);
	}
}
//+
///bitmap_blit_rgb
STATIC void bitmap_blit_rgb(struct FeelinBitMapBlit *fbb, APTR source, uint16 source_x, uint16 source_y, uint16 target_x, uint16 target_y, uint16 target_w, uint16 target_h)
{
	#ifdef F_USE_PICASSO96
	struct RenderInfo rInfo;

	rInfo.Memory      = source;
	rInfo.BytesPerRow = fbb->Modulo;
	rInfo.RGBFormat   = RGBFB_R8G8B8;

	IP96_ p96WritePixelArray
	(
		&rInfo,

		source_x, source_y,

		fbb->RPort,

		target_x, target_y, target_w, target_h
	);
	#else
	ICYBERGFX WritePixelArray
	(
		source,
		source_x, source_y,

		fbb->Modulo,
		fbb->RPort,

		target_x, target_y,
		target_w, target_h,

		RECTFMT_RGB
	);
	#endif
}
//+
///bitmap_blit_rgb_alpha
STATIC void bitmap_blit_rgb_alpha(struct FeelinBitMapBlit *fbb, APTR source, uint16 source_x, uint16 source_y, uint16 target_x, uint16 target_y, uint16 target_w, uint16 target_h)
{
	uint32 target_mod = target_w * sizeof (uint32);
	uint32 *line_buffer = IFEELIN F_New(target_mod);

	if (line_buffer != NULL)
	{
		uint32 target_x2 = target_x + target_w;
		uint32 target_y2 = target_y + target_h;

		for ( ; target_y < target_y2 ; target_y++, source_y++)
		{
			uint32 *src = (APTR)((uint32)(source) + source_x * sizeof (uint32) + source_y * fbb->Modulo);
			uint32 *buf = line_buffer;
			uint32 j;

			if ((0xFF & fbb->background) != 0)
			{
				uint32 *k = buf;
				uint32 *k2 = (uint32 *) ((uint32)(buf) + (target_w * sizeof (uint32)));

				while (k < k2)
				{
					*k++ = fbb->background;
				}
			}
			else
			{
				#ifdef F_USE_PICASSO96
				struct RenderInfo rInfo;

				rInfo.Memory      = line_buffer;
				rInfo.BytesPerRow = target_mod;
				rInfo.RGBFormat   = COLOR_ORDER;

				IP96_ p96ReadPixelArray
				(
					&rInfo,

					0, 0,
					fbb->RPort, target_x, target_y, target_w, 1
				);
				#else
				ICYBERGFX ReadPixelArray
				(
					line_buffer,
					
					0,0, target_mod,
					
					fbb->RPort,
					
					target_x,target_y, target_w,1,
					
					COLOR_ORDER
				);
				#endif
			}

			for (j = target_x ; j < target_x2 ; j++, src++)
			{
				uint32 rc;
				uint32 s_rgba = *buf;
				
				#if BIG_ENDIAN_MACHINE
				uint32 d_rgba = *src;
				#else
				uint32 d_rgba = AROS_BE2LONG(*src);
				#endif
 
				int32 a = /*255 -*/ F_PIXEL_A(d_rgba);
				int32 s,d;

				s = F_PIXEL_R(s_rgba);
				d = F_PIXEL_R(d_rgba);

				rc = s + (((d - s) * a) >> 8);
				rc <<= 8;

				s = F_PIXEL_G(s_rgba);
				d = F_PIXEL_G(d_rgba);

				rc |= s + (((d - s) * a) >> 8);
				rc <<= 8;

				s = F_PIXEL_B(s_rgba);
				d = F_PIXEL_B(d_rgba);

				rc |= s + (((d - s) * a) >> 8);
				rc <<= 8;

				*buf++ = rc;
			}

			#ifdef F_USE_PICASSO96
			struct RenderInfo rInfo;
			rInfo.Memory      = line_buffer;
			rInfo.BytesPerRow = target_mod;
			rInfo.RGBFormat   = COLOR_ORDER;

			IP96_ p96WritePixelArray
			(
				&rInfo,

				0,0,

				fbb->RPort,

				target_x, target_y, target_w, 1
			);
			#else
			ICYBERGFX WritePixelArray
			(
				line_buffer,
				
				0,0, target_mod,
				
				fbb->RPort,
				
				target_x,target_y, target_w,1,
				
				COLOR_ORDER
			);
			#endif

		}

		IFEELIN F_Dispose(line_buffer);
	}
}
//+
///bitmap_blit_rgb_0
STATIC void bitmap_blit_rgb_0(struct FeelinBitMapBlit *fbb, APTR source, uint16 source_x, uint16 source_y, uint16 target_x, uint16 target_y, uint16 target_w, uint16 target_h)
{
	#ifdef F_USE_PICASSO96
	struct RenderInfo rInfo;

	rInfo.Memory      = source;
	rInfo.BytesPerRow = fbb->Modulo;
	rInfo.RGBFormat   = RGBFB_R8G8B8A8;

	IP96_ p96WritePixelArray
	(
		&rInfo,

		source_x, source_y,

		fbb->RPort,

		target_x, target_y, target_w, target_h
	);
	#else
	ICYBERGFX WritePixelArray
	(
		source,
		source_x, source_y,

		fbb->Modulo,
		fbb->RPort,

		target_x, target_y,
		target_w, target_h,

		RECTFMT_RGBA
	);
	#endif
}
//+
///bitmap_blit_bitmap
STATIC void bitmap_blit_bitmap(struct FeelinBitMapBlit *fbb, APTR source, uint16 source_x, uint16 source_y, uint16 target_x, uint16 target_y, uint16 target_w, uint16 target_h)
{
	IGRAPHICS BltBitMapRastPort
	(
		source,
		source_x, source_y,

		fbb->RPort,

		target_x, target_y,
		target_w, target_h,

		0x0C0
	);
}
//+

///bitmap_blit_tile
STATIC void bitmap_blit_tile
(
	struct FeelinBitMapBlit *fbb,
	
	APTR source,

	uint16 source_x,
	uint16 source_y,
	uint16 source_w,
	uint16 source_h,

	uint16 target_x,
	uint16 target_y,
	uint16 target_w,
	uint16 target_h
)
{

	/* the width/height of the rectangle to blit as the  first  column/row.
	the  width  of  the  first tile is either the rest of the tile right to
	offx or the width of the dest rect, if the rect is narrow */

	int32 first_w = MIN(source_w - source_x, target_w);
	int32 first_h = MIN(source_h - source_y, target_h);

	/* the left/top edge of the second column (if used) */

	int32 second_x = target_x + first_w;
	int32 second_y = target_y + first_h;

	/* the width/height of the second column */

	int32 second_w = MIN(source_x, target_x + target_w - second_x);
	int32 second_h = MIN(source_y, target_y + target_h - second_y);

	/* 'pos' is used as starting position in the "exponential" blit,  while
	'size' is used as bitmap size */

	uint32 pos;
	uint32 size;

	/* the width of the second tile (we want the whole tile to be  source_w
	pixels  wide,  if we use 'source_w - source_x' pixels for the left part
	we'll use 'source_x' for the right part) */

/*** blit the source with offsets (if any) *********************************/

///DB_BLIT
	#ifdef DB_BLIT
	IFEELIN F_Log(0,"01> %3ld : %3ld TO %3ld : %3ld, %3ld x %3ld",
		source_x,source_y, target_x, target_y, first_w, first_h);
	#endif
//+

	fbb->blit_func
	(
		fbb,
 
		source,
		source_x, source_y,

		target_x, target_y,
		first_w, first_h
	);


/*** blit the width offset part (if any) ***********************************/

	if (second_w > 0)
	{

		/* if second_w was 0 or the dest rect was to narrow, we won't  need
		a second column */

///DB_BLIT
	#ifdef DB_BLIT
	IFEELIN F_Log(0,"02> %3ld : %3ld >> %3ld : %3ld, [3m%3ld[0m x %3ld",
			0,source_y, second_x, target_y, second_w, first_h);
	#endif
//+

		fbb->blit_func
		(
			fbb,
 
			source,
			0, source_y,

			second_x, target_y,
			second_w, first_h
		);

	}

/*** blit the height offset part (if any) **********************************/

	if (second_h > 0)
	{
///DB_BLIT
	#ifdef DB_BLIT
	IFEELIN F_Log(0,"03> %3ld : %3ld >> %3ld : %3ld, %3ld x %3ld",
			source_x,0, target_x, second_y, first_w, second_h);
	#endif
//+

		fbb->blit_func
		(
			fbb,
 
			source,
			source_x, 0,

			target_x, second_y,
			first_w, second_h
		);


		if (second_w > 0)
		{
///DB_BLIT
			#ifdef DB_BLIT
			IFEELIN F_Log(0,"04> %3ld : %3ld >> %3ld : %3ld, %3ld x %3ld",
				0,0, second_x, second_y, second_w, second_h);
			#endif
//+

			fbb->blit_func
			(
				fbb,
 
				source,
				0, 0,

				second_x, second_y,
				second_w, second_h
			);
		}
	}

/*** blit the first row ****************************************************/

	for (pos = target_x + source_w, size = MIN(source_w,target_x + target_w - 1 - pos + 1) ; pos <= target_x + target_w - 1 ; )
	{
///DB_BLIT
	#ifdef DB_BLIT
		IFEELIN F_Log(0,"!5> %3ld : %3ld >> %3ld : %3ld, %3ld x %3ld",
			target_x,target_y, pos, target_y, size, MIN(source_h, target_h));
	#endif
//+

		IGRAPHICS ClipBlit
		(
			fbb->RPort,
			
			target_x,
			target_y,
			
			fbb->RPort,
			
			pos, target_y,
			size, MIN(source_h, target_h),

			0x0C0
		);

		pos += size;

		size = MIN(size << 1, target_x + target_w - 1 - pos + 1);
	}

/*** blit the first row down several times *********************************/

	for (pos = target_y + source_h, size = MIN(source_h, target_y + target_h - 1 - pos + 1) ; pos <= target_y + target_h - 1 ; )
	{
///DB_BLIT
	#ifdef DB_BLIT
		IFEELIN F_Log(0,"!6> %3ld : %3ld >> %3ld : %3ld, %3ld x %3ld",
			target_x,target_y, target_x, pos, target_w, size);
	#endif
//+

		IGRAPHICS ClipBlit
		(
			fbb->RPort,

			target_x,
			target_y,

			fbb->RPort,

			target_x, pos,
			target_w, size,

			0x0C0
		);

		pos += size;
		size = MIN(size << 1, target_y + target_h - 1 - pos + 1);
	}
}
//+

#if 0

/*** scale *************************************************************************************/

#if 0
///bitmap_transform_rgb
STATIC void bitmap_transform_rgb(struct FeelinBitMapTransform *fbt, uint8 *source, uint32 *target, uint32 pixels)
{
	uint32 stop = (uint32)(source) + ((pixels - 1) * sizeof (uint8) * 3);

//    uint32 n=1;

	while ((uint32)(source) < stop)
	{
		uint32 rgba;

		rgba = *source++;
		rgba <<= 8;
		rgba |= *source++;
		rgba <<= 8;
		rgba |= *source++;
		rgba <<= 8;
		rgba |= 0xFF;

		*target++ = rgba;

//        n++;
	}

//    F_Log(0,"%ld pixels transformed (number %ld)",n, number);
}
//+
///bitmap_transform_palette
STATIC void bitmap_transform_palette(struct FeelinBitMapTransform *fbt, uint8 *source, uint32 *target, uint32 pixels)
{
	uint32 stop = (uint32)(source) + (pixels * sizeof (uint8));

	while ((uint32)(source) < stop)
	{
		*target++ = fbt->colors[*source++];
	}
}
//+
#endif
  
///bitmap_scale_draw
STATIC void bitmap_scale_draw(struct FeelinBitMapScale *fbs, APTR source)
{
	uint32 *target = IFEELIN F_New(fbs->target_modulo);

	if (target)
	{
		struct FeelinBitMapBlit fbb;

		uint32 i;

		if (FF_COLOR_TYPE_ALPHA & fbs->source_color_type)
		{
			fbb.blit_func = bitmap_blit_rgb_alpha;
		}
		else
		{
			fbb.blit_func = bitmap_blit_rgb_0;
		}

		fbb.Modulo = fbs->target_modulo;
		//fbb.RPort = fbs->fbb->RPort;
		fbb.RPort = fbs->user_data;

		for (i = 0 ; i < fbs->target_h ; i++)
		{
			fbs->scale_y_func(fbs, source, target, i);

			fbb.blit_func
			(
				&fbb,

				target,

				0,
				0,

				fbs->target_x,
				fbs->target_y + i,
				fbs->target_w,
				1
			);
		}

		IFEELIN F_Dispose(target);
	}
}
//+

///bitmap_blit_scale
STATIC void bitmap_blit_scale
(
	struct FeelinBitMapBlit *fbb,
	
	APTR source,

	uint16 source_x,
	uint16 source_y,
	uint16 source_w,
	uint16 source_h,

	uint16 target_x,
	uint16 target_y,
	uint16 target_w,
	uint16 target_h,
	
	uint32 filter
)
{
	struct FeelinBitMapScale fbs;
	
	#if 0
///
	fbs.fbb = fbb;
	
	fbs.scale_func = bitmap_scale_draw;
	
	fbs.source_w = source_w;
	fbs.source_h = source_h;
	fbs.source_modulo = fbb->Modulo;
	fbs.source_modulo_real = fbb->Modulo;

	fbs.target_x = target_x;
	fbs.target_y = target_y;
	fbs.target_w = target_w;
	fbs.target_h = target_h;
	fbs.target_modulo = target_w * sizeof (uint32);

	fbs.xfrac = 0;
	fbs.yfrac = 0;
	fbs.spy = (fbs.source_h * GSIMUL) / fbs.target_h;

	fbs.scale_y_func = bitmap_y_scale;
	fbs.scale_x_func = bitmap_x_scale;
	
	fbs.transform.func   = NULL;
	fbs.transform.buffer = NULL;
	fbs.transform.colors = fbb->ColorArray;
				
	// select scale function according to 'filter'
 
	switch (filter)
	{
		case FV_BitMap_ScaleFilter_Bilinear:
		{
			fbs.get_pixel_func = bitmap_get_pixel_bilinear;
		}
		break;
				
		case FV_BitMap_ScaleFilter_Average:
		{
			fbs.get_pixel_func = bitmap_get_pixel_average;
		}
		break;
			 
		default:
		{
			fbs.get_pixel_func = bitmap_get_pixel_nearest;
		}
		break;
	}

/*
		F_Log(0,"source (%ld : %ld, %ld x %ld) target (%ld : %ld, %ld x %ld)",
			source_x, source_y, source_w, source_h,
			target_x, target_y, target_w, target_h);
*/
	source = (APTR)((uint32)(source) + source_y * fbb->Modulo + source_x * fbb->PixelSize);
	// + source_x * sizeof(uint32));

	if (fbb->PixelSize != 4)
	{
//        fbs.source_modulo = source_w * sizeof (uint32);
//        fbs.transform_buffer = F_New(fbs.source_modulo * 3);
//        fbs.scale_y_func = bitmap_y_scale_transform;

		if (fbs.transform_buffer)
		{
			switch (fbb->PixelSize)
			{
				case 1: //FV_PIXEL_TYPE_PALETTE:
				{
					fbs.transform_func = bitmap_transform_palette;

//                    source = (APTR)((uint32)(source) + source_x * sizeof (uint8));
				}
				break;

				case 3: //FV_PIXEL_TYPE_RGB:
				{
					fbs.transform_func = bitmap_transform_rgb;

//                    source = (APTR)((uint32)(source) + source_x * sizeof (uint8) * 3);
				}
				break;

				default:
				{
					goto __done;
				}
			}
		}
		else goto __done;
	}

	if (fbs.transform_func)
	{
		fbs.source_modulo = source_w * sizeof (uint32);
		fbs.transform.buffer = IFEELIN F_New(fbs.source_modulo * 3);
		fbs.scale_y_func = bitmap_y_scale_transform;
		
		if (fbs.transform.buffer == NULL)
		{
			return;
		}
	}
 
	/*
	else
	{
		source = (APTR)((uint32)(source) + source_x * sizeof(uint32));
	}
	*/

	if (fbs.scale_func && fbs.scale_y_func && fbs.scale_x_func)
	{
		fbs.scale_func(&fbs, source);
	}

__done:

	IFEELIN F_Dispose(fbs.transform_buffer);
//+
	#else
	
	// source
	
	fbs.source_w            = source_w;
	fbs.source_h            = source_h;
	fbs.source_modulo       = fbb->Modulo;
	fbs.source_modulo_real  = fbb->Modulo;
	fbs.source_pixel_size   = fbb->PixelSize;
	fbs.source_color_type   = fbb->ColorType;
			
	// target
 
	fbs.target_x = target_x;
	fbs.target_y = target_y;
	fbs.target_w = target_w;
	fbs.target_h = target_h;
	fbs.target_modulo = target_w * sizeof (uint32);

	// transform
	
	fbs.transform.colors = fbb->ColorArray;
			
	// our data
			
	fbs.user_data = fbb->RPort;

	// our function
 
	fbs.scale_func = bitmap_scale_draw;
					
	// let's go
	
	bitmap_scale(&fbs, source, source_x, source_y, filter);
 
	#endif
}
//+

#if 0
///bitmap_blit_frame
void bitmap_blit_frame(FClass *Class, FObject Obj, struct FS_BitMap_Blit *Msg)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = (struct TagItem *) &Msg->Tag1, item;

	uint16 source_w = LOD->cpp.Width;
	uint16 source_h = LOD->cpp.Height;
	
	uint16 target_x = Msg->TargetBox->x;
	uint16 target_y = Msg->TargetBox->y;
	uint16 target_w = Msg->TargetBox->w;
	uint16 target_h = Msg->TargetBox->h;

	FInner *margins = NULL;
	
	#if 1
	uint16 x,y,w,h;
	#endif
	
	while  (IFEELIN F_DynamicNTI(&Tags, &item, Class))
	switch (item.ti_Tag)
	{
		case FA_BitMap_Margins: margins = (FInner *) item.ti_Data; break;
	}
						
	if (margins == NULL)
	{
		IFEELIN F_Log(0,"You need to defined the FA_BitMap_Margins attribute while using the 'Frame' blit mode");
		
		return;
	}

	#if 1
///
	bitmap_blit_select
	(
		LOD->cpp.PixelArray,

		0,
		0,

		LOD->cpp.PixelArrayMod,

		Msg->rp,

		target_x,
		target_y,
		margins->l,
		margins->t,

		LOD->cpp.PixelType,
		LOD->cpp.ColorArray
	);

	bitmap_blit_select
	(
		LOD->cpp.PixelArray,

		source_w - margins->r,
		0,

		LOD->cpp.PixelArrayMod,

		Msg->rp,

		target_x + target_w - margins->r,
		target_y,
		margins->r,
		margins->t,

		LOD->cpp.PixelType,
		LOD->cpp.ColorArray
	);

	bitmap_blit_select
	(
		LOD->cpp.PixelArray,

		0,
		source_h - margins->b,

		LOD->cpp.PixelArrayMod,

		Msg->rp,

		target_x,
		target_y + target_h - margins->b,
		margins->l,
		margins->b,

		LOD->cpp.PixelType,
		LOD->cpp.ColorArray
	);
				
	bitmap_blit_select
	(
		LOD->cpp.PixelArray,

		source_w - margins->r,
		source_h - margins->b,

		LOD->cpp.PixelArrayMod,

		Msg->rp,

		target_x + target_w - margins->r,
		target_y + target_h - margins->b,
		margins->r,
		margins->b,

		LOD->cpp.PixelType,
		LOD->cpp.ColorArray
	);

	x = target_x + margins->l;
	y = target_y;
	w = target_w - margins->l - margins->r;
	h = margins->t;
	
	while (w)
	{
		uint32 draw_w = MIN(source_w - margins->l - margins->r, w);

		bitmap_blit_select
		(
			LOD->cpp.PixelArray,
			margins->l, 0,

			LOD->cpp.PixelArrayMod,

			Msg->rp,

			x, y,
			draw_w, MIN(source_h, h),

			LOD->cpp.PixelType,
			LOD->cpp.ColorArray
		);

		x += draw_w;
		w -= draw_w;
	}
	
	x = target_x + margins->l;
	y = target_y + target_h - margins->b;
	w = target_w - margins->l - margins->r;
	h = margins->b;

	while (w)
	{
		uint32 draw_w = MIN(source_w - margins->l - margins->r, w);

		bitmap_blit_select
		(
			LOD->cpp.PixelArray,
			margins->l, source_h - margins->b,

			LOD->cpp.PixelArrayMod,

			Msg->rp,

			x, y,
			draw_w, MIN(source_h, h),

			LOD->cpp.PixelType,
			LOD->cpp.ColorArray
		);

		x += draw_w;
		w -= draw_w;
	}
							
	x = target_x;
	y = target_y + margins->t;
	w = margins->l;
	h = target_h - margins->t - margins->b;

	while (h)
	{
		uint32 draw_h = MIN(source_h - margins->t - margins->b, h);

		bitmap_blit_select
		(
			LOD->cpp.PixelArray,
			0, margins->t,

			LOD->cpp.PixelArrayMod,

			Msg->rp,

			x, y,
			w, draw_h,

			LOD->cpp.PixelType,
			LOD->cpp.ColorArray
		);

		y += draw_h;
		h -= draw_h;
	}

	x = target_x + target_w - margins->r;
	y = target_y + margins->t;
	w = margins->r;
	h = target_h - margins->t - margins->b;

	while (h)
	{
		uint32 draw_h = MIN(source_h - margins->t - margins->b, h);

		bitmap_blit_select
		(
			LOD->cpp.PixelArray,
			source_w - margins->r, margins->t,

			LOD->cpp.PixelArrayMod,

			Msg->rp,

			x, y,
			w, draw_h,

			LOD->cpp.PixelType,
			LOD->cpp.ColorArray
		);

		y += draw_h;
		h -= draw_h;
	}
//+
	#endif
  
	#if 1
	bitmap_blit_scale
	(
		LOD->cpp.PixelArray,

		margins->l,
		margins->t,
		source_w - margins->l - margins->r,
		source_h - margins->t - margins->b,

		LOD->cpp.PixelArrayMod,

		Msg->rp,

		target_x + margins->l,
		target_y + margins->t,
		target_w - margins->l - margins->r,
		target_h - margins->t - margins->b,

		LOD->cpp.PixelType,
		LOD->cpp.ColorArray
	);
	#else
	bitmap_blit_select
	(
		LOD->cpp.PixelArray,

		margins->l,
		margins->t,

		LOD->cpp.PixelArrayMod,

		Msg->rp,

		target_x + margins->l,
		target_y + margins->t,
		source_w - margins->l - margins->r,
		source_h - margins->t - margins->b,

		LOD->cpp.PixelType,
		LOD->cpp.ColorArray
	);
	#endif
}
//+
#endif

#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///BitMap_Blit
F_METHODM(void,BitMap_Blit,FS_BitMap_Blit)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = (struct TagItem *) &Msg->Tag1, item;
	
	APTR source;
	uint16 source_w;
	uint16 source_h;
	uint16 source_x = Msg->SourceX;
	uint16 source_y = Msg->SourceY;
	uint32 blit_mode = FV_BitMap_BlitMode_Copy;
	//uint32 scale_filter = FV_BitMap_ScaleFilter_Nearest;

	struct FeelinBitMapBlit fbb;

	fbb.blit_func = NULL;
	fbb.RPort = Msg->rp;

	fbb.PixelSize = LOD->cpp.PixelSize;
	fbb.ColorType = LOD->cpp.ColorType;
	fbb.ColorArray = LOD->cpp.ColorArray;
	fbb.background = 0;
	fbb.Modulo = LOD->cpp.PixelArrayMod;

	source = LOD->cpp.PixelArray;
	source_w = LOD->cpp.Width;
	source_h = LOD->cpp.Height;

	#if 0

	IFEELIN F_Log
	(
		0, "source (%ld x %ld) pixel size (%ld) pixel array (0x%08lx) color type (%ld) color array (0x%08lx)",
		
		LOD->cpp.Width,
		LOD->cpp.Height,

		LOD->cpp.PixelSize,
		LOD->cpp.PixelArray,

		LOD->cpp.ColorType,
		LOD->cpp.ColorArray
	);

	#endif

/*** select blit function according to color type **********************************************/
	
	switch (fbb.PixelSize)
	{
		case 1:
		{
			if (FF_BITMAP_RENDERED_BITMAP & LOD->flags)
			{
				fbb.blit_func = bitmap_blit_bitmap;
			}
			else if (fbb.ColorType == FV_COLOR_TYPE_PALETTE)
			{
				fbb.blit_func = bitmap_blit_palette;
			}
			else if (fbb.ColorType == FV_COLOR_TYPE_PALETTE_ALPHA)
			{
				fbb.blit_func = bitmap_blit_palette_alpha;
			}
		}
		break;

		case 3:
		{
			fbb.blit_func = bitmap_blit_rgb;
		}
		break;

		case 4:
		{
			if (FF_COLOR_TYPE_ALPHA & fbb.ColorType)
			{
				fbb.blit_func = bitmap_blit_rgb_alpha;
			}
			else
			{
				fbb.blit_func = bitmap_blit_rgb_0;
			}
		}
		break;

		default:
		{
			IFEELIN F_Log(0,"PixelSize %ld is not supported", fbb.PixelSize);
			
			return;
		}
	}
	
	if (fbb.blit_func == NULL)
	{
		IFEELIN F_Log(0,"No suitable function to blit bitmap");
		
		return;
	}
	
	#if 1

/*** read further parameters *******************************************************************/

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_COLORARRAY:
		{
			fbb.ColorArray = (uint32 *) item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_BLITMODE:
		{
			blit_mode = item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_BACKGROUND:
		{
			fbb.background = (item.ti_Data << 8) | 0xFF;
		}
		break;
/*
		case FA_BitMap_ScaleFilter:
		{
			scale_filter = item.ti_Data;
		}
		break;
*/
	}

	#endif

/*** perform blit according to blit mode *******************************************************/
 
	switch (blit_mode)
	{
		case FV_BitMap_BlitMode_Copy:
		{
			fbb.blit_func
			(
				&fbb,
 
				source,
				source_x, source_y,

				Msg->TargetBox->x, Msg->TargetBox->y,
				MIN(source_w - source_x, Msg->TargetBox->w), MIN(source_h - source_y, Msg->TargetBox->h)
			);
		}
		break;
		
		case FV_BitMap_BlitMode_Tile:
		{
			bitmap_blit_tile
			(
				&fbb,

				source,
				source_x, source_y,
				source_w, source_h,

				Msg->TargetBox->x, Msg->TargetBox->y,
				Msg->TargetBox->w, Msg->TargetBox->h
			);
		}
		break;
		 
		#if 0
		case FV_BitMap_BlitMode_Scale:
		{
			bitmap_blit_scale
			(
				&fbb,
				
				source,
				source_x, source_y,
				source_w - source_x, source_h - source_y,

				Msg->TargetBox->x, Msg->TargetBox->y,
				Msg->TargetBox->w, Msg->TargetBox->h,
				
				scale_filter
			);
		}
		break;
		#endif
		 
		#if 0
		case FV_BitMap_BlitMode_Frame:
		{
			bitmap_blit_frame(Class, Obj, Msg);
		}
		break;
		#endif
		
		default:
		{
			IFEELIN F_Log(0,"%ld is not a valid blit mode",blit_mode);
		}
		break;
	}
}
//+
