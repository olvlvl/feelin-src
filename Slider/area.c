#include "Private.h"

//#define DB_ASKMINMAX

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///Slider_DrawKnob
STATIC void Slider_DrawKnob(FClass *Class,FObject Obj,bits32 Flags)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_Draw(LOD->knob,Flags);

	if ((LOD->own_bitmap != NULL) && _area_is_drawable)
	{
		IGRAPHICS BltBitMapRastPort(LOD->own_bitmap,0,0,_area_rp,LOD->kx,LOD->ky,LOD->kw,LOD->kh,0x0C0);
	}
}
//+
#if 0
///Slider_KnobState
void Slider_KnobState(FClass *Class,FObject Obj,uint32 State)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_Set(LOD->knob, FA_Widget_Selected, State);

	if ((LOD->own_bitmap != NULL) && _area_is_drawable)
	{
		IGRAPHICS BltBitMapRastPort(LOD->own_bitmap,0,0,_area_rp,LOD->kx,LOD->ky,LOD->kw,LOD->kh,0x0C0);
	}
}
//+
#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Slider_AskMinMax
F_METHOD(bool32,Slider_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint16 minw=0;
	uint16 minh=0;
	uint32 num_min, num_max, num_val, num_array;

	IFEELIN F_Set(_area_render,FA_Render_Forbid,TRUE);
	IFEELIN F_Set(LOD->own_render,FA_Render_Forbid,TRUE);

	IFEELIN F_Do(Obj,FM_Get,

		F_RESOLVED_ID(VALUE), &num_val,
		F_RESOLVED_ID(MIN), &num_min,
		F_RESOLVED_ID(MAX), &num_max,
		F_RESOLVED_ID(STRINGARRAY), &num_array,

		TAG_DONE);

	if (num_array != NULL)
	{
		for ( ; num_min < num_max + 1; num_min++)
		{
			IFEELIN F_Do(LOD->knob, FM_Set,

				FA_Text_Contents, IFEELIN F_Do(Obj, F_RESOLVED_ID(STRINGIFY), num_min),
				FA_NoNotify, TRUE,

				TAG_DONE);

			_knob_minw = _knob_minh = 0;
			_knob_maxw = _knob_maxh = FV_Area_Max;

			IFEELIN F_Do(LOD->knob, FM_Area_AskMinMax);

			minw = MAX(minw, _knob_minw);
			minh = MAX(minh, _knob_minh);
		}
	}
	else
	{
		IFEELIN F_Do(LOD->knob, FM_Set,

			FA_Text_Contents,     IFEELIN F_Do(Obj, F_RESOLVED_ID(STRINGIFY), num_min),
			FA_NoNotify, TRUE,

			TAG_DONE);

		_knob_minw = _knob_minh = 0;
		_knob_maxw = _knob_maxh = FV_Area_Max;

		IFEELIN F_Do(LOD->knob, FM_Area_AskMinMax);

		minw = MAX(minw, _knob_minw);
		minh = MAX(minh, _knob_minh);

		#ifdef DB_ASKMINMAX
		IFEELIN F_Log(0,"Min (%16.16s) >> min (%ld x %ld)", F_Get(LOD->knob,FA_Text_Contents), minw, minh);
		#endif

		IFEELIN F_Do(LOD->knob, FM_Set,

			FA_Text_Contents,     IFEELIN F_Do(Obj, F_RESOLVED_ID(STRINGIFY), num_max),
			FA_NoNotify, TRUE,

			TAG_DONE);

		_knob_minw = _knob_minh = 0;
		_knob_maxw = _knob_maxh = FV_Area_Max;

		IFEELIN F_Do(LOD->knob, FM_Area_AskMinMax);

		minw = MAX(minw, _knob_minw);
		minh = MAX(minh, _knob_minh);

		#ifdef DB_ASKMINMAX
		IFEELIN F_Log(0,"Max (%16.16s) >> min (%ld x %ld)", F_Get(LOD->knob,FA_Text_Contents), minw, minh);
		#endif
	}

	_knob_minw = _knob_maxw = minw;
	_knob_minh = _knob_maxh = minh;

	if (_area_is_horizontal)
	{
		_area_minw += minw * 3;
		_area_minh += minh;
	}
	else
	{
		_area_minw += minw;
		_area_minh += minh * 3;
	}

/*** set val again *****************************************************************************/

	IFEELIN F_Do(LOD->knob, FM_Set,

		FA_Text_Contents, IFEELIN F_Do(Obj, F_RESOLVED_ID(STRINGIFY), num_val),
		FA_NoNotify, TRUE,

		TAG_DONE);

	#ifdef DB_ASKMINMAX
	IFEELIN F_Log(0, "knob min (%ld x %ld) slider min (%ld x %ld)",_knob_minw, _knob_minh, _area_minw, _area_minh);
	#endif

	IFEELIN F_Set(LOD->own_render,FA_Render_Forbid,FALSE);
	IFEELIN F_Set(_area_render,FA_Render_Forbid,FALSE);

	return F_SUPERDO();
}
//+
///Slider_Show
F_METHOD(bool32,Slider_Show)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	slider_cache_update(Class,Obj);

	F_OBJDO(LOD->knob);

	return TRUE;
}
//+
///Slider_Hide
F_METHOD(uint32,Slider_Hide)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_OBJDO(LOD->knob);
	
	return F_SUPERDO();
}
//+
///Slider_Draw
F_METHODM(void, Slider_Draw, FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 val = IFEELIN F_Get(Obj, F_RESOLVED_ID(VALUE));
	
	uint32 x1 = _area_cx,y1 = _area_cy,x2 = _area_cx2,y2 = _area_cy2, ox1,oy1,ox2,oy2;

	if (LOD->last_drawn_value != val)
	{
		LOD->last_drawn_value = val;

		IFEELIN F_Set(LOD->knob,FA_Text_Contents,IFEELIN F_Get(Obj, F_RESOLVED_ID(BUFFER)));
	}

	F_SUPERDO();

	if (((FF_Draw_Object & Msg->Flags) && LOD->last_layout_value != val) ||
		 !(FF_Slider_Scroll & LOD->flags))
	{
		IFEELIN F_ClassDo(Class, Obj, FM_Area_Layout);
	}

	ox1 = LOD->kx; ox2 = ox1 + LOD->kw - 1;
	oy1 = LOD->ky; oy2 = oy1 + LOD->kh - 1;
	
	if (FF_Draw_Update & Msg->Flags)
	{
		if (FF_Draw_MoveKnob & Msg->Flags)
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

	if ((FF_Draw_Object & Msg->Flags) || !(FF_Draw_MoveKnob & Msg->Flags))
	{
		Slider_DrawKnob(Class, Obj, FF_Draw_Object);
	}

	if (_area_is_horizontal)
	{
		if (((LOD->previous_pos < ox1) || (FF_Draw_Object & Msg->Flags)) && (x1 <= ox1-1))
		{
			IFEELIN F_Erase(Obj,x1,y1,ox1-1,y2,0);
		}
		if (((LOD->previous_pos > ox1) || (FF_Draw_Object & Msg->Flags)) && (ox2 + 1 <= x2))
		{
			IFEELIN F_Erase(Obj,ox2+1,y1,x2,y2,0);
		}
	}
	else
	{
		if (((LOD->previous_pos < oy1) || (FF_Draw_Object & Msg->Flags)) && (y1 <= oy1-1))
		{
			IFEELIN F_Erase(Obj,x1,y1,x2,oy1-1,0);
		}
		if (((LOD->previous_pos > oy1) || (FF_Draw_Object & Msg->Flags)) && (oy2 + 1 <= y2))
		{
			IFEELIN F_Erase(Obj,x1,oy2+1,x2,y2,0);
		}
	}
}
//+
///Slider_SetState
F_METHODM(bool32,Slider_SetState,FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	switch (Msg->State)
	{
		case FV_Area_State_Neutral:
		{
//            Slider_KnobState(Class, Obj, FALSE);

			IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_KEY);
		}
		break;

		case FV_Area_State_Focus:
		{
			IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_KEY, 0);

//            Slider_KnobState(Class, Obj, TRUE);
		}
		break;

		#if 0

		case FV_Area_State_Ghost:
		{
			IFEELIN F_Log(0, "ghost");
		}
		break;

		#endif
	}

	F_SUPERDO();

	F_OBJDO(LOD->knob);

	//slider_cache_update(Class, Obj);

	return TRUE;
}
//+

