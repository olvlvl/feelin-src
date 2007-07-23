#include "Private.h"

/*************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Image_AskMinMax
F_METHOD(uint32,Image_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 render_w=0;
	uint32 render_h=0;
	uint32 select_w=0;
	uint32 select_h=0;

	IFEELIN F_Do(LOD->render, FM_Get,

		FA_ImageDisplay_Width,  &render_w,
		FA_ImageDisplay_Height, &render_h,
		
		TAG_DONE);

	IFEELIN F_Do(LOD->select, FM_Get,

		FA_ImageDisplay_Width,  &select_w,
		FA_ImageDisplay_Height, &select_h,

		TAG_DONE);

	if (_widget_is_setminw) _area_minw += MAX(render_w, select_w);
	if (_widget_is_setminh) _area_minh += MAX(render_h, select_h);
	if (_widget_is_setmaxw) _area_maxw  = MAX(render_w, select_w);
	if (_widget_is_setmaxh) _area_maxh  = MAX(render_h, select_h);

	return F_SUPERDO();
}
//+
///Image_Draw
F_METHODM(void,Image_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FRect rect;
	FObject image = _widget_is_selected ? LOD->select : LOD->render;

	F_SUPERDO();

	rect.x1 = _area_cx;
	rect.y1 = _area_cy;
	rect.x2 = _area_cx2;
	rect.y2 = _area_cy2;

	if (image == NULL)
	{
		image = LOD->render;
	}

	if ((image == NULL) || (IFEELIN F_Get(image, FA_ImageDisplay_Mask)))
	{
		IFEELIN F_Do(Obj, FM_Area_Erase, &rect, 0);
	}

	if (image != NULL)
	{
		bits32 flags = 0;

		switch (_area_state)
		{
			case FV_Area_State_Touch: flags = FF_ImageDisplay_Draw_Touch; break;
			case FV_Area_State_Focus: flags = FF_ImageDisplay_Draw_Focus; break;
			case FV_Area_State_Ghost: flags = FF_ImageDisplay_Draw_Ghost; break;
		}

//    	  IFEELIN F_Log(0, "state (%ld) flags (0x%08lx)", _area_state, flags);

		IFEELIN F_Do(image, FM_ImageDisplay_Draw, _area_render, &rect, flags);
	}
	else if ((_area_cw > 4) && (_area_ch > 4))
	{
		struct RastPort *rp = _area_rp;

		_FPen(FV_Pen_Shadow);
		_Move(rect.x1,rect.y1);
		_Draw(rect.x2-1,rect.y2);
		_Move(rect.x1+1,rect.y2);
		_Draw(rect.x2,rect.y1);

		_FPen(FV_Pen_HalfShine);
		_Move(rect.x1+1,rect.y1);
		_Draw(rect.x2,rect.y2);
		_Move(rect.x1,rect.y2);
		_Draw(rect.x2-1,rect.y1);
	}
}
//+
