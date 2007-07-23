#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///list_popstring_delete
void list_popstring_delete(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0,FF_EVENT_TICK | FF_EVENT_WINDOW);

	IFEELIN F_DisposeObj(LOD->pop_window);

	LOD->pop_window = NULL;
	LOD->pop_string = NULL;
}
//+
///list_popstring_handle
STATIC bits32 list_popstring_handle(FClass *Class,FObject Obj,FEvent *fev)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	bits32 rc = 0;

	#if 0
	if (fev->Class != FF_EVENT_TICK)
	{
		IFEELIN F_Log(0,"time (0x%08lx:0x%08lx) class (0x%08lx) code (0x%04lx) flags (0x%08lx) key (%ld) delay (%ld)",fev->Seconds,fev->Micros,fev->Class,fev->Code,fev->Flags,fev->Key,LOD->pop_delay);
	}
	#endif

	if (fev->Class == FF_EVENT_TICK)
	{
		if ((--LOD->pop_delay) == 0)
		{
			list_popstring_delete(Class,Obj);
		}
		else
		{
			rc = IFEELIN F_Do(LOD->pop_string, FM_Widget_HandleEvent, fev);
		}
	}
	else if (fev->Class == FF_EVENT_WINDOW)
	{
		if (fev->Code == FV_EVENT_WINDOW_INACTIVE)
		{
			list_popstring_delete(Class,Obj);
		}
	}
	else if (fev->Key || fev->DecodedChar)
	{
		if (fev->Key == FV_KEY_PRESS)
		{
			list_popstring_delete(Class,Obj);

			rc = FF_HandleEvent_Eat;
		}
		else
		{
			STRPTR str;

			LOD->pop_delay = DEF_LIST_POPSTRING_DELAY;

			rc = IFEELIN F_Do(LOD->pop_string, FM_Widget_HandleEvent, fev);

			str = (STRPTR) IFEELIN F_Get(LOD->pop_string, (uint32) "FA_String_Contents");

			if (str)
			{
				uint32 pos = IFEELIN F_Do(Obj, F_METHOD_ID(FINDSTRING), str);

				if (pos != FV_List_FindString_NotFound)
				{
					IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(ACTIVE), pos);
				}
			}
		}
	}

	return rc;
}
//+
///list_popstring_create
STATIC void list_popstring_create(FClass *Class,FObject Obj,FEvent *fev)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	LOD->pop_window = WindowObject,

		FA_Window_Borderless, TRUE,
		FA_Window_Active, FALSE,
		FA_Window_Open, TRUE,
		FA_Window_Right, IFEELIN F_Get(_area_win, FA_Window_Left) + _area_x2 - 10,
		FA_Window_Bottom, IFEELIN F_Get(_area_win, FA_Window_Top) + _area_y2 - 10,
		FA_Element_Style, "border-frame: sweet; padding: 5px;",

		Child, LOD->pop_string = StringObject,

			"Displayable",10,
