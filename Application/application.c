#include "Private.h"

//#define DB_BROKER
//#define DB_APPPORT
//#define DB_CHECK_DEPRECATED_MSG
//#define DB_POSTCLEAN
//#define DB_UPDATE

///app_collect_signals
bits32 app_collect_signals(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct in_SignalHandler *handler;

	bits32 wait = LOD->user_signals | SIGBREAKF_CTRL_C | (1 << LOD->app_port->mp_SigBit) | (1 << LOD->timers_port->mp_SigBit);

	if (LOD->broker_port)
	{
		wait |= (1 << LOD->broker_port->mp_SigBit);
	}

	for (handler = (struct in_SignalHandler *) LOD->signal_handlers_list.Head ; handler ; handler = (struct in_SignalHandler *) handler->node.Next)
	{
		wait |= handler->signals;
	}

	LOD->wait_signals = wait;
	
	return wait;
}
//+

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///app_check_broker_port
STATIC int32 app_check_broker_port(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->broker_port)
	{
		CxMsg *msg = (CxMsg *) IEXEC GetMsg(LOD->broker_port);

		if (msg)
		{
			uint32 id = ICOMMODITIES CxMsgID(msg);
			uint32 type = ICOMMODITIES CxMsgType(msg);

			IEXEC ReplyMsg((struct Message *)(msg));

			if (type == CXM_COMMAND)
			{
				switch (id)
				{
					case CXCMD_DISABLE:
					{
						#ifdef DB_BROKER
						IFEELIN F_Log(0,"CXCMD_DISABLE");
						#endif
						
						ICOMMODITIES ActivateCxObj(LOD->broker,FALSE);
					}
					break;

					case CXCMD_ENABLE:
					{
						#ifdef DB_BROKER
						IFEELIN F_Log(0,"CXCMD_ENABLE");
						#endif
						
						ICOMMODITIES ActivateCxObj(LOD->broker,TRUE);
					}
					break;
					
					case CXCMD_APPEAR:
					{
						#ifdef DB_BROKER
						IFEELIN F_Log(0,"CXCMD_APPEAR");
						#endif
						
						IFEELIN F_Set(Obj,FA_Application_Sleep,FALSE);
					}
					break;
					
					case CXCMD_DISAPPEAR:
					{
						#ifdef DB_BROKER
						IFEELIN F_Log(0,"CXCMD_DISAPPEAR");
						#endif
						
						IFEELIN F_Set(Obj,FA_Application_Sleep,TRUE);
					}
					break;
					
					case CXCMD_KILL:
					{
						#ifdef DB_BROKER
						IFEELIN F_Log(0,"CXCMD_KILL");
						#endif
						
						IFEELIN F_Do(Obj,FM_Application_Shutdown);
					}
					break;
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///App_Run
F_METHOD(void,App_Run)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
			
	int8 read_again = FALSE;

	/* Messages are read  one  by  one  from  each  message  port.  A  port
	receiving  a  lot  of  messages  will  not  block  the application. The
	variable 'read_again' is set to TRUE each time a message is read.  Thus
	if  after  a  first  pass  the variable 'read_again' is still FALSE the
	application can be set to wait for new signals.

	While the flag FF_Application_Run is set, the application runs. If  it's
	cleared the method exists. */

	LOD->flags |= FF_Application_Run;

	/* read objects data and preferences */

	IFEELIN F_Do(Obj, FM_Application_Load, FV_Application_ENV);

	if (_application_isnt_inherited_css)
	{
		IFEELIN F_Do(LOD->css, F_AUTO_ID(READ), FV_Preference_ENV);
	}

/* FIXME: I should read some user settings to know if the application starts
in sleep mode. */

	IFEELIN F_Set(Obj, FA_Application_Sleep, FALSE);

	app_collect_signals(Class,Obj);

	/* The application runs with its sempahore locked, which is unlock  only
	when the main loop is left e.g. when invoking objects. */

	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

	while ((FF_Application_Run & LOD->flags) != 0)
	{
		FPush *push;
		struct Message *msg;
		bits32 sig;

/*** Waiting signals ***************************************************************************/
		
		if (read_again == FALSE)
		{
			IFEELIN F_Do(Obj,FM_Unlock);
		   
			sig = IEXEC Wait(LOD->wait_signals);
		   
			IFEELIN F_Do(Obj,FM_Lock,FF_Lock_Exclusive);
		}
		else
		{
			sig = 0; read_again = FALSE;
		}

/*** Signals ***********************************************************************************/

		if (LOD->signal_handlers_list.Head != NULL)
		{
			struct in_SignalHandler *handler;
			APTR next = LOD->signal_handlers_list.Head;

			while ((handler = IFEELIN F_NextNode(&next)) != NULL)
			{
				if (sig & handler->signals)
				{
					IFEELIN F_Do(Obj, FM_Unlock);
					IFEELIN F_Do(handler->target, handler->method);
					IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);
				}
			}
		}

		if ((sig & SIGBREAKF_CTRL_C) != 0)
		{
			IFEELIN F_Do(Obj, FM_Application_Shutdown);
		}

/*** Pushed Methods ****************************************************************************/
					
		while ((push = (FPush *) IFEELIN F_LinkRemove(&LOD->pushed_methods_list, LOD->pushed_methods_list.Head)) != NULL)
		{
///DB_PUSH
			if (CUD->db_Push)
			{
				IFEELIN F_Log(0,"PUSH: %s{%08lx}.0x%08lx { 0x%08lx 0x%08lx 0x%08lx 0x%08lx }",_object_classname(push->Target),push->Target,push->Method,((uint32 *)(push->Msg))[0],((uint32 *)(push->Msg))[1],((uint32 *)(push->Msg))[2],((uint32 *)(push->Msg))[3]);
			}
//+

			IFEELIN F_Do(Obj,FM_Unlock);
			IFEELIN F_DoA(push->Target,push->Method,push->Msg);
			IFEELIN F_Do(Obj,FM_Lock,FF_Lock_Exclusive);
			IFEELIN F_Dispose(push);
		}

/*** Application Port (also used for ARexx) ****************************************************/

		msg = IEXEC GetMsg(LOD->app_port);
	   
		if (msg)
		{
			read_again = TRUE;

			if (IREXXSYS IsRexxMsg((struct RexxMsg *)(msg)))
			{
				STRPTR cmd = ((struct RexxMsg *)(msg)) ->rm_Args[0];

				if (cmd)
				{
					if (IUTILITY Stricmp(cmd,"QUIT") == 0)
					{
						IFEELIN F_Do(Obj,FM_Application_Shutdown);
					}
					else if (IUTILITY Stricmp(cmd,"HIDE") == 0)
					{
						IFEELIN F_Set(Obj,FA_Application_Sleep,TRUE);
					}
					else if (IUTILITY Stricmp(cmd,"SHOW") == 0)
					{
						IFEELIN F_Set(Obj,FA_Application_Sleep,FALSE);
					}
					else
					{
						((struct RexxMsg *)(msg))->rm_Result1 = -3;
					}
				}
			}
			else
			{
				if (((FPost *) msg)->Type == FV_Application_Post_Type_Event)
				{
					FEvent *fev = ((FPost *) msg)->Data;
 
					#ifdef DB_CHECK_DEPRECATED_MSG
					
					if ((uint32) _object_class(fev->Window) == 0xDEADBEEF)
					{
						IFEELIN F_Log(0,"deprecated msg (0x%08lx) class (0x%08lx) time (%ld:0x%08lx)",fev,fev->Class,fev->Seconds,fev->Micros);
					}
					else
					#endif
					{
						#ifdef DB_APPPORT
						IFEELIN F_Log(0,"post (0x%08lx) class (0x%08lx) window (0x%08lx)",
							((FPost *) msg)->Data,
							fev->Class,
							fev->Window);
						#endif
					 
						IFEELIN F_Do(Obj,FM_Unlock);

						IFEELIN F_Do(fev->Target, FM_Window_DispatchEvent, fev);

						IFEELIN F_Do(Obj,FM_Lock, FF_Lock_Exclusive);
					}
				}
			}
			
			IEXEC ReplyMsg(msg);
		}

/*** Broker Port *******************************************************************************/

		if (app_check_broker_port(Class,Obj))
		{
			read_again = TRUE;
		}
			
/*** Timer *************************************************************************************/

		if (!IsMsgPortEmpty(LOD->timers_port))
		{
			msg = IEXEC GetMsg(LOD->timers_port);
			
			if (msg)
			{
				struct in_TimerHandler *handler = (APTR) ((uint32)(msg) - sizeof (struct in_TimerHandler) + sizeof (struct timerequest));

				handler->request.tr_node.io_Command = TR_ADDREQUEST;
				handler->request.tr_time.tv_secs    = handler->seconds;
				handler->request.tr_time.tv_micro   = handler->micros;

				IFEELIN F_Do(Obj,FM_Unlock);
				
				if (IFEELIN F_Do(handler->target, handler->method))
				{
					IEXEC SendIO((struct IORequest *) &handler->request);
				}

				IFEELIN F_Do(Obj,FM_Lock,FF_Lock_Exclusive);
			}
			
			read_again = TRUE;
		}

		if (sig)
		{
			IFEELIN F_Do(Obj,FM_Unlock);
			
			IFEELIN F_SuperDo(Class, Obj, FM_Set, FA_Application_Signal, sig, TAG_DONE);
			
			IFEELIN F_Do(Obj,FM_Lock,FF_Lock_Exclusive);
		}
	
		/* The FF_Application_Update flag is set when the application  needs
		to be updated because preferences have been modified. A flag is used
		instead of a  method  because  this  is  the  easier  way  to  solve
		semaphore  hanging troubles. Because application update always takes
		place from the main loop, it is totaly safe. */

		#ifdef F_ENABLE_PREFERENCES

		if (FF_APPLICATION_UPDATE & LOD->flags)
		{
			LOD->flags &= ~FF_APPLICATION_UPDATE;
			
			#ifdef DB_UPDATE
			IFEELIN F_Log(0,"update: unlock");
			#endif
 
			IFEELIN F_Do(Obj,FM_Unlock);
				
			#ifdef DB_UPDATE
			IFEELIN F_Log(0,"update: sleep");
			#endif
			 
			IFEELIN F_Do(Obj,FM_Application_Sleep);
			
			#ifdef DB_UPDATE
			IFEELIN F_Log(0,"update: awake");
			#endif
 
			IFEELIN F_Do(Obj,FM_Application_Awake);
				
			#ifdef DB_UPDATE
			IFEELIN F_Log(0,"update: lock");
			#endif
			 
			IFEELIN F_Do(Obj,FM_Lock,FF_Lock_Exclusive);
		}

		#endif
	}

	IFEELIN F_Do(Obj,FM_Unlock);
}
//+
///App_Shutdown
F_METHOD(void,App_Shutdown)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	LOD->flags &= ~FF_Application_Run;

	IEXEC Signal(LOD->app_port->mp_SigTask,1 << LOD->app_port->mp_SigBit);
}
//+
///App_Awake
F_METHOD(void,App_Awake)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct Task *task = IEXEC FindTask(NULL);
	BOOL err = FALSE;

	IFEELIN F_Do(Obj,FM_Lock,FF_Lock_Exclusive);

	if (task != LOD->app_port->mp_SigTask)
	{
		IFEELIN F_Log(0,"Method invoked by a foreign task (0x%08lx '%s'). AppTask 0x%08lx",task,task->tc_Node.ln_Name,LOD->app_port->mp_SigTask);
	}

	if (IFEELIN F_Do(Obj, FM_Application_Setup) != FALSE)
	{
		FFamilyNode *node;

		for (node = (FFamilyNode *) IFEELIN F_Get(Obj,FA_Family_Head) ; node ; node = node->Next)
		{
			if (IFEELIN F_Get(node->Object,FA_Window_Open) != FALSE)
			{
//				  IFEELIN F_Log(0, "open window (0x%08lx)", node->Object);

				if (!IFEELIN F_Do(node->Object,FM_Window_Open))
				{
					#ifdef F_NEW_ELEMENT_ID
					STRPTR id = (STRPTR) IFEELIN F_Get(node->Object, FA_Element_ID);
					#else
					STRPTR id = (STRPTR) IFEELIN F_Get(node->Object, FA_ID);
					#endif

					if (id != NULL)
					{
						IFEELIN F_Log(FV_LOG_USER,"Unable to Open %s{%08lx} (%s)", _object_classname(node->Object), node->Object, id);
					}
					else
					{
						IFEELIN F_Log(FV_LOG_USER,"Unable to Open %s{%08lx}", _object_classname(node->Object), node->Object);
					}

					err = TRUE;

					break;
				}
			}
		}
	}
	else
	{
		IFEELIN F_Log(FV_LOG_USER, "Setup failed");

		err = TRUE;
	}

	if (err)
	{
		IFEELIN F_Do(Obj, FM_Application_Shutdown);
	}

	IFEELIN F_Do(Obj, FM_Unlock);
}
//+
///App_Sleep
F_METHOD(void,App_Sleep)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct Task *task = IEXEC FindTask(NULL);

	IFEELIN F_Do(Obj,FM_Lock,FF_Lock_Exclusive);

	if (task != LOD->app_port->mp_SigTask)
	{
		IFEELIN F_Log(0,"Method invoked from a foreign task (0x%08lx '%s'). AppTask 0x%08lx",task,task->tc_Node.ln_Name,LOD->app_port->mp_SigTask);
	}

	if (FF_Application_Setup & LOD->flags)
	{
		FFamilyNode *node;

		for (node = (FFamilyNode *) IFEELIN F_Get(Obj,FA_Family_Head) ; node ; node = node->Next)
		{
			if (IFEELIN F_Get(node->Object, FA_Window_System))
			{
				IFEELIN F_Do(node->Object,FM_Window_Close);
			}
		}

		IFEELIN F_Do(Obj,FM_Application_Cleanup);
	}

	IFEELIN F_Do(Obj,FM_Unlock);
}
//+
///App_PushMethod
F_METHODM(bool32,App_PushMethod,FS_Application_PushMethod)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FPush *push;

	if (Msg->Target == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Target is NULL !", Msg->Target);
		
		return FALSE;
	}

	if (Msg->Count > 256)
	{
		IFEELIN F_Log(FV_LOG_DEV,"Suspicious number of arguments (%ld): Target %s{%08lx}, Method 0x%08lx",Msg->Count,_object_classname(Msg->Target),Msg->Target,Msg->Method);
		
		return FALSE;
	}

	push = IFEELIN F_NewP(LOD->pool, sizeof (FPush) + ((Msg->Count + 1) * sizeof (uint32)));

	if (push == NULL)
	{
		return FALSE;
	}

	push->Target = Msg->Target;
	push->Method = IFEELIN F_DynamicFindID((STRPTR)(Msg->Method));
	push->Msg = push + 1;

	IEXEC CopyMem(Msg + 1, push->Msg, Msg->Count * sizeof (uint32));
				
	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);
	
	IFEELIN F_LinkTail(&LOD->pushed_methods_list, (FNode *) push);
	
	IFEELIN F_Do(Obj, FM_Unlock);

	IEXEC Signal(LOD->app_port->mp_SigTask, 1 << LOD->app_port->mp_SigBit);

	return TRUE;
}
//+
///App_AddSignalHandler
F_METHOD(APTR,App_AddSignalHandler)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, *item;

	FObject target = NULL;
	uint32 method = 0;
	bits32 signals = 0;
	uint32 seconds = 0;
	uint32 micros = 0;

	while ((item = IUTILITY NextTagItem(&Tags)) != NULL)
	switch (item->ti_Tag)
	{
		case FA_SignalHandler_Target:   target = (FObject) item->ti_Data; break;
		case FA_SignalHandler_Method:   method = item->ti_Data; break;
		case FA_SignalHandler_Signals:  signals = (bits32) item->ti_Data; break;
		case FA_SignalHandler_Seconds:  seconds = item->ti_Data; break;
		case FA_SignalHandler_Micros:   micros = item->ti_Data; break;

		default:
		{
			IFEELIN F_Log(FV_LOG_DEV, "unknown attribute (0x%08lx) with value (0x%08lx) - abording handler", item->ti_Tag, item->ti_Data);

			return NULL;
		}
	}

	if (target == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Target not defined or NULL");

		return NULL;
	}

	if (method == 0)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Method not defined or zero");

		return NULL;
	}

	if (seconds || micros)
	{
		struct in_TimerHandler *handler = IFEELIN F_NewP(LOD->pool, sizeof (struct in_TimerHandler));

		if (handler)
		{
			handler->target = target;
			handler->method = method;
			handler->seconds = seconds;
			handler->micros = micros;

			IEXEC CopyMem(LOD->time_request, &handler->request, sizeof (struct timerequest));

			handler->request.tr_node.io_Command = TR_ADDREQUEST;
			handler->request.tr_time.tv_secs  = seconds;
			handler->request.tr_time.tv_micro = micros;

			IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

			IFEELIN F_LinkTail(&LOD->timer_handlers_list,(FNode *) handler);

			IEXEC SendIO((struct IORequest *) &handler->request);

			IFEELIN F_Do(Obj, FM_Unlock);

			return (APTR) (((uint32) handler) + sizeof (FNode));
		}
		else
		{
			IFEELIN F_Log(FV_LOG_USER,"unable to allocate timer handler");
		}
	}
	else if (signals)
	{
		struct in_SignalHandler *handler = IFEELIN F_NewP(LOD->pool, sizeof (struct in_SignalHandler));

		if (handler != NULL)
		{
			handler->target = target;
			handler->method = method;
			handler->signals = signals;

			IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

			IFEELIN F_LinkTail(&LOD->signal_handlers_list, (FNode *) handler);

			app_collect_signals(Class, Obj);

			IFEELIN F_Do(Obj, FM_Unlock);

			return (APTR) (((uint32) handler) + sizeof (FNode));
		}
	}
	else
	{
		IFEELIN F_Log(FV_LOG_DEV, "Neither Seconds, Micros and Signals are defined or zero");
	}

	return NULL;
}
//+
///App_RemSignalHandler
F_METHODM(bool32,App_RemSignalHandler,FS_Application_DeleteSignalHandler)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Handler)
	{
		APTR handler = (APTR) (((uint32) Msg->Handler) - sizeof (FNode));
		FNode *node;

		IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

/*** check timer handlers **********************************************************************/

		for (node = LOD->timer_handlers_list.Head ; node ; node = node->Next)
		{
			if (node == handler) break;
		}

		if (node)
		{
			if (IEXEC CheckIO((struct IORequest *) &((struct in_TimerHandler *) node)->request) == NULL)
			{
			   IEXEC AbortIO((struct IORequest *) &((struct in_TimerHandler *) node)->request);
			}

			IEXEC WaitIO((struct IORequest *) &((struct in_TimerHandler *) node)->request);

			IFEELIN F_LinkRemove(&LOD->timer_handlers_list, node);

			IFEELIN F_Dispose(node);

			IFEELIN F_Do(Obj, FM_Unlock);

			return TRUE;
		}

/*** check signal handlers *********************************************************************/

		for (node = LOD->signal_handlers_list.Head ; node ; node = node->Next)
		{
			if (node == handler) break;
		}

		if (node)
		{
			IFEELIN F_LinkRemove(&LOD->signal_handlers_list, node);

			IFEELIN F_Dispose(node);

			app_collect_signals(Class,Obj);

			IFEELIN F_Do(Obj,FM_Unlock);

			return TRUE;
		}

/*** unkown handler ****************************************************************************/

		IFEELIN F_Log(FV_LOG_DEV,"unkown signal or timer handler (0x%08lx)",Msg->Handler);
	}

	return FALSE;
}
//+
///App_Update
F_METHOD(void,App_Update)
{
	#ifdef F_ENABLE_PREFERENCES
	
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	#ifdef DB_UPDATE
	
	IFEELIN F_Log(0,"signal task to update");
	
	#endif

	LOD->flags |= FF_APPLICATION_UPDATE;

	IEXEC Signal(LOD->app_port->mp_SigTask,1 << LOD->app_port->mp_SigBit);
	
	#endif
}
//+
///App_Post
F_METHODM(uint32,App_Post,FS_Application_Post)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if ((Msg->Post != NULL) &&
		(Msg->Post->sys.mn_Node.ln_Type == NT_MESSAGE) &&
		(Msg->Post->sys.mn_ReplyPort != NULL) &&
		(Msg->Post->sys.mn_ReplyPort->mp_Node.ln_Type == NT_MSGPORT))
	{
		switch (Msg->Post->Type)
		{
			case FV_Application_Post_Type_Event: break;
			default: return FALSE;
		}
	
		if (LOD->app_port)
		{
			IEXEC PutMsg(LOD->app_port, (struct Message *) Msg->Post);
			
			return TRUE;
		}
	}
	return FALSE;
}
//+
///App_PostClean
F_METHODM(void,App_PostClean,FS_Application_PostClean)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	
	struct Message *msg;
 
	#ifdef DB_POSTCLEAN
	IFEELIN F_Log(0,"lock >> BEGIN");
	#endif
	 
	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);
	
	#ifdef DB_POSTCLEAN
	IFEELIN F_Log(0,"lock >> DONE");
	#endif
 
	msg = (struct Message *) LOD->app_port->mp_MsgList.lh_Head;

	#ifdef DB_POSTCLEAN
	IFEELIN F_Log(0,"parse messages (0x%08lx) >> BEGIN",msg);
	#endif

	while (msg != NULL)
	{
		struct Node *succ = msg->mn_Node.ln_Succ;
		struct FS_Application_HookPostClean hook_msg;
		
		hook_msg.Post = (FPost *) msg;
		
		if ((IUTILITY CallHookPkt(Msg->CleanHook, NULL, &hook_msg)) == FF_Application_PostClean_Eat)
		{
			#ifdef DB_POSTCLEAN
			IFEELIN F_Log(0,"remove (0x%08lx) and reply >> BEGIN",msg);
			#endif
			
			IEXEC Remove((struct Node *) msg);
			IEXEC ReplyMsg(msg);
			
			#ifdef DB_POSTCLEAN
			IFEELIN F_Log(0,"remove (0x%08lx) and reply >> DONE",msg);
			#endif
		}
	
		msg = (struct Message *) succ;
	}

	#ifdef DB_POSTCLEAN
	IFEELIN F_Log(0,"parse messages >> DONE",msg);
	#endif
								
	#ifdef DB_POSTCLEAN
	IFEELIN F_Log(0,"unlock >> BEGIN");
	#endif
	
	IFEELIN F_Do(Obj, FM_Unlock);
	
	#ifdef DB_POSTCLEAN
	IFEELIN F_Log(0,"unlock >> DONE");
	#endif
}
//+
