#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Gauge_New
F_METHOD(uint32,Gauge_New)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	F_SAVE_AREA_PUBLIC;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_INFO:
		{
			LOD->info = (STRPTR) item.ti_Data;
		}
		break;
	}

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Area_Fillable,       FALSE,
		FA_Widget_Chainable,    FALSE,
	   "FA_Numeric_Format",     NULL,

		TAG_MORE, Msg);
}
//+
///Gauge_Setup
F_METHODM(bool32, Gauge_Setup, FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	STRPTR value;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	value = NULL;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(IMAGE), &value, NULL);

	if (value != NULL)
	{
		LOD->image = ImageDisplayObject,

			FA_ImageDisplay_Spec, value,
			FA_ImageDisplay_Origin, &_area_content,
			FA_ImageDisplay_Owner, Obj,

		End;

		if (LOD->image != NULL)
		{
			if (IFEELIN F_Do(LOD->image, FM_ImageDisplay_Setup, Msg->Render) == FALSE)
			{
				IFEELIN F_DisposeObj(LOD->image);

				LOD->image = NULL;
			}
		}
	}

	return TRUE;
}
//+
///Gauge_Cleanup
F_METHOD(uint32, Gauge_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
												
	if (LOD->image != NULL)
	{
		IFEELIN F_Do(LOD->image, FM_ImageDisplay_Cleanup);
		IFEELIN F_DisposeObj(LOD->image);
		
		LOD->image = NULL;
	}

	return F_SUPERDO();
}
//+
///Gauge_AskMinMax
F_METHOD(uint32,Gauge_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_area_minh += 10;
	_area_minw += 10;

	return F_SUPERDO();
}
//+
///Gauge_Draw
F_METHODM(void,Gauge_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct RastPort *rp = _area_rp;
	uint32 *pn;
	int16 x1 = _area_cx;
	int16 x2 = _area_cx2;
	int16 y1 = _area_cy;
	int16 y2 = _area_cy2;
	int16 s,mp;
	int32 val,max,min;

	F_SUPERDO();
	
	pn = _area_palette->Pens;

	IFEELIN F_Do
	(
		Obj,FM_Get,
	   
		F_RESOLVED_ID(VALUE),   &val,
		F_RESOLVED_ID(MIN),     &min,
		F_RESOLVED_ID(MAX),     &max,
	   
		TAG_DONE
	);

	if (max == min)
	{
	   goto _Gauge_Draw_Fill;
	}

	s  = _area_is_horizontal ? (x2 - x1 + 1) : (y2 - y1 + 1);
	mp = val * s / (max - min);

	if (LOD->image != NULL)
	{
		FRect r;
					
		r.x1 = x1;
		r.y2 = y2;
 
		if (_area_is_horizontal)
		{
			mp = mp + x1 - 1;
			
			r.y1 = y1;
			r.x2 = mp;
 
			x1 = mp + 1;
 
		}
		else
		{
			mp = y2 - mp + 1;
					
			r.y1 = mp;
			r.x2 = x2;
 
			y2 = mp - 1;
		}

		IFEELIN F_Do(LOD->image, FM_ImageDisplay_Draw, _area_render, &r, 0);
	}
	else
	{
		if (_area_is_horizontal)
		{
		   if (mp)
		   {
			  mp = x1 + mp - 1;
			  s  = mp - x1 + 1;

			  if (s > 0)
			  {
				 _APen(pn[FV_Pen_Dark]);  _Move(mp,y1);  _Draw(mp,y2);
			  }

			  if (s > 1)
			  {
				 _APen(pn[FV_Pen_Dark]) ; _Move(x1,y1) ; _Draw(x1,y2);
			  }

			  if (s > 2);
			  {
				 x1 += 1;

				 _APen(pn[FV_Pen_Fill]);        _Plot(x1,y1);     _Plot(x1,y2-2);
				 _APen(pn[FV_Pen_HalfShine]);   _Plot(x1,y1+1);   _Plot(x1,y2-3);
				 _APen(pn[FV_Pen_Shine]);       _Move(x1,y1+2);   _Draw(x1,y2-4);
				 _APen(pn[FV_Pen_HalfShadow]);  _Plot(x1,y2-1);
				 _APen(pn[FV_Pen_Shadow]);      _Plot(x1,y2);
				 x1 -= 1;
			  }

			  if (s > 3)
			  {
				 mp -= 1;
				 _Move(mp,y1+1); _Draw(mp,y2-1);
				 _APen(pn[FV_Pen_HalfShadow]);  _Plot(mp,y1);
				 _APen(pn[FV_Pen_HalfDark]);    _Plot(mp,y2);
				 mp += 1;
			  }

			  if (s > 4)
			  {
				 mp -= 2;
				 _Plot(mp,y2);
				 _APen(pn[FV_Pen_Shadow]);      _Plot(mp,y2-1);
				 _APen(pn[FV_Pen_HalfShadow]);  _Plot(mp,y2-2);   _Plot(mp,y1);
				 _APen(pn[FV_Pen_Fill]);        _Plot(mp,y2-3);   _Plot(mp,y1+1);
				 _APen(pn[FV_Pen_HalfShine]);   _Move(mp,y1+2);   _Draw(mp,y2-4);
				 mp += 2;
			  }

			  if (s > 5)
			  {
				 x1 += 2;
				 _Plot(x1,y2-4); _Plot(x1,y1+2);
				 _APen(pn[FV_Pen_HalfDark]);    _Plot(x1,y2);
				 _APen(pn[FV_Pen_Shadow]);      _Plot(x1,y2-1);
				 _APen(pn[FV_Pen_HalfShadow]);  _Plot(x1,y2-2);   _Plot(x1,y1);
				 _APen(pn[FV_Pen_Fill]);        _Plot(x1,y2-3);   _Plot(x1,y1+1);
				 _APen(pn[FV_Pen_Shine]);       _Move(x1,y2-5);   _Draw(x1,y1+3);
				 x1 -= 2;
			  }

			  if (s > 6)
			  {
				 x1 += 3;
				 mp -= 3;
				 _Boxf(x1,y1+3,mp,y2-5);
				 _APen(pn[FV_Pen_HalfDark]);    _Move(x1,y2);     _Draw(mp,y2);
				 _APen(pn[FV_Pen_Shadow]);      _Move(x1,y2-1);   _Draw(mp,y2-1);
				 _APen(pn[FV_Pen_HalfShadow]);  _Move(x1,y2-2);   _Draw(mp,y2-2); _Move(x1,y1);   _Draw(mp,y1);
				 _APen(pn[FV_Pen_Fill]);        _Move(x1,y2-3);   _Draw(mp,y2-3); _Move(x1,y1+1); _Draw(mp,y1+1);
				 _APen(pn[FV_Pen_HalfShine]);   _Move(x1,y2-4);   _Draw(mp,y2-4); _Move(x1,y1+2); _Draw(mp,y1+2);
				 mp += 3;
			  }
			  x1 = mp + 1;
		   }
		}
		else
		{
		   if (mp)
		   {
			  mp = y2 - mp + 1;
			  s  = y2 - mp + 1;

			  if (s > 0)
			  {
				 _APen(pn[FV_Pen_Dark]); _Move(x1,y2); _Draw(x2,y2);
			  }

			  if (s > 1)
			  {
				 _APen(pn[FV_Pen_Dark]); _Move(x1,mp); _Draw(x2,mp);
			  }

			  if (s > 2)
			  {
				 mp += 1;
				 _APen(pn[FV_Pen_Fill]);       _Plot(x1,mp);   _Plot(x2-2,mp);
				 _APen(pn[FV_Pen_HalfShine]);  _Plot(x1+1,mp); _Plot(x2-3,mp);
				 _APen(pn[FV_Pen_Shine]);      _Move(x1+2,mp); _Draw(x2-4,mp);
				 _APen(pn[FV_Pen_HalfShadow]); _Plot(x2-1,mp);
				 _APen(pn[FV_Pen_Shadow]);     _Plot(x2,mp);
				 mp -= 1;
			  }

			  if (s > 3)
			  {
				 y2 -= 1;
				 _Move(x1+1,y2); _Draw(x2-1,y2);
				 _APen(pn[FV_Pen_HalfDark]);   _Plot(x2,y2);
				 _APen(pn[FV_Pen_HalfShadow]); _Plot(x1,y2);
				 y2 += 1;
			  }

			  if (s > 4)
			  {
				 y2 -= 2;
				 _Plot(x1,y2); _Plot(x2-2,y2);
				 _APen(pn[FV_Pen_HalfDark]);  _Plot(x2,y2);
				 _APen(pn[FV_Pen_Shadow]);    _Plot(x2-1,y2);
				 _APen(pn[FV_Pen_Fill]);      _Plot(x1+1,y2); _Plot(x2-3,y2);
				 _APen(pn[FV_Pen_HalfShine]); _Move(x1+2,y2); _Draw(x2-4,y2);
				 y2 += 2;
			  }

			  if (s > 5)
			  {
				 mp += 2;
				 _Plot(x1+2,mp); _Plot(x2-4,mp);
				 _APen(pn[FV_Pen_HalfDark]);   _Plot(x2,mp);
				 _APen(pn[FV_Pen_Shadow]);     _Plot(x2-1,mp);
				 _APen(pn[FV_Pen_HalfShadow]); _Plot(x1,mp);   _Plot(x2-2,mp);
				 _APen(pn[FV_Pen_Fill]);       _Plot(x1+1,mp); _Plot(x2-3,mp);
				 _APen(pn[FV_Pen_Shine]);      _Move(x1+3,mp); _Draw(x2-5,mp);
				 mp -= 2;
			  }

			  if (s > 6)
			  {
				 y2 -= 3;
				 mp += 3;
				 _Boxf(x1+3,mp,x2-5,y2);
				 _APen(pn[FV_Pen_HalfDark]);   _Move(x2,mp);   _Draw(x2,y2);
				 _APen(pn[FV_Pen_Shadow]);     _Move(x2-1,mp); _Draw(x2-1,y2);
				 _APen(pn[FV_Pen_HalfShadow]); _Move(x2-2,mp); _Draw(x2-2,y2); _Move(x1,mp)   ; _Draw(x1,y2);
				 _APen(pn[FV_Pen_Fill]);       _Move(x2-3,mp); _Draw(x2-3,y2); _Move(x1+1,mp) ; _Draw(x1+1,y2);
				 _APen(pn[FV_Pen_HalfShine]);  _Move(x2-4,mp); _Draw(x2-4,y2); _Move(x1+2,mp) ; _Draw(x1+2,y2);
				 mp -= 3;
			  }
			  y2 = mp - 1;
		   }
		}
	}

_Gauge_Draw_Fill:

	IFEELIN F_Erase(Obj, x1, y1, x2, y2, 0);
}
//+

