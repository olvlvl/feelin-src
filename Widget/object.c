#include "Private.h"

//#define DB_VERBOSE_SAME_STATE

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///widget_select_state
void widget_select_state(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	uint32 state;

	if (_widget_is_disabled)
	{
		state = FV_Area_State_Ghost;
	}
	else if (_widget_is_pressed)
	{
		state = FV_Area_State_Touch;
	}
	else if (_widget_is_active)
	{
		state = FV_Area_State_Focus;
	}
	else
	{
		state = FV_Area_State_Neutral;
	}

	if (state != _area_state)
	{
		_area_state = state;

		if (_area_render != NULL)
		{
			IFEELIN F_Do(Obj, FM_Area_SetState, state);

			IFEELIN F_Do(Obj, FM_Area_Layout);
		}

		/* we don't redraw the object if it wasn't damaged, which may happen
		if the object have already took care of redrawing itself */

		if (_area_is_drawable && _area_is_damaged)
		{
			IFEELIN F_Draw(Obj, FF_Draw_Object);
		}
	}

	#ifdef DB_VERBOSE_SAME_STATE

	else
	{
		IFEELIN F_Log(0, "same state (%ld)", state);
	}

	#endif
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Widget_New
F_METHOD(uint32,Widget_New)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	#ifdef F_NEW_WIDGET_PUBLIC

	IFEELIN F_Do(Obj, FM_Get,

		FA_Element_PublicData, &(_widget_public ElementPublic),
		FA_Area_PublicData, &(_widget_public AreaPublic),

		TAG_DONE);

	#else
	
	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif
	F_SAVE_AREA_PUBLIC;
	
	#endif

	_widget_weight = 100;

	_widget_set_showable;
	_widget_set_chainable;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Widget_Chainable:
		{
			if (item.ti_Data)
			{
				_widget_set_chainable;
			}
			else
			{
				_widget_clear_chainable;
			}
		}
		break;
		
		case FA_Widget_Mode:
		{
		   LOD->mode = item.ti_Data;
		}
		break;

		case FA_Widget_Weight:
		{
			_widget_weight = item.ti_Data;
		}

		case FA_Widget_SetMin:
		{
			switch (item.ti_Data)
		   	{
				case FV_Widget_SetNone:
				{
					_widget_flags &= ~(FF_Widget_SetMinW | FF_Widget_SetMinH);
				}
				break;
			
				case FV_Widget_SetBoth:
				{
					_widget_flags |= (FF_Widget_SetMinW | FF_Widget_SetMinH);
				}
				break;
			
				case FV_Widget_SetWidth:
				{
					_widget_flags |= FF_Widget_SetMinW;
					_widget_flags &= ~FF_Widget_SetMinH;
				}
				break;
			
				case FV_Widget_SetHeight:
				{
					_widget_flags |= FF_Widget_SetMinH;
					_widget_flags &= ~FF_Widget_SetMinW;
				}
				break;
		   }
		}
		break;

		case FA_Widget_SetMax:
		{
		   	switch (item.ti_Data)
		   	{
				case FV_Widget_SetNone:
				{
					_widget_flags &= ~(FF_Widget_SetMaxW | FF_Widget_SetMaxH);
				}
				break;
			
				case FV_Widget_SetBoth:
				{
					_widget_flags |= (FF_Widget_SetMaxW | FF_Widget_SetMaxH);
				}
				break;
			
				case FV_Widget_SetWidth:
				{
					_widget_flags |= FF_Widget_SetMaxW;
					_widget_flags &= ~FF_Widget_SetMaxH;
				}
				break;
			
				case FV_Widget_SetHeight:
				{
					_widget_flags |= FF_Widget_SetMaxH;
					_widget_flags &= ~FF_Widget_SetMaxW;
				}
				break;
		   	}
		}
		break;
	}

	return F_SUPERDO();
}
//+
///Widget_Get
F_METHOD(uint32,Widget_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Widget_PublicData:      F_STORE(&LOD->public); break;
		case FA_Widget_Active:          F_STORE(_widget_is_active); break;
		case FA_Widget_Selected:        F_STORE(_widget_is_selected); break;
		case FA_Widget_Pressed:         F_STORE(_widget_is_pressed); break;
		case FA_Widget_Hidden:          F_STORE(LOD->hidden_count > 0); break;
		case FA_Widget_Disabled:        F_STORE(LOD->disabled_count > 0); break;
		case FA_Widget_Draggable:       F_STORE(_widget_is_draggable); break;
		case FA_Widget_Dropable:        F_STORE(_widget_is_dropable); break;
		case FA_Widget_Accel:           F_STORE(LOD->accel); break;
		case FA_Widget_Chainable:       F_STORE(_widget_is_chainable); break;
		case FA_Widget_Align:           F_STORE(_widget_align); break;
	}

	return F_SUPERDO();
}
//+
///Widget_Set
F_METHOD(uint32,Widget_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	bool32 set_state = FALSE;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Widget_Active:
		{
//			  IFEELIN F_Log(0, "FA_Widget_Active (%ld)", item.ti_Data);

			if (item.ti_Data != 0)
			{
				_widget_set_active;
			}
			else
			{
				_widget_clear_active;
			}
			
			set_state = TRUE;
		}
		break;

		case FA_Widget_Selected:
		{
			if (item.ti_Data != 0)
			{
				_widget_set_selected;
			}
			else
			{
				_widget_clear_selected;
			}

			//set_state = TRUE;

			if (_area_is_drawable)
			{
				IFEELIN F_Draw(Obj, FF_Draw_Object);
			}
		}
		break;

		case FA_Widget_Pressed:
		{
			if (item.ti_Data != 0)
			{
				_widget_set_pressed;
			}
			else
			{
				_widget_clear_pressed;
			}

			set_state = TRUE;
		}
		break;

///FA_Widget_Hidden
		case FA_Widget_Hidden:
		{
			uint32 method;

			if (~(FF_Hidden_Check | FF_Hidden_NoNesting) & item.ti_Data)
			{
				if (FF_Hidden_Check & item.ti_Data)
				{
					if (LOD->hidden_count < 1) // shown
					{
						if (FF_Hidden_NoNesting & item.ti_Data)
						{
							LOD->hidden_count = 1;
						}
						else
						{
							LOD->hidden_count++;
						}
					}
					else break;
				}
				else if (FF_Hidden_NoNesting & item.ti_Data)
				{
					LOD->hidden_count = 1;
				}
				else
				{
					LOD->hidden_count++;
				}
			}
			else
			{
				if (FF_Hidden_Check & item.ti_Data)
				{
					if (LOD->hidden_count > 0) // hidden
					{
						if (FF_Hidden_NoNesting & item.ti_Data)
						{
							LOD->hidden_count = 0;
						}
						else
						{
							LOD->hidden_count--;
						}
					}
					else break;
				}
				else if (FF_Hidden_NoNesting & item.ti_Data)
				{
					LOD->hidden_count = 0;
				}
				else
				{
					LOD->hidden_count--;
				}
			}

			if (LOD->hidden_count == 1)
			{
				_widget_clear_showable;

				method = FM_Area_Hide;
			}
			else if (LOD->hidden_count == 0)
			{
				_widget_set_showable;

				method = FM_Area_Show;
			}
			else break;

			if (_area_render != NULL)
			{
				IFEELIN F_Do(Obj, method);

				IFEELIN F_Do(_area_win, FM_Window_RequestRethink, Obj);
			}
		}
		break;
//+
///FA_Widget_Disabled
		case FA_Widget_Disabled:
		{
			if (~(FF_Disabled_Check | FF_Disabled_NoNesting) & item.ti_Data)
			{
				if (FF_Disabled_Check & item.ti_Data)
				{
					if (LOD->disabled_count < 1) // enabled
					{
						if (FF_Disabled_NoNesting & item.ti_Data)
						{
							LOD->disabled_count = 1;
						}
						else
						{
							LOD->disabled_count++;
						}
					}
					else break;
				}
				else if (FF_Disabled_NoNesting & item.ti_Data)
				{
					LOD->disabled_count = 1;
				}
				else
				{
					LOD->disabled_count++;
				}
			}
			else
			{
				if (FF_Disabled_Check & item.ti_Data)
				{
					if (LOD->disabled_count > 0) // disabled
					{
						if (FF_Disabled_NoNesting & item.ti_Data)
						{
							LOD->disabled_count = 0;
						}
						else
						{
							LOD->disabled_count--;
						}
					}
					else break;
				}
				else if (FF_Disabled_NoNesting & item.ti_Data)
				{
					LOD->disabled_count = 0;
				}
				else
				{
					LOD->disabled_count--;
				}
			}
 
			if (LOD->disabled_count == 1)
			{
				_widget_set_disabled;

				widget_try_rem_handler(Class,Obj);
			}
			else if (LOD->disabled_count == 0)
			{
				_widget_clear_disabled;

				widget_try_add_handler(Class,Obj);
			}
			else break;

			set_state = TRUE;

			//widget_select_state(Class, Obj);
			//IFEELIN F_Draw(Obj, FF_Draw_Object);
		}
		break;
//+
	  
		case FA_Widget_Draggable:
		{
			if (item.ti_Data != 0)
			{
				_widget_set_draggable;
			}
			else
			{
				_widget_clear_draggable;
			}
		}
		break;
	  
		case FA_Widget_Dropable:
		{
			if (item.ti_Data != 0)
			{
				_widget_set_dropable;
			}
			else
			{
				_widget_clear_dropable;
			}
		}
		break;
	  
		case FA_Widget_Accel:
		{
			LOD->accel = (uint8) item.ti_Data;
		}
		break;
/*
		case FA_ContextHelp:
		{
			LOD->context_help = (STRPTR) item.ti_Data;
			
			if (_area_render)
			{
				if (LOD->context_help)
				{
					IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_HELP, 0);
				}
				else
				{
					IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_HELP);
				}
			}
		}
		break;
		
		case FA_ContextMenu:
		{
			LOD->context_menu = (FObject) item.ti_Data;
		}
		break;
*/
		case FA_Widget_Align:
		{
			_widget_align = item.ti_Data;

			if (_area_is_drawable)
			{
				IFEELIN F_Do(_area_win, FM_Window_RequestRethink);
			}
		}
		break;
	}

	if (set_state)
	{
		widget_select_state(Class, Obj);
	}

	return F_SUPERDO();
}
//+

