#include "Private.h"

//#define DB_ASKMINMAX
//#define DB_SCHEME
//#define DB_SETSTATE

///area_set_state
bool32 area_set_state(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FPalette *palette;

	/*FIXME-060811: Should also select the 'font' property */

	switch (_area_state)
	{
		case FV_Area_State_Neutral:
		{
			palette = LOD->palette_neutral;
			LOD->state = &LOD->properties[FV_Area_State_Neutral];
		}
		break;

		case FV_Area_State_Touch:
		{
			palette = LOD->palette_touch;
			LOD->state = &LOD->properties[FV_Area_State_Touch];
		}
		break;

		case FV_Area_State_Ghost:
		{
			palette = LOD->palette_ghost;
			LOD->state = &LOD->properties[FV_Area_State_Ghost];
		}
		break;

		case FV_Area_State_Focus:
		{
			palette = LOD->palette_focus;
		}
		break;

		default:
		{
			IFEELIN F_Log(FV_LOG_DEV, "(%ld) is an invalid state", _area_state);

			return FALSE;
		}
	}

	#ifdef DB_SETSTATE
	IFEELIN F_Log(0, "state (%ld) palette (0x%08lx) background (0x%08lx)", Msg->State, palette, background);
	#endif

	/*FIXME-060811: I should check if the object is  really  damaged  before
	setting the flag */

	_area_flags |= FF_Area_Damaged;

	LOD->state = &LOD->properties[_area_state];

	_area_palette = palette;

	if (_area_render != NULL)
	{
		_area_render->Palette = palette;
	}

	if (LOD->state->background != NULL)
	{
		_area_background = LOD->background;

//		  IFEELIN F_Log(0, "back: (0x%08lx) (%s)", _area_background, LOD->state->background);

		IFEELIN F_Set(LOD->background, FA_ImageDisplay_Spec, (uint32) LOD->state->background);
	}
	else
	{
		#ifdef F_NEW_GLOBALCONNECT
		FAreaPublic *parent_pub = F_GET_AREA_PUBLIC(_element_parent);
		#else
		FAreaPublic *parent_pub = F_GET_AREA_PUBLIC(_area_parent);
		#endif

		_area_background = parent_pub ? parent_pub->Background : LOD->background_inherited;

//		  IFEELIN F_Log(0, "inherited back: (0x%08lx) ?= (0x%08lx)", _area_background, parent_pub ? parent_pub->Background : NULL);
	}

	_area_margin.l = LOD->state->margin.l;
	_area_margin.r = LOD->state->margin.r;
	_area_margin.t = LOD->state->margin.t;
	_area_margin.b = LOD->state->margin.b;

	_area_padding.l = LOD->state->padding.l;
	_area_padding.r = LOD->state->padding.r;
	_area_padding.t = LOD->state->padding.t;
	_area_padding.b = LOD->state->padding.b;

	return TRUE;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Area_Show
F_METHOD(bool32, Area_Show)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FRender * render = _area_render;

	if (!render)
	{
		IFEELIN F_Log(FV_LOG_DEV, "The object has not been setuped yet, Render is NULL !");

		return FALSE;
	}

	if (!render->RPort)
	{
		IFEELIN F_Log(FV_LOG_DEV, "RPort of Render (0x%08lx) is NULL !", render);

		return FALSE;
	}

	_area_set_drawable;
	_area_set_damaged;

	return TRUE;
}
//+
///Area_Hide
F_METHOD(bool32, Area_Hide)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
		
	_area_clear_drawable;
	_area_set_damaged;

	if (_area_render != NULL)
	{
		if (Obj == (FObject) IFEELIN F_Get(_area_win, FA_Window_ActiveObject))
		{
		   IFEELIN F_Set(_area_win, FA_Window_ActiveObject, NULL);
		}

		return TRUE;
	}

	return FALSE;
}
//+
///Area_AskMinMax
F_METHOD(bool32,Area_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct LocalPropertiesData *properties = &LOD->properties[_area_state];

	uint32 add_w = _area_margin.l + _area_border.l + _area_padding.l + _area_margin.r + _area_border.r + _area_padding.r;
	uint32 add_h = _area_margin.t + _area_border.t + _area_padding.t + _area_margin.b + _area_border.b + _area_padding.b;

	uint16 minw = _area_minw;
	uint16 maxw = _area_maxw;
	uint16 minh = _area_minh;
	uint16 maxh = _area_maxh;

	bits32 flags = properties->flags;

	/* if the style property 'width' is defined, its value is used for  both
	_area_minw and _area_maxw, the object has then a fixed width. */

	if ((flags & FF_AREA_DEFINED_W) != 0)
	{
		minw = maxw = properties->width;
	}
	else
	{
		if ((FF_AREA_DEFINED_MIN_W & flags) != 0)
		{
			minw = MAX(minw, properties->minmax.MinW);
		}

		if ((FF_AREA_DEFINED_MAX_W & flags) != 0)
		{
			maxw = MIN(maxw, properties->minmax.MaxW);
		}
	}

	/* if the style property 'height' is defined, its value is used for both
	_area_minh and _area_maxh, the object has then a fixed height. */

	if ((flags & FF_AREA_DEFINED_H) != 0)
	{
		minh = maxh = properties->height;
	}
	else
	{
		if ((FF_AREA_DEFINED_MIN_H & flags) != 0)
		{
			minh = MAX(minh, properties->minmax.MinH);
		}

		if ((FF_AREA_DEFINED_MAX_H & flags) != 0)
		{
			maxh = MIN(maxh, properties->minmax.MaxH);
		}
	}

	/* now we add the minimum and maximum dimension of the content box */

	minw += add_w;
	maxw += add_w;
	minh += add_h;
	maxh += add_h;

	/* and we limit dimensions */

	maxw = MAX(minw, maxw);
	maxh = MAX(minh, maxh);
	maxw = MIN(maxw, FV_Area_Max);
	maxh = MIN(maxh, FV_Area_Max);

	/*FIXME-060806: Should 'auto' be disabled if  a  maximum  dimension  has
	already been set ? It would seams to be for the best... */

	//if ((FF_AREA_AUTO_W & flags) && (maxw == FV_Area_Max))
	if ((FF_AREA_AUTO_W & flags) != 0)
	{
		maxw = minw;
	}

	//if ((FF_AREA_AUTO_H & flags) && (maxh == FV_Area_Max))
	if ((FF_AREA_AUTO_H & flags) != 0)
	{
		maxh = minh;
	}

	_area_minw = minw;
	_area_minh = minh;
	_area_maxw = maxw;
	_area_maxh = maxh;

	#ifdef DB_ASKMINMAX
	IFEELIN F_Log(0, "min (%ld x %ld) max (%ld x %ld)", _area_minw, _area_minh, _area_maxw, _area_maxh);
	#endif

	return TRUE;
}
//+
///Area_Layout
F_METHOD(bool32,Area_Layout)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	uint32 add_x = _area_margin.l + _area_border.l + _area_padding.l;
	uint32 add_y = _area_margin.t + _area_border.t + _area_padding.t;

	_area_cx = _area_x + add_x;
	_area_cy = _area_y + add_y;
	_area_cw = _area_w - add_x - _area_margin.r - _area_border.r - _area_padding.l;
	_area_ch = _area_h - add_y - _area_margin.b - _area_border.b - _area_padding.b;

