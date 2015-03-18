#include "Private.h"

#define CODE_NOMATH

#ifdef CODE_NOMATH
#include "gradient.h"
#else
#include <math.h>
#endif

#define SHIFT (sizeof(uint32)*8 - (8 + 1))

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///id_gradient_draw_vertical
STATIC void id_gradient_draw_vertical
(
   struct RastPort *rp,
   int16 x1, int16 y1, int16 x2, int16 y2,
   int16 oy1, int16 oy2,
   uint32 start_rgb,
   uint32 end_rgb
)
{
	int32 delta_max = (oy2 - oy1 > 0) ? oy2 - oy1 : 1;
	int32 width = x2  - x1 + 1;

	int32 step_r = ((int32)(((0x00FF0000 & end_rgb) >> 16) - ((0x00FF0000 & start_rgb) >> 16)) << SHIFT) / delta_max;
	int32 step_g = ((int32)(((0x0000FF00 & end_rgb) >>  8) - ((0x0000FF00 & start_rgb) >>  8)) << SHIFT) / delta_max;
	int32 step_b = ((int32)( (0x000000FF & end_rgb)        -  (0x000000FF & start_rgb)       ) << SHIFT) / delta_max;

	int32 y, offset_y = y1 - oy1;

	int32 red   = ((1 << SHIFT) >> 1) + (((0x00FF0000 & start_rgb) >> 16) << SHIFT) + offset_y*step_r;
	int32 green = ((1 << SHIFT) >> 1) + (((0x0000FF00 & start_rgb) >>  8) << SHIFT) + offset_y*step_g;
	int32 blue  = ((1 << SHIFT) >> 1) + ( (0x000000FF & start_rgb)        << SHIFT) + offset_y*step_b;

	for (y = y1; y <= y2; y++)
	{
		#ifdef F_USE_PICASSO96
		IP96_   p96RectFill(rp, x1, y, x1+width-1, y,((red >> SHIFT) << 16) + ((green >> SHIFT) << 8) + (blue >> SHIFT));
		#else
		ICYBERGFX FillPixelArray(rp, x1, y, width, 1,((red >> SHIFT) << 16) + ((green >> SHIFT) << 8) + (blue >> SHIFT));
		#endif

		red   += step_r;
		green += step_g;
		blue  += step_b;
	}
}
//+
///id_gradient_draw_horizontal
STATIC void id_gradient_draw_horizontal
(
	struct RastPort *rp,
	int16 x1, int16 y1, int16 x2, int16 y2,
	int16 ox1, int16 ox2,
	uint32 start_rgb,
	uint32 end_rgb
)
{
	int32 delta_max = (ox2 - ox1 > 0) ? ox2 - ox1 : 1;
	int32 height = y2  - y1 + 1;

	int32 step_r = ((int32)(((0x00FF0000 & end_rgb) >> 16) - ((0x00FF0000 & start_rgb) >> 16)) << SHIFT) / delta_max;
	int32 step_g = ((int32)(((0x0000FF00 & end_rgb) >>  8) - ((0x0000FF00 & start_rgb) >>  8)) << SHIFT) / delta_max;
	int32 step_b = ((int32)( (0x000000FF & end_rgb)        -  (0x000000FF & start_rgb)       ) << SHIFT) / delta_max;

	int32 x, offset_x = x1 - ox1;

   /* 1 << (SHIFT - 1) is 0.5 in  fixed  point  math.  We  add  it  to  the
   variable  so  that,  at the moment in which the variable is converted to
   integer, rounding is done properly. That is, a+x, with 0 < x <  0.5,  is
   rounded down to a, and a+x, with 0.5 <= x < 1, is rounded up to a+1. */

   int32 red   = ((1 << SHIFT) >> 1) + (((0x00FF0000 & start_rgb) >> 16) << SHIFT) + offset_x * step_r;
   int32 green = ((1 << SHIFT) >> 1) + (((0x0000FF00 & start_rgb) >>  8) << SHIFT) + offset_x * step_g;
   int32 blue  = ((1 << SHIFT) >> 1) + ( (0x000000FF & start_rgb)        << SHIFT) + offset_x * step_b;

   for (x = x1; x <= x2; x++)
   {
	  #ifdef F_USE_PICASSO96
	  IP96_   p96RectFill(rp, x, y1, x, y1+height-1,((red >> SHIFT) << 16) + ((green >> SHIFT) << 8) + (blue >> SHIFT));
	  #else
	  ICYBERGFX FillPixelArray(rp, x, y1, 1, height,((red >> SHIFT) << 16) + ((green >> SHIFT) << 8) + (blue >> SHIFT));
	  #endif

	  red   += step_r;
	  green += step_g;
	  blue  += step_b;
   }
}
//+
///id_gradient_draw_angle
struct myrgb
{
	int red,green,blue;
};

