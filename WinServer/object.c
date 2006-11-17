#include "Private.h"

//#define DB_ADDMEMBER
//#define DB_REMMEMBER
//#define DB_MODIFY
//#define DB_MODIFY_LOCK
//#define DB_POST_CLEAN

#define F_ENABLE_POSTCLEAN

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/
		
#ifdef F_ENABLE_POSTCLEAN
///code_postclean

struct in_PostCleanData                         { struct LocalObjectData *LOD; FObject Target; };
 
F_HOOKM(uint32,code_postclean,FS_Application_HookPostClean)
{
	struct in_PostCleanData *data = Hook->h_Data;
	
	if (F_IS_SENDED_MSG(data->LOD,Msg->Post))
	{
		#ifdef DB_POST_CLEAN
		IFEELIN F_Log(0,"post_clean: (0x%08lx) target (0x%08lx ?= 0x%08lx)",
			Msg->Post,data->Target,_event(Msg->Post)->Target);
		#endif

		if (_event(Msg->Post)->Target == data->Target)
		{
			#ifdef DB_POST_CLEAN
			IFEELIN F_Log(0,"post_clean: window (0x%08lx)",data);
			#endif
			
			return FF_Application_PostClean_Eat;
		}
	}
	#ifdef DB_POST_CLEAN
	else
	{
		IFEELIN F_Log(0,"post_clean: post (0x%08lx)",Msg->Post);
	}
	#endif
	return 0;
}
//+
#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///WinServer_New
F_METHOD(uint32,WinServer_New)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	#ifdef F_ENABLE_EVENT_KEY

/// KeyStringTable
	STATIC STRPTR KeyStringTable[] =
	{
		"return",
		"upstroke return",
		"-repeat up",
		"-repeat down",
		"-repeat shift up",
		"-repeat shift down",
		"alt up",
		"alt down",
		"-repeat left",
		"-repeat right",
		"-repeat shift left",
		"-repeat shift right",
		"alt left",
		"alt right",
		"-repeat backspace",
		"-repeat del",
		"alt backspace",
		"alt del",
		"shift backspace",
		"shift del",
		"-repeat tab",
		"-repeat shift tab",
		"control return",
		"esc",

		 NULL
	};
//+

	uint32 i;

	for (i = 1 ; i < FV_KEY_COUNT ; i++)
	{
		ICOMMODITIES ParseIX(KeyStringTable[i - 1], &LOD->keys[i - 1]);
	}
	
	#endif /* F_ENABLE_EVENT_KEY */

	LOD->pool = IFEELIN F_CreatePool(1,
	
		FA_Pool_ItemSize, 1024,
		FA_Pool_Name, "winserver-data",
		
		TAG_DONE);
		
	if (LOD->pool)
	{
		LOD->Server = Obj;

		LOD->Thread = ThreadObject,

			"FA_Thread_Entry",    Thread_Main,
			"FA_Thread_Name",     "winserver.thread",
			"FA_Thread_UserData", LOD,

		End;

		if (LOD->Thread)
		{
			return F_SUPERDO();
		}
	}
	return 0;
}
//+
///WinServer_Dispose
F_METHOD(void,WinServer_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_DisposeObj(LOD->Thread); LOD->Thread = NULL;
	
	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);
 
	while (LOD->family.Head)
	{
		IFEELIN F_Do(Obj, FM_RemMember, ((struct in_FamilyNode *) LOD->family.Head)->Public.Object);
	}
	
	IFEELIN F_DeletePool(LOD->pool); LOD->pool = NULL;

	IFEELIN F_Do(Obj, FM_Unlock);
 
	F_SUPERDO();
}
//+
///WinServer_AddMember

enum    {

		FV_TAG_LEFT,
		FV_TAG_TOP,
		FV_TAG_WIDTH,
		FV_TAG_HEIGHT,
		FV_TAG_MINWIDTH,
		FV_TAG_MINHEIGHT,
		FV_TAG_MAXWIDTH,
		FV_TAG_MAXHEIGHT,
		FV_TAG_NOTIFYDEPTH,
		FV_TAG_AUTOADJUST,
		FV_TAG_FLAGS,
		FV_TAG_BACKFILL,
		FV_TAG_CUSTOMSCREEN,
		FV_TAG_ACTIVATE,
		FV_TAG_BACKDROP,
		FV_TAG_SIMPLEREFRESH,
		
