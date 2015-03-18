#include "Private.h"

#undef  _Text
#define _Text(str,len)   IGRAPHICS Text(rp,str,len)

///Scale_New
F_METHOD(uint32,Scale_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif
	F_SAVE_AREA_PUBLIC;

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Widget_Chainable, FALSE,

		TAG_MORE, Msg);
}
//+
///Scale_Setup
F_METHOD(bool32,Scale_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO())
	{
		struct RastPort rp;
		
		IGRAPHICS InitRastPort(&rp);
		IGRAPHICS SetFont(&rp,_area_font);

		LOD->Height   = _area_font->tf_YSize;
		LOD->MinWidth = IGRAPHICS TextLength(&rp,"0% 100%",7);

		#ifdef __AROS__
		DeinitRastPort(&rp);
		#endif
 
		return TRUE;
	 }
	 return FALSE;
}
//+
///Scale_AskMinMax
F_METHOD(void,Scale_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_SUPERDO();

	#ifdef F_NEW_GLOBALCONNECT
	if (IFEELIN F_Get(_element_parent, FA_Area_Orientation) == FV_Area_Orientation_Vertical)
	#else
	if (IFEELIN F_Get(_area_parent, FA_Area_Orientation) == FV_Area_Orientation_Vertical)
	#endif
	{
		_area_minw += LOD->MinWidth;
		_area_minh += LOD->Height + 4;
	}
}
//+
///Scale_Draw
F_METHODM(void,Scale_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct RastPort *rp = _area_rp;

	F_SUPERDO();

	#ifdef F_NEW_GLOBALCONNECT
	if (IFEELIN F_Get(_element_parent, FA_Area_Orientation) == FV_Area_Orientation_Vertical)
	#else
	if (IFEELIN F_Get(_area_parent, FA_Area_Orientation) == FV_Area_Orientation_Vertical)
	#endif
	{
		int16  i,k;
		uint8 buf[255];
		BOOL  drawpct;

		/* main horizontal bar */

		_FPen(FV_Pen_Text);
		_Move(_area_cx,_area_cy + 3);
		_Draw(_area_cx2,_area_cy + 3);

		/* testing possible layouts: 1 = 0...100 ; 2 = 0...50..100 ; etc ... */

		for (i = 2 ; i < _area_cw ; i *= 2)
		{
			int16 j,total_width = 0,too_big = FALSE;

			for (j = 0; j <= i; j++)
			{
				IFEELIN F_StrFmt(buf, "%ld%%", j);

				total_width += IGRAPHICS TextLength(rp,buf, IFEELIN F_StrLen(buf));

				if (total_width > (3 * _area_cw / 8))
				{
					too_big = TRUE; break;
				}
			}
			if (too_big) break;
		}

		for (k = 0, drawpct = TRUE ; k <= i ; k++)
		{
			if (drawpct == TRUE)
			{
				int16 val = k * 100 / i;
				int16 width;

				IFEELIN F_StrFmt(buf, "%ld%%", val);

				width = IGRAPHICS TextLength(rp,buf,IFEELIN F_StrLen(buf));

				if (val == 100)
				{
					_Move(_area_cx + _area_cw - width + 1,_area_cy + _area_font->tf_Baseline + 5);
				}
				else if (val == 0)
				{
					_Move(_area_cx,_area_cy + _area_font->tf_Baseline + 5);
				}
				else
				{
					_Move(_area_cx + _area_cw * k / i - width / 2,_area_cy + _area_font->tf_Baseline + 5);
				}

				_DrMd(JAM1);

				{
					uint32 pen;

					if ((val >= 0 && val <= 6) || (val <= 100 && val >= 93))
					{
						pen = FV_Pen_Shine;
					}
					else if ((val > 6 && val <= 12) || (val < 93 && val >= 87))
					{
						pen = FV_Pen_HalfShine;
					}
					else if ((val > 12 && val <= 18) || (val < 87 && val >= 81))
					{
						pen = FV_Pen_Fill;
					}
					else if ((val > 18 && val <= 25) || (val < 81 && val >= 75))
					{
						pen = FV_Pen_HalfShadow;
					}
					else if ((val > 25 && val <= 31) || (val < 75 && val >= 68))
					{
						pen = FV_Pen_Shadow;
					}
					else if ((val > 31 && val <= 37) || (val < 68 && val >= 62))
					{
						pen = FV_Pen_HalfDark;
					}
					else
					{
						pen = FV_Pen_Dark;
					}

					_FPen(pen);
				}

				_Text(buf,IFEELIN F_StrLen(buf));
				_DrMd(JAM2);

				_FPen(FV_Pen_Dark);
				_Move(_area_cx + _area_cw * k / i, _area_cy);
				_Draw(_area_cx + _area_cw * k / i, _area_cy + 3);

				drawpct = FALSE;
			}
			else // draw intermediate lines
			{
				_FPen(FV_Pen_Dark);
				_Move(_area_cx + _area_cw * k / i, _area_cy + 1);
				_Draw(_area_cx + _area_cw * k / i, _area_cy + 3);

				drawpct = TRUE;
			}
		}
	}
}
//+
