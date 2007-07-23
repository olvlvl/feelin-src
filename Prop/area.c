#include "Private.h"

///prop_layout_knob
void prop_layout_knob(FClass *Class, FObject Obj, int16 x, int16 y, uint16 w, uint16 h)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (_knob_area_public)
	{
		if (_knob_x != x)
		{
			_knob_x = x;
			_knob_set_damaged;
		}

		if (_knob_y != y)
		{
			_knob_y = y;
			_knob_set_damaged;
		}

		if (_knob_w != w)
		{
			_knob_w = w;
			_knob_set_damaged;
		}

		if (_knob_h != h)
		{
			_knob_h = h;
			_knob_set_damaged;
		}

		IFEELIN F_Do(LOD->knob, FM_Area_Layout);
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Prop_Show
F_METHOD(uint32,Prop_Show)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	return F_OBJDO(LOD->knob);
}
//+
///Prop_Hide
F_METHOD(void,Prop_Hide)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_OBJDO(LOD->knob);
	F_SUPERDO();
}
//+
///Prop_SetState
F_METHODM(bool32,Prop_SetState,FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	if (Msg->State == FV_Area_State_Focus)
	{
		IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_KEY, 0);

		IFEELIN F_Set(LOD->knob, FA_Widget_Active, TRUE);
	}
	else
	{
		IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_KEY);

		IFEELIN F_Set(LOD->knob, FA_Widget_Active, FALSE);
	}

	//return F_OBJDO(LOD->knob);

	return TRUE;
}
//+
///Prop_AskMinMax
F_METHOD(uint32,Prop_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_knob_area_public)
	{
		_knob_minw = _knob_minh = 0;
		_knob_maxw = _knob_maxh = FV_Area_Max;

		F_OBJDO(LOD->knob);
	}

	#ifdef DB_ASKMINMAX
	IFEELIN F_Log(0, "knob min (%ld x %ld) max (%ld x %ld)", _knob_minw, _knob_minh, _knob_maxw, _knob_maxh);
	#endif

	if (_area_is_horizontal)
	{
		_area_minw += 10 + _knob_minw;
		_area_minh +=  4 + _knob_minh;
	}
	else
	{
		_area_minw +=  4 + _knob_minw;
		_area_minh += 10 + _knob_minh;
	}

	return F_SUPERDO();
}
//+
///Prop_Layout
F_METHOD(bool32,Prop_Layout)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	int16 x, x2, y, y2;
	uint16 w, h;
	uint32 s,p;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	x = _area_cx;
	y = _area_cy;
	x2 = _area_cx2;
	y2 = _area_cy2;

	if (_area_is_horizontal)
	{
		w = x2 - x + 1;

		LOD->previous_pos = _knob_x;
	}
	else
	{
		w = y2 - y + 1;

		LOD->previous_pos = _knob_y;
	}

	if ((LOD->entries == 0) || (LOD->entries - LOD->visible == 0))
	{
		s = w;
		p = 0;
	}
	else
	{
		s = LOD->visible * w / LOD->entries;

		if (_area_is_horizontal)
		{
			s = MAX(s, _knob_minw);
		}
		else
		{
			s = MAX(s, _knob_minh);
		}

		p = (w - s) * LOD->first / (LOD->entries - LOD->visible);
	}

	if (_area_is_horizontal)
	{
		x += p;
		w = s;
		h = y2 - y + 1;
	}
	else
	{
		y += p;
		w = x2 - x + 1;
		h = s;
	}

	prop_layout_knob(Class, Obj, x, y, w, h);

	return TRUE;
}
//+
///Prop_Draw
F_METHODM(void,Prop_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	int16 x1 = _area_cx, x2 = x1 + _area_cw - 1;
	int16 y1 = _area_cy, y2 = y1 + _area_ch - 1;
	int16 ox1,oy1,ox2,oy2;

	F_SUPERDO();

	if (!(FF_Prop_Scroll & LOD->flags))
	{
		IFEELIN F_ClassDo(Class, Obj, FM_Area_Layout);
	}

	ox1 = _knob_x, ox2 = ox1 + _knob_w - 1,
	oy1 = _knob_y, oy2 = oy1 + _knob_h - 1;

	if (FF_Draw_Update & Msg->Flags)
	{
		if (FF_Prop_Scroll & LOD->flags)
		{
			if (_area_is_horizontal)
			{
				IGRAPHICS ClipBlit(_area_rp,LOD->previous_pos,oy1,_area_rp,ox1,oy1,ox2 - ox1 + 1,oy2 - oy1 + 1,0x0C0);
			}
			else
			{
				IGRAPHICS ClipBlit(_area_rp,ox1,LOD->previous_pos,_area_rp,ox1,oy1,ox2 - ox1 + 1,oy2 - oy1 + 1,0x0C0);
			}
		}
	}

	if ((FF_Draw_Object & Msg->Flags) ||
	   ((FF_Draw_Update & Msg->Flags) && !(FF_Prop_Scroll & LOD->flags)))
	{
		IFEELIN F_Draw(LOD->knob, FF_Draw_Object);
	}

	if (_area_is_horizontal)
	{
		if ((FF_Draw_Object & Msg->Flags) || (LOD->previous_pos < ox1)) IFEELIN F_Erase(Obj,x1,y1,ox1-1,y2,0);
		if ((FF_Draw_Object & Msg->Flags) || (LOD->previous_pos > ox1)) IFEELIN F_Erase(Obj,ox2+1,y1,x2,y2,0);
	}
	else
	{
		if ((FF_Draw_Object & Msg->Flags) || (LOD->previous_pos < oy1)) IFEELIN F_Erase(Obj,x1,y1,x2,oy1-1,0);
		if ((FF_Draw_Object & Msg->Flags) || (LOD->previous_pos > oy1)) IFEELIN F_Erase(Obj,x1,oy2+1,x2,y2,0);
	}
}
//+