#if 0

/*** Preferences *******************************************************************************/

STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("gauge",  "frame",            FV_TYPE_STRING, "Spec", "<frame id='18' />"),
	F_PREFERENCES_ADD("gauge",  "back",             FV_TYPE_STRING, "Spec", "halfshadow"),
	F_PREFERENCES_ADD("gauge",  "back-horizontal",  FV_TYPE_STRING, "Spec", "halfshadow"),
	F_PREFERENCES_ADD("gauge",  "color-scheme",     FV_TYPE_STRING, "Spec", NULL),
	F_PREFERENCES_ADD("gauge",  "image",            FV_TYPE_STRING, "Spec", NULL),
	F_PREFERENCES_ADD("gauge",  "image-horizontal", FV_TYPE_STRING, "Spec", NULL),

	F_ARRAY_END
};

///Prefs_New
F_METHOD(FObject,Prefs_New)
{
	struct p_LocalObjectData *LOD = F_LOD(Class,Obj);
	
	F_SAVE_AREA_PUBLIC;

	LOD->Val = 1;

	if (IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Gauge",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/gauge.xml",

		TAG_MORE,Msg) != 0)
	{

		IFEELIN F_Do
		(
			Obj, (uint32) "GetObjects",

			"example", &LOD->example,

			NULL
		);

		return Obj;
	}

	return NULL;
}
//+
///Prefs_Show
F_METHOD(uint32,Prefs_Show)
{
	struct p_LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render)
	{
		if (LOD->timer_handler == NULL)
		{
			LOD->timer_handler = (APTR) IFEELIN F_Do
			(
				_area_app, FM_Application_CreateSignalHandler,

				FA_SignalHandle_Target,    Obj,
				FA_SignalHandle_Method,    FM_Gauge_Update,
				FA_SignalHandle_Micros,    30000,

				TAG_DONE
			);
		}
	}

	return F_SUPERDO();
}
//+
///Prefs_Hide
F_METHOD(void,Prefs_Hide)
{
	struct p_LocalObjectData *LOD = F_LOD(Class,Obj);

	F_SUPERDO();

	if (_area_render != NULL)
	{
		if (LOD->timer_handler != NULL)
		{
			IFEELIN F_Do(_area_app, FM_Application_DeleteSignalHandler, LOD->timer_handler);

			LOD->timer_handler = NULL;
		}
	}
}
//+
///Prefs_Update
F_METHOD(int32,Prefs_Update)
{
	struct p_LocalObjectData *LOD = F_LOD(Class,Obj);

	int32 min,max,val;

	IFEELIN F_Do(LOD->example, FM_Get, "Min",&min, "Max",&max, TAG_DONE);
	val = IFEELIN F_Do(LOD->example, (uint32) "Increase",LOD->Val);

	if (val == min)
	{
		LOD->Val = 1;
	}
	else if (val == max)
	{
		LOD->Val = -1;
	}

	return TRUE; // If we return FALSE the timer event won't be requested again
}
//+

#endif
