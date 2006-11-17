#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Prop_HandleEvent
F_METHODM(uint32,Prop_HandleEvent,FS_Widget_HandleEvent)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FEvent *fev = Msg->Event;
	uint32 val,size;

	switch (fev->Class)
	{
		case FF_EVENT_KEY:
		{
		   if (_area_is_horizontal)
		   {
			  switch (fev->Key)
			  {
				 case FV_KEY_LEFT:       val = LOD->first - 1;              break;
				 case FV_KEY_RIGHT:      val = LOD->first + 1;              break;
				 case FV_KEY_STEPLEFT:   val = LOD->first - LOD->visible;   break;
				 case FV_KEY_STEPRIGHT:  val = LOD->first + LOD->visible;   break;
				 case FV_KEY_FIRST:      val = 0;                           break;
				 case FV_KEY_LAST:       val = LOD->entries - LOD->visible; break;
				 default:                return 0;
			  }
		   }
		   else
		   {
			  switch (fev->Key)
			  {
				 case FV_KEY_UP:         val = LOD->first - 1;              break;
				 case FV_KEY_DOWN:       val = LOD->first + 1;              break;
				 case FV_KEY_STEPUP:     val = LOD->first - LOD->visible;   break;
				 case FV_KEY_STEPDOWN:   val = LOD->first + LOD->visible;   break;
				 case FV_KEY_TOP:        val = 0;                       	break;
				 case FV_KEY_BOTTOM:     val = LOD->entries - LOD->visible; break;
				 default:                return 0;
			  }
		   }

		   IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), val);

		   return FF_HandleEvent_Eat;
		}
		break;

		case FF_EVENT_BUTTON:
		{
			if (fev->Code == FV_EVENT_BUTTON_SELECT)
			{
				if (FF_EVENT_BUTTON_DOWN & fev->Flags)
				{
					if (F_IS_INSIDE(fev->MouseX,_area_cx,_area_cx2) &&
						F_IS_INSIDE(fev->MouseY,_area_cy,_area_cy2))
					{
						if (F_IS_INSIDE(fev->MouseX, _knob_x, _knob_x + _knob_w - 1) &&
							F_IS_INSIDE(fev->MouseY, _knob_y, _knob_y + _knob_h - 1))
						{
							LOD->mouse_off = 1 + (_area_is_horizontal ? fev->MouseX - _knob_x : fev->MouseY - _knob_y);
							LOD->saved    = LOD->first;
							LOD->flags   |= FF_Prop_Scroll;

							IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,FF_EVENT_MOTION,0);
							
							IFEELIN F_Do(LOD->knob, FM_Set,

								FA_Widget_Selected, TRUE,
								FA_Widget_Pressed, TRUE,

								TAG_DONE);
						}
						else
						{
							if (_area_is_horizontal)
							{
								val = (fev->MouseX < _knob_x) ? (-LOD->visible) : (LOD->visible);
							}
							else
							{
								val = (fev->MouseY < _knob_y) ? (-LOD->visible) : (LOD->visible);
							}

							IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), LOD->first + val);
						}

						return FF_HandleEvent_Eat;
					}
				}
				else
				{
					if (FF_Prop_Scroll & LOD->flags)
					{
						IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,NULL,FF_EVENT_MOTION);
						
						IFEELIN F_Do(LOD->knob, FM_Set,

							FA_Widget_Selected, FALSE,
							FA_Widget_Pressed, FALSE,

							TAG_DONE);

						LOD->flags &= ~FF_Prop_Scroll;
					}
				}
				break;
			}
			else if ((fev->Code == FV_EVENT_BUTTON_MENU) && (FF_EVENT_BUTTON_DOWN & fev->Flags))
			{
				if (FF_Prop_Scroll & LOD->flags)
				{
					LOD->flags &= ~FF_Prop_Scroll;

					if (!_widget_is_active)
					{
						IFEELIN F_Set(LOD->knob, FA_Widget_Selected, FALSE);
					}

					IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,0,FF_EVENT_MOTION);
					IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), LOD->saved);

					return FF_HandleEvent_Eat;
				}
			}
			else if (fev->Code == FV_EVENT_BUTTON_WHEEL)
			{
				if (F_IS_INSIDE(fev->MouseX,_area_x,_area_x2) &&
					F_IS_INSIDE(fev->MouseY,_area_y,_area_y2))
				{
					if (FF_EVENT_BUTTON_DOWN & fev->Flags)
					{
						IFEELIN F_Do(Obj, F_METHOD_ID(INCREASE), LOD->step);
					}
					else
					{
						IFEELIN F_Do(Obj, F_METHOD_ID(DECREASE), LOD->step);
					}
				}
			}
		}
		break;

		case FF_EVENT_MOTION:
		{
			int32 bnd1,bnd2,npos;

			if (FF_Prop_Scroll & LOD->flags)
			{
				if (_area_is_horizontal)
				{
					size = _knob_w;
					bnd1 = _area_cx;
					bnd2 = _area_cx2;
					npos = fev->MouseX - LOD->mouse_off + 1;
				}
				else
				{
					size = _knob_h;
					bnd1 = _area_cy;
					bnd2 = _area_cy2;
					npos = fev->MouseY - LOD->mouse_off + 1;
				}

				if (npos < bnd1)
				{
					npos = bnd1;
				}

				if (npos > bnd2 - size + 1)
				{
					npos = bnd2 - size + 1;
				}

				if (npos != (_area_is_horizontal ? _knob_x : _knob_y))
				{
					val = (npos - bnd1) * (LOD->entries - LOD->visible) / (bnd2 - bnd1 + 1 - size);

					if (_area_is_horizontal)
					{
						LOD->previous_pos = _knob_x;
						_knob_x = npos;
						_knob_w = size;
					}
					else
					{
						LOD->previous_pos = _knob_y;
						_knob_y = npos;
						_knob_h = size;
					}

					IFEELIN F_Do(LOD->knob, FM_Area_Layout);

					if (val != LOD->first)
					{
						IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), val);
					}
					else
					{
						IFEELIN F_Draw(Obj, FF_Draw_Update | FF_Draw_Damaged);
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
