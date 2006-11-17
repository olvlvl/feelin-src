#include "Private.h"

//#define DB_DISPATCHEVENT
//#define DB_ADDEVENTHANDLER
//#define DB_REMEVENTHANDLER
//#define DB_RETHINK_LAYOUT
#define F_ENABLE_RETHINK_WITH_FORBID

///window_refresh
static void window_refresh(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_Draw(Obj, FF_Draw_Object);

	/* FIXME:? re-activate activeobject to draw its activate state */

	IFEELIN F_Set(Obj, FA_Window_ActiveObject, (uint32) LOD->ActiveObj);
}
//+
#if 0
///window_help
STATIC void window_help(FClass *Class, FObject Obj, FEvent *fev)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (fev->Code == FV_EVENT_HELP_OPEN)
	{
		if (!LOD->PopHelp)
		{
			STRPTR help = (STRPTR) IFEELIN F_Do(LOD->decorator,FM_BuildContextHelp,fev->MouseX,fev->MouseY);
			if (!help) help = (STRPTR) IFEELIN F_Do(LOD->root,FM_BuildContextHelp,fev->MouseX,fev->MouseY);

			if (help)
			{
				LOD->PopHelp = PopHelpObject,

					"FA_PopHelp_Text",     help,
					"FA_PopHelp_Window",   Obj,
					"FA_PopHelp_Open",     TRUE,

				End;
			}
		}
	}
	else if (fev->Code == FV_EVENT_HELP_CLOSE)
	{
		IFEELIN F_DisposeObj(LOD->PopHelp); LOD->PopHelp = NULL;
	}
}
//+
#endif

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///code_rethinkrequest

struct FS_HOOK_RETHINKREQUEST                   { FClass *Class; };

F_HOOKM(void,code_rethinkrequest,FS_HOOK_RETHINKREQUEST)
{
	struct FeelinClass *Class = Msg->Class;
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	bool32 resize=FALSE;

	uint16 prev_w = _area_w;
	uint16 prev_h = _area_h;

	struct Window *window = LOD->window;

	#ifdef DB_RETHINK_LAYOUT
	IFEELIN F_Log(0,"process rethink requests (%ld) forbid (%ld)", LOD->RethinkNest, IFEELIN F_Get(_area_render, FA_Render_Forbid));
	#endif

	while (LOD->RethinkNest != 0)
	{
		LOD->sys_flags &= ~FF_WINDOW_SYS_KNOWNMINMAX;

		window_resizable(Class, Obj);
		
		IFEELIN F_Do(Obj, FM_Area_AskMinMax);

		#ifdef DB_RETHINK_LAYOUT
		IFEELIN F_Log(0,"prev (%03ldx%03ld) now (%03ldx%03ld) min (%03ldx%03ld) max (%03ldx%03ld)",
			prev_w, prev_h, _area_w, _area_h, _area_minw, _area_minh, _area_maxw, _area_maxh);
		#endif

		/* 'RethinkNest' is decreased here to avoid unecessary  pushes  from
		FM_Window_RequestRethink */

		LOD->RethinkNest--;
	}

	_area_w = MAX(_area_w, _area_minw);
	_area_w = MIN(_area_w, _area_maxw);
	_area_h = MAX(_area_h, _area_minh);
	_area_h = MIN(_area_h, _area_maxh);

	if (((_area_minw == _area_maxw) && (_area_w != prev_w)) || (prev_w < _area_w))
	{
		resize = TRUE;
	}

	if (((_area_minh == _area_maxh) && (_area_h != prev_h)) || (prev_h < _area_h))
	{
		resize = TRUE;
	}

/**/

	if (window != NULL)
	{

		/* WindowLimits() doen't work correctly. MinMax values  need  to  be
		set directly before calling the function. */

		window->MinWidth  = _area_minw;
		window->MinHeight = _area_minh;
		window->MaxWidth  = _area_maxw;
		window->MaxHeight = _area_maxh;

		if (resize)
		{
			#ifdef DB_RETHINK_LAYOUT

			IFEELIN F_Log(0, "change window box (%ld : %ld, %ld x %ld)",

				(_area_w > window->Width) ? (_win_x - (_area_w - window->Width) / 2) : (_win_x + (window->Width - _area_w) / 2),
				(_area_h > window->Height) ? (_win_y - (_area_h - window->Height) / 2) : (_win_y + (window->Height - _area_h) / 2),

				_area_w, _area_h);

			#endif

			IINTUITION ChangeWindowBox
			(
				window,

				(_area_w > window->Width) ? (_win_x - (_area_w - window->Width) / 2) : (_win_x + (window->Width - _area_w) / 2),
				(_area_h > window->Height) ? (_win_y - (_area_h - window->Height) / 2) : (_win_y + (window->Height - _area_h) / 2),

				_area_w, _area_h
			);
		}

		IINTUITION WindowLimits(LOD->window, _area_minw, _area_minh, _area_maxw, _area_maxh);
	}

	#ifdef DB_RETHINK_LAYOUT
	IFEELIN F_Log(0, "layout window: (%ld x %ld)", _area_w, _area_h);
	#endif

	IFEELIN F_Do(Obj, FM_Area_Layout);

	#ifdef F_ENABLE_RETHINK_WITH_FORBID
	IFEELIN F_Set(_area_render, FA_Render_Forbid, FALSE);
	#endif

	if (resize)
	{
		return;
	}

/**/

	if ((FF_WINDOW_SYS_REFRESH_NEED & LOD->sys_flags) != 0)
	{
		#ifdef DB_RETHINK_LAYOUT
		IFEELIN F_Log(0,"redraw window");
		#endif

		IFEELIN F_Draw(Obj, FF_Draw_Update);
	}
	else if (resize == FALSE)
	{
		#ifdef DB_RETHINK_LAYOUT
		IFEELIN F_Log(0,"redraw root");
		#endif

		if (FF_WINDOW_SYS_COMPLEX & LOD->sys_flags)
		{
			IFEELIN F_Draw(LOD->root, FF_Draw_Damaged | FF_Draw_Object);
		}
		else
		{
			IFEELIN F_Draw(LOD->root, FF_Draw_Object);
		}
	}
	else
	{
		#ifdef DB_RETHINK_LAYOUT
		IFEELIN F_Log(0, "nothing to redraw ??");
		#endif
	}
}
//+

