#include "Private.h"

//#define DB_MODIFYHANDLER
//#define DB_HANDLEEVENT
//#define DB_HANDLEEVENT_CHECK
//#define DB_TRY_ADD_HANDLER

struct myDnDDisplay
{
	uint32                           Pen;
	int32                            Way;
};

STATIC struct myDnDDisplay          DnD;

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///widget_try_add_handler
void widget_try_add_handler(FClass *Class,FObject Obj)
{
	#if 0

	IFEELIN F_Log(0, "add handler disabled");

	#else

	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render == NULL)
	{
		IFEELIN F_Log(0, "Render is NULL !!");

		return;
	}

	if (LOD->handler != NULL)
	{
		#ifdef DB_TRY_ADD_HANDLER
		IFEELIN F_Log(0,"handler (0x%08lx) already active", LOD->handler);
		#endif

		return;
	}

	if (LOD->handler_events == 0)
	{
		#ifdef DB_TRY_ADD_HANDLER
		IFEELIN F_Log(0,"no events requested");
		#endif

		return;
	}

	if (!_area_is_drawable)
	{
		#ifdef DB_TRY_ADD_HANDLER
		IFEELIN F_Log(0,"object is not drawable");
		#endif

		return;
	}

	if (_widget_is_disabled)
	{
		#ifdef DB_TRY_ADD_HANDLER
		IFEELIN F_Log(0,"object is disabled");
		#endif

		return;
	}

	#ifdef DB_TRY_ADD_HANDLER
	IFEELIN F_Log(0,"add events (0x%08lx)",LOD->handler_events);
	#endif

	LOD->handler = (APTR) IFEELIN F_Do(_area_win, FM_Window_CreateEventHandler,

		FA_EventHandler_Events, LOD->handler_events,
		FA_EventHandler_Target, Obj,

		TAG_DONE);

	#endif
}
//+
///widget_try_rem_handler
void widget_try_rem_handler(FClass *Class,FObject Obj)
{
	#if 0

	IFEELIN F_Log(0, "rem handler disabled");

	#else

	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if ((LOD->handler != NULL) && (_area_render != NULL))
	{
		IFEELIN F_Do(_area_win, FM_Window_DeleteEventHandler, LOD->handler);

		LOD->handler = NULL;
	}

	#endif
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Widget_ModifyEvents
F_METHODM(APTR,Widget_ModifyEvents,FS_Widget_ModifyEvents)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	bits32 events;

	events = (LOD->handler_events | Msg->Add) & ~Msg->Sub;

	#ifdef DB_MODIFYHANDLER
	IFEELIN F_Log(0,"add (0x%06lx) sub (0x%06lx) handler (0x%06lx) >> (0x%06lx)",Msg->Add,Msg->Sub,LOD->handler_events,events);
	#endif

	if (_area_render == NULL)
	{
		LOD->handler_events = events;

		return NULL;
	}

	if (LOD->handler != NULL)
	{
		if (LOD->handler_events == events)
		{
			return LOD->handler;
		}

		IFEELIN F_Do(_area_win, FM_Window_DeleteEventHandler, LOD->handler);

		LOD->handler = NULL;
	}

	LOD->handler_events = events;

	widget_try_add_handler(Class,Obj);

	return LOD->handler;
}
//+
///Widget_HandleEvent
F_METHODM(bits32,Widget_HandleEvent,FS_Widget_HandleEvent)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FEvent *fev = Msg->Event;

	#ifdef DB_HANDLEEVENT_CHECK
  
	if (FF_Widget_Disabled & _flags)
	{
		IFEELIN F_Log(0,"Object is disabled but have received FM_Widget_HandleEvent !!");

		return 0;
	}
	
	if (LOD->handler == NULL)
	{
		IFEELIN F_Log(0,"no handler, but event received");
	}

	#endif

	 if (fev->Key)
	 {
/// Key
		if (_widget_isnt_active)
		{

			/* we only handle key events if we are the active object */

			return 0;
		}

		switch (fev->Key)
		{
			case FV_KEY_PRESS:
			{
				switch (LOD->mode)
				{
					#ifdef F_NEW_WIDGET_MODE
					case FV_Widget_Mode_Button:
					#else
					case FV_Widget_Mode_Release:
					#endif
					{
						if (_widget_isnt_selected)
						{
							IFEELIN F_Do
							(
								Obj, FM_Set,
								
								FA_Widget_Selected, TRUE,
								FA_Widget_Pressed, TRUE,
								
								TAG_DONE
							);
						}
					}
					break;

					#ifdef F_NEW_WIDGET_MODE
					case FV_Widget_Mode_Touch:
					#else
					case FV_Widget_Mode_Immediate:
					#endif
					{
						IFEELIN F_SuperDo(Class, Obj, FM_Set, FA_Widget_Selected, TRUE, TAG_DONE);
					}
					break;

					case FV_Widget_Mode_Toggle:
					{
						IFEELIN F_Set(Obj, FA_Widget_Selected, _widget_is_selected ? FALSE : TRUE);
					}
					break;
				}
			}
			return FF_HandleEvent_Eat;

			case FV_KEY_RELEASE:
			{
				#ifdef F_NEW_WIDGET_MODE
				if (LOD->mode == FV_Widget_Mode_Button)
				#else
				if (LOD->mode == FV_Widget_Mode_Release)
				#endif
				{
					if (_widget_is_selected)
					{
						IFEELIN F_Do
						(
							Obj, FM_Set,

							FA_Widget_Selected, FALSE,
							FA_Widget_Pressed, FALSE,
							
							TAG_DONE
						);
					}
				}
			}
			return FF_HandleEvent_Eat;
		}
	}
