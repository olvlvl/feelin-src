#include "Private.h"

//#define DB_PUSH
//#define DB_POP
//#define DB_MAIN
//#define DB_DISPOSE

//#define F_ENABLE_SNOOZE

/* Les messages Thread sont reconnaissables parce que 'ln_Name' == Thread */

/*

YOMGUI @ Gofromiel: J'ai recode tous  le  mechanisme,  car  celui-qui  etait
present ne protege rien en realite!

GOFROMIEL: Ben tu as bien fais, c'est super !

*/

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

/// thread_main
SAVEDS void thread_main(void)
{
	struct Process *proc = (struct Process *) IEXEC FindTask(NULL);
	struct MsgPort *port = IEXEC CreateMsgPort();

	/* The message port of the process must only be used for initialization.
	Once  the thread is initialized, and a new message private port created,
	the developer must use the FM_Thread_Wait() and FM_Thread_Pop()  methods
	to replace the Wait() and GetMsg() ones. */

	for (;;)
	{
		FThreadMsg *msg = (FThreadMsg *) IEXEC GetMsg(&proc->pr_MsgPort);

		if (msg)
		{
			if (msg->Action == FV_Thread_Init)
			{
				struct FS_Thread_Init *params = (APTR) msg->Params;
				struct FeelinClass *Class = params->Class;
				FObject Obj = params->Object;
				
				struct LocalObjectData *LOD = F_LOD(Class,Obj);
				FThreadMsg *death_msg = NULL;

				if (port)
				{
					struct Hook *hook = params->Hook;
					struct FS_Thread_Run hook_msg;

					hook_msg.Public = &LOD->pub;

					LOD->pub.Port = port;
					LOD->pub.Process = proc;
 
					msg->Return = TRUE;

					IEXEC ReplyMsg((struct Message *) msg);

					death_msg = (FThreadMsg *) IUTILITY CallHookPkt(hook, Obj, &hook_msg);
				}
				else
				{
					msg->Return = FALSE;

					IEXEC ReplyMsg((struct Message *) msg);
				}

				#ifdef DB_MAIN
				IFEELIN F_Log(0,"THREAD_MAIN: EXIT - process (0x%08lx) thread port (0x%08lx) >> BEGIN",proc, port);
				#endif
 
				/* Loop until win the semaphore */
				while (!IEXEC AttemptSemaphore(&LOD->arbitrer))
				{
					#ifdef DB_MAIN
					IFEELIN F_Log(0,"THREAD_MAIN: EXIT - ATTEMPT ARBITRER >> FAILED - Waiting to win the arbitrer semaphore");
					#endif

					IEXEC Wait(1UL << proc->pr_MsgPort.mp_SigBit);
				}

				#ifdef DB_MAIN
				IFEELIN F_Log(0,"THREAD_MAIN: EXIT - FLUSH MSG PORT");
				#endif
  

				/* Flush msg on the process port */
				
				while ((msg = (FThreadMsg *) IEXEC GetMsg(&proc->pr_MsgPort)) != NULL)
				{
					#ifdef DB_MAIN
					IFEELIN F_Log(0,"THREAD_MAIN: EXIT - PURGE PROC PORT: msg (0x%08lx)", msg);
					#endif

					IEXEC ReplyMsg((struct Message *) msg);
				}
				
				/* Setting 'Process' to NULL signals that this process  does
				no longer exist */

				LOD->pub.Process = NULL;
				LOD->pub.Port = NULL;

				#ifdef DB_MAIN
				IFEELIN F_Log(0,"THREAD_MAIN: EXIT - RELEASE ARBITER");
				#endif

				/* Now, we can release the  arbitrer,  because  push  method
				doesn't use our public port (=NULL) */
				
				IEXEC ReleaseSemaphore(&LOD->arbitrer);
				
				#ifdef DB_MAIN
				IFEELIN F_Log(0,"DEATH MSG (0x%08lx)",death_msg);
				#endif

				if (death_msg)
				{
					IEXEC ReplyMsg((struct Message *) death_msg);
				}

				/* Flush msg on the public port */
				
				if (port)
				{
					#ifdef DB_MAIN
					IFEELIN F_Log(0,"THREAD_MAIN: EXIT - PURGE THREAD PORT: port (0x%08lx)",port);
					#endif
					
					while ((msg = (FThreadMsg *) IEXEC GetMsg(port)) != NULL)
					{
						#ifdef DB_MAIN
						IFEELIN F_Log(0,"THREAD_MAIN: EXIT - PURGE THREAD PORT: port (0x%08lx) msg (0x%08lx)", port, msg);
						#endif

						IEXEC ReplyMsg((struct Message *) msg);
					}

					IEXEC DeleteMsgPort(port);
				}

				#ifdef DB_MAIN
				IFEELIN F_Log(0,"THREAD_MAIN: EXIT >> DONE");
				#endif

				return;
			}
			else
			{
				/* re-queue message */

				IEXEC PutMsg(&proc->pr_MsgPort, (struct Message *) msg);
			}
		}

		IEXEC Wait(1UL << proc->pr_MsgPort.mp_SigBit);
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Thread_New
F_METHOD(FObject, Thread_New)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = Msg, item;

	struct Hook *hook = NULL;
	uint32 stacksize = 8192, pri = 0;
	
	STRPTR name = NULL;

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class) != NULL)
	{
		switch (item.ti_Tag)
		{
			
			case FV_ATTRIBUTE_HOOK: hook = (struct Hook *) item.ti_Data; break;
			case FV_ATTRIBUTE_STACKSIZE: stacksize = item.ti_Data; break;
			case FV_ATTRIBUTE_PRIORITY: pri = item.ti_Data; break;
			case FV_ATTRIBUTE_NAME: name = (STRPTR) item.ti_Data; break;
		}
	}

	LOD->pub.id_Send = F_METHOD_ID(SEND);
	LOD->pub.id_Pop = F_METHOD_ID(POP);
	LOD->pub.id_Wait = F_METHOD_ID(WAIT);
 
	IEXEC InitSemaphore(&LOD->arbitrer);

	if (hook)
	{
		LOD->pub.Process = IDOS_ CreateNewProcTags
		(
			#ifdef __MORPHOS__
			NP_CodeType, 		CODETYPE_PPC,
			NP_PPCStackSize, 	stacksize,
			#else
			NP_StackSize, 		stacksize,
			#endif

			#ifdef __amigaos4__
			NP_Child,           1,
			#endif
			
			NP_Entry,           (uint32) thread_main,
			NP_Priority,        pri,
			NP_Name,            (uint32) name,
			
			TAG_DONE
		);

		if (LOD->pub.Process)
		{
			uint32 rc;

			#ifdef DB_NEW
			F_Log(0, "Process 0x%08lx - FM_Thread_Send 0x%08lx (%ld)", (uint32) LOD->process, F_METHOD_ID(SEND), FM_Thread_Send);
			#endif

			rc = IFEELIN F_Do
			(
				Obj, F_METHOD_ID(SEND),
				
				FV_Thread_Init, F_MSG_THREAD_INIT(Class, Obj, hook)
			);
			
			if (rc)
			{
				/* User init msg */
											
				rc = IFEELIN F_Do(Obj, F_METHOD_ID(SEND), FV_Thread_Hello);
				
				if (rc)
				{
					return Obj;
				}
				else
				{
					IFEELIN F_Log(FV_LOG_DEV, "Process returns %ld to FV_Thread_Hello", rc);
				}
			}
			else
			{
				IFEELIN F_Log(FV_LOG_DEV, "Process (%s) returns %ld to FV_Thread_Init", name ? name : (STRPTR) "private", rc);
			}
		}
	}
	return NULL;
}
//+
///Thread_Dispose
F_METHOD(void, Thread_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (IFEELIN F_Do(Obj, F_METHOD_ID(SEND), FV_Thread_Bye) == FALSE)
	{
		/* process is not dead, but doesn't respond */
		
		IFEELIN F_Alert("Thread.Dispose",
			"Thread didn't respond to FV_Thread_Bye.\n"
			"The process 0x%08lx will be killed", (uint32) LOD->pub.Process);
 
		IEXEC Forbid();    
		
		if (LOD->pub.Process)
		{
			IEXEC RemTask((struct Task *) (LOD->pub.Process));

			LOD->pub.Process = NULL;
		}
		
		IEXEC Permit();
	}

	#ifdef DB_DISPOSE
	IFEELIN F_Log(0,"OBTAIN ARBITER >> BEGIN");
	#endif
	
	IEXEC ObtainSemaphore(&LOD->arbitrer);
	
	#ifdef DB_DISPOSE
	IFEELIN F_Log(0,"OBTAIN ARBITER >> DONE");
	#endif

	F_SUPERDO();
}
//+
///Thread_Get
F_METHOD(void, Thread_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_PRIORITY: F_STORE((LOD->pub.Process) ? LOD->pub.Process->pr_Task.tc_Node.ln_Pri : 0); break;
		case FV_ATTRIBUTE_PROCESS: F_STORE(LOD->pub.Process); break;
		case FV_ATTRIBUTE_PORT: F_STORE(LOD->pub.Port); break;
	}
}
//+
///Thread_Set
F_METHOD(void, Thread_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class) != NULL)
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_PRIORITY:
		{
			if (LOD->pub.Process)
			{
				IEXEC SetTaskPri((struct Task *) LOD->pub.Process, item.ti_Data);
			}
		}
		break;
	}
}
//+
///Thread_Send
F_METHODM(uint32, Thread_Send, FS_Thread_Send)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	
	/* prevent process death during a send */  

	IEXEC ObtainSemaphoreShared(&LOD->arbitrer);

	/* We check if our sub-process is not dead */
	if (LOD->pub.Process)
	{
		struct MsgPort *reply_port = IEXEC CreateMsgPort();
		
		if (reply_port)
		{
			FThreadMsg msg;
			
			msg.SysMsg.mn_Node.ln_Succ = NULL;
			msg.SysMsg.mn_Node.ln_Pred = NULL;
			msg.SysMsg.mn_Node.ln_Type = NT_MESSAGE;
			msg.SysMsg.mn_Node.ln_Pri = 0;
			msg.SysMsg.mn_Node.ln_Name = Obj; // recognition
			msg.SysMsg.mn_Length = sizeof(FThreadMsg);

			msg.Action = Msg->Cmd;
			msg.Params = (APTR) &Msg[1];
			msg.Return = 0;
	 
			msg.SysMsg.mn_ReplyPort = reply_port;

			#ifdef DB_PUSH
			IFEELIN F_Log(0, "Process (0x%08lx), 0x%08lx)(%s) Port (0x%08lx) Message (0x%08lx, %ld)",
				LOD->pub.Process, &LOD->pub.Process->pr_MsgPort, LOD->pub.Process->pr_Task.tc_Node.ln_Name,LOD->pub.Port,&msg, msg.Action);
			#endif

			//GOFROMIEL: ajouter un "time out"

			#ifdef F_ENABLE_SNOOZE
			uint32 i;
			#endif
			
			#ifdef DB_PUSH
			IFEELIN F_Log(0,"PUT message (0x%08lx) port (0x%08lx) reply (0x%08lx) >> BEGIN",
				&msg,
				(LOD->pub.Port) ? LOD->pub.Port : &LOD->pub.Process->pr_MsgPort,
				msg.SysMsg.mn_ReplyPort);
			#endif

			/* 'process->pr_MsgPort' must only be used for initialization */
  
			IEXEC PutMsg((LOD->pub.Port) ? LOD->pub.Port : &LOD->pub.Process->pr_MsgPort, (struct Message *) &msg);
			
			#ifdef DB_PUSH
			IFEELIN F_Log(0,"PUSH: RELEASE ARBITRER");
			#endif
			
			IEXEC ReleaseSemaphore(&LOD->arbitrer);
			
			#ifdef DB_PUSH
			IFEELIN F_Log(0,"PUT message (0x%08lx) port (0x%08lx) reply (0x%08lx) >> DONE",
				&msg,
				(LOD->pub.Port) ? LOD->pub.Port : &LOD->pub.Process->pr_MsgPort,
				msg.SysMsg.mn_ReplyPort);
			#endif
 
#ifdef F_ENABLE_SNOOZE
///
/*

YOMGUI: c'est quoi cette horreur !?

GOFROMIEL: c'est un timeout. Une meilleure idée ?? Sachant qu'on ne peut
pas utiliser le timer ?

YOMGUI: et pourquoi pas?
		
GOFROMIEL: Parce que pas  ;-)  Non  sérieux,  c'est  parce  que  la  méthode
peut-être  utilisée  par  l'input haldler de WinServer, ce qui résulte en un
joli système bloqué...  maintenant  que  j'y  pense  et  sourtout  que  j'ai
totalement  remanié  le  code  qui  venait  de  AppServer,  je pense ne plus
utiliser cette méthode... pitet que j'utiliserai  le  timer  plus  tard.  De
toute façon le snooze est désactivé pour l'instant.
 
*/

			for (i = 0 ; i < 200 ; i++)
			{
				struct Message *replied = IEXEC GetMsg(msg.SysMsg.mn_ReplyPort);
					
				if (replied)
				{
					if (replied == &msg)
					{
						#ifdef DB_SEND
						IFEELIN F_Log(0,"message replied");
						#endif
						
						break;
					}
					else
					{
						IFEELIN F_Log(0,"Got unexpected message 0x%08lx, waiting 0x%08lx",replied,&msg);
					}
				}
				else
				{
					#ifdef DB_SEND
					IFEELIN F_Log(0,"waiting (%ld)",i);
					#endif
 
					//IGRAPHICS WaitTOF();
				}
			}
		
			if (i == 199)
			{
				IFEELIN F_Log(0,"Time's out !!");
			}
		
//+
#else
			
			#ifdef DB_PUSH
			IFEELIN F_Log(0,"TRY GETMSG: port (0x%08lx) >> BEGIN",reply_port);
			#endif
						
			if (IEXEC GetMsg(reply_port) == NULL)
			{
				#ifdef DB_PUSH
				IFEELIN F_Log(0,"TRY GETMSG: port (0x%08lx) >> DONE FAILED",reply_port);
				IFEELIN F_Log(0,"WAIT: port (0x%08lx) signals (0x%08lx) >> BEGIN",reply_port,reply_port->mp_SigBit);
				#endif
				
/*

YOMGUI: bon là par conter je suis pas d'accord!  On  peut  pas  appeler  une
fonction  'push'  et  mettre  un wait() dedans... c'est antagoniste! Le push
doit revenir immédiatement. Sinon c'est un 'send' bloquant. D'ailleur  en  y
refléchissant.  Pourquoi pas ne pas proposer un envoi de msg bloquant. Ainsi
l'utilisateur serait obligé de réfléchire pour  faire  des  belles  GUI  non
bloquantes ;-)

Pour l'instant je le laisse car cela doit certainement  casser  les  FC  qui
l'utilisent tel quel.

L'autre truc est qu'il faudra refaire la méthode Thread_Dispose alors.

GOFROMIEL: Ui ui, pour le moment c'est le design qui  convient.  J'ai  voulu
modifier  le  nom  pour  faire  mon  malin,  j'aurais  mieux fait de laisser
"FM_Thread_Send". Si tu veux, je peux renomer cette méthode  et  ajouter  un
vrai  "FM_Thread_Push"  qui  aurait  besoin  qu'on  lui fournisse un port de
réponse, mais qui ne bloquerait pas...

*/

				IEXEC Wait(1 << reply_port->mp_SigBit);
				
				#ifdef DB_PUSH
				IFEELIN F_Log(0,"WAIT: port (0x%08lx) signals (0x%08lx) >> DONE",reply_port,reply_port->mp_SigBit);
				IFEELIN F_Log(0,"GETMSG: port (0x%08lx) >> BEGIN",reply_port);
				#endif
	 
				IEXEC GetMsg(reply_port);
				
				#ifdef DB_PUSH
				IFEELIN F_Log(0,"GETMSG: port (0x%08lx) >> DONE",reply_port);
				#endif
			}
			#ifdef DB_PUSH
			else
			{
				IFEELIN F_Log(0,"TRY GETMSG: port (0x%08lx) >> DONE SUCCESS",reply_port);
			}
			#endif
 
#endif
			
			#ifdef DB_PUSH
			IFEELIN F_Log(0,"DELETEMSGPORT (0x%08lx) >> BEGIN",reply_port);
			#endif

			IEXEC DeleteMsgPort(reply_port);
			
			#ifdef DB_PUSH
			IFEELIN F_Log(0,"DELETEMSGPORT (0x%08lx) >> DONE\n\n\n",reply_port);
			#endif
		
			return msg.Return;
		}
	}
	
	IEXEC ReleaseSemaphore(&LOD->arbitrer);

	return 0;
}
//+
///Thread_Pop
F_METHOD(struct Message *,Thread_Pop)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct Message *msg = NULL;

	if (LOD->pub.Port)
	{
		#ifdef DB_POP
		IFEELIN F_Log(0,"GETMSG: port (0x%08lx) >> BEGIN",LOD->pub.Port);
		#endif
		
		msg = IEXEC GetMsg(LOD->pub.Port);
		
		#ifdef DB_POP
		IFEELIN F_Log(0,"GETMSG: port (0x%08lx) >> DONE: message (0x%08lx)",LOD->pub.Port,msg);
		#endif
	}
 
	return msg;
}
//+
///Thread_Wait
F_METHODM(bits32,Thread_Wait,FS_Thread_Wait)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
					
	if (LOD->pub.Port)
	{
		LOD->pub.Signals = (1 << LOD->pub.Port->mp_SigBit) | Msg->Bits;
	}
	else
	{
		LOD->pub.Signals = Msg->Bits;
	}
 
	return IEXEC Wait(LOD->pub.Signals);
}
//+