		FV_TAG_BORDERLESS,
		
		FV_TAG_SIZEGADGET,
		FV_TAG_DRAGBAR,
		FV_TAG_DEPTHGADGET,
		FV_TAG_CLOSEGADGET,
		
		FV_TAG_TERMINATOR,
		FV_TAG_COUNT
		
		};

 
F_METHODM(struct Window *, WinServer_AddMember, FS_AddMember)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	
	struct in_FamilyNode *node;

	FElementPublic *element_public = NULL;
	FAreaPublic *area_public = NULL;

	FObject window = Msg->Orphan;

	if (window == NULL)
	{
		return NULL;
	}

	IFEELIN F_Do(window, FM_Get,

		FA_Element_PublicData, &element_public,
		FA_Area_PublicData, &area_public,

		TAG_DONE);

	if ((element_public == NULL) || (area_public == NULL))
	{
		IFEELIN F_Log(FV_LOG_DEV, "child must be a subclass of the Area class !");

		return NULL;
	}
	
	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);
	
	for (node = (struct in_FamilyNode *) LOD->family.Head ; node ; node = (struct in_FamilyNode *) node->Public.Next)
	{
		if (node->Public.Object == window)
		{
			IFEELIN F_Log(FV_LOG_DEV, "%s{%lx} is already a member !",_object_classname(window), window);
			
			IFEELIN F_Do(Obj, FM_Unlock);

			return node->window;
		}
	}
	
	node = IFEELIN F_NewP(LOD->pool, sizeof (struct in_FamilyNode));
	
	if (node)
	{
		node->Public.Object = window;
		node->events_notify = IFEELIN F_Do(window, FM_Notify, FA_Window_Events,FV_Notify_Always, Obj, F_METHOD_ID(MODIFYEVENTS), 2, FV_Notify_Value, Msg->Orphan);

		#ifdef F_NEW_GLOBALCONNECT

		node->application = element_public->Application;

		#else

		node->application = (FObject) IFEELIN F_Get(window, FA_Parent);

		#endif

		if (node->events_notify)
		{
			FObject dsp = (FObject) IFEELIN F_Get(node->application, FA_Application_Display);
			struct Screen *scr = (struct Screen *) IFEELIN F_Get(dsp, (uint32) "FA_Display_Screen");
	
			if (scr)
			{
				struct TagItem *tags = IFEELIN F_NewP(LOD->pool,sizeof (struct TagItem) * FV_TAG_COUNT);
					
				if (tags)
				{
					uint32 deco = 0;
					#ifdef F_ENABLE_ACTIVABLE
					uint32 activable = TRUE;
					#endif
		 
					tags[FV_TAG_LEFT].ti_Tag           = WA_Left;
					tags[FV_TAG_TOP].ti_Tag            = WA_Top;
					tags[FV_TAG_WIDTH].ti_Tag          = WA_Width;
					tags[FV_TAG_HEIGHT].ti_Tag         = WA_Height;
					tags[FV_TAG_MINWIDTH].ti_Tag       = WA_MinWidth;
					tags[FV_TAG_MINHEIGHT].ti_Tag      = WA_MinHeight;
					tags[FV_TAG_MAXWIDTH].ti_Tag       = WA_MaxWidth;
					tags[FV_TAG_MAXHEIGHT].ti_Tag      = WA_MaxHeight;
					tags[FV_TAG_NOTIFYDEPTH].ti_Tag    = WA_NotifyDepth;
					tags[FV_TAG_AUTOADJUST].ti_Tag     = WA_AutoAdjust;
					tags[FV_TAG_FLAGS].ti_Tag          = WA_Flags;
					tags[FV_TAG_BACKFILL].ti_Tag       = WA_BackFill;
					tags[FV_TAG_CUSTOMSCREEN].ti_Tag   = WA_CustomScreen;
					tags[FV_TAG_ACTIVATE].ti_Tag       = WA_Activate;
					tags[FV_TAG_BACKDROP].ti_Tag       = WA_Backdrop;
					tags[FV_TAG_SIMPLEREFRESH].ti_Tag  = WA_SimpleRefresh;
					tags[FV_TAG_BORDERLESS].ti_Tag     = WA_Borderless;

					tags[FV_TAG_SIZEGADGET].ti_Tag     = WA_SizeGadget;
					tags[FV_TAG_DRAGBAR].ti_Tag        = WA_DragBar;
					tags[FV_TAG_DEPTHGADGET].ti_Tag    = WA_DepthGadget;
					tags[FV_TAG_CLOSEGADGET].ti_Tag    = WA_CloseGadget;

					tags[FV_TAG_NOTIFYDEPTH].ti_Data   = TRUE;
					tags[FV_TAG_AUTOADJUST].ti_Data    = TRUE;
					tags[FV_TAG_FLAGS].ti_Data         = WFLG_SIZEBBOTTOM | WFLG_REPORTMOUSE | WFLG_RMBTRAP;
					tags[FV_TAG_BACKFILL].ti_Data      = (uint32) LAYERS_NOBACKFILL;
					tags[FV_TAG_CUSTOMSCREEN].ti_Data  = (uint32) scr;

					tags[FV_TAG_SIMPLEREFRESH].ti_Data = FALSE;

					IFEELIN F_Do(window, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(REFRESH_SIMPLE), NULL, &(tags[FV_TAG_SIMPLEREFRESH].ti_Data));

					IFEELIN F_Do(window, FM_Get,
					
						FA_Window_Active,       &tags[FV_TAG_ACTIVATE].ti_Data,
						FA_Window_Backdrop,     &tags[FV_TAG_BACKDROP].ti_Data,
						FA_Window_Borderless,   &tags[FV_TAG_BORDERLESS].ti_Data,
						FA_Window_Resizable,    &tags[FV_TAG_SIZEGADGET].ti_Data,
						FA_Window_GadDragbar,   &tags[FV_TAG_DRAGBAR].ti_Data,
						FA_Window_GadDepth,     &tags[FV_TAG_DEPTHGADGET].ti_Data,
						FA_Window_GadClose,     &tags[FV_TAG_CLOSEGADGET].ti_Data,
						FA_Window_Decorator,    &deco,
						
						#ifdef F_ENABLE_ACTIVABLE
						FA_Window_Activable,    &activable,
						#endif

						FA_Window_Left,         &tags[FV_TAG_LEFT].ti_Data,
						FA_Window_Top,          &tags[FV_TAG_TOP].ti_Data,
						FA_Window_Width,        &tags[FV_TAG_WIDTH].ti_Data,
						FA_Window_Height,       &tags[FV_TAG_HEIGHT].ti_Data,
						
						TAG_DONE);

					tags[FV_TAG_MINWIDTH].ti_Data 	= area_public->MinMax.MinW;
					tags[FV_TAG_MINHEIGHT].ti_Data 	= area_public->MinMax.MinH;
					tags[FV_TAG_MAXWIDTH].ti_Data	= area_public->MinMax.MaxW;
					tags[FV_TAG_MAXHEIGHT].ti_Data 	= area_public->MinMax.MaxH;

					#if 0
					IFEELIN F_Log(0, "box (%ld : %ld) (%ld x %ld)",
						
						tags[FV_TAG_LEFT].ti_Data,
						tags[FV_TAG_TOP].ti_Data,
						tags[FV_TAG_WIDTH].ti_Data,
						tags[FV_TAG_HEIGHT].ti_Data);
					#endif
						
					if (deco != NULL)
					{
						tags[FV_TAG_BORDERLESS].ti_Data    = TRUE;
						tags[FV_TAG_SIZEGADGET].ti_Tag     = TAG_IGNORE,
						tags[FV_TAG_DRAGBAR].ti_Tag        = TAG_IGNORE;
						tags[FV_TAG_DEPTHGADGET].ti_Tag    = TAG_IGNORE;
						tags[FV_TAG_CLOSEGADGET].ti_Tag    = TAG_IGNORE;
					}
									
					#ifdef F_ENABLE_ACTIVABLE
					
					#ifdef DB_ADDMEMBER
					IFEELIN F_Log(0,"object (0x%08lx) activable (%s)", window, activable ? "yes" : "no");
					#endif
					
					if (activable)
					{
						node->flags |= FF_NODE_ACTIVABLE;
					}
					#endif
					
					#if 0
					{
						uint32 i;
						
						for (i = 0 ; i < FV_TAG_COUNT ; i++)
						{
							IDOS_ FPrintf(FeelinBase->Console, "0x%08lx - 0x%08lx\n",tags[i].ti_Tag,tags[i].ti_Data);
						}
					}
					#endif

					if ((tags[FV_TAG_WIDTH].ti_Data < 4096) && (tags[FV_TAG_WIDTH].ti_Data > 5) &&
						(tags[FV_TAG_HEIGHT].ti_Data < 4096) && (tags[FV_TAG_HEIGHT].ti_Data > 5))
					{
						node->window = IINTUITION OpenWindowTagList(NULL, tags);
						
						if (node->window != NULL)
						{
							node->window->UserData = (uint8 *) node;
							
							IFEELIN F_LinkTail(&LOD->family, (FNode *) node);
							
							IFEELIN F_Do(Obj, FM_Unlock);
							
							return node->window;
						}
					}
					else
					{
						IFEELIN F_Log(FV_LOG_DEV, "suspicious dimensions: %ld x %ld", tags[FV_TAG_WIDTH].ti_Data, tags[FV_TAG_HEIGHT].ti_Data);
					}
 
					IFEELIN F_Dispose(tags);
				}
			}
		
			IFEELIN F_Do(window, FM_UnNotify, node->events_notify);
		}
		
		IFEELIN F_Dispose(node);
	}

	IFEELIN F_Do(Obj, FM_Unlock);

	return NULL;
}
//+
///WinServer_RemMember
F_METHODM(APTR,WinServer_RemMember,FS_RemMember)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	
	struct in_FamilyNode *node;
	#ifdef F_ENABLE_POSTCLEAN
	struct Hook clean_hook;
	struct in_PostCleanData data;
	#endif
	
	APTR rc = NULL;

	#ifdef F_ENABLE_POSTCLEAN
	clean_hook.h_Entry = (HOOKFUNC) F_FUNCTION_GATE(code_postclean);
	clean_hook.h_Data = &data;
	
	data.LOD = LOD;
	#endif
 
	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

	for (node = (struct in_FamilyNode *) LOD->family.Head ; node ; node = (struct in_FamilyNode *) node->Public.Next)
	{
		if (node->Public.Object == Msg->Member)
		{
			#ifdef DB_REMMEMBER
			IFEELIN F_Log(0,"object (%s{%lx}) notify (0x%08lx) window (0x%08lx) events (0x%08lx)",_object_classname(node->Public.Object),node->Public.Object,node->events_notify,node->window,node->events);
			#endif

			IFEELIN F_Do(node->Public.Object, FM_UnNotify, node->events_notify); node->events_notify = NULL;

			if (node->events)
			{
				#ifdef DB_REMMEMBER
				IFEELIN F_Log(0,"clear events >> BEGIN (0x%08lx)",node->events);
				#endif

				IFEELIN F_Do(Obj, F_METHOD_ID(MODIFYEVENTS), 0, Msg->Member);

				#ifdef DB_REMMEMBER
				IFEELIN F_Log(0,"clear events >> DONE (0x%08lx)",node->events);
				#endif
			}

			node->window->UserData = NULL;
			IFEELIN F_LinkRemove(&LOD->family, (FNode *) node);

			#ifdef F_ENABLE_INPUT
			 
			if (LOD->window_active == node->window)
			{
				LOD->window_active = NULL;
				LOD->window_active_node = NULL;
			}

			if (LOD->window_focus == node->window)
			{
				LOD->window_focus = NULL;
				LOD->window_focus_node = NULL;
			}

			if (LOD->dc_window == node->window)
			{
				LOD->dc_window = NULL;
			}
		
			#endif
 
			IINTUITION CloseWindow(node->window); node->window = NULL;
 
			#ifdef F_ENABLE_POSTCLEAN
			data.Target = node->Public.Object;
		
			IFEELIN F_Do(node->application, FM_Application_PostClean, &clean_hook);
			#endif

			IFEELIN F_Dispose(node);
			
			#ifdef DB_REMMEMBER
			IFEELIN F_Log(0,"node removed !!");
			#endif
			
			rc = Msg->Member;

			break;
		}
	}

	IFEELIN F_Do(Obj, FM_Unlock);

	return rc;
}
//+

