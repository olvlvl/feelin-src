#include "Private.h"

//#define DB_HANDLE
//#define DB_WICHWINDOW
 
/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

#if F_CODE_DEPRECATED
///dnd_create_image
STATIC void dnd_create_image(struct FeelinDnDInfo *dnd,BOOL Transp)
{
	Bob_Update(dnd->Bob,0xABBA,0xABBA);

	if (Transp) Bob_MakeGhost(dnd->Bob);
	else        Bob_MakeSolid(dnd->Bob);

	IGRAPHICS ClipBlit(dnd->Render->RPort,dnd->SourceBox.x,dnd->SourceBox.y,
			&dnd->Bob->rport,0,0,dnd->SourceBox.w,dnd->SourceBox.h,0x0C0);
 //   Bob_Update(dnd->Bob,dnd->Scr->MouseX - dnd->offx,dnd->Scr->MouseY - dnd->offy);
}
//+
#else
///dnd_create_image
STATIC void
dnd_create_image(struct FeelinDnDInfo *dnd, BOOL Transp)
{
	LONG cx, cy, w, h;

	cx = MAX(dnd->Bob->box.x, 0);
	cy = MAX(dnd->Bob->box.y, 0);
	w = dnd->Bob->box.w - cx + dnd->Bob->box.x;
	h = dnd->Bob->box.h - cy + dnd->Bob->box.y;

	xblit(dnd->Bob->backup, 0, 0, dnd->Bob->drp->BitMap, cx, cy, w, h);

	IGRAPHICS ClipBlit
	(
		dnd->Render->RPort,
		
		dnd->SourceBox.x, dnd->SourceBox.y,
		
		dnd->Bob->srp,
		
		0, 0, dnd->SourceBox.w, dnd->SourceBox.h,
		
		0xc0
	);
			
	if (Transp)
	{
		Bob_MakeGhost(dnd->Bob);
	}
	else
	{
		Bob_MakeSolid(dnd->Bob);
	}
}
//+
#endif

/* j'ai mis Bob_Rem() hors fonction pour les changements de Yom */

///dnd_create
SAVEDS STATIC FDnDInfo * dnd_create(FClass *Class,FObject Obj,FObject Source,FBox *SourceBox,FRender *Render)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
   
	struct Window *win = (struct Window *) IFEELIN F_Get(Render->Window, FA_Window_System);

	if (win)
	{
		FDnDInfo *dnd = IFEELIN F_New(sizeof (FDnDInfo));
		
		if (dnd != NULL)
		{
			dnd->Source = Source;
			
			dnd->SourceBox.x = SourceBox->x;
			dnd->SourceBox.y = SourceBox->y;
			dnd->SourceBox.w = SourceBox->w;
			dnd->SourceBox.h = SourceBox->h;
			dnd->SourceWin   = win;
			
			dnd->Scr = win->WScreen;
			dnd->Render = Render;
			
			dnd->offx = win->MouseX - dnd->SourceBox.x;
			dnd->offy = win->MouseY - dnd->SourceBox.y;

			dnd->handler = (APTR) IFEELIN F_Do(dnd->Render->Window, FM_Window_CreateEventHandler,

				FA_EventHandler_Events,     FF_EVENT_TICK | FF_EVENT_BUTTON | FF_EVENT_MOTION,
				FA_EventHandler_Target,     Obj,
				FA_EventHandler_Priority,   100,

				TAG_DONE);

			if (dnd->handler != NULL)
			{
				dnd->Bob = Bob_Create(&dnd->Scr->RastPort, SourceBox, TRUE);
 
				if (dnd->Bob != NULL)
				{
					dnd_create_image(dnd,TRUE);

					#if 0
					Bob_Add(dnd->Bob);
					#endif
					
					LOD->flags |= FF_APPSERVER_DRAGGING;

					return (LOD->DnDInfo = dnd);
				}

				IFEELIN F_Do(dnd->Render->Window, FM_Window_DeleteEventHandler, dnd->handler);

				dnd->handler = NULL;
			}

			IFEELIN F_Dispose(dnd);
		}
	}
	return NULL;
}
//+
///dnd_delete
SAVEDS STATIC void dnd_delete(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
 
	FDnDInfo *dnd = LOD->DnDInfo;
  
	LOD->flags &= ~FF_APPSERVER_DRAGGING;
	
	if (dnd)
	{
		if (dnd->Bob)
		{
			#if 0
			Bob_Rem(dnd->Bob);
			#endif
			Bob_Delete(dnd->Bob);
		}

		if (dnd->handler != NULL)
		{
			IFEELIN F_Do(dnd->Render->Window, FM_Window_DeleteEventHandler, dnd->handler);

			dnd->handler = NULL;
		}

		LOD->DnDInfo = NULL;
   
		IFEELIN F_Dispose(dnd);
	}
}
//+