//			  "Justify", FV_String_Right,

		End,

	End;

	if (LOD->pop_window)
	{
		IFEELIN F_Set(LOD->pop_window, FA_Window_ActiveObject, (uint32) LOD->pop_string);

		if (IFEELIN F_Do(_area_app, FM_AddMember, LOD->pop_window, FV_AddMember_Head))
		{
			LOD->pop_delay = DEF_LIST_POPSTRING_DELAY;

			IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_TICK | FF_EVENT_WINDOW, 0);

			list_popstring_handle(Class,Obj,fev);
		}
		else
		{
			#if 0
			IFEELIN F_Log(FV_LOG_DEV, "Unable to add pop window to application");
			#endif

			list_popstring_delete(Class,Obj);
		}
	}
	#if 0
	else
	{
		IFEELIN F_Log(0, "Unable to create pop window");
	}
	#endif
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///List_HandleEvent
F_METHODM(uint32,List_HandleEvent,FS_Widget_HandleEvent)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FEvent *fev = Msg->Event;
	
	if ((FF_LIST_ACTIVE & LOD->Flags) && (fev->Key != FV_KEY_NONE))
	{
		uint32 active = 0;

		switch (fev->Key)
		{
			case FV_KEY_UP:         active = FV_List_Active_Up;         break;
			case FV_KEY_DOWN:       active = FV_List_Active_Down;       break;
			case FV_KEY_STEPUP:     active = FV_List_Active_PageUp;     break;
			case FV_KEY_STEPDOWN:   active = FV_List_Active_PageDown;   break;
			case FV_KEY_TOP:        active = FV_List_Active_Top;        break;
			case FV_KEY_BOTTOM:     active = FV_List_Active_Bottom;     break;
		}
							
		if (active)
		{
			LOD->Flags |= FF_LIST_ACTIVATION_INTERNAL;

			IFEELIN F_Do(Obj,FM_Set,

				F_ATTRIBUTE_ID(ACTIVE),     active,
				F_ATTRIBUTE_ID(ACTIVATION), FV_List_Activation_Key,

			TAG_DONE);
			
			list_popstring_delete(Class,Obj);

			return FF_HandleEvent_Eat;
		}
	}
 
	if (LOD->pop_string)
	{
		if (list_popstring_handle(Class,Obj,fev) == FF_HandleEvent_Eat)
		{
			return FF_HandleEvent_Eat;
		}
	}
 
	switch (fev->Class)
	{
		case FF_EVENT_KEY:
		{
			if ((FF_LIST_ACTIVE & LOD->Flags) || (fev->DecodedChar && F_IS_INSIDE(fev->MouseX,_area_x,_area_x2) && F_IS_INSIDE(fev->MouseY,_area_y,_area_y2)))
			{
				if (fev->DecodedChar && !(FF_EVENT_KEY_UP & fev->Flags))
				{
					list_popstring_create(Class,Obj,fev);
					
					return FF_HandleEvent_Eat;
				}
			}
		}
		break;
		
		case FF_EVENT_BUTTON:
		{
			if ((fev->Code == FV_EVENT_BUTTON_SELECT) && (FF_EVENT_BUTTON_DOWN & fev->Flags) && (LOD->LineCount))
			{
				if (fev->MouseX >= _area_cx &&
					fev->MouseX <= _area_x2 &&
					fev->MouseY >= _area_cy &&
					fev->MouseY <= _area_y2)
				{
					if (fev->MouseY <= _area_cy + LOD->TitleBarH)
					{
						IFEELIN F_Log(0,"mouse-button-select on title bar, should sort column");
						
						return FF_HandleEvent_Eat;
					}
					else if (fev->MouseY <= _area_cy + LOD->TitleBarH + LOD->Visible * LOD->MaximumH - 1)
					{
						uint32 pos = ((LOD->First) ? LOD->First->Position : 0) + (fev->MouseY - _area_cy - LOD->TitleBarH) / LOD->MaximumH;

						LOD->Flags |= FF_LIST_ACTIVATION_INTERNAL;
						
						if (LOD->Active && (LOD->Active->Position == pos) && (FF_EVENT_REPEAT & fev->Flags))
						{
							IFEELIN F_Do(Obj,FM_Set,

								F_ATTRIBUTE_ID(ACTIVE),     pos,
								F_ATTRIBUTE_ID(ACTIVATION), FV_List_Activation_DoubleClick,

							TAG_DONE);
						}
						else
						{
							IFEELIN F_Do(Obj,FM_Set,

								F_ATTRIBUTE_ID(ACTIVE),     pos,
								F_ATTRIBUTE_ID(ACTIVATION), FV_List_Activation_Click,

							TAG_DONE);
						}
 
						return FF_HandleEvent_Eat;
					}
				}
			}
			else if (fev->Code == FV_EVENT_BUTTON_WHEEL)
			{
				if (fev->MouseX >= _area_x &&
					fev->MouseX <= _area_x2 &&
					fev->MouseY >= _area_y &&
					fev->MouseY <= _area_y2)
				{
					uint32 pos = (LOD->First) ? LOD->First->Position : 0;
 
					if (FF_EVENT_BUTTON_DOWN & fev->Flags)
					{
						IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), pos + LOD->steps);
					}
					else
					{
						IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), pos - LOD->steps);
					}
				}
			}
		}
		break;
	}
	return 0;
}
//+
