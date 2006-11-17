#include "Private.h"

/*** Private ***********************************************************************************/

///DrPlot
STATIC void DrPlot(struct LocalObjectData *LOD,WORD x,WORD y)
{
	struct RastPort *rp = _area_rp;

	_FPen(FV_Pen_Shine); _Plot(x-1,y); _Plot(x-1,y-1); _Plot(x,y-1);
	_FPen(FV_Pen_Dark); _Plot(x,y+1); _Plot(x+1,y+1); _Plot(x+1,y);
}
//+

/*** Methods ***********************************************************************************/

///Balance_AskMinMax
F_METHOD(uint32,Balance_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT
	if (IFEELIN F_Get(_element_parent, FA_Area_Orientation) == FV_Area_Orientation_Horizontal)
	#else
	if (IFEELIN F_Get(_area_parent, FA_Area_Orientation) == FV_Area_Orientation_Horizontal)
	#endif
	{
		_balance_set_vertical;
	}

	if (_balance_is_vertical)
	{
		_area_minw = 5;
		_area_maxw = _area_minw;
	}
	else
	{
		_area_minh = 5;
		_area_maxh = _area_minh;
	}

	return F_SUPERDO();
}
//+ 
///Balance_Draw
F_METHOD(void,Balance_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	//struct RastPort *rp = _area_rp;

	int16 x1 = _area_x;
	//int16 x2 = _area_x2;
	int16 y1 = _area_y;
	/*
	int16 y2 = _area_y2;
	uint32 ap;
	uint32 bp;
	*/

	F_SUPERDO();

	/*
	ap = _area_pens[_widget_is_active ? FV_Pen_Shine  : FV_Pen_HalfShine];
	bp = _area_pens[_widget_is_active ? FV_Pen_Shadow : FV_Pen_HalfShadow];

	if (_widget_is_pressed)
	{
		uint32 temp = ap; ap = bp; bp = temp;
	}

	_APen(ap); _Move(x1,y2-1); _Draw(x1,y1); _Draw(x2-1,y1);
	_APen(bp); _Move(x1+1,y2); _Draw(x2,y2); _Draw(x2,y1+1);

	if (_widget_is_active)
	{
		_FPen(FV_Pen_HalfShine);
		_Boxf(x1+1,y1+1,x2-1,y2-1) ; _Plot(x1,y2) ; _Plot(x2,y1);
	}
	*/
	
	x1 = _area_w / 2 + x1;
	y1 = _area_h / 2 + y1;

	if (_balance_is_vertical)
	{
		DrPlot(LOD,x1,y1);
		DrPlot(LOD,x1,y1 - 4);
		DrPlot(LOD,x1,y1 + 4);
	}
	else
	{
		DrPlot(LOD,x1 - 4,y1);
		DrPlot(LOD,x1,    y1);
		DrPlot(LOD,x1 + 4,y1);
	}
}
//+
///Balance_SetState
F_METHODM(uint32,Balance_SetState,FS_Area_SetState)
{
	if (Msg->State == FV_Area_State_Focus)
	{
		IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_KEY, 0);
	}
	else
	{
		IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_KEY);
	}

	return F_SUPERDO();
}
//+

