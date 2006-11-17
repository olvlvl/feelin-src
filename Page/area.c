#include "Private.h"

struct FS_Page_Tree                             { int32 MouseX, MouseY; };

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///code_minmax
F_HOOK(bool32,code_minmax)
{
	struct LocalObjectData *LOD = F_LOD(((FClass *) Hook->h_Data), Obj);
	uint16 minw=0,minh=0,maxw=0,maxh=0,aw,ah;
	uint32 n=0;

	_sub_declare_all;

	for (_each)
	{
		n++;

		#ifdef DB_ASKMINMAX
		IFEELIN F_Log(0,"Min %5ld x %5ld - Max %5ld x %5ld - %s{%08lx}",_sub_minw,_sub_minh,_sub_maxw,_sub_maxh,_object_classname(sub),sub);
		#endif

		if (_sub_minw > minw) minw = _sub_minw;
		if (_sub_minh > minh) minh = _sub_minh;
		if (_sub_maxw > maxw) maxw = _sub_maxw;
		if (_sub_maxh > maxh) maxh = _sub_maxh;
	}

	aw = (LOD->font->tf_XSize * 3 + FV_TAB_PADDING_WIDTH) * n;

	minw = MAX(minw, aw);

	aw =

		FV_PAGE_BORDER_LEFT +
		LOD->style->padding_left +
		LOD->style->padding_right +
		LOD->style->padding_right;

	ah = LOD->tabs_height +

		FV_PAGE_BORDER_TOP +
		LOD->style->padding_top +
		LOD->style->padding_bottom +
		FV_PAGE_BORDER_BOTTOM;

	_area_minw  = minw + aw;
	_area_maxw  = maxw + aw;

	_area_minh += minh + ah;
	_area_maxh  = maxh + ah;

	return TRUE;
}
//+
///code_layout
F_HOOK(bool32,code_layout)
{
	struct LocalObjectData *LOD = F_LOD(((FClass *) Hook->h_Data), Obj);
	FWidgetNode *node;

	int16 x = _area_x + FV_PAGE_BORDER_LEFT;
	int16 y = _area_y + LOD->tabs_height + FV_PAGE_BORDER_TOP;
	uint16 w = _area_w - FV_PAGE_BORDER_LEFT - FV_PAGE_BORDER_RIGHT;
	uint16 h = _area_h - LOD->tabs_height - FV_PAGE_BORDER_TOP - FV_PAGE_BORDER_BOTTOM;

	LOD->back_origin.x = x;
	LOD->back_origin.y = y;
	LOD->back_origin.w = w;
	LOD->back_origin.h = h;

	x += LOD->style->padding_left;
	y += LOD->style->padding_top;
	w -= (LOD->style->padding_left + LOD->style->padding_right);
	h -= (LOD->style->padding_top + LOD->style->padding_bottom);

	for (_each)
	{
		int16 sx = (_sub_minw == _sub_maxw) ? (w - _sub_minw) / 2 + x : x;
		int16 sy = (_sub_minh == _sub_maxh) ? (h - _sub_minh) / 2 + y : y;
		uint16 sw = (_sub_minw == _sub_maxw) ? _sub_minw : w;
		uint16 sh = (_sub_minh == _sub_maxh) ? _sub_maxh : h;

		if ((_sub_x != sx) || (_sub_y != sy) || (_sub_w != sw) || (_sub_h != sh))
		{
			_sub_x = sx;
			_sub_y = sy;
			_sub_w = sw;
			_sub_h = sh;
			_sub_set_damaged;
		}

		IFEELIN F_Do(node->Widget, FM_Area_Layout);
	}

	return TRUE;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Page_Show
F_METHOD(bool32,Page_Show)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	/* we skip the Group class */

	if (IFEELIN F_SuperDo(Class->Super, Obj, Method, Msg) == FALSE)
	{
		return FALSE;
	}

	if (LOD->node_active != NULL)
	{
		IFEELIN F_Do(LOD->node_active->Widget, FM_Area_Show);
	}
	
	return TRUE;
}
//+
///Page_Hide
F_METHOD(void,Page_Hide)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->node_active)
	{
		IFEELIN F_Do(LOD->node_active->Widget, FM_Area_Hide);
	}

	/* we skip the Group class */
	
	IFEELIN F_SuperDo(Class->Super,Obj,Method,Msg);
}
//+
///Page_SetState
F_METHODM(uint32, Page_SetState, FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (_widget_is_active)
	{
		IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_KEY, 0);
	}
	else
	{
		IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_KEY);
	}

	return TRUE;//F_SUPERDO();
}
//+
///Page_TreeUp
F_METHODM(uint32,Page_TreeUp,FS_Page_Tree)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *node;

	if (Msg->MouseX >= _area_x && Msg->MouseX <= _area_x2 &&
		Msg->MouseY >= _area_y && Msg->MouseY <= _area_y2)
	{
		uint32 rc;

		node = LOD->node_active;

		if (node &&
			Msg->MouseX >= _sub_x && Msg->MouseX <= _sub_x2 &&
			Msg->MouseY >= _sub_y && Msg->MouseY <= _sub_y2)
		{
			rc = F_OBJDO(node->Widget);

			if (rc)
			{
				return rc;
			}
		}

		/* we skip the Group class */
		
		return IFEELIN F_SuperDo(Class->Super, Obj, Method, Msg);
	}

	return 0;
}
//+