/*****************************************************************
 Fill the given rectangle with a angle oriented gradient. The
 unit angle uses are degrees
******************************************************************/
STATIC int id_gradient_draw_angle
(
   struct RastPort *rp,
   int32 xt,
   int32 yt,
   int32 xb,
   int32 yb,
   uint32 start_rgb,
   uint32 end_rgb,
   int32 angle
)
{

   /* The basic idea of this algorithm is to calc the intersection  between
   the  diagonal of the rectangle (xs,ys) with dimension (xw,yw) a with the
   line starting at (x,y) (every pixel  inside  the  rectangle)  and  angle
   angle with direction vector (vx,vy).

   Having the intersection point we then know the color of the pixel.

   TODO: Turn the algorithm into  a  incremental  one  Remove  the  use  of
   floating point variables */

#ifndef CODE_NOMATH
   double rad = angle * PI / 180;
   double cosarc = cos(rad);
   double sinarc = sin(rad);
#endif
   struct myrgb startRGB,endRGB;
   int diffR, diffG, diffB;

   int r,t; /* some helper variables to short the code */
   int l,y,c;
   int y1; /* The intersection point */
   int incr_y1; /* increment of y1 */
   int xs,ys,xw,yw;
   int xadd,ystart,yadd;
#ifdef CODE_NOMATH
   int vx = vx_array[angle];
   int vy = vy_array[angle];
#else
   int vx = (int)(-cosarc*0xff);
   int vy = (int)(sinarc*0xff);
#endif
   int width = xb - xt + 1;
   int height = yb - yt + 1;

   uint8 *buf = IFEELIN F_New(width*3);
   if (!buf) return 0;
   
   startRGB.red   = ((0x00FF0000 & start_rgb) >> 16);
   startRGB.green = ((0x0000FF00 & start_rgb) >>  8);
   startRGB.blue  =  (0x000000FF & start_rgb);
   endRGB.red     = ((0x00FF0000 & end_rgb) >> 16);
   endRGB.green   = ((0x0000FF00 & end_rgb) >>  8);
   endRGB.blue    =  (0x000000FF & end_rgb);

   diffR = endRGB.red - startRGB.red;
   diffG = endRGB.green - startRGB.green;
   diffB = endRGB.blue - startRGB.blue;

   /* Normalize the angle */
   
   if (angle < 0) angle = 360 - ((-angle)%360);
   if (angle >= 0) angle = angle % 360;

   if (angle <= 90 || (angle > 180 && angle <= 270))
   {
	  /* The to be intersected diagonal goes from the top left edge to the bottom right edge */
	  xs = 0;
	  ys = 0;
	  xw = width;
	  yw = height;
   }
   else
   {
	  /* The to be intersected diagonal goes from the bottom left edge to the top right edge */
	  xs = 0;
	  ys = height;
	  xw = width;
	  yw = -height;
   }

   if (angle > 90 && angle <= 270)
   {
	  /* for these angle we have y1 = height - y1. Instead of
	   *
	   *  y1 = height - (-vy*(yw*  xs -xw*  ys)         + yw*(vy*  x -vx*  y))        /(-yw*vx + xw*vy);
	   *
	   * we can write
	   *
		 *  y1 =          (-vy*(yw*(-xs)-xw*(-ys+height)) + yw*(vy*(-x)-vx*(-y+height)))/(-yw*vx + xw*vy);
		 *
		 * so height - y1 can be expressed with the normal formular adapting some parameters.
	   *
	   * Note that if one would exchanging startRGB/endRGB the values would only work
	   * for linear color gradients
	   */

	  xadd = -1;
	  yadd = -1;
	  ystart = height;

	  xs = -xs;
	  ys = -ys + height;
   }
   else
   {
	  xadd = 1;
	  yadd = 1;
	  ystart = 0;
   }

   r = -vy*(yw*xs-xw*ys);
   t = -yw*vx + xw*vy;

	/* The formular as shown above is
	 *
	 *    y1 = ((-vy*(yw*xs-xw*ys) + yw*(vy*x-vx*y)) /(-yw*vx + xw*vy));
	 *
	 * We see that only yw*(vy*x-vx*y) changes during the loop.
	 *
	 * We write
	 *
	 *   Current Pixel: y1(x,y) = (r + yw*(vy*x-vx*y))/t = r/t + yw*(vy*x-vx*y)/t
	 *   Next Pixel:    y1(x+xadd,y) = (r + vw*(vy*(x+xadd)-vx*y))/t
	 *
	 *   t*(y1(x+xadd,y) - y1(x,y)) = yw*(vy*(x+xadd)-vx*y) - yw*(vy*x-vx*y) = yw*vy*xadd;
	 *
	 */

   incr_y1 = yw*vy*xadd;

   for (l = 0, y = ystart; l < height; l++, y+=yadd)
   {
	  uint8 *bufptr = buf;

	  /* Calculate initial y1 accu, can be brought out of the loop as  well
	  (x=0).  It's  probably  a  a  good  idea  to add here also a value of
	  (t-1)/2 to ensure the correct rounding This (and for  r)  is  also  a
	  place  were  actually a overflow can happen |yw|=16 |y|=16. So for vx
	  nothing is left, currently 9 bits are used for vx or vy */

	  int y1_mul_t_accu = r - yw*vx*y;

	  #ifdef F_USE_PICASSO96
	  struct RenderInfo rInfo;
	  #endif

	  for (c = 0 ; c < width ; c++)
	  {
		 int red,green,blue;

		 /* Calculate the intersection of two lines, this is not the fastet
		 way  to  do but it is intuitive. Note: very slow! Will be optimzed
		 later (remove FFP usage and making it incremental)...update:  it's
		 now  incremental  and  no  FFP  is  used  but  it  probably can be
		 optimized more by removing some more of the divisions and  further
		 specialize the stuff here (use of three accus). */

		 y1 = y1_mul_t_accu / t;

		 red = startRGB.red + (int)(diffR*y1/height);
		 green = startRGB.green + (int)(diffG*y1/height);
		 blue = startRGB.blue + (int)(diffB*y1/height);

		 /* By using full 32 bits this can be made faster as well */

		 *bufptr++ = red;
		 *bufptr++ = green;
		 *bufptr++ = blue;

		 y1_mul_t_accu += incr_y1;
	  }

		/* By bringing building the gradient array in the same format as  the
		RastPort BitMap a call * to WritePixelArray() can be made also faster
		*/

		#ifdef F_USE_PICASSO96
		rInfo.Memory      = buf;
		rInfo.BytesPerRow = width*3;
		rInfo.RGBFormat   = RGBFB_R8G8B8;

		IP96_ p96WritePixelArray
		(
			&rInfo,

			0, 0,

			rp,

			xt, yt+1, width, 1
		);
		#else
		ICYBERGFX WritePixelArray
		(
			buf,
			0,0,
			width*3 /* srcMod */,
			rp,
			xt,yt+l,width,1,
			RECTFMT_RGB
		);
		#endif
	}

	IFEELIN F_Dispose(buf);

	return 1;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

struct FS_Render_DrawGradient                   { FRect *Rect; FRect *Origin; uint32 Angle; uint32 StartRGB; uint32 EndRGB; uint32 Flags; };

///Render_DrawGradient
F_METHODM(void,Render_DrawGradient,FS_Render_DrawGradient)
{
   struct LocalObjectData *LOD = F_LOD(Class,Obj);
 
   if (!(FF_Render_TrueColors & LOD->Public.Flags) || !(RTGFXBASE) || !(LOD->Public.RPort))
	   
   {
	  return;
   }

   switch (Msg->Angle)
   {
	  case 0:
	  {
		 id_gradient_draw_vertical
		 (
			LOD->Public.RPort,

			Msg->Rect->x1,
			Msg->Rect->y1,
			Msg->Rect->x2,
			Msg->Rect->y2,

			(Msg->Origin) ? Msg->Origin->y1 : Msg->Rect->y1,
			(Msg->Origin) ? Msg->Origin->y2 : Msg->Rect->y2,

			Msg->StartRGB,
			Msg->EndRGB
		 );
	  }
	  break;

	  case 90:
	  {
		 id_gradient_draw_horizontal
		 (
			LOD->Public.RPort,

			Msg->Rect->x1,
			Msg->Rect->y1,
			Msg->Rect->x2,
			Msg->Rect->y2,

			(Msg->Origin) ? Msg->Origin->x1 : Msg->Rect->x1,
			(Msg->Origin) ? Msg->Origin->x2 : Msg->Rect->x2,

			Msg->StartRGB,
			Msg->EndRGB
		 );
	  }
	  break;

	  case 180:
	  {
		 id_gradient_draw_vertical
		 (
			LOD->Public.RPort,

			Msg->Rect->x1,
			Msg->Rect->y1,
			Msg->Rect->x2,
			Msg->Rect->y2,

			(Msg->Origin) ? Msg->Origin->y1 : Msg->Rect->y1,
			(Msg->Origin) ? Msg->Origin->y2 : Msg->Rect->y2,

			Msg->EndRGB,
			Msg->StartRGB
		 );
	  }
	  break;

	  case 270:
	  {
		 id_gradient_draw_horizontal
		 (
			LOD->Public.RPort,

			Msg->Rect->x1,
			Msg->Rect->y1,
			Msg->Rect->x2,
			Msg->Rect->y2,

			(Msg->Origin) ? Msg->Origin->x1 : Msg->Rect->x1,
			(Msg->Origin) ? Msg->Origin->x2 : Msg->Rect->x2,

			Msg->EndRGB,
			Msg->StartRGB
		 );
	  }
	  break;

	  default:
	  {
		 APTR clip=NULL;

		 if (Msg->Origin)
		 {
			if (Msg->Origin->x1 != Msg->Rect->x1 ||
				Msg->Origin->y1 != Msg->Rect->y1 ||
				Msg->Origin->x2 != Msg->Rect->x2 ||
				Msg->Origin->y2 != Msg->Rect->y2)
			{
			   clip = (APTR) IFEELIN F_Do(Obj,FM_Render_AddClip,Msg->Rect);
			}
		 }

		 id_gradient_draw_angle
		 (
			LOD->Public.RPort,

			(clip) ? Msg->Origin->x1 : Msg->Rect->x1,
			(clip) ? Msg->Origin->y1 : Msg->Rect->y1,
			(clip) ? Msg->Origin->x2 : Msg->Rect->x2,
			(clip) ? Msg->Origin->y2 : Msg->Rect->y2,

			Msg->StartRGB,
			Msg->EndRGB,

			Msg->Angle
		 );

		 if (clip)
		 {
			IFEELIN F_Do(Obj,FM_Render_RemClip,clip);
		 }
	  }
	  break;
   }
}
//+
///Render_Fill

#define F_PIXEL_RED(rgba)                       ((0xFF000000 & rgba) >> 24)
#define F_PIXEL_GREEN(rgba)                     ((0x00FF0000 & rgba) >> 16)
#define F_PIXEL_BLUE(rgba)                      ((0x0000FF00 & rgba) >>  8)
#define F_PIXEL_ALPHA(rgba)                      (0x000000FF & rgba)
 
F_METHODM(uint32, Render_Fill, FS_Render_Fill)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	uint32 count = 0;
	uint32 rgba = Msg->RGBA;

	int32 x = Msg->x1;
	int32 y = Msg->y1;
	int32 x2 = Msg->x2;
	int32 y2 = Msg->y2;

	uint32 w = x2 - x + 1;
	uint32 h = y2 - y + 1;

	if ((x2 < x) || (y2 < y))
	{
		IFEELIN F_Log(0, "coordinates error: (%ld <? %ld) (%ld <? %ld)", x2, x, y2, y);

		return 0;
	}

	if ((FF_Render_TrueColors & LOD->Public.Flags) && (RTGFXBASE != NULL))
	{
		int32 a = 255 - F_PIXEL_ALPHA(rgba);

		if (a)
		{
			uint32 mod = w * sizeof (uint32);
			uint32 *line_buffer = IFEELIN F_New(mod);

			if (line_buffer != NULL)
			{
				int32 r = F_PIXEL_RED(rgba);
				int32 g = F_PIXEL_GREEN(rgba);
				int32 b = F_PIXEL_BLUE(rgba);

				for ( ; y < y2 ; y++)
				{
					uint32 *buf = line_buffer;
					uint32 j;

					#ifdef F_USE_PICASSO96
					struct RenderInfo rInfo;

					rInfo.Memory      = line_buffer;
					rInfo.BytesPerRow = mod;
					rInfo.RGBFormat   = RGBFB_R8G8B8A8;

					IP96_ p96ReadPixelArray
					(
						&rInfo,

						0, 0,

						LOD->Public.RPort,

						x, y, w, 1
					);
					#else
					ICYBERGFX ReadPixelArray
					(
						line_buffer,
						0,0,
						mod,
						LOD->Public.RPort,
						x,y, w,1,
						RECTFMT_RGBA
					);
					#endif

					for (j = x ; j < x2 ; j++)
					{
						uint32 rc;
						uint32 rgba = *buf;
						int32 s;

						s = F_PIXEL_RED(rgba);

						rc = s + (((r - s) * a) >> 8);
						rc <<= 8;

						s = F_PIXEL_GREEN(rgba);

						rc |= s + (((g - s) * a) >> 8);
						rc <<= 8;

						s = F_PIXEL_BLUE(rgba);

						rc |= s + (((b - s) * a) >> 8);
						rc <<= 8;

						*buf++ = rc;
					}
					#ifdef F_USE_PICASSO96
					rInfo.Memory      = line_buffer;
					rInfo.BytesPerRow = mod;
					rInfo.RGBFormat   = RGBFB_R8G8B8A8;

					IP96_ p96WritePixelArray
					(
						&rInfo,

						0, 0,

						LOD->Public.RPort,

						x, y, w, 1
					);
					#else
					ICYBERGFX WritePixelArray
					(
						line_buffer,
						0,0,
						mod,
						LOD->Public.RPort,
						x,y, w,1,
						RECTFMT_RGBA
					);
					#endif

					count += w;
				}
				IFEELIN F_Dispose(line_buffer);
			}
		}
		else
		{
			#ifdef F_USE_PICASSO96
			IP96_ p96RectFill
			(
				LOD->Public.RPort,

				x, y, x + w - 1, y + h - 1,

				F_RGBA_TO_ARGB(Msg->RGBA)
			);
			#else
			ICYBERGFX FillPixelArray
			(
				LOD->Public.RPort,

				x, y, w, h,

				F_RGBA_TO_ARGB(Msg->RGBA)
			);
			#endif

		}
	}

	return count;
}
//+
