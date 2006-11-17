#include "Private.h"

//#define DB_ASKMINMAX
//#define DB_LAYOUT

//#define F_DISABLE_SUPER_DRAW

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

#if 0
///DrRect
STATIC void DrRect(FRender *Render,FRect *Rect,uint32 Flags)
{
	uint32 Pen = 3;//((uint32)(Rect) + (uint32)(Render)) % 128;
  
	if (FF_Erase_Region & Flags)
	{
		struct RegionRectangle *rr;
  
		for (rr = ((struct Region *)(Rect))->RegionRectangle ; rr ; rr = rr->Next)
		{
			if (rr->bounds.MinX <= rr->bounds.MaxX && rr->bounds.MinY <= rr->bounds.MaxY)
			{
				IGRAPHICS SetAPen(Render->RPort,Pen);
				IGRAPHICS RectFill(Render->RPort,((struct Region *)(Rect))->bounds.MinX + rr->bounds.MinX,
										((struct Region *)(Rect))->bounds.MinY + rr->bounds.MinY,
										((struct Region *)(Rect))->bounds.MinX + rr->bounds.MaxX,
										((struct Region *)(Rect))->bounds.MinY + rr->bounds.MaxY);
			}
			else
			{
				IFEELIN F_Log(0,"bad coordinates (%ld > %ld) (%ld > %ld)",rr->bounds.MinX,rr->bounds.MaxX,rr->bounds.MinY,rr->bounds.MaxY);
			}
		}
	}
	else
	{
		IGRAPHICS SetAPen(Render->RPort,Pen);
		IGRAPHICS RectFill(Render->RPort,Rect->x1,Rect->y1,Rect->x2,Rect->y2);
	}
}
//+
#endif

#define NOPEN -1

#define _Box(x1,y1,x2,y2)                       frame_draw_box(rp,x1,y1,x2,y2)
#define _Bevel(x1,y1,x2,y2,p1,p2,p3)            frame_draw_bevel(rp,x1,y1,x2,y2,p1,p2,p3)
#define _Line(x1,y1,x2,y2)                      _Move(x1,y1); _Draw(x2,y2)

