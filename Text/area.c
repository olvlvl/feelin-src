#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Text_AskMinMax
F_METHOD(uint32,Text_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->text != NULL)
	{
		uint32 tw = 0;
		uint32 th = 0;
		
		IFEELIN F_Do(LOD->TextDisplay, FM_Get,

			FA_TextDisplay_Width,   &tw,
			FA_TextDisplay_Height,  &th,

			TAG_DONE);

		if (_widget_is_setmaxw)
		{
			_area_maxw = _area_minw + tw;
		}

		if (_widget_is_setmaxh)
		{
			_area_maxh = _area_minh + th;
		}

		if (_widget_is_setminw)
		{
			_area_minw += tw;
		}

		if (_widget_is_setminh)
		{
			_area_minh += th;
		}
	}
	else
	{
		_area_minh += _area_font->tf_YSize;
	}

	return F_SUPERDO();
}
//+
///Text_Draw
F_METHODM(void, Text_Draw, FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FRect rect;
	
	uint32 tw, th;

	struct LocalPropertiesData *state = LOD->properties;

	rect.x1 = _area_cx; rect.x2 = rect.x1 + _area_cw - 1;
	rect.y1 = _area_cy; rect.y2 = rect.y1 + _area_ch - 1;

	IFEELIN F_Do(LOD->TextDisplay, FM_Get,

		FA_TextDisplay_Width,   &tw,
		FA_TextDisplay_Height,  &th,

		TAG_DONE);

//	  IFEELIN F_Log(0, "preparse (%s) tw (%ld) th (%ld)", LOD->Prep[0], tw, th);

	if ((_widget_is_setminh && (th > rect.y2 - rect.y1 + 1)) ||
		(_widget_is_setminw && (tw > rect.x2 - rect.x1 + 1)))
	{
		#if 1

		IFEELIN F_Log
		(
			0, "request rethink: w (%ld >? %ld) h (%ld >? %ld)",

			tw,
			rect.x2 - rect.x1 + 1,
			th,
			rect.y2 - rect.y1 + 1
		);

		#endif

		IFEELIN F_Do(_area_win, FM_Window_RequestRethink, Obj);
		
		return;
	}

	F_SUPERDO();

	if (FF_Draw_Update & Msg->Flags)
	{
		IFEELIN F_Do(Obj, FM_Area_Erase, &rect, 0);
	}

	if (state->position_x_type)
	{
		uint16 w = _area_cw;

		if (tw < w)
		{
			rect.x1 = rect.x1 + (w - tw) * state->position_x / 100 - 1;
			rect.x2 = rect.x1 + tw;
		}
	}
	else if ((FF_Text_HCenter & LOD->Flags) != 0)
	{
		uint16 w = _area_cw;

		if (tw < w)
		{
			rect.x1 = rect.x1 + (w - tw) / 2 - 1;
			rect.x2 = rect.x1 + tw;
		}
	}

	if (state->position_y_type)
	{
		uint16 h = _area_ch;

		if (th < h)
		{
			rect.y1 = rect.y1 + (h - th) * state->position_y / 100 - 1;
			rect.y2 = rect.y1 + th;
		}
	}
	else if ((FF_Text_VCenter & LOD->Flags) != 0)
	{
		uint16 h = rect.y2 - rect.y1 + 1;

		if (th < h)
		{
			rect.y1 = rect.y1 + (h - th) / 2 - 1;
			rect.y2 = rect.y1 + th;
		}
	}

	IFEELIN F_Do(LOD->TextDisplay, FM_TextDisplay_Draw, &rect);
}
//+
///Text_SetState
F_METHODM(uint32, Text_SetState, FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	STRPTR preparse = LOD->properties[FV_Area_State_Neutral].preparse;

	switch (Msg->State)
	{
		case FV_Area_State_Touch:
		{
			preparse = LOD->properties[FV_Area_State_Touch].preparse;
		}
		break;

		case FV_Area_State_Focus:
		{
			preparse = LOD->properties[FV_Area_State_Focus].preparse;
		}
		break;

		case FV_Area_State_Ghost:
		{
			preparse = LOD->properties[FV_Area_State_Ghost].preparse;
		}
		break;
	}

	LOD->preparse = preparse;

	IFEELIN F_Do(LOD->TextDisplay, FM_Set,

		FA_TextDisplay_PreParse, LOD->preparse,

		TAG_DONE);

	return F_SUPERDO();
}
//+