//+
	else
	{
		switch (fev->Class)
		{
///event:key
			case FF_EVENT_KEY:
			{
				if (FF_EVENT_REPEAT & fev->Flags)
				{
					return 0;
				}
				
				if (LOD->accel && (IUTILITY ToLower(LOD->accel) == IUTILITY ToLower(fev->DecodedChar)))
				{ 
					switch (LOD->mode)
					{
						#ifdef F_NEW_WIDGET_MODE
						case FV_Widget_Mode_Button:
						#else
						case FV_Widget_Mode_Release:
						#endif
						{
							if (FF_EVENT_KEY_UP & fev->Flags)
							{
								if (_widget_is_selected)
								{
									IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_TICK);

									IFEELIN F_Do
									(
										Obj, FM_Set,
										
										FA_Widget_Pressed, FALSE,
										FA_Widget_Selected, FALSE,
										
										TAG_DONE
									);
								}
							}
							else
							{
								IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_TICK, 0);

								IFEELIN F_Do
								(
									Obj, FM_Set,
									
									FA_Widget_Pressed, TRUE,
									FA_Widget_Selected, TRUE,
									
									TAG_DONE
								);
							}
						}
						break;

						case FV_Widget_Mode_Toggle:
						{
							if (!(FF_EVENT_KEY_UP & fev->Flags))
							{
								IFEELIN F_Set(Obj, FA_Widget_Selected, _widget_is_selected ? FALSE : TRUE);
							}
						}
						break;

						#ifdef F_NEW_WIDGET_MODE
						case FV_Widget_Mode_Touch:
						#else
						case FV_Widget_Mode_Immediate:
						#endif
						{
							IFEELIN F_SuperDo(Class, Obj, FM_Set, FA_Widget_Selected, TRUE, TAG_DONE);
						}
						break;
					}
					return FF_HandleEvent_Eat;
				}
				#ifdef F_NEW_WIDGET_MODE
				else if (_widget_is_selected && (LOD->mode == FV_Widget_Mode_Button))
				#else
				else if (_widget_is_selected && (LOD->mode == FV_Widget_Mode_Release))
				#endif
				{
					IFEELIN F_Do
					(
						Obj, FM_Set,
						
						FA_Widget_Selected, FALSE,
						FA_Widget_Pressed, FALSE,
						FA_NoNotify, TRUE,
						
						TAG_DONE
					);
				}
			}
			break;