///appserver_which_window
SAVEDS STATIC FObject appserver_which_window(FObject Obj,uint16 MouseX,uint16 MouseY,FRender *Render)
{
	struct Screen *scr;
	FObject win=NULL;

///DB_WICHWINDOW
#ifdef DB_WICHWINDOW
	IFEELIN F_Log(0,"Render 0x%08lx - Mouse (%03ld:%03ld)",Render,MouseX,MouseY);
#endif
//+

	IFEELIN F_Do(Obj,FM_Lock,FF_Lock_Shared);

	if ((scr = (struct Screen *) IFEELIN F_Get(Render->Display,(uint32) "FA_Display_Screen")) != NULL)
	{
		struct Layer *layer;

		if ((layer = ILAYERS WhichLayer(&scr->LayerInfo,MouseX,MouseY)) != NULL)
		{
			FFamilyNode *app_node;

			for (app_node = (FFamilyNode *) IFEELIN F_Get(Obj,FA_Family_Head) ; app_node ; app_node = app_node->Next)
			{
				FFamilyNode *win_node;

				for (win_node = (FFamilyNode *) IFEELIN F_Get(app_node->Object,FA_Family_Head) ; win_node ; win_node = win_node->Next)
				{
					if (layer->Window == (struct Window *) IFEELIN F_Get(win_node->Object, FA_Window_System))
					{
///DB_WICHWINDOW
#ifdef DB_WICHWINDOW
						IFEELIN F_Log(0,"   >>> %s{%08lx}",_object_classname(win_node->Object),win_node->Object);
#endif
//+
						win = win_node->Object; break;
					}
				}
				if (win) break;
			}
		}
	}

	IFEELIN F_Do(Obj,FM_Unlock);

	return win;
}
//+
 
/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/
 
///AppServer_DnDHandle
F_METHODM(void,AppServer_DnDHandle,FS_AppServer_DnDHandle)
{
   dnd_create(Class,Obj, Msg->Source,Msg->Box,Msg->Render);
}
//+
///AppServer_DnDEvent
F_METHODM(bits32,AppServer_HandleEvent,FS_Widget_HandleEvent)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
 
	FEvent *fev = Msg->Event;
	FDnDInfo *dnd = LOD->DnDInfo;
	int8 done = FALSE;