//	  IFEELIN F_Log(0, "box (%ld : %ld) (%ld x %ld) content (%ld : %ld) (%ld x %ld)", _area_x, _area_y, _area_w, _area_h, _area_cx, _area_cy, _area_cw, _area_ch);

	return TRUE;
}
//+
///Area_Draw
F_METHODM(bool32,Area_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_area_render->Palette = _area_palette;

	if (_area_font != NULL)
	{
		IGRAPHICS SetFont(_area_rp, _area_font);
	}

	#if 0

	/* the following code can be used to verbose atempts to  draw  undamaged
	objects */

	if (_area_isnt_damaged)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Area isn't damaged, please check your code !");

		return FALSE;
	}

	#endif

	_area_flags &= ~FF_Area_Damaged;

/*

	FIXME-060802: Simple refreshing mode is disabled for now.  But  remember
	to support it later.

	if (FF_Draw_Object & Msg->Flags)
	{
		bool32 fill = TRUE;

		if (FF_AREA_NOFILL & LOD->flags)
		{
			fill = FALSE;
		}
		else if ((FF_Render_Refreshing & _area_render->Flags) &&
					(FF_AREA_INHERITED_BACK & LOD->flags) &&
					(FF_AREA_INHERITED_PALETTE & LOD->flags) &&
				  !(FF_Render_Complex & _area_render->Flags))
		{
			fill = FALSE;
		}
	}
*/