//+
///event:button
			case FF_EVENT_BUTTON:
			{
				if (fev->Code == FV_EVENT_BUTTON_SELECT)
				{
					if (FF_EVENT_BUTTON_DOWN & fev->Flags)
					{
					 
/*** button-select-down ************************************************************************/
	
						if (F_IS_INSIDE(fev->MouseX, _area_x, _area_x2) &&
							F_IS_INSIDE(fev->MouseY, _area_y, _area_y2))
						{
							switch (LOD->mode)
							{
								#ifdef F_NEW_WIDGET_MODE
								case FV_Widget_Mode_Touch:
								#else
								case FV_Widget_Mode_Immediate:
								#endif
								{
									IFEELIN F_SuperDo(Class, Obj, FM_Set, FA_Widget_Selected, TRUE, TAG_DONE);
								}
								break;

								case FV_Widget_Mode_Toggle:
								{
									IFEELIN F_Set(Obj, FA_Widget_Selected, _widget_is_selected ? FALSE : TRUE);
								}
								break;

								#ifdef F_NEW_WIDGET_MODE
								case FV_Widget_Mode_Button:
								#else
								case FV_Widget_Mode_Release:
								#endif
								{
									IFEELIN F_Do
									(
										Obj, FM_Set,
										
										FA_Widget_Selected, TRUE,
										FA_Widget_Pressed, TRUE,
										
										TAG_DONE
									);

									_widget_set_button;

									IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_TICK | FF_EVENT_MOTION, 0);
								}
								break;
							}

							/* if the object is draggable, we force  the  'motion'
							event */

							if (_widget_is_draggable)
							{
							   LOD->flags |= FF_WIDGET_DRAGGING;

							   IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_MOTION, 0);
							}

							return FF_HandleEvent_Eat;
						}
					}
					else if (_widget_is_button)
					{
						_widget_clear_button;

/*** button-select-up **************************************************************************/
	
						/* our object was pressed, the mouse  button  is
						now released. first we change object state */
		
						if (_widget_is_selected)
						{
						   IFEELIN F_Set(Obj, FA_Widget_Selected, FALSE);
						}
						 
						/* It's time to set the 'Pressed'  attribute  to
						FALSE.  If  the  mouse  is outside the widget we
						disable to notification. */

						if (F_IS_INSIDE(fev->MouseX,_area_x, _area_x2) &&
							F_IS_INSIDE(fev->MouseY,_area_y, _area_y2))
						{
						   	IFEELIN F_Set(Obj, FA_Widget_Pressed, FALSE);
						}
						else
						{
							IFEELIN F_Do(Obj, FM_Set,

								FA_NoNotify, TRUE,
								FA_Widget_Pressed, FALSE,

								TAG_DONE);
						}
							
						/* remove 'tick' and 'motion' from the requested
						events */
		
						IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_TICK | FF_EVENT_MOTION);

						return FF_HandleEvent_Eat;
					}
				}

				if (!(FF_EVENT_BUTTON_DOWN & fev->Flags))
				{
					if (_widget_is_draggable)
					{
						IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_MOTION | FF_EVENT_TICK);
					}
				}
			}
			break;
//+
///event:motion
			case FF_EVENT_MOTION:
			{
			   
			   /* if the object is draggable we launch the DnD operation  on
			   the  first  mouve  of  the  mouse.  Otherwise,  we toggle the
			   selected state of the object depending on the mouse being  in
			   or out of it. */
	
			   if (_widget_is_draggable)
			   {
					IFEELIN F_Do
					(
						Obj, FM_Set,
						
						FA_Widget_Selected, FALSE,
						FA_Widget_Pressed, FALSE,
						FA_NoNotify, TRUE,
						
						TAG_DONE
					);

					IFEELIN F_Do(_area_app, FM_Application_PushMethod, Obj, FM_Widget_DnDDo, 2, fev->MouseX, fev->MouseY);
			   
					IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_TICK | FF_EVENT_MOTION);

					return FF_HandleEvent_Eat;
				}
				else if (_widget_is_button)
				{
					if (F_IS_INSIDE(fev->MouseX, _area_x, _area_x2) &&
						F_IS_INSIDE(fev->MouseY, _area_y, _area_y2))
					{

						/* inside area box */

						if (_widget_isnt_selected)
						{
							IFEELIN F_Set(Obj, FA_Widget_Selected, TRUE);
						}

						if (_widget_isnt_pressed)
						{
							IFEELIN F_Do(Obj, FM_Set,

								FA_NoNotify, TRUE,
								FA_Widget_Pressed, TRUE,

								TAG_DONE);
						}
					}
					else
					{

						/* outside area box */

						if (_widget_is_selected)
						{
							IFEELIN F_Set(Obj, FA_Widget_Selected, FALSE);
						}

						if (_widget_is_pressed)
						{
							IFEELIN F_Do(Obj, FM_Set,

								FA_NoNotify, TRUE,
								FA_Widget_Pressed, FALSE,

								TAG_DONE);
						}
					}
				}
			}
			break;
//+
///event:tick
			case FF_EVENT_TICK:
			{
				if (_widget_is_pressed)
				{
					IFEELIN F_Set(Obj, FA_Widget_PressedTick, TRUE);
				}
			}
			break;
//+
		}
	}
	return 0;
}
//+

///Widget_DnDDo
F_METHODM(void,Widget_DnDDo,FS_Widget_DnDDo)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	FObject appserver = IFEELIN F_SharedOpen("AppServer");