///frame_draw_line
void frame_draw_line(struct RastPort *rp,uint32 Pen,uint32 Count,...)
{
   va_list ap;
   va_start(ap,Count);

   if (Count <= 2) return;

   _APen(Pen);
   _Move(va_arg(ap,uint32),va_arg(ap,uint32));

   while (--Count > 0)
   {
	  _Draw(va_arg(ap,uint32),va_arg(ap,uint32));
   }
}
//+
///frame_draw_box
void frame_draw_box(struct RastPort *rp,uint32 x1,uint32 y1,uint32 x2,uint32 y2)
{
   _Move(x1,y2) ; _Draw(x1,y1) ; _Draw(x2,y1) ; _Draw(x2,y2) ; _Draw(x1,y2);
}
//+
///frame_draw_cadre
void frame_draw_cadre(struct RastPort *rp,uint32 x1,uint32 y1,uint32 x2,uint32 y2,uint32 p1,uint32 p2)
{
   _APen(p1) ; _Move(x1,y2)     ; _Draw(x1,y1)     ; _Draw(x2,y1);
			 ; _Move(x1+1,y2-1) ; _Draw(x2-1,y2-1) ; _Draw(x2-1,y1+1);
   _APen(p2) ; _Box(x1+1,y1+1,x2,y2);
}
//+
///frame_draw_bevel
void frame_draw_bevel(struct RastPort *rp,uint32 x1,uint32 y1,uint32 x2,uint32 y2,uint32 p1,uint32 p2,uint32 p3)
{
   if (p3 != NOPEN)
   {
	  _APen(p3); _Plot(x1,y2); _Plot(x2,y1);
   }
   
   _APen(p1); _Move(x1,y2-1); _Draw(x1,y1); _Draw(x2-1,y1); _APen(p2); _Move(x1+1,y2); _Draw(x2,y2); _Draw(x2,y1+1);
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

#if 0
///Border_AskMinMax
F_METHOD(uint32,Border_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	uint32 w = 0;
	uint32 h;
	uint32 border_t = 0;
	uint32 border_b = 0;
	uint32 padding_h = 0;

	uint32 i;

	if (LOD->caption != NULL)
	{
		if ((FF_CAPTION_POSITION_DOWN & LOD->caption->position) != 0)
		{
			border_b = LOD->caption->height;
		}
		else
		{
			border_t = LOD->caption->height;
		}
	}

	for (i = 0 ; i < FV_AREA_STATE_COUNT ; i++)
	{
		struct in_Frame *frame = LOD->frames[i];

		if (frame != NULL)
		{
			w = MAX(w, frame->border.l + frame->border.r + frame->padding.l + frame->padding.r);

			border_t = MAX(border_t, frame->border.t);
			border_b = MAX(border_b, frame->border.b);
			padding_h = MAX(padding_h, frame->padding.t + frame->padding.b);
		}
	}


	h = border_t + border_b + padding_h;

	#ifdef DB_ASKMINMAX
	IFEELIN F_Log(0, "min (%ld x %ld) max (%ld x %ld) add (%ld, %ld)", _area_minw, _area_minh, _area_maxw, _area_maxh, w, h);
	#endif

	_area_minw += w;
	_area_maxw += w;
	_area_minh += h;
	_area_maxh += h;

	return F_SUPERDO();
}
//+
#endif

///Border_Draw
F_METHODM(void,Border_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct RastPort *rp = _area_rp;

	int16 x1 = _area_x + _area_margin.l;
	int16 y1 = _area_y + _area_margin.t;
	int16 x2 = _area_x2 - _area_margin.r;
	int16 y2 = _area_y2 - _area_margin.b;

	FRect p,r,tr; /* text rectangle */

	struct Region *clear_region;
	struct Region *title_region=NULL;

//    struct Region *parent_clear_region = NULL;
	
	APTR clip=NULL;

	struct in_Caption *caption = LOD->caption;
	FPadding *brd = &_area_border;

	struct in_Frame *frame = LOD->frame;

	F_SUPERDO();

	/* this region is used to collect rectangles to erase */

	clear_region = IGRAPHICS NewRegion();

	if (caption != NULL)
	{
		uint32 tw;

		tr.y1 = (FF_CAPTION_POSITION_BOTTOM & caption->position) ? y2 - caption->height + 1 : y1;
		tr.y2 = (FF_CAPTION_POSITION_BOTTOM & caption->position) ? y2 : y1 + caption->height - 1;

		if ((int32)(x2 - x1 + 1 - 20 - brd->l - brd->r) > 0)
		{
			if (caption->width > (x2 - x1 + 1 - 20 - brd->l - brd->r))
			{
				tr.x1 = x1 + 10 + brd->l;
				tr.x2 = x2 - 10 - brd->r;

				IFEELIN F_Do(caption->display, FM_Set,

					FA_TextDisplay_Width,   tr.x2 - tr.x1 + 1,
					FA_TextDisplay_Height,  tr.y2 - tr.y1 + 1,

				TAG_DONE);

				tw = IFEELIN F_Get(caption->display, FA_TextDisplay_Width);
			}
			else
			{
				tw = caption->width;
			}
		}
		else
		{
			tw = 0;
		}

		if (tw)
		{
			uint32 y;

			switch (~FF_CAPTION_POSITION_BOTTOM & caption->position)
			{
				case FV_CAPTION_POSITION_CENTER:
				{
					tr.x1 = (x2 - x1 + 1 - tw) / 2 + x1 - 1;
				}
				break;

				case FV_CAPTION_POSITION_RIGHT:
				{
					tr.x1 = x2 - brd->r - 10 - tw;
				}
				break;

				default:
				{
					tr.x1 = x1 + brd->l + 10;
				}
				break;
			}

			tr.x2 = tr.x1 + tw;

			/* clear the upper or lower background of the text */

			p.x1 = x1;
			p.x2 = x2;

			if (FF_CAPTION_POSITION_BOTTOM & caption->position)
			{
				p.y2 = y2;
				y = y2 -= (caption->height - caption->border) / 2;
				p.y1 = y + 1;
			}
			else
			{
				p.y1 = y1;
				y = y1 += (caption->height - caption->border) / 2;
				p.y2 = y - 1;
			}

			if (FF_CAPTION_POSITION_BOTTOM & caption->position)
			{
				if (clear_region)
				{
					r.x1 = tr.x1 - 5; r.y1 = y - caption->border + 1;
					r.x2 = tr.x2 + 5; r.y2 = y;

					IGRAPHICS OrRectRegion(clear_region,(struct Rectangle *) &r);

					r.x1 = x1 + brd->l; r.y1 = _area_y + _area_h - 1 - brd->b;
					r.x2 = x2 - brd->r; r.y2 = y - caption->border;

					IGRAPHICS OrRectRegion(clear_region,(struct Rectangle *) &r);
				}
			}
			else
			{
				if (clear_region)
				{
					r.x1 = tr.x1 - 5; r.y1 = y;
					r.x2 = tr.x2 + 5; r.y2 = y + caption->border - 1;

					IGRAPHICS OrRectRegion(clear_region,(struct Rectangle *) &r);

					r.x1 = x1 + brd->l; r.y1 = y + caption->border;
					r.x2 = x2 - brd->r; r.y2 = _area_y + brd->t - 1;

					IGRAPHICS OrRectRegion(clear_region,(struct Rectangle *) &r);
				}
			}

			/* clip title region, to prevent frame drawing */

			if ((title_region = IGRAPHICS NewRegion()) != NULL)
			{
				r.x1 = x1; r.y1 = y1;
				r.x2 = x2; r.y2 = y2;

				IGRAPHICS OrRectRegion(title_region,(struct Rectangle *) &r);

				r.x1 = tr.x1 - 5; r.y1 = tr.y1;
				r.x2 = tr.x2 + 5; r.y2 = tr.y2;

				IGRAPHICS ClearRectRegion(title_region,(struct Rectangle *) &r);
			}
		}
		else
		{
			if (clear_region != NULL)
			{
				r.x1 = _area_x + brd->l;
				r.x2 = _area_x + _area_w - 1 - brd->r;

				if (FF_CAPTION_POSITION_BOTTOM & caption->position)
				{
					r.y1 = _area_y2 - brd->b + 1;
					r.y2 = _area_y2;
				}
				else
				{
					r.y1 = _area_y;
					r.y2 = _area_y + brd->t - 1;
				}

				IGRAPHICS OrRectRegion(clear_region,(struct Rectangle *) &r);
			}

			caption = NULL;
		}
	}

	/* MOVE LATER

	if (clear_region != NULL)
	{
		#if 1

		IFEELIN F_Do(Obj, FM_Area_Erase, clear_region, FF_Erase_Region);

		#else

		IFEELIN F_Do(_area_background, FM_ImageDisplay_Draw, _area_render, clear_region, FF_ImageDisplay_Draw_Region);

		#endif

		IGRAPHICS DisposeRegion(clear_region);
	}

	*/

	if (title_region)
	{
		clip = (APTR) IFEELIN F_Do(_area_render,FM_Render_AddClipRegion,title_region);

		IGRAPHICS DisposeRegion(title_region);
	}

	switch (frame->type)
	{
		case FV_FRAME_TYPE_BORDER:
		{
			frame_draw_border(Class, Obj, x1, y1, x2, y2, _area_render, clear_region);
		}
		break;

		case FV_FRAME_TYPE_FRAME:
		{

/// Drawing Frame
		uint32 *pn = _area_palette->Pens;
		uint32 shine       = pn[FV_Pen_Shine],
			   halfshine   = pn[FV_Pen_HalfShine],
			   fill        = pn[FV_Pen_Fill],
			   halfshadow  = pn[FV_Pen_HalfShadow],
			   shadow      = pn[FV_Pen_Shadow],
			   halfdark    = pn[FV_Pen_HalfDark],
			   dark        = pn[FV_Pen_Dark];

		switch (LOD->frame->id)
	  	{
///01
		 case 1:
__01:
			_APen(dark);
__01a:
			_Box(x1,y1,x2,y2);

			break;
//+
///02 >> 01a
		 case 2:
			_APen(shine);
			goto __01a;
//+
///03 >> 01a
		 case 3:
			_APen(halfdark);
			goto __01a;
//+
///04 >> 01a
		 case 4:
			_APen(halfshine);
			goto __01a;
//+
///05
		 case 5:
			_APen(halfdark);
__05:
			_Line(x1+1,y1, x2-1,y1);
			_Line(x2,y1+1, x2,y2-1);
			_Line(x2-1,y2, x1+1,y2);
			_Line(x1,y2-1, x1,y1+1);
			_APen(fill);
			_Plot(x1,y1) ; _Plot(x2,y1) ; _Plot(x2,y2) ; _Plot(x1,y2);
			break;
//+
///06 >> 05
		 case 6:
			_APen(halfshine);
			goto __05;
//+
///08->07
		 case 8:
			shadow = halfshine;
			halfshadow = fill;
//+
///07
		 case 7:
		 {
			_APen(fill);

			_Boxf(x1,y1,x1+1,y1+1);
			_Boxf(x1,y2-1,x1+1,y2);
			_Boxf(x2-1,y1,x2,y1+1);
			_Boxf(x2-1,y2-1,x2,y2);

			_Line(x1+3,y1+1,x2-3,y1+1);
			_Line(x1+3,y2-1,x2-3,y2-1);
			_Line(x1+1,y1+3,x1+1,y2-3);
			_Line(x2-1,y1+3,x2-1,y2-3);

			#if 0
			struct Region *region;

			if (region = IGRAPHICS NewRegion())
			{
			   FRect r;

			   r.x1 = x1; r.y1 = y1; r.x2 = x1+1; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1; r.y1 = y2-1; r.x2 = x1+1; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y1; r.x2 = x2; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y2-1; r.x2 = x2; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+3; r.y1 = y1+1; r.x2 = x2-3; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1+3; r.y1 = y2-1; r.x2 = x2-3; r.y2 = y2-1; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+1; r.y1 = y1+3; r.x2 = x1+1; r.y2 = y2-3; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y1+3; r.x2 = x2-1; r.y2 = y2-3; IGRAPHICS OrRectRegion(region,&r);

			   IFEELIN F_Do(Obj,FM_Area_Erase,region,FF_Erase_Region);

			   IGRAPHICS DisposeRegion(region);
			}
			#endif

			_APen(shadow);
			_Line(x1,y1+3, x1,y2-3); _Line(x1+3,y1, x2-3,y1); _Plot(x1+1,y1+1) ; _Plot(x1+1,y1+2); _Plot(x1+2,y1+1); _Plot(x2-1,y1+1) ; _Plot(x2-2,y1+1); _Plot(x2-1,y1+2);
			_Line(x2,y1+3, x2,y2-3); _Line(x1+3,y2, x2-3,y2); _Plot(x1+1,y2-1) ; _Plot(x1+1,y2-2); _Plot(x1+2,y2-1); _Plot(x2-1,y2-1) ; _Plot(x2-2,y2-1); _Plot(x2-1,y2-2);
			_APen(halfshadow);
			_Plot(x1,y1+2); _Plot(x1+2,y1); _Plot(x2-2,y1); _Plot(x2,y1+2);
			_Plot(x1,y2-2); _Plot(x1+2,y2); _Plot(x2-2,y2); _Plot(x2,y2-2);
		 }
		 break;
//+
///09
		 case 9:
			frame_draw_cadre(rp,x1,y1,x2,y2,shine,dark);
			break;
//+
///10
		 case 10:
			frame_draw_cadre(rp,x1,y1,x2,y2,dark,shine);
			break;
//+
///11
		 case 11:
			frame_draw_cadre(rp,x1,y1,x2,y2,halfshine,halfdark);
			break;
//+
///12
		 case 12:
			frame_draw_cadre(rp,x1,y1,x2,y2,halfdark,halfshine);
			break;
//+
#if 0
///13
		 case 13:
		 {
			_APen(fill);
			_Boxf(x1,y1,x1+1,y1+1);
			_Boxf(x1,y2-1,x1+1,y2);
			_Boxf(x2-1,y1,x2,y1+1);
			_Boxf(x2-1,y2-1,x2,y2);

			_Line(x1+3,y1+1,x2-3,y1+1);
			_Line(x1+3,y2-1,x2-3,y2-1);
			_Line(x1+1,y1+3,x1+1,y2-3);
			_Line(x2-1,y1+3,x2-1,y2-3);

			#if 0
			struct Region *region;

			if (region = IGRAPHICS NewRegion())
			{
			   FRect r;

			   r.x1 = x1; r.y1 = y1; r.x2 = x1+1; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1; r.y1 = y2-1; r.x2 = x1+1; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y1; r.x2 = x2; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y2-1; r.x2 = x2; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+3; r.y1 = y1+1; r.x2 = x2-3; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1+3; r.y1 = y2-1; r.x2 = x2-3; r.y2 = y2-1; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+1; r.y1 = y1+3; r.x2 = x1+1; r.y2 = y2-3; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y1+3; r.x2 = x2-1; r.y2 = y2-3; IGRAPHICS OrRectRegion(region,&r);

			   IFEELIN F_Do(Obj,FM_Area_Erase,region,FF_Erase_Region | FF_Erase_Fill);

			   IGRAPHICS DisposeRegion(region);
			}
			#endif

			_APen(shadow);
			_Line(x1+3,y2, x2-3,y2); _Plot(x1+1,y2-2); _Plot(x1+1,y2-1); _Plot(x1+2,y2-1);
			_Line(x2,y1+3, x2,y2-3); _Plot(x2-2,y2-1); _Plot(x2-1,y2-1); _Plot(x2-1,y2-2); _Plot(x2-2,y1+1); _Plot(x2-1,y1+1); _Plot(x2-1,y1+2);
			_APen(halfshadow);
			_Plot(x1,y2-2); _Plot(x1+2,y2); _Plot(x2-2,y2); _Plot(x2,y2-2); _Plot(x2,y1+2); _Plot(x2-2,y1);
			_APen(fill);
			_Plot(x1,y2-3); _Plot(x2-3,y1);
			_APen(halfshine);
			_Line(x1,y1+2, x1,y2-4); _Line(x1+2,y1, x2-4,y1);
			_APen(shine);
			_Plot(x1+1,y1+1); _Plot(x1+2,y1+1); _Plot(x1+1,y1+2);
		 }
		 break;
//+
///14
		 case 14:
		 {
			_APen(fill);
			_Boxf(x1,y1,x1+1,y1+1);
			_Boxf(x1,y2-1,x1+1,y2);
			_Boxf(x2-1,y1,x2,y1+1);
			_Boxf(x2-1,y2-1,x2,y2);
			_Line(x1+3,y1+1,x2-3,y1+1);
			_Line(x1+3,y2-1,x2-3,y2-1);
			_Line(x1+1,y1+3,x1+1,y2-3);
			_Line(x2-1,y1+3,x2-1,y2-3);

			#if 0
			struct Region *region;

			if (region = IGRAPHICS NewRegion())
			{
			   FRect r;

			   r.x1 = x1; r.y1 = y1; r.x2 = x1+1; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1; r.y1 = y2-1; r.x2 = x1+1; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y1; r.x2 = x2; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y2-1; r.x2 = x2; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+3; r.y1 = y1+1; r.x2 = x2-3; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1+3; r.y1 = y2-1; r.x2 = x2-3; r.y2 = y2-1; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+1; r.y1 = y1+3; r.x2 = x1+1; r.y2 = y2-3; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y1+3; r.x2 = x2-1; r.y2 = y2-3; IGRAPHICS OrRectRegion(region,&r);

			   DrRect(Msg->Render,region,FF_Erase_Region);

			   IGRAPHICS DisposeRegion(region);
			}
			#endif

			_APen(shadow);
			_Plot(x1+1,y1+1); _Plot(x1+2,y1+1); _Plot(x1+1,y1+2);
			_Plot(x2-1,y1+1); _Plot(x2-2,y1+1); _Plot(x2-1,y1+2);
			_Plot(x1+1,y2-1); _Plot(x1+1,y2-2); _Plot(x1+2,y2-1);
			_Line(x1,y1+3, x1,y2-3);
			_Line(x1+3,y1, x2-3,y1);
			_APen(halfshadow);
			_Plot(x1,y1+2); _Plot(x1+2,y1); _Plot(x1,y2-2); _Plot(x1+2,y2); _Plot(x2,y1+2); _Plot(x2-2,y1);
			_APen(fill);
			_Plot(x1+3,y2); _Plot(x2,y1+3);
			_APen(halfshine);
			_Line(x1+4,y2, x2-2,y2);
			_Line(x2,y1+4, x2,y2-2);
			_APen(shine);
			_Plot(x2-1,y2-1); _Plot(x2-2,y2-1); _Plot(x2-1,y2-2);
		 }
		 break;
//+
#endif
///15
		 case 15:
		 {
			_APen(fill);
			_Boxf(x1,y1,x1+1,y1+1);
			_Boxf(x1,y2-2,x1+2,y2); _Plot(x1+2,y2-3); _Plot(x1+3,y2-2);
			_Boxf(x2-2,y1,x2,y1+2); _Plot(x2-3,y1+2); _Plot(x2-2,y1+3);
			_Boxf(x2-1,y2-1,x2,y2);
			_Line(x1+4,y1+2,x2-4,y1+2);
			_Line(x1+4,y2-2,x2-4,y2-2);
			_Line(x1+2,y1+4,x1+2,y2-4);
			_Line(x2-2,y1+4,x2-2,y2-4);

			#if 0
			struct Region *region;

			if (region = IGRAPHICS NewRegion())
			{
			   FRect r;

			   r.x1 = x1; r.y1 = y1; r.x2 = x1+1; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1; r.y1 = y2-1; r.x2 = x1+1; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y1; r.x2 = x2; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y2-1; r.x2 = x2; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+4; r.y1 = y1+2; r.x2 = x2-4; r.y2 = y1+2; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1+4; r.y1 = y2-2; r.x2 = x2-4; r.y2 = y2-2; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+2; r.y1 = y1+4; r.x2 = x1+2; r.y2 = y2-4; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-2; r.y1 = y1+4; r.x2 = x2-2; r.y2 = y2-4; IGRAPHICS OrRectRegion(region,&r);

			   DrRect(Msg->Render,region,FF_Erase_Region);

			   IGRAPHICS DisposeRegion(region);
			}
			#endif

			_APen(shadow);
			_Line(x1+4,y1+1, x2-3,y1+1); _Draw(x2,y1+4); _Draw(x2,y2-3); _Draw(x2-3,y2); _Draw(x1+4,y2); _Draw(x1+1,y2-3); _Draw(x1+1,y1+4); _Draw(x1+4,y1+1);
			_Plot(x1+2,y1+2); _Plot(x2-1,y1+2); _Plot(x1+2,y2-1); _Plot(x2-1,y2-1);
			_APen(halfshadow);
			_Plot(x1+3,y1+1); _Plot(x1+1,y1+3); _Plot(x2-2,y1+1); _Plot(x2,y1+3);
			_Plot(x1+1,y2-2); _Plot(x1+3,y2);   _Plot(x2-2,y2);   _Plot(x2,y2-2);
			_APen(halfshine);
			_Plot(x1+2,y1); _Plot(x1,y1+2); _Plot(x2-1,y2-3); _Plot(x2-3,y2-1);
			_APen(shine);
			_Line(x1,y2-3, x1,y1+3); _Draw(x1+3,y1); _Draw(x2-3,y1); _Plot(x1+1,y1+1);
			_Line(x1+4,y2-1, x2-4,y2-1); _Draw(x2-1,y2-4); _Draw(x2-1,y1+4); _Plot(x2-2,y2-2);
		 }
		 break;
//+
///16
		 case 16:
		 {
			_APen(fill);
			_Boxf(x1,y1,x1+1,y1+1);
			_Boxf(x1,y2-2,x1+2,y2); _Plot(x1+2,y2-3); _Plot(x1+3,y2-2);
			_Boxf(x2-2,y1,x2,y1+2); _Plot(x2-3,y1+2); _Plot(x2-2,y1+3);
			_Boxf(x2-1,y2-1,x2,y2);
			_Line(x1+4,y1+2,x2-4,y1+2);
			_Line(x1+4,y2-2,x2-4,y2-2);
			_Line(x1+2,y1+4,x1+2,y2-4);
			_Line(x2-2,y1+4,x2-2,y2-4);

			#if 0
			struct Region *region;

			if (region = IGRAPHICS NewRegion())
			{
			   FRect r;

			   r.x1 = x1; r.y1 = y1; r.x2 = x1+1; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1; r.y1 = y2-1; r.x2 = x1+1; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y1; r.x2 = x2; r.y2 = y1+1; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-1; r.y1 = y2-1; r.x2 = x2; r.y2 = y2; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+4; r.y1 = y1+2; r.x2 = x2-4; r.y2 = y1+2; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x1+4; r.y1 = y2-2; r.x2 = x2-4; r.y2 = y2-2; IGRAPHICS OrRectRegion(region,&r);

			   r.x1 = x1+2; r.y1 = y1+4; r.x2 = x1+2; r.y2 = y2-4; IGRAPHICS OrRectRegion(region,&r);
			   r.x1 = x2-2; r.y1 = y1+4; r.x2 = x2-2; r.y2 = y2-4; IGRAPHICS OrRectRegion(region,&r);

			   DrRect(Msg->Render,region,FF_Erase_Region);

			   IGRAPHICS DisposeRegion(region);
			}
			#endif

			_APen(shadow);
			_Line(x1,y2-4, x1,y1+3); _Draw(x1+3,y1); _Draw(x2-4,y1);
			_Line(x1+4,y2-1, x2-4,y2-1); _Draw(x2-1,y2-4); _Draw(x2-1,y1+4);
			_Plot(x1+1,y1+1); _Plot(x2-2,y2-2);
			_APen(halfshadow);
			_Plot(x1,y2-3); _Plot(x1+2,y1); _Plot(x2-3,y1); _Plot(x1,y1+2); _Plot(x2,y2-3); _Plot(x2-1,y2-3); _Plot(x2-3,y2-1);
			_APen(halfshine);
			_Plot(x1+3,y1+1); _Plot(x2-2,y1+1); _Plot(x1+1,y1+3); _Plot(x2,y1+3);
			_Plot(x1+1,y2-2); _Plot(x2,y2-2); _Plot(x1+3,y2); _Plot(x2-2,y2);
			_APen(shine);
			_Line(x1+4,y1+1, x2-3,y1+1); _Draw(x2,y1+4); _Draw(x2,y2-3); _Draw(x2-3,y2); _Draw(x1+4,y2); _Draw(x1+1,y2-3); _Draw(x1+1,y1+4); _Draw(x1+4,y1+1);
			_Plot(x1+2,y1+2); _Plot(x1+2,y2-1); _Plot(x2-1,y1+2); _Plot(x2-1,y2-1);
		 }
		 break;
//+
///17 >> 23
			case 17:
			{
				_Bevel(x1,y1,x2,y2,shadow,shine,fill);

				x1++; y1++;
				x2--; y2--;
			}
			goto __23;
//+
///18
		 case 18:
__18:
			_Bevel(x1,y1,x2,y2,shadow,shine,fill);
			_APen(dark);
			_Box(x1+1,y1+1,x2-1,y2-1);
			break;
//+
///19 >> 18
		 case 19:
			frame_draw_line(rp,shadow,3,     x1+2,y2-2, x1+2,y1+2, x2-2,y1+2);
			frame_draw_line(rp,halfshadow,3, x1+3,y2-2, x1+3,y1+3, x2-2,y1+3);
			goto __18;
//+
///20->21
		 case 20:
			_Bevel(x1+3,y1+3,x2-3,y2-3,shine,shadow,fill);
			_Bevel(x1+4,y1+4,x2-4,y2-4,halfshine,halfshadow,fill);
//+
///21
		 case 21:
__21:
			_APen(fill);
			_Boxf(x1,y2-2,x1+2,y2);
			_Boxf(x2-2,y1,x2,y1+2);
			_APen(dark);
			_Box(x1+2,y1+2,x2-2,y2-2);
			_APen(shadow);
			_Line(x1+1,y2-2, x1+1,y1+1); _Draw(x2-2,y1+1);
			_APen(halfshadow);
			_Line(x1,y2-3, x1,y1); _Draw(x2-3,y1);
			_APen(halfshine);
			_Line(x1+3,y2, x2,y2); _Draw(x2,y1+3);
			_APen(shine);
			_Line(x1+2,y2-1, x2-1,y2-1); _Draw(x2-1,y1+2);
			break;
//+
///22 >> 21
		 case 22:
			_APen(halfdark);
			_Line(x1+3,y2-3, x1+3,y1+3); _Draw(x2-3,y1+3);
			_APen(shadow);
			_Line(x1+4,y2-3, x1+4,y1+4); _Draw(x2-3,y1+4);
			_APen(halfshadow);
			_Line(x1+5,y2-3, x1+5,y1+5); _Draw(x2-3,y1+5);
			goto __21;
//+
///25 >> 23
		 case 25:
			_Bevel(x1+2,y1+2,x2-2,y2-2,halfshine,halfshadow,fill);
//+
///23
		 case 23:
__23:
			_Bevel(x1+1,y1+1,x2-1,y2-1,shine,shadow,fill);
			goto __01;
//+
///24
		 case 24:
			frame_draw_line(rp,shadow,3,     x1+1,y2-1, x1+1,y1+1, x2-1,y1+1);
			frame_draw_line(rp,halfshadow,3, x1+2,y2-1, x1+2,y1+2, x2-1,y1+2);
			goto __01;
//+
///26
		 case 26:
			_APen(halfdark);
			_Line(x1+1,y2-1, x1+1,y1+1); _Draw(x2-1,y1+1);
			_APen(shadow);
			_Line(x1+2,y2-1, x1+2,y1+2); _Draw(x2-1,y1+2);
			_APen(halfshadow);
			_Line(x1+3,y2-1, x1+3,y1+3); _Draw(x2-1,y1+3);
			goto __01;
//+
///28->27
		 case 28:
		 {
			uint32 temp = shadow; shadow = shine; shine = temp;
		 }
//+
///27
		 case 27:
			_Bevel(x1+1,y1+1,x2-1,y2-1,shine,shadow,fill);
			_APen(dark);
			goto __05;
//+
///30->29
		 case 30:
		 {
			uint32 temp = shadow; shadow = shine; shine = temp; temp = halfshadow; halfshadow = halfshine; halfshine = temp;
		 }
//+
///29
		 case 29:
			_APen(fill);   _Boxf(x1,y1,x1+1,y1+1) ; _Boxf(x2-1,y1,x2,y1+1);
						   _Boxf(x1,y2-1,x1+1,y2) ; _Boxf(x2-1,y2-1,x2,y2);
			_APen(shine);
			_Line(x1+1,y2-2, x1+1,y1+2); _Draw(x1+2,y1+1); _Draw(x2-2,y1+1);
			_APen(shadow);
			_Line(x1+2,y2-1, x2-2,y2-1); _Draw(x2-1,y2-2); _Draw(x2-1,y1+2);
			_Bevel(x1+2,y1+2,x2-2,y2-2,halfshine,halfshadow,fill);
			_APen(shine)      ; _Plot(x1+2,y1+2);
			_APen(shadow)     ; _Plot(x2-2,y2-2);
			frame_draw_line(rp,dark,9,    x1+2,y1, x2-2,y1, x2,y1+2, x2,y2-2, x2-2,y2, x1+2,y2, x1,y2-2, x1,y1+2, x1+2,y1);
			break;
//+
///31
		 case 31:
			frame_draw_line(rp,fill,3,    x1+1,y2-2, x1+1,y1+1, x2-2,y1+1);
			_Line(x1,y2-1, x1,y2); _Line(x2-1,y1, x2,y1);
__31a:
			_APen(dark);
			_Line(x1+1,y2, x2,y2); _Draw(x2,y1+1);
			_Bevel(x1,y1,x2-1,y2-1,shine,shadow,NOPEN);
			_APen(fill);   _Plot(x1,y2); _Plot(x1,y2-1); _Plot(x2,y1); _Plot(x2-1,y1);
			break;
//+
///32
		 case 32:
			_APen(fill);   _Line(x1+2,y2-1, x2-1,y2-1);  _Draw(x2-1,y1+2);
__32a:
			_APen(dark);   _Line(x1,y2-1, x1,y1);        _Draw(x2-1,y1);
			_APen(shadow); _Line(x1+1,y2-1, x1+1,y1+1);  _Draw(x2-1,y1+1);
			_APen(shine);  _Line(x1+2,y2, x2,y2);        _Draw(x2,y1+2);
			_APen(fill);   _Plot(x1,y2); _Plot(x1+1,y2); _Plot(x2,y1); _Plot(x2,y1+1);
			break;
//+
#if 0
///33 >> 31a
		 case 33:
			frame_draw_line(rp,fill,3,    x1+2,y2-3, x1+2,y1+2, x2-3,y1+2);
			_Bevel(x1+1,y1+1,x2-2,y2-2,halfshine,halfshadow,fill);
			goto __31a;
//+
///34 >> 32a
		 case 34:
			frame_draw_line(rp,fill,3,    x1+3,y2-2, x2-2,y2-2, x2-2,y1+3);
			_Bevel(x1+2,y1+2,x2-1,y2-1,halfshadow,halfshine,fill);
			goto __32a;
//+
#endif
///35
		 case 35:
			_Bevel(x1,y1,x2,y2,shadow,dark,fill);
			_Bevel(x1+1,y1+1,x2-1,y2-1,shine,shadow,fill);
			_Bevel(x1+2,y1+2,x2-2,y2-2,halfshine,halfshadow,fill);
			break;
//+
///36
		 case 36:
			_APen(fill);
			_Line(x1+2,y2-1, x2-1,y2-1); _Draw(x2-1,y1+2);
			_APen(halfshine);
			_Line(x1,y2, x1,y1); _Draw(x2,y1);
			_APen(halfdark);
			_Line(x1+1,y2-2, x1+1,y1+1); _Draw(x2-2,y1+1);
			_APen(shadow);
			_Line(x1+2,y2-2, x1+2,y1+2); _Draw(x2-2,y1+2);
			_APen(dark);
			_Line(x1+1,y2, x2,y2); _Draw(x2,y1+1);
			_APen(halfshadow) ; _Plot(x1+1,y2-1) ; _Plot(x2-1,y1+1);
			break;
//+
#if 0
///37
		 case 37:
			dark = halfdark; shine = halfshine; shadow = halfshadow;
			goto __23;
//+
///38
		 case 38:
			dark = halfdark; shine = halfshadow; shadow = halfshine;
			goto __23;
//+
///39
		 case 39:
			_APen(fill); _Plot(x1+1,y1+2) ; _Plot(x1+2,y1+1); _Plot(x1+1,y2-1) ; _Plot(x2-1,y1+1);
			_APen(dark); _Box(x1,y1,x2,y2);
			frame_draw_line(rp,shine,4,      x1+1,y2-2, x1+1,y1+3, x1+3,y1+1, x2-2,y1+1);
			 _Line(x1+2,y1+3, x1+3,y1+2);
			frame_draw_line(rp,shadow,4,     x1+2,y2-1, x2-4,y2-1, x2-1,y2-4, x2-1,y1+2); _Plot(x2-2,y2-2);
			frame_draw_line(rp,halfshine,4,  x1+2,y1+5, x1+2,y1+4, x1+4,y1+2, x1+5,y1+2);
			frame_draw_line(rp,halfshadow,4, x2-5,y2-2, x2-4,y2-2, x2-2,y2-4, x2-2,y2-5);
			frame_draw_line(rp,halfdark,3,   x2-4,y2-1, x2-1,y2-1, x2-1,y2-4);
			_APen(dark); _Plot(x2-1,y2-1);
			_APen(shadow); _Plot(x1+1,y1+1);
			break;
//+
///40
		 case 40:
			_APen(halfshadow) ; _Plot(x1+1,y2-1) ; _Plot(x2-1,y1+1);
							  ; _Plot(x1+1,y1+2) ; _Plot(x1+2,y1+1);
			_APen(halfdark)   ; _Box(x1,y1,x2,y2);
			frame_draw_line(rp,shadow,4,     x1+1,y2-2, x1+1,y1+3, x1+3,y1+1, x2-2,y1+1); _Line(x1+2,y1+3, x1+3,y1+2);
			frame_draw_line(rp,fill,4,       x1+2,y2-1, x2-4,y2-1, x2-1,y2-4, x2-1,y1+2); _Plot(x2-2,y2-2);
			frame_draw_line(rp,halfshine,3,  x2-4,y2-1, x2-1,y2-1, x2-1,y2-4); _Plot(x1+1,y1+1);
			_APen(shine)      ; _Plot(x2-1,y2-1);
			break;
//+
#endif
///41
		 case 41:
			frame_draw_line(rp,fill,3,       x1+1,y2-1, x1+1,y1+1, x2-1,y1+1);
			frame_draw_line(rp,fill,3,       x1+4,y2-3, x2-3,y2-3, x2-3,y1+4);
			frame_draw_line(rp,shine,3,      x1+2,y2-2, x1+2,y1+2, x2-1,y1+2);
			frame_draw_line(rp,halfshine,3,  x1+3,y2-3, x1+3,y1+3, x2-2,y1+3);
			frame_draw_line(rp,halfshadow,3, x1+3,y2-2, x2-2,y2-2, x2-2,y1+4);
			frame_draw_line(rp,shadow,3,     x1+2,y2-1, x2-1,y2-1, x2-1,y1+2); _APen(halfdark);
			goto __05;
//+
///43->42
		 case 43:
		 {
			uint32 temp;

			temp = shine ; shine = shadow ; shadow = temp;
			temp = halfshine ; halfshine = halfshadow ; halfshadow = temp;
		 }
//+
///42
		 case 42:
			_Bevel(x1,y1,x2,y2,shine,shadow,fill);
			_Bevel(x1+1,y1+1,x2-1,y2-1,halfshine,halfshadow,fill);
		 break;
//+
#if 0
///44
		 case 44:
		 {
			uint16 ptrn = rp->LinePtrn;
			uint32 x3 = (x2 - x1 + 1) / 3 + x1;

			_APen(fill); _Boxf(x1,y2-1,x1+1,y2); _Boxf(x2-1,y1,x2,y1+1);

			frame_draw_line(rp,dark,3,       x1+2,y2, x2,y2, x2,y1+2);

			IGRAPHICS SetABPenDrMd(rp,dark,halfdark,JAM2);

			SetDrPt(rp,0xAAAA);
			_Line(x1+2,y2-1, x3,y2-1);
			SetDrPt(rp,ptrn);

			IGRAPHICS SetDrMd(rp,JAM1);

			_APen(halfdark)   ; _Plot(x1+1,y2-1);

			frame_draw_line(rp,halfdark,5,   x1+2,y2-2, x3,y2-2, x3+1,y2-1, x2-1,y2-1, x2-1,y1+1);
			frame_draw_line(rp,halfshine,5,  x1,y2-2, x1,y1, x3,y1, x3+1,y1+1, x2-3,y1+1);
			_APen(shine);
			 _Line(x3+1,y1, x2-2,y1);
			frame_draw_line(rp,fill,3,       x1+1,y2-3, x1+1,y1+1, x3,y1+1);  _Plot(x2-2,y1+1);

			_APen(halfshadow) ; _Plot(x1+1,y2-2);
			frame_draw_line(rp,halfdark,5,   x1+2,y2-2, x3,y2-2,   x3+1,y2-1, x2-1,y2-1, x2-1,y1+1);
			frame_draw_line(rp,shadow,3,     x3+1,y2-2, x2-2,y2-2, x2-2,y1+2);

			_APen(halfshadow) ; _Boxf(x1+2,y1+2,x3,y2-3);
			_APen(fill)       ; _Boxf(x3+1,y1+2,x2-3,y2-3);
		 }
		 break;
//+
///45
		 case 45:
		 {
			uint32 x3 = (x2 - x1 + 1) / 3 + x1 + 1;
			frame_draw_line(rp,dark,3,       x1,y2,     x1,y1,     x2,y1);
			frame_draw_line(rp,halfdark,3,   x1+1,y2-1, x1+1,y1+1, x3,y1+1);
			frame_draw_line(rp,shadow,3,     x1+2,y2-2, x1+2,y1+2, x3,y1+2);

								_Line(x3+1,y1+1, x2-1,y1+1);
			_APen(halfshadow) ; _Line(x3+1,y1+2, x2-2,y1+2);   _Plot(x1+2,y2-1);
			_APen(fill)       ; _Line(x1+3,y2-1, x3,y2-1);     _Plot(x2-1,y1+2);
			_APen(halfshine)  ; _Line(x1+1,y2,   x3,y2);
			frame_draw_line(rp,halfshine,3,  x3+1,y2-1, x2-1,y2-1, x2-1,y1+3);
			frame_draw_line(rp,shine,3,      x3+1,y2, x2,y2, x2,y1+1);
			x1++; y1++; x2++; y2++;
			_APen(halfshadow) ; _Boxf(x1+2,y1+2,x3,y2-3);
			_APen(fill)       ; _Boxf(x3+1,y1+2,x2-3,y2-3);
		 }
		 break;
//+
///46
		 case 46:
			_Bevel(x1,y1,x2,y2,shadow,shadow,fill);
			_Bevel(x1+1,y1+1,x2-1,y2-1,shine,halfdark,fill);
		 break;
//+
///47
		 case 47:
			_Bevel(x1,y1,x2,y2,shadow,shine,fill);
			_APen(halfdark);  _Move(x1+1,y2-1); _Draw(x1+1,y1+1); _Draw(x2-1,y1+1);
			_APen(shadow);    _Move(x1+2,y2-1); _Draw(x2-1,y2-1); _Draw(x2-1,y1+2);
		 break;
//+
#endif
///48
		 case 48:
			_Bevel(x1,y1,x2,y2,shine,shadow,fill);
			_APen(dark); _Box(x1+1,y1+1,x2-1,y2-1);
			_Bevel(x1+2,y1+2,x2-2,y2-2,shine,shadow,fill);
			break;
//+
///49
		 case 49:
							  ; _Bevel(x1,y1,x2,y2,shine,shadow,fill);
			_APen(dark)       ; _Box(x1+1,y1+1,x2-1,y2-1);
			frame_draw_line(rp,halfshadow,3, x1+2,y2-3, x1+2,y1+2, x2-3,y1+2);
			frame_draw_line(rp,halfshine,3,  x1+2,y2-2, x2-2,y2-2, x2-2,y1+2);
			break;
//+

			case 50:
  			{
  				_APen(shadow);       _Line(x1,y2,x1,y1); _Draw(x2,y1);
  				_APen(halfshadow);   _Line(x1+1,y2,x1+1,y1+1); _Draw(x2,y1+1);
  		   	}
  		   	break;

   		    case 51:
   		    {
				_APen(shadow);       _Line(x1,y1,x2,y1);
				_APen(halfshadow);   _Line(x1,y1+1,x2-1,y1+1);
				_APen(shine);        _Line(x2,y1+1,x2,y2);
				_APen(halfshine);    _Line(x2-1,y1+2,x2-1,y2);
   		    }
   		    break;

   		    case 52:
   		    {
				_APen(shine);        _Line(x2,y1,x2,y2); _Draw(x1,y2);
				_APen(halfshine);    _Line(x2-1,y1,x2-1,y2-1); _Draw(x1,y2-1);
   		    }
   		    break;

   		    case 53:
   		    {
				_APen(shine);        _Line(x1+1,y2,x2,y2);
				_APen(halfshine);    _Line(x1+2,y2-1,x2,y2-1);
				_APen(shadow);       _Line(x1,y1,x1,y2);
				_APen(halfshadow);   _Line(x1+1,y1,x1+1,y2-1);
   		    }
   		    break;

			/* added 060720 : ? */

			case 54:
			{
				_APen(dark);
				_Move(x1+2, y1);
				_Draw(x2-3, y1);
				_Draw(x2-1, y1+2);
				_Draw(x2-1, y2-3);
				_Draw(x2-3, y2-1);
				_Draw(x1+2, y2-1);
				_Draw(x1, y2-3);
				_Draw(x1, y1+2);
				_Draw(x1+2, y1);

				_APen(shine);
				_Move(x1+1, y2-3);
				_Draw(x1+1, y1+2);
				_Move(x1+2, y1+1);
				_Draw(x2-3, y1+1);

				_APen(fill);
				_Move(x1+2, y2-2);
				_Draw(x2-3, y2-2);
				_Move(x2-2, y2-3);
				_Draw(x2-2, y1+2);

				_APen(shadow);
				_Move(x1+3, y2);
				_Draw(x2-3, y2);
				_Draw(x2, y2-3);
				_Draw(x2, y1+3);

				_APen(halfshadow);
				_Plot(x1+2, y2);
				_Plot(x2, y1+2);
				_Move(x2-2, y2);
				_Draw(x2, y2-2);

				// parent fill

				_APen(fill);
				_Plot(x1, y1);
				_Plot(x1+1,y1);
				_Plot(x1,y1+1);

				_Move(x2,y1);
				_Draw(x2-2,y1);
				_Move(x2,y1+1);
				_Draw(x2-1,y1+1);

				_Move(x1,y2);
				_Draw(x1,y2-2);
				_Move(x1+1,y2);
				_Draw(x1+1,y2-1);

				_Plot(x2,y2);
				_Plot(x2-1,y2);
				_Plot(x2,y2-1);
			}
			break;

			case 56:
			{
				uint32 tmp = shine; shine = dark; dark = tmp;
			}
			// -> 55

			case 55:
			{
				_APen(shine);
				_Move(x1, y2);
				_Draw(x1, y1);
				_Draw(x2, y1);

				_APen(dark);
				_Move(x2,y1+1);
				_Draw(x2,y2);
				_Draw(x1+1,y2);
			}
			break;
		}
//+

		}
		break;
	}

	if (clip != NULL)
	{
		IFEELIN F_Do(_area_render, FM_Render_RemClip, clip);
	}

	if (clear_region != NULL)
	{
		IFEELIN F_Do(Obj, FM_Area_Erase, clear_region, FF_Erase_Region);

		IGRAPHICS DisposeRegion(clear_region);
	}

	if (caption != NULL)
	{
		#ifdef F_NEW_GLOBALCONNECT
		IFEELIN F_Do(_element_parent, FM_Area_Erase, &p, FF_Erase_Fill);
		#else
		IFEELIN F_Do(_area_parent, FM_Area_Erase, &p, FF_Erase_Fill);
		#endif

		IFEELIN F_Do(caption->display, FM_TextDisplay_Draw, &tr);
	}
}
//+
///Border_SetState
F_METHODM(uint32, Border_SetState, FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (Msg->State < FV_AREA_STATE_COUNT)
	{
		LOD->frame = &LOD->properties->frames[Msg->State];

		_area_border.l = LOD->frame->border_left.width;
		_area_border.r = LOD->frame->border_right.width;
		_area_border.t = LOD->frame->border_top.width;
		_area_border.b = LOD->frame->border_bottom.width;

		if (LOD->caption != NULL)
		{
			if ((FF_CAPTION_POSITION_BOTTOM & LOD->caption->position) != 0)
			{
				_area_border.b = MAX(_area_border.b, LOD->caption->height);
			}
			else
			{
				_area_border.t = MAX(_area_border.t, LOD->caption->height);
			}
		}
	}

	return F_SUPERDO();
}
//+