/*** inherit values ***

	FIXME-060816: I don't like it here, but  I'm  having  difficulties  with
	background inheritence...

*/

	if (LOD->state->background == NULL)
	{
		_area_background = LOD->parent_lod ? LOD->parent_lod->public.Background : LOD->background_inherited;

//		  IFEELIN F_Log(0, "inherited back: (0x%08lx) ?= (0x%08lx)", _area_background, parent_pub ? parent_pub->Background : NULL);
	}

/*** fill the content box ***********************************************************************

	Following are the erasing selectors. Note that margin erasing is handled
	by the parent.

*/

	if ((FF_Draw_Object & Msg->Flags) != 0)
	{
		FRect r;

		r.x1 = _area_x + _area_margin.l + _area_border.l;
		r.x2 = _area_x + _area_w - 1 - _area_margin.r - _area_border.r;
		r.y1 = _area_y + _area_margin.t + _area_border.t;
		r.y2 = _area_y + _area_h - 1 - _area_margin.b - _area_border.b;

		/* if we  handle  the  complete  filling  of  the  object,  we  fill
		everything, including the content box */

		if (_area_is_fillable)
		{
			IFEELIN F_Do(_area_background, FM_ImageDisplay_Draw, _area_render, &r, 0);
		}

		/* if a subclass fills its content box itself, we still have to fill
		its padding space. */

		else if (_area_padding.l || _area_padding.r || _area_padding.t || _area_padding.b)
		{
			struct Region *region = IGRAPHICS NewRegion();

			if (region != NULL)
			{
				IGRAPHICS OrRectRegion(region, (struct Rectangle *) &r);

				r.x1 = _area_cx;
				r.y1 = _area_cy;
				r.x2 = _area_cx2;
				r.y2 = _area_cy2;

				IGRAPHICS ClearRectRegion(region, (struct Rectangle *) &r);

				IFEELIN F_Do(_area_background, FM_ImageDisplay_Draw, _area_render, region, FF_ImageDisplay_Draw_Region);

			 	IGRAPHICS DisposeRegion(region);
			}
		}
	}

	return TRUE;
}
//+
///Area_Erase
F_METHODM(bool32,Area_Erase,FS_Area_Erase)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_isnt_drawable)
	{
		return FALSE;
	}

/*

	FIXME-060802: Simple refreshing mode is disabled for now.  But  remember
	to support it later.

	if ((Msg->Flags & FF_Erase_Fill) == 0)
	{
		if ((FF_Render_Refreshing & _area_render->Flags) &&
		  !(FF_Render_Complex & _area_render->Flags))
		{
			if (_area_background == LOD->background_inherited)
			{
				return TRUE;
			}
		}
	}

*/

	if ((FF_Erase_Box & Msg->Flags) != 0)
	{
		FRect r;

		r.x1 = ((FBox *)(Msg->Rect))->x;
		r.y1 = ((FBox *)(Msg->Rect))->y;
		r.x2 = ((FBox *)(Msg->Rect))->x + ((FBox *)(Msg->Rect))->w - 1;
		r.y2 = ((FBox *)(Msg->Rect))->y + ((FBox *)(Msg->Rect))->h - 1;

		IFEELIN F_Do(_area_background, FM_ImageDisplay_Draw, _area_render, &r, 0);
	}
	else
	{
		IFEELIN F_Do(_area_background, FM_ImageDisplay_Draw, _area_render, Msg->Rect,
			(FF_Erase_Region & Msg->Flags) ? FF_ImageDisplay_Draw_Region : 0);
	}

	return TRUE;
}
//+
///Area_SetState
F_METHODM(bool32, Area_SetState, FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (Msg->State < FV_AREA_STATE_COUNT)
	{
		_area_state = Msg->State;

		return area_set_state(Class, Obj);
	}

	return FALSE;
}
//+
///Area_Move
F_METHODM(void,Area_Move,FS_Area_Move)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	#ifdef DB_MOVE
	int16 x = _area_x, y = _area_y;
	#endif
 
	if (FF_Move_Relative & Msg->Flags)
	{
		if (Msg->x || Msg->y)
		{
			_area_x += Msg->x;
			_area_y += Msg->y;
		
			_area_set_damaged;
		}
	}
	else
	{
		if ((_area_x != Msg->x) || (_area_y != Msg->y))
		{
			_area_x = Msg->x;
			_area_y = Msg->y;
			
			_area_set_damaged;
		}
	}

	#ifdef DB_MOVE
	IFEELIN F_Log(0,"x (%ld->%ld) y (%ld->%ld)",x,_area_x,y,_area_y);
	#endif
}
//+

