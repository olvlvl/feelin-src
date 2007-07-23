#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Bar_New
F_METHOD(uint32,Bar_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif
	F_SAVE_AREA_PUBLIC;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_TITLE:
		{
			LOD->Title = (STRPTR) item.ti_Data;
		}
		break;
	}

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Widget_Chainable, FALSE,
		(LOD->Title) ? FA_Widget_SetMax : TAG_IGNORE, FV_Widget_SetHeight,
		
	TAG_MORE, Msg);
}
//+
///Bar_Get
F_METHOD(void,Bar_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;
	
	bool32 up=FALSE;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_TITLE:
		{
			F_STORE(LOD->Title);
		}
		break;

		default: up = TRUE;
	}

	if (up) F_SUPERDO();
}
//+
///Bar_Setup
F_METHOD(bool32,Bar_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	STRPTR data;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}
		
	#ifdef F_NEW_GLOBALCONNECT
	if (IFEELIN F_Get(_element_parent, FA_Area_Orientation) == FV_Area_Orientation_Horizontal)
	#else
	if (IFEELIN F_Get(_area_parent, FA_Area_Orientation) == FV_Area_Orientation_Horizontal)
	#endif
	{
		_bar_set_vertical;
	}
	else
	{
		_bar_clear_vertical;
	}

	// color-shadow

	data = NULL;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(COLOR_SHADOW), &data, NULL);

	if (data)
	{
		LOD->ColorUp = (FColor *) IFEELIN F_Do(_area_display, FM_Display_CreateColor, data, _area_palette);
	}
	
	// color-shine

	data = NULL;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(COLOR_SHINE), &data, NULL);

	if (data)
	{
		LOD->ColorDown = (FColor *) IFEELIN F_Do(_area_display, FM_Display_CreateColor, data, _area_palette);
	}

	// title

	if (LOD->Title)
	{
		LOD->PreParse = NULL;

		IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(PREPARSE), &LOD->PreParse, NULL);

		LOD->TD = TextDisplayObject,

			FA_TextDisplay_Contents,  LOD->Title,
			FA_TextDisplay_PreParse,  LOD->PreParse,
			FA_TextDisplay_Font,      _area_font,

			End;

		if (LOD->TD)
		{
			return IFEELIN F_Do(LOD->TD,FM_TextDisplay_Setup,_area_render);
		}
	}
	return TRUE;
}
//+
///Bar_Cleanup
F_METHOD(uint32,Bar_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render)
	{
		if (LOD->ColorUp)
		{
			IFEELIN F_Do(_area_display,FM_Display_RemColor,LOD->ColorUp);

			LOD->ColorUp = NULL;
		}
	 
		if (LOD->ColorDown)
		{
			IFEELIN F_Do(_area_display,FM_Display_RemColor,LOD->ColorDown);

			LOD->ColorDown = NULL;
		}

		if (LOD->TD)
		{
			IFEELIN F_Do(LOD->TD,FM_TextDisplay_Cleanup);
			IFEELIN F_DisposeObj(LOD->TD);

			LOD->TD = NULL;
		}
	}
	return F_SUPERDO();
}
//+
///Bar_AskMinMax
F_METHOD(uint32,Bar_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->Title)
	{
		if (!_bar_is_vertical)
		{
			_area_minh += IFEELIN F_Get(LOD->TD,FA_TextDisplay_Height);
		}
	}
	else
	{
		if (_bar_is_vertical)
		{
			_area_minw += 2;
			_area_maxw  = _area_minw;
		}
		else
		{
			_area_minh += 2;
			_area_maxh  = _area_minh;
		}
	}
	return F_SUPERDO();
}
//+
///Bar_Draw
F_METHODM(void,Bar_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct RastPort *rp = _area_rp;
	uint32 dark,shine;
	uint16 x1,y1,x2,y2,w,h, bw;
	
	F_SUPERDO();

	x1 = _area_x; w = _area_w; x2 = x1 + w - 1; 
	y1 = _area_y; h = _area_h; y2 = y1 + h - 1; 
							
	dark = (LOD->ColorUp) ? LOD->ColorUp->Pen : _area_pens[FV_Pen_Dark];
	shine = (LOD->ColorDown) ? LOD->ColorDown->Pen : _area_pens[FV_Pen_Shine];
 
	if (_bar_is_vertical)
	{
		if (w > 2) x1 = w / 2 + x1 - 1;

		_APen(dark);  _Move(x1,y1);   _Draw(x1,y2);
		_APen(shine); _Move(x1+1,y1); _Draw(x1+1,y2);
	}
	else
	{
		if (LOD->Title)
		{
			FRect rect;
			uint32 td_w=0,td_h=0;

			rect.x1 = x1+10+1; rect.y1 = y1;
			rect.x2 = x2-10-1; rect.y2 = y2;

			IFEELIN F_Do
			(
				LOD->TD, FM_Set,
				
				FA_TextDisplay_Width, rect.x2 - rect.x1 + 1,
				FA_TextDisplay_Height, rect.y2 - rect.y1 + 1,
				
				TAG_DONE
			);

			IFEELIN F_Do
			(
				LOD->TD, FM_Get,
				
				FA_TextDisplay_Width, &td_w,
				FA_TextDisplay_Height, &td_h,
				
				TAG_DONE
			);

			if (td_w == 0)
			{
				goto __done;
			}

			if (td_h < (y2 - y1 + 1))
			{
				rect.y1 = ((y2 - y1 + 1) - td_h) / 2 + rect.y1;
			}
 
			bw = (w - 10 - td_w) / 2;
			y1 = (h / 2) + y1;

			_APen(shine); _Move(x1,y1);   _Draw(x1+bw-1,y1);   _Move(x2-bw+1,y1);   _Draw(x2,y1);
			_APen(dark);  _Move(x1,y1-1); _Draw(x1+bw-1,y1-1); _Move(x2-bw+1,y1-1); _Draw(x2,y1-1);

			rect.x1 = x1+bw+5; rect.x2 = x2-bw-5;

			IFEELIN F_Do(LOD->TD,FM_TextDisplay_Draw,&rect);
		}
		else
		{
__done:
			if (h > 2) y1 = h / 2 + y1 - 1;

			_APen(dark);  _Move(x1,y1);   _Draw(x2,y1);
			_APen(shine); _Move(x1,y1+1); _Draw(x2,y1+1);
		}
	}
}
//+

STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("bar",    "font",         FV_TYPE_STRING, "Contents", NULL),
	F_PREFERENCES_ADD("bar",    "preparse",     FV_TYPE_STRING, "Contents", NULL),
	F_PREFERENCES_ADD("bar",    "color-shine",  FV_TYPE_STRING, "Spec",     "shine"),
	F_PREFERENCES_ADD("bar",    "color-shadow", FV_TYPE_STRING, "Spec",     "dark"),
 
	F_ARRAY_END
};

F_METHOD(uint32,Prefs_New)
{
	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Bar",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/bar.xml",

	TAG_MORE,Msg);
}

