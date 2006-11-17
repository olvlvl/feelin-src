#include "Private.h"

//#define DB_USE_DRRECT

#ifdef DB_USE_DRRECT
///DrRect
STATIC void DrRect(FRender *Render,FRect *Rect,ULONG Flags)
{
	uint32 Pen = (Rect->x1 + Rect->y1) % 128;
 
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
#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Group_Draw
F_METHODM(uint32, Group_Draw, FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#if 0
	uint32 clip=0;
	#endif

	FWidgetNode *node;

	if (LOD->quiet < 0)
	{
		return TRUE;
	}
 
/*** draw group private ************************************************************************/

	if (_area_is_fillable)
	{
		/* FIXME-060804: this is complex rendering, simple rendering  should
		be reimplemented */

		struct Region *region = IGRAPHICS NewRegion();

		if (region != NULL)
		{
			FRect r;

			_sub_declare_all;

			r.x1 = _area_cx; r.x2 = _area_cx2;
			r.y1 = _area_cy; r.y2 = _area_cy2;

			IGRAPHICS OrRectRegion(region, (struct Rectangle *) &r);

			for (_each)
			{
				if (_sub_is_showable)
				{
					r.x1 = _sub_x + _sub_margin.l;
					r.y1 = _sub_y + _sub_margin.t;
					r.x2 = _sub_x + _sub_w - 1 - _sub_margin.r;
					r.y2 = _sub_y + _sub_h - 1 - _sub_margin.t;

					IGRAPHICS ClearRectRegion(region, (struct Rectangle *) &r);
				}
			}

			if (region->RegionRectangle != NULL)
			{
				IFEELIN F_Do(Obj, FM_Area_Erase, region, FF_Erase_Region);
			}

			IGRAPHICS DisposeRegion(region);
		}

		/* END FIXME */

	    _area_clear_fillable;

		F_SUPERDO();

		_area_set_fillable;
	}
	else
	{
		F_SUPERDO();
	}

	#if 0

/*** clip **************************************************************************************/

	if (LOD->virtual_data)
	{
		FRect r;

		r.x1 = _area_cx;
		r.y1 = _area_cy;
		r.x2 = _area_cx2;
		r.y2 = _area_cy2;

		clip = IFEELIN F_Do(_area_render, FM_Render_AddClip, &r);
	}

	#endif

/*** draw children *****************************************************************************/

	for (_each)
	{
		if (_sub_isnt_group && (_sub_isnt_damaged || _sub_isnt_drawable))
		{
			continue;
		}

		IFEELIN F_Draw(node->Widget, Msg->Flags);
	}

	#if 0

	if (clip != 0)
	{
		IFEELIN F_Do(_area_render, FM_Render_RemClip, clip);
	}

	#endif

	return TRUE;
}
//+