struct FS_WinServer_ModifyEvents                { bits32 Events; FObject Target; };

///WinServer_ModifyEvents
F_METHODM(bits32,WinServer_ModifyEvents,FS_WinServer_ModifyEvents)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	bits32 events = 0;

	struct in_FamilyNode *node;
	struct in_FamilyNode *trgt = NULL;

	#ifdef DB_MODIFYEVENTS
	IFEELIN F_Log(0,"BEGIN >> 0x%08lx (0x%08lx)",Msg->Events, LOD->events);
	#endif
 
	#ifdef DB_MODIFY_LOCK
	IFEELIN F_Log(0,"modify.lock >> BEGIN");
	#endif
	 
	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);
	
	#ifdef DB_MODIFY_LOCK
	IFEELIN F_Log(0,"modify.lock >> DONE");
	#endif

	for (node = (struct in_FamilyNode *) LOD->family.Head ; node ; node = (struct in_FamilyNode *) node->Public.Next)
	{
		if (node->Public.Object == Msg->Target)
		{
			node->events = Msg->Events;
			
			#ifdef F_ENABLE_EVENT_WBDROP
			if (FF_EVENT_WBDROP & node->events)
			{
				if (!node->wbdrop_handler)
				{
					node->wbdrop_handler = IWORKBENCH AddAppWindow(MAKE_ID('A','P','P','W'),(uint32) node,node->window,LOD->ThreadPublic->Port,NULL);
				
					#ifdef DB_MODIFY
					IFEELIN F_Log(0,"wbdrop request: object (0x%08lx) window (0x%08lx) port (0x%08lx) wbbase (0x%08lx) handler (0x%08lx)",node->Public.Object,node->window,LOD->ThreadPublic->Port,WorkbenchBase,node->wbdrop_handler);
					#endif
				}
			}
			else if (node->wbdrop_handler)
			{
				#ifdef DB_MODIFY
				IFEELIN F_Log(0,"wbdrop reject for (0x%08lx) handler (0x%08lx)",node->Public.Object,node->wbdrop_handler);
				#endif
				
				IWORKBENCH RemoveAppWindow(node->wbdrop_handler); node->wbdrop_handler = NULL;
			}
			#endif
			
			trgt = node;
		}

		events |= node->events;
	}

	#ifdef F_ENABLE_INTUITION
	 
	if (LOD->intuition_port && trgt)
	{
		#ifdef DB_MODIFYEVENTS
		IFEELIN F_Log(0,"intuiport (0x%08lx) target node (0x%08lx)",LOD->intuition_port, trgt);
		#endif

		if (trgt)
		{
			bits32 idcmp = 0;

			#ifndef F_ENABLE_EVENT_TICK
			if (FF_EVENT_TICK & Msg->Events)
			{
				idcmp |= IDCMP_INTUITICKS;
			}
			#endif
			#ifndef F_ENABLE_EVENT_KEY
			if (FF_EVENT_KEY & Msg->Events)
			{
				idcmp |= IDCMP_RAWKEY;
			}
			#endif
			#ifndef F_ENABLE_EVENT_BUTTON
			if (FF_EVENT_BUTTON & Msg->Events)
			{
				idcmp |= IDCMP_MOUSEBUTTONS;
			}
			#endif
			#ifndef F_ENABLE_EVENT_MOTION
			if (FF_EVENT_MOTION & Msg->Events)
			{
				idcmp |= IDCMP_MOUSEMOVE;
			}
			#endif
			if (FF_EVENT_WINDOW & Msg->Events)
			{
				idcmp |=
					IDCMP_CLOSEWINDOW |
					#ifndef F_ENABLE_WINDOW_ACTIVE
					IDCMP_ACTIVEWINDOW |
					IDCMP_INACTIVEWINDOW |
					#endif
					IDCMP_NEWSIZE |
					IDCMP_CHANGEWINDOW |
					IDCMP_REFRESHWINDOW
				;
			}
			#ifndef F_ENABLE_EVENT_DISK
			if (FF_EVENT_DISK & Msg->Events)
			{
				idcmp |= IDCMP_DISKINSERTED | IDCMP_DISKREMOVED;
			}
			#endif

			
			#if 0

			IFEELIN F_Log(0,"%s-%s-%s-%s-%s-%s",
				(FF_EVENT_TICK & Msg->Events) ? "tick" : "",
				(FF_EVENT_KEY & Msg->Events) ? "key" : "",
				(FF_EVENT_BUTTON & Msg->Events) ? "button" : "",
				(FF_EVENT_MOTION & Msg->Events) ? "motion" : "",
				(FF_EVENT_WINDOW & Msg->Events) ? "window" : "",
				(FF_EVENT_DISK & Msg->Events) ? "disk" : "");

			#endif

			if (idcmp)
			{
				trgt->window->UserPort = LOD->intuition_port;
			}
			else
			{
				struct IntuiMessage *msg;
				struct Node *succ;

				trgt->window->UserPort = NULL;

				msg = (struct IntuiMessage *)(LOD->intuition_port->mp_MsgList.lh_Head);

				while ((succ = msg->ExecMessage.mn_Node.ln_Succ) != NULL)
				{
					if (msg->IDCMPWindow == trgt->window)
					{
						#ifdef DB_MODIFYEVENTS
						IFEELIN F_Log(0,"remove intuimsg (0x%08lx) class (0x%08lx)",msg,msg->Class);
						#endif
						 
						IEXEC Remove((struct Node *)(msg));
						IEXEC ReplyMsg((struct Message *)(msg));
					}
					msg = (struct IntuiMessage *)(succ);
				}
			}

			IINTUITION ModifyIDCMP(trgt->window, idcmp);
		}
	}
	#endif
	
	#ifdef DB_MODIFY_LOCK
	IFEELIN F_Log(0,"modify.unlock >> BEGIN");
	#endif

	IFEELIN F_Do(Obj, FM_Unlock);
	
	#ifdef DB_MODIFY_LOCK
	IFEELIN F_Log(0,"modify.unlock >> DONE");
	#endif

	return LOD->events = events;
}
//+
	