/* 	FIXME-060812

	Use WinServer as soon as possible

*/

	if (appserver == NULL)
	{
		return;
	}

	IFEELIN F_Do(appserver, (uint32) "FM_AppServer_DnDHandle", Obj, &_area_box, _area_render);

	IFEELIN F_SharedClose(appserver);
}
//+
///Widget_DnDQuery
F_METHODM(FObject,Widget_DnDQuery,FS_Widget_DnDQuery)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if ((FF_WIDGET_DROPABLE & LOD->flags) &&
		 _widget_isnt_disabled &&
		 _area_is_drawable && (Obj != Msg->Source))
	{
		if (Msg->MouseX >= _area_x && Msg->MouseX <= _area_x2 &&
			Msg->MouseY >= _area_y && Msg->MouseY <= _area_y2)
		{
			Msg->Box->x = _area_x; Msg->Box->w = _area_w;
			Msg->Box->y = _area_y; Msg->Box->h = _area_h;

			return Obj;
		}
	}

	return NULL;
}
//+
///Widget_DnDBegin
F_METHODM(void,Widget_DnDBegin,FS_Widget_DnDBegin)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	DnD.Pen = FV_Pen_Shine;
	DnD.Way = 1;

	if (_area_render != NULL)
	{
		struct RastPort *rp = _area_rp;

		_APen(_area_pens[DnD.Pen]);
		_Move(_area_x - 1,_area_y - 1);
		_Draw(_area_x2 + 1,_area_y - 1);
		_Draw(_area_x2 + 1,_area_y2 + 1);
		_Draw(_area_x - 1,_area_y2 + 1);
		_Draw(_area_x - 1,_area_y - 1);
	}
}
//+
///Widget_DnDFinish
F_METHODM(void,Widget_DnDFinish,FS_Widget_DnDFinish)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render != NULL)
	{
		#ifdef F_NEW_GLOBALCONNECT
		FObject parent = _element_parent;
		#else
		FObject parent = _area_parent;
		#endif

		struct Region *region;

		uint32 x1 = _area_x - 1, x2 = x1 + _area_w + 1;
		uint32 y1 = _area_y - 1, y2 = y1 + _area_h + 1;

		if ((region = IGRAPHICS NewRegion()) != NULL)
		{
			FRect rect;

			rect.x1 = x1; rect.x2 = x2;
			rect.y1 = y1; rect.y2 = y2; IGRAPHICS OrRectRegion(region, (struct Rectangle *) &rect);

			rect.x1 += 1; rect.x2 -= 1;
			rect.y1 += 1; rect.y2 -= 1; IGRAPHICS ClearRectRegion(region, (struct Rectangle *) &rect);

			IFEELIN F_Do(parent, FM_Area_Erase, region, FF_Erase_Region);

			IGRAPHICS DisposeRegion(region);
		}
		else
		{
			IFEELIN F_Erase(parent,x1,y1,x2,y1,0);
			IFEELIN F_Erase(parent,x2,y1,x2,y2,0);
			IFEELIN F_Erase(parent,x1,y2,x2,y2,0);
			IFEELIN F_Erase(parent,x1,y1,x1,y2,0);
		}
	}
}
//+
///Widget_DnDReport
F_METHODM(void,Widget_DnDReport,FS_Widget_DnDReport)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Update == FV_Widget_DnDReport_Timer)
	{
		if (_area_render != NULL)
		{
			struct RastPort *rp = _area_rp;
			struct Region *region;
			
			APTR clip = NULL;

			int16 x1 = _area_x, x2 = x1 + _area_w - 1;
			int16 y1 = _area_y, y2 = y1 + _area_h - 1;

			DnD.Pen += DnD.Way;

			/* Rendering are clipped to preserve the drag bob */

			if ((region = IGRAPHICS NewRegion()) != NULL)
			{
				FRect r;

				r.x1 = x1 - 1; r.x2 = x2 + 1;
				r.y1 = y1 - 1; r.y2 = y2 + 1;

				IGRAPHICS OrRectRegion(region,(struct Rectangle *) &r);

				r.x1 = Msg->DragBox->x ; r.x2 = r.x1 + Msg->DragBox->w - 1;
				r.y1 = Msg->DragBox->y ; r.y2 = r.y1 + Msg->DragBox->h - 1;

				IGRAPHICS XorRectRegion(region,(struct Rectangle *) &r);

				clip = (APTR) IFEELIN F_Do(_area_render, FM_Render_AddClipRegion, region);

				IGRAPHICS DisposeRegion(region);
			}

			_APen(_area_pens[DnD.Pen]);
			_Move(x1 - 1,y1 - 1);
			_Draw(x2 + 1,y1 - 1);
			_Draw(x2 + 1,y2 + 1);
			_Draw(x1 - 1,y2 + 1);
			_Draw(x1 - 1,y1 - 1);

			if (clip)
			{
				IFEELIN F_Do(_area_render,FM_Render_RemClip,clip);
			}

			if (DnD.Pen == FV_Pen_Shine)
			{
				DnD.Way = 1;
			}
			else if (DnD.Pen == FV_Pen_Dark)
			{
				DnD.Way = -1;
			}
		}
	}
}
//+
///Widget_DnDDrop
F_METHODM(uint32,Widget_DnDDrop,FS_Widget_DnDDrop)
{
	return 0;
}
//+
