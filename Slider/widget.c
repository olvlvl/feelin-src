#include "Private.h"

//#define DB_ASKMINMAX

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///Slider_HandleEvent
F_METHODM(bits32,Slider_HandleEvent,FS_Widget_HandleEvent)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FEvent *fev = Msg->Event;
	uint32    v,size;
	uint16    bnd1,bnd2;
	int16     npos;

	switch (fev->Class)
	{
		case FF_EVENT_KEY:
		{
			return F_SUPERDO();
		}

		case FF_EVENT_BUTTON:
		{
			if (fev->Code == FV_EVENT_BUTTON_SELECT)
			{
				if (FF_EVENT_BUTTON_DOWN & fev->Flags)
				{
					if (F_IS_INSIDE(fev->MouseX,_area_cx,_area_cx2) &&
						F_IS_INSIDE(fev->MouseY,_area_cy,_area_cy2))
					{
						int32 val = IFEELIN F_Get(Obj, F_RESOLVED_ID(VALUE));

						if (F_IS_INSIDE(fev->MouseX,LOD->kx,LOD->kx + LOD->kw - 1) &&
							F_IS_INSIDE(fev->MouseY,LOD->ky,LOD->ky + LOD->kh -1))
						{
							LOD->mouse_off = 1 + (_area_is_horizontal ? fev->MouseX - LOD->kx : fev->MouseY - LOD->ky);
							LOD->saved = val;

							_slider_set_scroll;

							IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,FF_EVENT_MOTION,0);

							//Slider_KnobState(Class,Obj,TRUE);

							IFEELIN F_SuperDo(Class, Obj, FM_Set, FA_Widget_Pressed, TRUE, TAG_DONE);
						}
						else
						{
							if (_area_is_horizontal)
							{
								v = val + ((fev->MouseX < LOD->kx) ? -1 : 1);
							}
							else
							{
								v = val + ((fev->MouseY < LOD->ky) ? -1 : 1);
							}

							IFEELIN F_Set(Obj, F_RESOLVED_ID(VALUE), v);
						}
						return FF_HandleEvent_Eat;
					}
				}
				else if (FF_Slider_Scroll & LOD->flags)
				{
					IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,NULL,FF_EVENT_MOTION);
					
					/*
					if (!_widget_is_active)
					{
						Slider_KnobState(Class,Obj,FALSE);
					}
					*/

					LOD->flags &= ~FF_Slider_Scroll;

					IFEELIN F_SuperDo(Class,Obj,FM_Set,FA_Widget_Pressed,FALSE,TAG_DONE);
				}
			}
			else if (fev->Code == FV_EVENT_BUTTON_MENU)
			{
				if ((FF_EVENT_BUTTON_DOWN & fev->Flags) && _slider_is_scroll)
				{
					LOD->flags &= ~FF_Slider_Scroll;

					/*
					if (!_widget_is_active)
					{
						Slider_KnobState(Class, Obj, FALSE);
					}
					*/

					IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,0,FF_EVENT_MOTION);
					IFEELIN F_Set(Obj, F_RESOLVED_ID(VALUE), LOD->saved);
					IFEELIN F_SuperDo(Class,Obj,FM_Set,FA_Widget_Pressed,FALSE,FA_NoNotify,TRUE,TAG_DONE);

					return FF_HandleEvent_Eat;
				}
			}
			else
			{
				return F_SUPERDO();
			}
		}
		break;

		case FF_EVENT_MOTION:
		{
			if (FF_Slider_Scroll & LOD->flags)
			{
				int32 val,min,max;

				IFEELIN F_Do
				(
					Obj, FM_Get,
					
					F_RESOLVED_ID(VALUE),   &val,
					F_RESOLVED_ID(MIN),     &min,
					F_RESOLVED_ID(MAX),     &max,
					
					TAG_DONE
				);

				if (_area_is_horizontal)
				{
					size = LOD->kw;
					bnd1 = _area_cx;
					bnd2 = bnd1 + _area_cw - 1;
					npos = fev->MouseX - LOD->mouse_off + 1;
				}
				else
				{
					size = LOD->kh;
					bnd1 = _area_cy;
					bnd2 = bnd1 + _area_ch - 1;
					npos = fev->MouseY - LOD->mouse_off + 1;
				}

				if (npos < bnd1)
				{
					if (val > min)
					{
						IFEELIN F_Set(Obj, F_RESOLVED_ID(VALUE), min);
						return FF_HandleEvent_Eat;
					}
					npos = bnd1;
				}

				if (npos > (bnd2 - size + 1))
				{
					if (val < max)
					{
						IFEELIN F_Set(Obj, F_RESOLVED_ID(VALUE), max);
						return FF_HandleEvent_Eat;
					}
					npos = bnd2 - size + 1;
				}

				if (npos != (_area_is_horizontal ? LOD->kx : LOD->ky))
				{
					v = (npos - bnd1) * (max - min) / (bnd2 - bnd1 + 1 - size) + min;

					if (_area_is_horizontal)
					{
						LOD->previous_pos = LOD->kx ; LOD->kx = npos;

						if (LOD->own_bitmap == NULL)
						{
							_knob_x = npos;
							_knob_w = size;
						}
					}
					else
					{
						LOD->previous_pos = LOD->ky ; LOD->ky = npos;

						if (LOD->own_bitmap == NULL)
						{
							_knob_y = npos;
							_knob_h = size;
						}
					}

					IFEELIN F_Do(LOD->knob, FM_Area_Layout);

					if (v != val)
					{
						IFEELIN F_Set(Obj, F_RESOLVED_ID(VALUE), v);
					}
					else
					{
						IFEELIN F_Draw(Obj,FF_Draw_Update | FF_Draw_Damaged | FF_Draw_MoveKnob);
					}
				}
				return FF_HandleEvent_Eat;
			}
		}
		break;
	}
	return 0;
}
//+
