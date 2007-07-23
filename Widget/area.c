#include "Private.h"

//#define DB_SCHEME

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Widget_Show
F_METHOD(bool32, Widget_Show)
{
	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

//	  widget_select_state(Class, Obj);

//	  IFEELIN F_Do(Obj, FM_Area_SetState, _area_state);

	widget_try_add_handler(Class, Obj);

	return TRUE;
}
//+
///Widget_Hide
F_METHOD(bool32, Widget_Hide)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}
		
	widget_try_rem_handler(Class, Obj);

	if (Obj == (FObject) IFEELIN F_Get(_area_win, FA_Window_ActiveObject))
	{
	   IFEELIN F_Set(_area_win, FA_Window_ActiveObject, NULL);
	}

	return TRUE;
}
//+
///Widget_AskMinMax
F_METHOD(uint32, Widget_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (_widget_is_setmaxw && (_area_maxw == FV_Area_Max))
	{
		_area_maxw = _area_minw;
	}

	if (_widget_is_setmaxh && (_area_maxh == FV_Area_Max))
	{
		_area_maxh = _area_minh;
	}

	return F_SUPERDO();
}
//+