#if 0
///Slider_GoActive
F_METHOD(void,Slider_GoActive)
{
	IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,FF_EVENT_KEY,0);

	Slider_KnobState(Class,Obj,TRUE);
}
//+
///Slider_GoInactive
F_METHOD(void,Slider_GoInactive)
{
	Slider_KnobState(Class,Obj,FALSE);

	IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,0,FF_EVENT_KEY);
}
//+
///Slider_GoEnabled
F_METHOD(void,Slider_GoEnabled)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
 
	F_OBJDO(LOD->knob);
	
	F_SUPERDO();

/*
	if (LOD->own_bitmap && (FF_Area_CanDraw && _flags))
	{
		IGRAPHICS BltBitMapRastPort(LOD->own_bitmap,0,0,_area_rp,LOD->kx,LOD->ky,LOD->kw,LOD->kh,0x0C0);
	}
*/
}
//+
///Slider_GoDisabled
F_METHOD(void,Slider_GoDisabled)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_OBJDO(LOD->knob);
	
	F_SUPERDO();
/*
	if (LOD->own_bitmap && (FF_Area_CanDraw && _flags))
	{
		IGRAPHICS BltBitMapRastPort(LOD->own_bitmap,0,0,_area_rp,LOD->kx,LOD->ky,LOD->kw,LOD->kh,0x0C0);
	}
*/
}
//+
#endif
