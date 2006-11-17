#include "Private.h"

/*** Methods ***********************************************************************************/

///String_AskMinMax
F_METHOD(uint32,String_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_area_minw += _area_font->tf_XSize * LOD->displayable;
	_area_minh += _area_font->tf_YSize;

	if (_widget_is_setmaxw)
	{
		_area_maxw = _area_minw;
	}

	if (_widget_is_setmaxh)
	{
		_area_maxh = _area_minh;
	}

	return F_SUPERDO();
}
//+
///String_SetState
F_METHOD(uint32,String_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_widget_is_active)
	{
		IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, LOD->BlinkSpeed ? FF_EVENT_TICK | FF_EVENT_KEY : FF_EVENT_KEY,0);
	}
	else
	{
		IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,0, LOD->BlinkSpeed ? FF_EVENT_TICK | FF_EVENT_KEY : FF_EVENT_KEY);
	}

	return F_SUPERDO();
}
//+