/************************************************************************************************
*** Public **************************************************************************************
************************************************************************************************/

///window_collect_events
bits32 window_collect_events(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct in_EventHandler *handler;

	bits32 events = FF_EVENT_KEY | FF_EVENT_BUTTON | FF_EVENT_WINDOW;

	for (handler = (struct in_EventHandler *) LOD->event_handlers_list.Head ; handler ; handler = (struct in_EventHandler *) handler->node.Next)
	{
		events |= handler->events;
	}

	LOD->events = events;

	if ((FF_WINDOW_SYS_NOTIFY_EVENTS & LOD->sys_flags) != 0)
	{
		IFEELIN F_Set(Obj, FA_Window_Events, events);
	}

	return events;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Window_DispatchEvent
F_METHODM(bool32,Window_DispatchEvent,FS_Window_DispatchEvent)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FEvent *fev = Msg->Event;
/*
	#ifdef DB_DISPATCHEVENT
	IFEELIN F_Log(0,"Class 0x%08lx (%04lx - %04lx) - Key %02lx Decoded (%lc) - Mouse (%3ld : %3ld)",fev->Class,fev->Code,fev->Qualifier,fev->Key,(fev->DecodedChar) ? fev->DecodedChar : '#', fev->MouseX, fev->MouseY);
	#endif
*/
	#if 0
		
	if (fev->Class == FF_EVENT_HELP)
	{
		#ifdef DB_DISPATCHEVENT
		IFEELIN F_Log(0,"HELP ! (%ld) time (%ld:%ld) mouse (%ld:%ld)",fev->Code,fev->Seconds,fev->Micros,fev->MouseX,fev->MouseY);
		#endif
		
		window_help(Class,Obj,fev);
	}

	#endif
 
	if (fev->Class == FF_EVENT_WINDOW)
	{
		switch (fev->Code)
		{
			case FV_EVENT_WINDOW_SIZE:
			{
				//IFEELIN F_Log(0, "window:size (%ld x %ld) in (%ld x %ld)", LOD->window->Width, LOD->window->Height, _area_w, _area_h);

				if ((LOD->window->Width == _area_w) && (LOD->window->Height == _area_h))
				{
					//IFEELIN F_Log(0, "superfluous 'size' message");

					break;
				}

				_area_w = LOD->window->Width;
				_area_h = LOD->window->Height;

				#ifdef DB_DISPATCHEVENT
				IFEELIN F_Log(0,"event-window-layout >> BEGIN");
				#endif

				#if 1
				if (((FF_WINDOW_SYS_REFRESH_SIMPLE & LOD->sys_flags) == 0) &&
					((FF_WINDOW_SYS_LAYOUT_EVENT & LOD->sys_flags) == 0))
				{
					LOD->sys_flags |= FF_WINDOW_SYS_LAYOUT_EVENT;
				}
				#endif
 
				IFEELIN F_Do(Obj, FM_Area_Layout);

				/* a FV_EVENT_WINDOW_REFRESH event must come right after the
				FV_EVENT_WINDOW_SIZE. */
				
				#if 1
				if (FF_WINDOW_SYS_REFRESH_SIMPLE & LOD->sys_flags)
				{
					window_refresh(Class, Obj);
					
					LOD->sys_flags |= FF_WINDOW_SYS_REFRESH_DONE;
				}
				#endif

				#ifdef DB_DISPATCHEVENT
				IFEELIN F_Log(0,"event-window-layout >> DONE");
				#endif
			}
			break;

			case FV_EVENT_WINDOW_REFRESH:
			{
				#ifdef DB_DISPATCHEVENT
				IFEELIN F_Log(0,"event-window-refresh");
				#endif
 
				if (FF_WINDOW_SYS_LAYOUT_EVENT & LOD->sys_flags)
				{
					LOD->sys_flags &= ~FF_WINDOW_SYS_LAYOUT_EVENT;
				}
 
				if (FF_WINDOW_SYS_REFRESH_SIMPLE & LOD->sys_flags)
				{
					if ((FF_WINDOW_SYS_REFRESH_DONE & LOD->sys_flags) == 0)
					{
						#if 1

						IINTUITION BeginRefresh(LOD->window);

						window_refresh(Class, Obj);

						IINTUITION EndRefresh(LOD->window, TRUE);

						#else
						
						window_refresh(Class, Obj);
						
						#endif
					}
					else
					{
						LOD->sys_flags &= ~FF_WINDOW_SYS_REFRESH_DONE;
					}
				}
				else
				{
					IFEELIN F_Draw(Obj, FF_Draw_Object);
				}
			}
			break;

			case FV_EVENT_WINDOW_CLOSE:
			{
				#ifdef F_NEW_GLOBALCONNECT
				IFEELIN F_Do(_element_application, FM_Application_PushMethod, Obj, FM_Set, 2, FA_Window_CloseRequest,TRUE);
				#else
				IFEELIN F_Do(_area_parent, FM_Application_PushMethod, Obj, FM_Set, 2, FA_Window_CloseRequest,TRUE);
				#endif
			}
			break;

			case FV_EVENT_WINDOW_ACTIVE:
			{
				LOD->win_flags |= FF_WINDOW_WIN_ACTIVE;
				
				IFEELIN F_SuperDo(Class,Obj,FM_Set,FA_Window_Active,TRUE, TAG_DONE);
			}
			break;

			case FV_EVENT_WINDOW_INACTIVE:
			{
				LOD->win_flags &= ~FF_WINDOW_WIN_ACTIVE;
				
				IFEELIN F_SuperDo(Class,Obj,FM_Set,FA_Window_Active,FALSE, TAG_DONE);
			}
			break;

			case FV_EVENT_WINDOW_CHANGE:
			{
				_win_x = LOD->window->LeftEdge;
				_area_w = LOD->window->Width;
				_win_y = LOD->window->TopEdge;
				_area_h = LOD->window->Height;
			}
			break;
		}
	}

/*	FIXME-060910

	check event flags to skip list

*/

/** now we check event handlers linked to our object *******************************************/
	
	if (LOD->event_handlers_list.Head)
	{
		struct in_EventHandler *node;
		APTR next;

		if (LOD->ActiveObj)
		{
			next = LOD->event_handlers_list.Head;

			while ((node = IFEELIN F_NextNode(&next)) != NULL)
			{
				if (node->target == LOD->ActiveObj)
				{
					if (node->events & fev->Class)
					{
						if (FF_HandleEvent_Eat & IFEELIN F_ClassDoA(node->target_class, node->target, FM_Widget_HandleEvent, Msg))
						{
							return TRUE;
						}
					}

					//break; /* maybe we have more than 1 hanlder on the active object */
				}
			}
		}

		/** now we can proceed with the rest of the list **/

		next = LOD->event_handlers_list.Head;

		while ((node = IFEELIN F_NextNode(&next)) != NULL)
		{
			/*
			#ifdef DB_DISPATCHEVENT
			IFEELIN F_Log(0,"HANDLER 0x%08lx - %s{%08lx}",node,_object_classname(node->target),node->target);
			#endif
			*/

			if (node->events & fev->Class)
			{
				if (FF_HandleEvent_Eat & IFEELIN F_ClassDoA(node->target_class, node->target, FM_Widget_HandleEvent, Msg))
				{
					return TRUE;
				}
			}
		}

	}

/*** Nobody ate the event, we handle what we can ***********************************************/

	switch (fev->Key)
	{
		case FV_KEY_NEXTOBJ:
		case FV_KEY_DOWN:
		case FV_KEY_RIGHT:
		{
			IFEELIN F_Set(Obj,FA_Window_ActiveObject,FV_Window_ActiveObject_Next);
		}
		break;

		case FV_KEY_PREVOBJ:
		case FV_KEY_UP:
		case FV_KEY_LEFT:
		{
			IFEELIN F_Set(Obj,FA_Window_ActiveObject,FV_Window_ActiveObject_Prev);
		}
		break;

		case FV_KEY_CLOSEWINDOW:
		{
			#ifdef F_NEW_GLOBALCONNECT
			IFEELIN F_Do(_element_application, FM_Application_PushMethod, Obj,FM_Set,2,FA_Window_CloseRequest,TRUE);
			#else
			IFEELIN F_Do(_area_parent, FM_Application_PushMethod, Obj,FM_Set,2,FA_Window_CloseRequest,TRUE);
			#endif
		}
		break;
	}

	return TRUE;
}
//+
///Window_CreateEventHandler
F_METHOD(APTR, Window_CreateEventHandler)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, *item;

	bits32 events = 0;
	int8 priority = 0;
	FObject target = NULL;
	FClass *target_class = NULL;

	while ((item = IUTILITY NextTagItem(&Tags)) != NULL)
	switch (item->ti_Tag)
	{
		case FA_EventHandler_Events:    events = (bits32) item->ti_Data; break;
		case FA_EventHandler_Target:    target = (FObject) item->ti_Data; break;
		case FA_EventHandler_Class:     target_class = (FClass *) item->ti_Data; break;
		case FA_EventHandler_Priority:  priority = (int8) item->ti_Data; break;

		default:
		{
			IFEELIN F_Log(FV_LOG_DEV, "unknown attribute (0x%08lx) with data (0x%08lx) - cancelling handler", item->ti_Tag, item->ti_Data);

			return NULL;
		}
	}

	if (events && target)
	{
		struct in_EventHandler *handler = IFEELIN F_NewP(LOD->nodes_pool, sizeof (struct in_EventHandler));

		#ifdef DB_ADDEVENTHANDLER
		IFEELIN F_Log(0,"events (0x%08lx) target (%s{0x%08lx}) handler (0x%08lx)",events,_object_classname(target),target,handler);
		#endif

		if (handler != NULL)
		{
			struct in_EventHandler *prev;

			handler->priority = priority;
			handler->events = events;
			handler->target = target;
			handler->target_class = target_class ? target_class : _object_class(target);

			for (prev = (struct in_EventHandler *) LOD->event_handlers_list.Tail ; prev ; prev = (struct in_EventHandler *) prev->node.Prev)
			{
				if (handler->priority <= prev->priority) break;
			}

			IFEELIN F_LinkInsert(&LOD->event_handlers_list, (FNode *) handler, (FNode *) prev);

			window_collect_events(Class,Obj);

			/* we don't return the real pointer so that the  handler  cannot
			be disposed by mistake using F_Dispose() */

			return (APTR) (((uint32) handler) + sizeof (FNode));
		}
		else
		{
			IFEELIN F_Log(FV_LOG_USER,"unable to allocate event handler");
		}
	}

	return NULL;
}
//+
///Window_DeleteEventHandler
F_METHODM(uint32, Window_DeleteEventHandler, FS_Window_DeleteEventHandler)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Handler)
	{
		struct in_EventHandler *handler = (struct in_EventHandler *) (((uint32) Msg->Handler) - sizeof (FNode));
		struct in_EventHandler *node;

		for (node = (struct in_EventHandler *) LOD->event_handlers_list.Head ; node ; node = (struct in_EventHandler *) node->node.Next)
		{
			if (node == handler) break;
		}

		if (node)
		{
			IFEELIN F_LinkRemove(&LOD->event_handlers_list, (FNode *) node);
			IFEELIN F_Dispose(node);

			window_collect_events(Class,Obj);
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV, "unkown handler (0x%08lx)", Msg->Handler);
		}
	}

	return 0;
}
//+
///Window_AddChainable
F_METHODM(bool32,Window_AddChainable,FS_Window_AddChainable)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Widget != NULL)
	{
		FAreaPublic *apd = F_GET_AREA_PUBLIC(Msg->Widget);
		FWidgetPublic *wpd = F_GET_WIDGET_PUBLIC(Msg->Widget);

		if ((apd != NULL) && (wpd != NULL))
		{
			struct in_CycleNode *node;

			for (node = (struct in_CycleNode *) LOD->CycleChain.Head ; node ; node = node->Next)
			{
				if (node->Widget == Msg->Widget)
				{
					IFEELIN F_Log(FV_LOG_DEV, "%s{%08lx} already member of the cycle chain", _object_classname(Msg->Widget), Msg->Widget);

					return FALSE;
				}
			}

			node = IFEELIN F_NewP(LOD->nodes_pool, sizeof (struct in_CycleNode));

			if (node)
			{
				node->Widget = Msg->Widget;
				node->AreaPublic = apd;
				node->WidgetPublic = wpd;

				IFEELIN F_LinkTail(&LOD->CycleChain, (FNode *) node);

				return TRUE;
			}
		}
	}
	return FALSE;
}
//+
///Window_RemChainable
F_METHODM(bool32,Window_RemChainable,FS_Window_RemChainable)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct in_CycleNode *node;

	for (node = (struct in_CycleNode *) LOD->CycleChain.Head ; node ; node = node->Next)
	{
		if (node->Widget == Msg->Widget)
		{
			IFEELIN F_LinkRemove(&LOD->CycleChain, (FNode *) node);
			
			IFEELIN F_Dispose(node);

			return TRUE;
		}
	}
	return FALSE;
}
//+
///Window_Zoom
F_METHODM(void,Window_Zoom,FS_Window_Zoom)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->window != NULL)
	{
		switch (Msg->Zoom)
		{
			case FV_Window_Zoom_Toggle:
			{
				int16 x = LOD->window_zoom.x;
				int16 y = LOD->window_zoom.y;
				uint16 w = LOD->window_zoom.w;
				uint16 h = LOD->window_zoom.h;
				
				LOD->window_zoom.x = _win_x;
				LOD->window_zoom.y = _win_y;
				LOD->window_zoom.w = _area_w;
				LOD->window_zoom.h = _area_h;
				
				IINTUITION ChangeWindowBox
				(
					LOD->window,

					x, y, w, h
				);
			}
			break;
			
			/*FIXME: Max and Full must be different. Max is the whole screen
			but the menu bar, whil Full is the whole screen. */
 
			case FV_Window_Zoom_Max:
			case FV_Window_Zoom_Full:
			{
				IINTUITION ChangeWindowBox
				(
					LOD->window,
					
					0,0,
					
					LOD->window->WScreen->Width,
					LOD->window->WScreen->Height
				);
			}
			break;
		}
	}
}
//+
///Window_Depth
F_METHODM(void,Window_Depth,FS_Window_Depth)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->window)
	{
		switch (Msg->Depth)
		{
			case FV_Window_Depth_Toggle:
			{
				if (LOD->window->RPort->Layer->front)
				{
					IINTUITION WindowToFront(LOD->window);
				}
				else
				{
					IINTUITION WindowToBack(LOD->window);
				}
			}
			break;

			case FV_Window_Depth_Front:
			{
				IINTUITION WindowToFront(LOD->window);
			}
			break;

			case FV_Window_Depth_Back:
			{
				IINTUITION WindowToBack(LOD->window);
			}
			break;
		}
	}
}
//+
///Window_RequestRethink
F_METHOD(bool32,Window_RequestRethink)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef DB_RETHINK_LAYOUT
	IFEELIN F_Log(0,"nest (%ld)", LOD->RethinkNest);
	#endif

	if (_area_is_drawable)
	{
		LOD->RethinkNest++;

		if (LOD->RethinkNest == 1)
		{
		    #ifdef F_ENABLE_RETHINK_WITH_FORBID
			IFEELIN F_Set(_area_render, FA_Render_Forbid, TRUE);
			#endif

			#ifdef F_NEW_GLOBALCONNECT

			IFEELIN F_Do
			(
				_element_application, FM_Application_PushMethod,
				Obj, FM_CallHookEntry,2,
				F_FUNCTION_GATE(code_rethinkrequest), Class
			);

			#else

			IFEELIN F_Do
			(
				_area_parent, FM_Application_PushMethod,
				Obj, FM_CallHookEntry,2,
				F_FUNCTION_GATE(code_rethinkrequest), Class
			);

			#endif
		}

		return TRUE;
	}

	return FALSE;
}
//+

