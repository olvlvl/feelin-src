#include "Private.h"

/************************************************************************************************
*** Configuration *******************************************************************************
************************************************************************************************/

//#define F_USE_DRRECT
#define F_USE_REGION
 
/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

#ifdef F_USE_DRRECT

#define F_ERASE_REGION(region)                  DrRect(_area_render, region, FF_Erase_Region)
#define F_ERASE_RECT(rect)                      DrRect(_area_render, rect, 0)

///DrRect
STATIC void DrRect(FRender *Render,FRect *Rect,ULONG Flags)
{
	ULONG Pen = (Rect->x1 + Rect->y1) % 128;

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
		}
	}
	else
	{
		IGRAPHICS SetAPen(Render->RPort,Pen);
		IGRAPHICS RectFill(Render->RPort,Rect->x1,Rect->y1,Rect->x2,Rect->y2);
	}
}
//+

#else

#define F_ERASE_REGION(region)                  IFEELIN F_Do(Obj, FM_Area_Erase, region, FF_Erase_Region)
#define F_ERASE_RECT(rect)                      IFEELIN F_Do(Obj, FM_Area_Erase, rect, 0)

#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Page_Draw
F_METHODM(void,Page_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT
	FObject parent = _element_parent;
	#else
	FObject parent = _area_parent;
	#endif
	FWidgetNode *node = LOD->node_active;

	if (_area_is_damaged)
	{
		struct RastPort *rp = _area_rp;
		struct Region *parent_erase_region = IGRAPHICS NewRegion();
		
		uint32 *pens;
		
		int16 x1 = _area_x;
		int16 x2 = _area_x2;
		int16 y1 = _area_y;
		int16 y2 = _area_y2;
		int16 y;

		FWidgetNode *node;
		
		/* Pass FM_Area_Draw to FC_Area */
 
		IFEELIN F_SuperDo(Class->Super, Obj, Method, Msg);

		pens = _area_pens;

		y = y1 + LOD->tabs_height - 1;

		/* left, right and bottom sides the object */

		_APen(pens[FV_Pen_Dark]);     _Move(x1,y+3);    _Draw(x1,y2);     _Draw(x2,y2); _Draw(x2,y+3);
		_APen(pens[FV_Pen_Shine]);    _Move(x1+1,y+3);  _Draw(x1+1,y2-2);
		_APen(pens[FV_Pen_Shadow]);   _Move(x1+2,y2-1); _Draw(x2-1,y2-1); _Draw(x2-1,y+3);
		_APen(pens[FV_Pen_Fill]);     _Plot(x1+1,y2-1);

		#if 0
 
		_APen(_area_x % 255);
		_Boxf(_area_x, _area_y, _area_x2, y - 1);
		
		#endif

		/* erase the page contents. if render mode is complex, we only erase the
		borders of the contents */

		if ((FF_Render_Complex & _area_render->Flags) != 0)
		{
			struct Region *region;

			if ((region = IGRAPHICS NewRegion()) != NULL)
			{
				FRect r;

				node = LOD->node_active;

				r.x1 = x1 + FV_PAGE_BORDER_LEFT;
				r.x2 = x2 - FV_PAGE_BORDER_RIGHT;
				r.y1 = y + 1 + FV_PAGE_BORDER_TOP;
				r.y2 = y2 - FV_PAGE_BORDER_BOTTOM;
				
				IGRAPHICS OrRectRegion(region,(struct Rectangle *) &r);

				r.x1 = _sub_x; r.x2 = r.x1 + _sub_w - 1;
				r.y1 = _sub_y; r.y2 = r.y1 + _sub_h - 1;
				
				IGRAPHICS ClearRectRegion(region,(struct Rectangle *) &r);

				F_ERASE_REGION(region);

				IGRAPHICS DisposeRegion(region);
			}
		}
		else
		{
			FRect r;

			r.x1 = _area_x + 2; r.x2 = x2 - 2;
			r.y1 = y + 3;  r.y2 = y2 - 2;
				
			F_ERASE_RECT(&r);
		}

		{
			int16 x = x1;
			uint16 w;
			uint32 i = 0;
			FRect td_rect;
			
			y2 = y + FV_PAGE_BORDER_TOP;

			/* count the number of tabs */

			for (_each)
			{
				i++;
			}

			/* compute general width */

			for (_each)
			{
				w = (_area_x2 - x + 1) / i-- - 1;
				
				x1 = x + 1;
				x2 = x + w - 1;
				
				#if 0
				
				_APen(x % 255);
				_Boxf(x, y1, x2, y2);
				
				#else

				/* left line/separator */
				
				if (node == LOD->node_active || node->Prev == LOD->node_active)
				{
					_APen(pens[FV_Pen_Dark]);
					_Move(x, y1 + 1);
					_Draw(x, y2 - 1);
					
					if (node->Prev)
					{
						_APen(pens[FV_Pen_Shine]);
					}
					
					_Plot(x, y2);
											
					if (parent_erase_region)
					{
						FRect r;
						
						r.x1 = x;
						r.y1 = y1;
						r.x2 = x;
						r.y2 = y1;
 
						IGRAPHICS OrRectRegion(parent_erase_region, (struct Rectangle *) &r);
					}
					else
					{
						IFEELIN F_Erase(parent, x, y1, x, y1, 0);
					}
				}
				else
				{
					_APen(pens[FV_Pen_Shadow]);
					_Move(x, y1 + LOD->style->inactive_margin_top + 1);
					_Draw(x, y2 - 2);
						
					_APen(pens[FV_Pen_Dark]);
					_Plot(x, y2 - 1);
 
					if (node->Prev)
					{
						_APen(pens[FV_Pen_Shine]);
					}
					
					_Plot(x, y2);
					
					if (parent_erase_region)
					{
						FRect r;
						
						r.x1 = x;
						r.y1 = y1;
						r.x2 = x;
						r.y2 = y1 + LOD->style->inactive_margin_top;
						
						IGRAPHICS OrRectRegion(parent_erase_region, (struct Rectangle *) &r);
					}
					else
					{
						IFEELIN F_Erase(parent, x, y1, x, y1 + LOD->style->inactive_margin_top, 0);
					}
				}
 
				if (((FF_Draw_Update & Msg->Flags) && (node == LOD->node_update || node == LOD->node_active)) ||
					(FF_Draw_Object & Msg->Flags))
				{
					uint32 tw;
 
					if (node == LOD->node_active)
					{
						FRect r;
						
						_APen(pens[FV_Pen_Dark]);
						_Move(x1, y1);
						_Draw(x2, y1);
						
						_APen(pens[FV_Pen_Shine]);
						_Move(x1, y2);
						_Draw(x1, y1 + 1);
						_Draw(x2 - 1, y1 + 1);
						
						_APen(pens[FV_Pen_Shadow]);
						_Move(x2, y1 + 2);
						_Draw(x2, y2 - 1);
						
						_APen(pens[FV_Pen_Fill]);
						_Plot(x2,y1+1);
						_Plot(x2,y2);
						
						r.x1 = x1 + 1;
						r.y1 = y1 + 2;
						r.x2 = x2 - 1;
						r.y2 = y2;
						
						if (LOD->back_active)
						{
							IFEELIN F_Do(LOD->back_active, FM_ImageDisplay_Draw, _area_render, &r, 0);
						}
						else
						{
							F_ERASE_RECT(&r);
						}
						
						td_rect.y1 = y1 + FV_TAB_BORDER_TOP + LOD->style->active_padding_top;
					}
					else
					{
						FRect r;
						
						_APen(pens[FV_Pen_Dark]);
						_Move(x1, y2 - 1);
						_Draw(x2, y2 - 1);
						
						_APen(pens[FV_Pen_Shine]);
						_Move(x1, y2);
						_Draw(x2, y2);
						
						_APen(pens[FV_Pen_Shadow]);
						_Move(x1, y1 + LOD->style->inactive_margin_top);
						_Draw(x2, y1 + LOD->style->inactive_margin_top);
						
						if (parent_erase_region)
						{
							r.x1 = x1;
							r.y1 = y1;
							r.x2 = x2;
							r.y2 = y1 + LOD->style->inactive_margin_top - 1;
							
							IGRAPHICS OrRectRegion(parent_erase_region, (struct Rectangle *) &r);
						}
						else
						{
							IFEELIN F_Erase(parent, x1, y1, x2, y1 + LOD->style->inactive_margin_top - 1, 0);
						}
						
						r.x1 = x1;
						r.y1 = y1 + LOD->style->inactive_margin_top + 1;
						r.x2 = x2;
						r.y2 = y2 - 2;
						
						if (LOD->back_inactive)
						{
							IFEELIN F_Do(LOD->back_inactive, FM_ImageDisplay_Draw, _area_render, &r, 0);
						}
						else
						{
							F_ERASE_RECT(&r);
						}
 
						td_rect.y1 = y1 + LOD->style->inactive_margin_top + FV_ITAB_BORDER_TOP + LOD->style->inactive_padding_top;
					}

					td_rect.x1 = x + FV_TAB_PADDING_WIDTH / 2;
					td_rect.x2 = x + w - 1 - FV_TAB_PADDING_WIDTH / 2;
					td_rect.y2 = td_rect.y1 + FV_FONT_SIZE - 1;

					IFEELIN F_Do
					(
						LOD->TD, FM_Set,

						FA_TextDisplay_Width,    td_rect.x2 - td_rect.x1 + 1,
						FA_TextDisplay_PreParse, (node == LOD->node_active) ? LOD->style->active_tab_prep : LOD->style->inactive_tab_prep,
						FA_TextDisplay_Contents, IFEELIN F_Get(node->Widget, FA_Group_Name),

						TAG_DONE
					);
					
					tw = IFEELIN F_Get(LOD->TD, FA_TextDisplay_Width);
																			
					if (tw < (td_rect.x2 - td_rect.x1 + 1))
					{
						td_rect.x1 = (td_rect.x2 - td_rect.x1 + 1 - tw) / 2 + td_rect.x1;
						td_rect.x2 = td_rect.x1 + tw;
					}

//                    F_ERASE_RECT(&td_rect);
 
					IFEELIN F_Do(LOD->TD, FM_TextDisplay_Draw, &td_rect);
				}
				
				#endif
 
				x += w;
			}
			
			#if 1
 
			if (LOD->node_active == (FWidgetNode *) IFEELIN F_Get(Obj, FA_Family_Tail))
			{
				_APen(pens[FV_Pen_Dark]);
				_Move(x, y1 + 1);
				_Draw(x, y2);
				
				_APen(pens[FV_Pen_Shadow]);
				_Plot(x - 1, y2);
					
				if (parent_erase_region)
				{
					FRect r;

					r.x1 = x;
					r.y1 = y1;
					r.x2 = x;
					r.y2 = y1;

					IGRAPHICS OrRectRegion(parent_erase_region, (struct Rectangle *) &r);
				}
				else
				{
					IFEELIN F_Erase(parent, x, y1, x, y1, 0);
				}
			}
			else
			{
				_APen(pens[FV_Pen_Shadow]);
				_Move(x, y1 + LOD->style->inactive_margin_top + 1);
				_Draw(x, y2 - 2);
				
				_APen(pens[FV_Pen_Dark]);
				_Plot(x, y2 - 1);
				_Plot(x, y2);
				
				_APen(pens[FV_Pen_Fill]);
				_Plot(x - 1, y2);
				
				if (parent_erase_region)
				{
					FRect r;

					r.x1 = x;
					r.y1 = y1;
					r.x2 = x;
					r.y2 = y1 + LOD->style->inactive_margin_top;

					IGRAPHICS OrRectRegion(parent_erase_region, (struct Rectangle *) &r);
				}
				else
				{
					IFEELIN F_Erase(parent, x, y1, x, y1 + LOD->style->inactive_margin_top, 0);
				}
			}
			
			#endif
 
			if (parent_erase_region)
			{
				IFEELIN F_Do(parent, FM_Area_Erase, parent_erase_region, FF_Erase_Region);

				IGRAPHICS DisposeRegion(parent_erase_region);
			}
		}
	}

	if (node != NULL)
	{
		if ((FF_Draw_Damaged & Msg->Flags) != 0)
		{
			if (!_sub_is_group == 0)
			{
				if (!_sub_is_damaged)
				{
					return;
				}
			}

			IFEELIN F_Draw(node->Widget, Msg->Flags);
		}
		else
		{
			IFEELIN F_Draw(node->Widget, FF_Draw_Object);
		}
	}
}
//+