//    IFEELIN F_Log(0,"Class %04lx - Code %04lx - Flags %08lx",Msg->FEv->Class,Msg->FEv->Code,Msg->FEv->Flags);
  
	switch (fev->Class)
	{
		case FF_EVENT_TICK:
		{
			if (dnd->Target)
			{
				IFEELIN F_Do
				(
					dnd->Target, FM_Widget_DnDReport,
					
					dnd->msex - dnd->TargetWin->LeftEdge,
					dnd->msey - dnd->TargetWin->TopEdge,
					
					(uint32) dnd->Source,
					FV_Widget_DnDReport_Timer,
					(uint32) &dnd->BobBox
				);
			}
		}
		break;

		case FF_EVENT_KEY:
		{
			done = TRUE;
		}
		break;

		case FF_EVENT_BUTTON:
		{
			#if 0
			Bob_Update(dnd->Bob,0xABBA,0xABBA);
			#endif

			if ((fev->Code == FV_EVENT_BUTTON_SELECT) && (!(FF_EVENT_BUTTON_DOWN & fev->Flags)))
			{
				if (dnd->Target)
				{
					
					/* FIXME: suspicious code, but we don't care  since  the
					whole dnd will soon be handled by WinServer */
 
					#if 1

					FAreaPublic *pub = F_GET_AREA_PUBLIC(dnd->Target);
					
					if ((pub != NULL) && (pub->Render != NULL))
					{
						IFEELIN F_Do
						(
							pub->Render->Application, FM_Application_PushMethod,
							
							dnd->Target, FM_Widget_DnDDrop, 3,

							dnd->msex - dnd->TargetWin->LeftEdge,
							dnd->msey - dnd->TargetWin->TopEdge,

							dnd->Source
						);
					}
					
					#else
					
					IFEELIN F_Do
					(
						dnd->Target, FM_Widget_DnDDrop,
						
						dnd->msex - dnd->TargetWin->LeftEdge,
						dnd->msey - dnd->TargetWin->TopEdge,
						
						(uint32) dnd->Source
					);
					
					#endif
			   }
			}

			done = TRUE;
		}
		break;

		case FF_EVENT_MOTION:
		{
			dnd->msex = dnd->Scr->MouseX;
			dnd->msey = dnd->Scr->MouseY;
			
			if (dnd->Target)
			{
				if (dnd->msex >= dnd->TargetBox.x &&
					dnd->msex <= dnd->TargetBox.x + dnd->TargetBox.w - 1 &&
					dnd->msey >= dnd->TargetBox.y &&
					dnd->msey <= dnd->TargetBox.y + dnd->TargetBox.h - 1)
				{
					IFEELIN F_Do
					(
						dnd->Target, FM_Widget_DnDReport,
						
						dnd->msex - dnd->TargetWin->LeftEdge,
						dnd->msey - dnd->TargetWin->TopEdge,
						
						(uint32) dnd->Source,
						FV_Widget_DnDReport_Motion,
						(uint32) &dnd->BobBox
					);
				}
				else
				{
					dnd_create_image(dnd,TRUE);

					IFEELIN F_Do(dnd->Target, FM_Widget_DnDFinish, dnd->Source);

					dnd->Target = NULL;
					dnd->TargetWin = NULL;
				}
			}
			
			if (!dnd->Target)
			{
				FObject winobj = appserver_which_window(Obj,dnd->msex,dnd->msey,dnd->Render);

				if (winobj != NULL)
				{
					struct Window *win=NULL;
					FObject sub=NULL;

					IFEELIN F_Do
					(
						winobj, FM_Get,
						
						FA_Window_System, (uint32) &win,
						FA_Child, (uint32) &sub,
						
						TAG_DONE
					);

//                    IFEELIN F_Log(0,"DnDQuery ROOT %s{%08lx}",_object_classname(sub),sub);
						
					dnd->Target = (FObject) IFEELIN F_Do
					(
						sub, FM_Widget_DnDQuery,
						
						dnd->msex - win->LeftEdge,
						dnd->msey - win->TopEdge,
						
						(uint32) dnd->Source,
						(uint32) &dnd->TargetBox
					);
 
					if (dnd->Target != NULL)
					{
///DB_HANDLE
						#ifdef DB_HANDLE
						IFEELIN F_Log(0,"DEST 0x%08lx (%s)",dnd->Target,_object_classname(dnd->Target));
						#endif
//+
						dnd->TargetWin    = win;
						dnd->TargetBox.x += win->LeftEdge;   // convert window coordinates to screen coordinates
						dnd->TargetBox.y += win->TopEdge;    // convert window coordinates to screen coordinates

						IFEELIN F_Do(dnd->Target, FM_Widget_DnDBegin, (uint32) dnd->Source);

						dnd_create_image(dnd,FALSE);
					}
					else
					{
//                        IFEELIN F_Log(0,"DnDQuery ROOT %s{%08lx} >> FAILED",_object_classname(sub),sub);
						dnd->TargetWin = NULL;
					}
				}
			}

			Bob_Update(dnd->Bob,dnd->msex - dnd->offx,dnd->msey - dnd->offy);

			if (dnd->Target)
			{
				dnd->BobBox.x = dnd->msex - dnd->offx - dnd->TargetWin->LeftEdge;
				dnd->BobBox.y = dnd->msey - dnd->offy - dnd->TargetWin->TopEdge;
				dnd->BobBox.w = dnd->SourceBox.w;
				dnd->BobBox.h = dnd->SourceBox.h;
///DB_HANDLE
				#ifdef DB_HANDLE
				IFEELIN F_Log(0,"TargetWin 0x%08lx - %ld : %ld - %ld x %ld",dnd->TargetWin,dnd->BobBox.x,dnd->BobBox.y,dnd->BobBox.w,dnd->BobBox.h);
				#endif
//+
			}
		 }
		 break;
	}

	if (done)
	{
		FObject target = dnd->Target;
		FObject source = dnd->Source;

		#if 0
		Bob_Update(dnd->Bob,0xABBA,0xABBA);
		#endif
		
		dnd_delete(Class,Obj);

		IFEELIN F_Do(target, FM_Widget_DnDFinish, (uint32) source);
	}
  
	return FF_HandleEvent_Eat;
}
//+

