#include "Private.h"
		  
#define DB_ADDOBJECTID
#define DB_REMOBJECTID
 
/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///app_create_ports
STATIC int32 app_create_ports(struct LocalObjectData *LOD)
{
	LOD->app_port = IEXEC CreateMsgPort();
	
	if (LOD->app_port == NULL)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to create Application's message port");
		
		return FALSE;
	}

	LOD->timers_port = IEXEC CreateMsgPort();
	
	if (LOD->timers_port == NULL)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to create Timers' message port");
		
		return FALSE;
	}
 
	LOD->time_request = IEXEC CreateIORequest(LOD->timers_port,sizeof (struct timerequest));
	
	if (LOD->time_request == NULL)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to create IORequest for timer.device");
		
		return FALSE;
	}
					
	if (IEXEC OpenDevice("timer.device",UNIT_VBLANK,(struct IORequest *) LOD->time_request,NULL))
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to open timer.device");
	}

	if (LOD->base)
	{
		LOD->app_port->mp_Node.ln_Pri = 2;
		LOD->app_port->mp_Node.ln_Name = LOD->base;

		IEXEC AddPort(LOD->app_port);
	}

	return TRUE;
}
//+
///app_delete_ports
STATIC void app_delete_ports(struct LocalObjectData *LOD)
{
	if (LOD->time_request)
	{
		IEXEC CloseDevice((struct IORequest *) LOD->time_request);

		IEXEC DeleteIORequest(LOD->time_request);
		
		LOD->time_request = NULL;
	}

	if (LOD->timers_port)
	{
		struct Message *msg, *last = NULL;
 
		IEXEC Forbid();

		while ((msg = IEXEC GetMsg(LOD->timers_port)) != NULL)
		{
			if (msg == last)
			{
				IFEELIN F_Log(0, "circle in pending messages !");

				break;
			}

			IEXEC ReplyMsg(msg);

			last = msg;
		}

		IEXEC DeleteMsgPort(LOD->timers_port);

		LOD->timers_port = NULL;

		IEXEC Permit();
	}

	if (LOD->app_port)
	{
		struct Message *msg;

		IEXEC Forbid();

		if (LOD->base)
		{
			if (IEXEC FindPort(LOD->base))
			{
				IEXEC RemPort(LOD->app_port);
			}
		}

		while ((msg = IEXEC GetMsg(LOD->app_port)) != NULL)
		{
			if (IREXXSYS IsRexxMsg((struct RexxMsg *) msg))
			{
				IEXEC ReplyMsg(msg);
			}
			else
			{
				IFEELIN F_Dispose(msg);
			}
		}

		IEXEC DeleteMsgPort(LOD->app_port);
		
		LOD->app_port = NULL;

		IEXEC Permit();
	}
}
//+
///app_create_broker
STATIC int32 app_create_broker(struct LocalObjectData *LOD,struct TagItem *Tags)
{
	struct MsgPort *xp;

	if ((LOD->title == NULL) || (LOD->version == NULL) || (LOD->description == NULL))
	{
		return FALSE;
	}
		
	xp = IEXEC CreateMsgPort();
   
	if (xp)
	{
		struct NewBroker *nb = IFEELIN F_NewP(LOD->pool,sizeof (struct NewBroker));
		
		if (nb)
		{
			APTR bk;
			int32 er;
			uint32 len = IFEELIN F_StrLen(LOD->description);
			
			if (len > 39)
			{
				IFEELIN F_Log
				(
					FV_LOG_DEV,
					"Broker description (0x%08lx)(%s)(%ld) is too long (limit is 39 characters)",
					LOD->description, LOD->description, len
				);
				
				LOD->description[39] = '\0';
			}
			
			nb->nb_Version = NB_VERSION;
			nb->nb_Name    = LOD->title;
			nb->nb_Title   = LOD->version + 6;
			nb->nb_Descr   = LOD->description;
			nb->nb_Unique  = IUTILITY GetTagData(FA_Application_Unique,FALSE,Tags);
			nb->nb_Pri     = IUTILITY GetTagData(FA_Application_BrokerPri,0,Tags);
			nb->nb_Port    = xp;
			nb->nb_Flags   = COF_SHOW_HIDE;
				
			bk = ICOMMODITIES CxBroker(nb,&er);
   
			if (bk)
			{
				LOD->broker = bk;
				LOD->broker_port = xp;
				LOD->broker_init = nb;

				ICOMMODITIES ActivateCxObj(bk,TRUE);

				return er;
			}
			IFEELIN F_Dispose(nb);
		}
		IEXEC DeleteMsgPort(xp);
	}

	IFEELIN F_Log(FV_LOG_USER,"Unable to Create Commodity");

	return 0;
}
//+
///app_delete_broker
STATIC void app_delete_broker(struct LocalObjectData *LOD)
{
	if (LOD->broker_init)
	{
		if (LOD->broker_port)
		{
			struct Message *msg;
			
			IEXEC Forbid();
			ICOMMODITIES DeleteCxObj(LOD->broker); LOD->broker = NULL;

			while ((msg = IEXEC GetMsg(LOD->broker_port)))
			{
				IEXEC ReplyMsg(msg);
			}

			IEXEC DeleteMsgPort(LOD->broker_port); LOD->broker_port = NULL;
			IEXEC Permit();
		}

		IFEELIN F_Dispose(LOD->broker_init); LOD->broker_init = NULL;
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Application_New
F_METHOD(uint32,Application_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	/* default values */

	LOD->sleep_count = 1;

	/* parse initial values */

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Application_Title:       LOD->title = (STRPTR)(item.ti_Data); break;
		case FA_Application_Version:     LOD->version = (STRPTR)(item.ti_Data); break;
		case FA_Application_Copyright:   LOD->copyright = (STRPTR)(item.ti_Data); break;
		case FA_Application_Author:      LOD->author = (STRPTR)(item.ti_Data); break;
		case FA_Application_Description: LOD->description = (STRPTR)(item.ti_Data); break;
		case FA_Application_Base:        LOD->base = (STRPTR)(item.ti_Data); break;
	}

	LOD->pool = IFEELIN F_CreatePool(1024,
	
		FA_Pool_Items, 1,
		FA_Pool_Name, "application-data",
		
		TAG_DONE);
				
	if (LOD->pool != NULL)
	{
		
		/* creation time ! First, let's create all necessary  message  ports
		*/

		if (app_create_ports(LOD))
		{
			
			/* Try to create a broker. Currently, a failure is not fatal */

			switch (app_create_broker(LOD,(struct TagItem *) Msg))
			{
				case CBERR_SYSERR:   IFEELIN F_Log(FV_LOG_USER,"System unable to create Broker");
				case CBERR_DUP:      IFEELIN F_Log(FV_LOG_USER,"Application already exists");
				case CBERR_VERSION:  IFEELIN F_Log(FV_LOG_USER,"Unknown Broker version");
			}

			LOD->appserver = IFEELIN F_SharedOpen("AppServer");
		 
			if (LOD->appserver != NULL)
			{
				#if 0
					
				/* A new FC_Preference object should only be created  if
				the  application  has  the FA_Application_Base attribute
				defined. Only applications with a  base  name  can  have
				local  preferences.  If  base  name  is not defined, the
				FC_Preference  object  should  be  inherited  from   the
				AppServer's one.

				AppServer's FC_Preference object is used as a reference.
				Preference   items   not   defined  in  the  application
				personnal FC_Preference object  are  obtained  from  the
				reference  object.  More  over,  changes  applied to the
				reference object are mirrored to  FC_Preference  objects
				using the reference. */

				if (LOD->base || LOD->theme)
				{
					LOD->preferences = PreferenceObject,
							
						"FA_Preference_Name",         LOD->base,
						"FA_Preference_Reference",    IFEELIN F_Get(LOD->appserver,(uint32) "FA_AppServer_Preference"),

					End;
				}

				/* If the FC_Preference object fails to  create,  or  if
				the   application   has   no   basename,  the  AppServer
				FC_Preference object must be used instead. */

				if (!LOD->preferences)
				{
					LOD->preferences = (FObject) IFEELIN F_Get(LOD->appserver,(uint32) "FA_AppServer_Preference");

					LOD->flags |= FF_APPLICATION_INHERITED_PREFS;
				}

				if (LOD->preferences)
					
				{
						
					/*    A    notification    is    set    upon     the
					FA_Preference_Update  attribute of the FC_Preference
					object, allowing the  application  to  be  aware  of
					preferences modifications, and update itself to mach
					these modifications */

					#ifdef F_ENABLE_PREFERENCES

					LOD->preferences_notify_handler = (FNotifyHandler *) IFEELIN F_Do(LOD->preferences,FM_Notify,"FA_Preference_Update",TRUE,Obj,FM_Application_Update,0);
						
					if (LOD->preferences_notify_handler)

					#endif /* F_ENABLE_PREFERENCES */
					{
							
						/* Now that everything is ready, the application
						can  be added to the application server. Calling
						our super class  (FC_Family)  add  all  children
						defined in the TAGS */

						if (IFEELIN F_Do(LOD->appserver, FM_AddMember, Obj, FV_AddMember_Tail))
						{
							return F_SUPERDO();
						}
					}
				}

				#else

				/* Now that everything is ready, the application can  be
				added to the application server. Calling our super class
				(FC_Family) add all children defined in the TAGS */

				if (IFEELIN F_Do(LOD->appserver, FM_AddMember, Obj, FV_AddMember_Tail))
				{
					return F_SUPERDO();
				}

				#endif
			}
		}
	}

	/* Something went wrong.  Children  defined  in  the  taglist  must  be
	disposed. */

	Tags = Msg;

	{
		struct TagItem *rtag;

		while ((rtag = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
		{
			if (item.ti_Tag == FA_Child)
			{
				IFEELIN F_DisposeObj((FObject)(item.ti_Data));

				rtag->ti_Tag = TAG_IGNORE;
				rtag->ti_Data = NULL;
			}
		}
	}

	return NULL;
}
//+
///Application_Dispose
F_METHOD(uint32,Application_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	/* Puting the application to sleep close all windows and dispose graphic
	resources */

	//
	// put application to sleep
	// all children will be invoked with the Cleanup method
	// they should free a lot of resources
	//

	IFEELIN F_Do(Obj, FM_Application_Sleep);

	//
	// dispose the preferences
	//

	if (LOD->css)
	{
		if (LOD->css_notify_handler)
		{
			IFEELIN F_Do(LOD->css, FM_UnNotify, LOD->css_notify_handler);

			LOD->css_notify_handler = NULL;
		}

		if (!(LOD->flags & FF_APPLICATION_INHERITED_CSS))
		{
			IFEELIN F_DisposeObj(LOD->css);
		}

		LOD->css = NULL;
	}

	//
	// save the persistent values of idetified elements
	//

	IFEELIN F_Do(Obj, FM_Application_Save, FV_Application_BOTH);

	//
	// pass the method to our superclass
	// it should dispose all of our children
	//

	F_SUPERDO();

	//
	// free resources that might have been left behind by children
	//
									
	while (LOD->timer_handlers_list.Head)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Removing forgotten signal handler (0x%08lx)", LOD->timer_handlers_list.Head);

		IFEELIN F_Do(Obj, FM_Application_DeleteSignalHandler, F_PUBLICIZE_SIGNAL_HANDLER(LOD->timer_handlers_list.Head));
	}

	//
	// quit and release the appserver
	//

	if (LOD->appserver)
	{
		IFEELIN F_Do(LOD->appserver, FM_RemMember, Obj);
		IFEELIN F_SharedClose(LOD->appserver);
	}

	//
	// delete the broker's port and resources
	//

	app_delete_broker(LOD);

	//
	// delete additionnal message ports
	//

	app_delete_ports(LOD);

	//
	// delete our memory pool
	//

	if (LOD->pool)
	{
		IFEELIN F_DeletePool(LOD->pool);
	
		LOD->pool = NULL;
	}

	return 0;
}
//+
///Application_Set
F_METHOD(void,Application_Set)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while  (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Application_UserSignals:
		{
			LOD->user_signals = item.ti_Data;

			app_collect_signals(Class,Obj);
		}
		break;

		case FA_Application_Sleep:
		{
			if (item.ti_Data)
			{
				LOD->sleep_count++; // 1 & more the application is put to sleep
			}
			else
			{
				LOD->sleep_count--; // 0 & less the application is awakened
			}

			if (LOD->sleep_count == 1)
			{
				IFEELIN F_Do(Obj,FM_Application_PushMethod,Obj,FM_Application_Sleep,0);
			}
			else if (LOD->sleep_count == 0)
			{
				IFEELIN F_Do(Obj,FM_Application_PushMethod,Obj,FM_Application_Awake,0);
			}
		}
		break;
	    /*
		case FA_Application_Menu:
		{
			IFEELIN F_Do(LOD->menu, FM_Disconnect);
			
			LOD->menu = (FObject) item.ti_Data;
			
			IFEELIN F_Do(LOD->menu, FM_Connect, LOD->menu);
		}
		break;
		*/
	
		#if 0//def F_ENABLE_PREFERENCES
		
		case FA_Application_Theme:
		{
			LOD->theme = (STRPTR) item.ti_Data;
		
			if (FF_APPLICATION_INHERITED_PREFS & LOD->flags)
			{
				FObject prefs = LOD->preferences = PreferenceObject,

					"FA_Preference_Name", LOD->base,
					"FA_Preference_Reference", IFEELIN F_Get(LOD->appserver,(uint32) "FA_AppServer_Preference"),

					End;
					
				if (prefs)
				{
					LOD->preferences = prefs;
 
					LOD->flags &= ~FF_APPLICATION_INHERITED_PREFS;
				}
			}
		}
		break; 
	
		#endif
	}
	F_SUPERDO();
}
//+
///Application_Get
F_METHOD(uint32, Application_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		#ifndef F_NEW_GLOBALCONNECT
		case FA_Parent:                  	F_STORE(LOD->appserver); break;
		case FA_Application:				F_STORE(Obj); break;
		#endif
		
		case FA_Application_Title:       	F_STORE(LOD->title);        break;
		case FA_Application_Version:     	F_STORE(LOD->version);      break;
		case FA_Application_Copyright:   	F_STORE(LOD->copyright);    break;
		case FA_Application_Author:      	F_STORE(LOD->author);       break;
		case FA_Application_Description: 	F_STORE(LOD->description);  break;
		case FA_Application_Base:        	F_STORE(LOD->base); break;
		case FA_Application_UserSignals: 	F_STORE(LOD->user_signals); break;
		case FA_Application_BrokerPort:  	F_STORE(LOD->broker_port); break;
		case FA_Application_Broker:      	F_STORE(LOD->broker); break;

		case FA_Application_Display:     	F_STORE(LOD->display); break;
		case FA_Application_Preference:  	F_STORE(LOD->css); break;
		case FA_Application_Sleep:       	F_STORE(LOD->sleep_count > 0); break;
	}

	return F_SUPERDO();
}
//+
///Application_AddMember
F_METHODM(uint32,Application_AddMember,FS_AddMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FObject orphan = Msg->Orphan;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	#ifdef F_NEW_GLOBALCONNECT
	
	if (IFEELIN F_Do(orphan, FM_Element_GlobalConnect, Obj, NULL) == FALSE)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Child (0x%08lx) refuses global connection", orphan);

		return FALSE;
	}
	
	#endif

	if ((FF_Application_Setup & LOD->flags) != 0)
	{
		if (IFEELIN F_Get(orphan, FA_Window_Open) != FALSE)
		{
			#if 0
			IFEELIN F_Log(0, "open window (0x%08lx)", orphan);
			#endif

			IFEELIN F_Do(orphan, FM_Window_Open);
		}
	}

	return TRUE;
}
//+
///Application_RemMember
F_METHODM(bool32, Application_RemMember, FS_RemMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
		
	if (Msg->Member == NULL)
	{
		return FALSE;
	}

	if (((FF_Application_Setup & LOD->flags) != 0) &&
		(IFEELIN F_Get(Msg->Member, FA_Window_System) != 0))
	{
		IFEELIN F_Do(Msg->Member, FM_Window_Close);
	}
			
	#ifdef F_NEW_GLOBALCONNECT

	IFEELIN F_Do(Msg->Member, FM_Element_GlobalDisconnect);

	#endif

	return F_SUPERDO();
}
//+
///Application_Connect
F_METHODM(bool32, Application_Connect, FS_Connect)
{
	return TRUE;
}
//+
///Application_Disconnect
F_METHOD(bool32, Application_Disconnect)
{
	return TRUE;
}
//+