///Widget_LoadPersistentAttributes
F_METHOD(FPDRHandle *, Widget_LoadPersistentAttributes)
{
	FPDRHandle *handle = (FPDRHandle *) F_SUPERDO();

	if (handle)
	{
		FPDRAttribute *attr;

		for (attr = handle->Attributes ; attr ; attr = attr->Next)
		{
			if (attr->Atom == F_ATOM(SELECTED))
			{
				if (F_PDRDOCUMENT_GET_NUMERIC(attr))
				{
					IFEELIN F_Set(Obj, FA_Widget_Selected, TRUE);
				}
				else
				{
					IFEELIN F_Set(Obj, FA_Widget_Selected, FALSE);
				}
			}
		}
	}

	return handle;
}
//+
///Widget_SavePersistentAttributes
F_METHOD(FPDRHandle *, Widget_SavePersistentAttributes)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FPDRHandle *handle = (FPDRHandle *) F_SUPERDO();

	if (handle)
	{
		FPDRAttribute *attr;

		for (attr = handle->Attributes ; attr ; attr = attr->Next)
		{
			if (attr->Atom == F_ATOM(SELECTED))
			{
				F_PDRDOCUMENT_SET_TYPED(attr, _widget_is_selected, FV_TYPE_BOOLEAN);
			}
		}
	}

	return handle;
}
//+