#if 0
 
#ifdef F_ENABLE_ZONES
	
struct FS_WinServer_ModifyZone                  { FObject Window; FEventHandler *Handler; };
 
///WinServer_ModifyZone
F_METHODM(uint32,WinServer_ModifyZone,FS_WinServer_ModifyZone)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	struct in_FamilyNode *node;
	struct FeelinZone *zone;
	struct FeelinZone *prev = NULL;
	
	if (!Msg->Window || !Msg->Target || !Msg->Handler)
	{
		return FALSE;
	}
	
	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);
	
	for (node = (struct in_FamilyNode *) LOD->family.Head ; node ; node = (struct in_FamilyNode *) node->Public.Next)
	{
		if (node->Public.Object == Msg->Window)
		{
			break;
		}
	}

	if (!node)
	{
		IFEELIN F_Do(Obj, FM_Unlock);
 
		return FALSE;
	}

	for (zone = node->zones ; zone ; zone = zone->next)
	{
		if (zone->target == Msg->Hander->Object)
		{
			if (((FF_EVENT_HELP | FF_EVENT_FOCUS) && Msg->Events) == 0)
			{
				if (prev)
				{
					prev->next = zone->next;
				}
				else
				{
					node->zones = zone->next;
				}
			
				IFEELIN F_Dispose(zone);
				
				IFEELIN F_Do(Obj, FM_Unlock);
				
				return TRUE;
			}
 
			break;
		}
		prev = zone;
	}

	if ((zone == NULL) && (((FF_EVENT_HELP | FF_EVENT_FOCUS) && Msg->Handler->Events) != 0))
	{
		FAreaPublic *pub = (FAreaPublic *) IFEELIN F_Get(Msg->Handler->Object, FA_Area_PublicData);
		
		if (pub)
		{
			zone = IFEELIN F_NewP(LOD->pool, sizeof (struct FeelinZone));
		}
		else
		{
			IFEELIN F_Log(0,"Only FC_Area subclasses may receive FF_EVENT_HELP or FF_EVENT_FOCUS events");
		}
		
		if (!zone)
		{
			IFEELIN F_Do(Obj, FM_Unlock);
			
			return FALSE;
		}
	
		zone->next = node->zones;
		zone->target = Msg->Handler->Object;
		zone->box = &pub->Box;
		node->zones = zone;
	}

	zone->handler = Msg->Handler;
	
	IFEELIN F_Log(0,"window (0x%08lx) zone (0x%08lx) target (0x%08lx - %3ld:%3ld, %3ldx%3ld) events (0x%08lx)",
	
		node->Public.Object,
		zone,
		zone->target,
		zone->box->x,
		zone->box->y,
		zone->box->w,
		zone->box->h,
		zone->events);
	
	IFEELIN F_Do(Obj, FM_Unlock);
					
	return TRUE;
}
//+
#endif

#endif
