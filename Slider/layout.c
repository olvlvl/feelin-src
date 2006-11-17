#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///slider_cache_update
void slider_cache_update(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
		
	#ifdef DB_CACHE
	IFEELIN F_Log(0,"cache_update - kw (%ld)(%ld) kh (%ld)(%ld)",LOD->kw,_knob_w,LOD->kh,_knob_h);
	#endif

	if (LOD->own_render && _area_rp)
	{
		if ((LOD->kw != _knob_w) || (LOD->kh != _knob_h) || (!LOD->own_bitmap))
		{

			/* Setting OwnRender rastport  to  the  default  rastport  is  a
			fallback  e.i. if bitmap allocation fails rendering will be done
			on the  default  rastport.  OwnBitMap  is  checked  to  know  if
			rendering is buffered. */

			if (LOD->own_bitmap)
			{
				IGRAPHICS FreeBitMap(LOD->own_bitmap); LOD->own_bitmap = NULL;
			}
		
			LOD->own_bitmap = IGRAPHICS AllocBitMap(LOD->kw,LOD->kh,IGRAPHICS GetBitMapAttr(_area_rp->BitMap,BMA_DEPTH),BMF_CLEAR,_area_rp->BitMap);

			if (LOD->own_bitmap)
			{
				LOD->own_rport->BitMap = LOD->own_bitmap;
			}
			else
			{
				LOD->own_rport->BitMap = _area_rp->BitMap;
			}
		}
	}

	if (LOD->own_bitmap)
	{
		IFEELIN F_Layout(LOD->knob, 0, 0, LOD->kw, LOD->kh, 0);
	}
	else
	{
		IFEELIN F_Layout(LOD->knob, LOD->kx, LOD->ky, LOD->kw, LOD->kh, 0);
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Slider_Layout
F_METHOD(bool32,Slider_Layout)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	int32 num_val,num_min,num_max;
	uint32 v,r,p;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	IFEELIN F_Do(Obj,FM_Get,
		
		F_RESOLVED_ID(VALUE),   &num_val,
		F_RESOLVED_ID(MIN),     &num_min,
		F_RESOLVED_ID(MAX),     &num_max,
		
		TAG_DONE);

	v = num_val - num_min;
	r = num_max - num_min;

	LOD->last_layout_value = num_val;

	if (_area_is_horizontal)
	{
		LOD->kw = _knob_minw ;
		LOD->kh = _area_ch;
		p = (_area_cw - LOD->kw) * v / r + _area_cx;
		LOD->previous_pos = LOD->kx;

		LOD->kx = p;
		LOD->ky = _area_cy;
	}
	else
	{
		LOD->kh = _knob_minh ;
		LOD->kw = _area_cw;
		p = (_area_ch - LOD->kh) * v / r + _area_cy;
		LOD->previous_pos = LOD->ky;

		LOD->kx = _area_cx;
		LOD->ky = p;
	}                            

	slider_cache_update(Class,Obj);

	return TRUE;
}
//+
