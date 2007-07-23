#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Render_New
F_METHOD(FObject,Render_New)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	FObject parent = NULL;
	FObject application = NULL;
	FObject display = NULL;
	FObject window = NULL;
	
	while  (IFEELIN F_DynamicNTI(&Tags, &item, Class))
	switch (item.ti_Tag)
	{
		case FA_Render_Friend: parent = (FObject) item.ti_Data; break;
		case FA_Render_Application: application = (FObject) item.ti_Data; break;
		case FA_Render_Display: display = (FObject) item.ti_Data; break;
		case FA_Render_Window: window = (FObject) item.ti_Data; break;
	}

	IFEELIN F_Do(parent, FM_Get,
	
		FA_Render_Application, &LOD->Public.Application,
		FA_Render_Display, &LOD->Public.Display,
		FA_Render_Window, &LOD->Public.Window,
		FA_Render_RPort, &LOD->Public.RPort,
		FA_Render_Palette, &LOD->Public.Palette,
		
		TAG_DONE);

	if (application)
	{
		LOD->Public.Application = application;
	}
	if (display)
	{
		LOD->Public.Display = display;
	}
	if (window)
	{
		LOD->Public.Window = window;
	}
/*
	if (LOD->Public.Application == NULL)
	{
		LOD->Public.Application = (FObject) IFEELIN F_Get(LOD->Public.Window,FA_Application);
	}
*/
	if (LOD->Public.Display == NULL)
	{
		LOD->Public.Display = (FObject) IFEELIN F_Get(LOD->Public.Application,FA_Application_Display);
	}

	if (LOD->Public.Application && LOD->Public.Display)
	{
		struct BitMap *bmp;

		if ((bmp = (struct BitMap *) IFEELIN F_Get(LOD->Public.Display,(uint32) "FA_Display_BitMap")) != NULL)
		{
			if (IGRAPHICS GetBitMapAttr(bmp,BMA_DEPTH) > 8)
			{
				LOD->Public.Flags |= FF_Render_TrueColors;
			}
		}
	 
		IFEELIN F_DoA(Obj,FM_Set,Msg);

		return Obj;
	}

	IFEELIN F_Log(FV_LOG_DEV,"Application 0x%08lx - Display 0x%08lx",LOD->Public.Application,LOD->Public.Display);

	return NULL;
}
//+
///Render_Get

F_METHOD(void,Render_Get)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while  (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Render_Application:   F_STORE(LOD->Public.Application); break;
		case FA_Render_Display:       F_STORE(LOD->Public.Display); break;
		case FA_Render_Window:        F_STORE(LOD->Public.Window); break;
		case FA_Render_RPort:         F_STORE(LOD->Public.RPort); break;
		case FA_Render_Forbid:        F_STORE(LOD->ForbidNest > 0); break;
	}
}
//+
///Render_Set
F_METHOD(void,Render_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Render_RPort:
		{
			LOD->Public.RPort = (struct RastPort *) item.ti_Data;
		}
		break;

		case FA_Render_Palette:
		{
			LOD->Public.Palette = (FPalette *) item.ti_Data;
		}
		break;

		case FA_Render_Forbid:
		{
			#if 0

			IFEELIN F_Log(0,"FA_Render_Forbid is disabled");

			#else

			if (item.ti_Data)
			{
				LOD->ForbidNest++;
			}
			else
			{
				LOD->ForbidNest--;
			}

			if (LOD->ForbidNest == 1)
			{
				LOD->Public.Flags |= FF_Render_Forbid;
			}
			else if (LOD->ForbidNest == 0)
			{
				LOD->Public.Flags &= ~FF_Render_Forbid;
			}

			#endif
		}
		break;

		case FA_Render_Window:
		{
			LOD->Public.Window = (FObject)(item.ti_Data);
		}
		break;
	}
}
//+

///Render_AddClipRegion
F_METHODM(struct FeelinTraceClip *,Render_AddClipRegion,FS_Render_AddClipRegion)
{
	struct LocalObjectData  *LOD  = F_LOD(Class,Obj);
	struct FeelinTraceLayer *trace_lay  = NULL;
	struct FeelinTraceClip  *trace_clip = NULL;
	struct Window           *win;
	struct Layer            *lay;

	#ifdef DB_SEMAPHORE
	IFEELIN F_Log(0,"RENDER_LOCK");
	#endif

	F_LOCK_ARBITER;

	win = (struct Window *) IFEELIN F_Get(LOD->Public.Window, FA_Window_System);

	if (win)
	{
		lay = win->WLayer;
	}
	else if (LOD->Public.RPort)
	{
		lay = LOD->Public.RPort->Layer;
	}
	else
	{
		lay = NULL;
	}

	if (!lay)
	{
		IFEELIN F_Log(FV_LOG_DEV,"Unable to obtain layer"); goto __error;
	}

	for (trace_lay = CUD->TraceLayers ; trace_lay ; trace_lay = trace_lay->Next)
	{
		if (trace_lay->Layer == lay)
		{
			if (trace_lay->TraceClipList.Tail)
			{
				struct Region *region;

				if ((region = ((struct FeelinTraceClip *)(trace_lay->TraceClipList.Tail))->Region) != NULL)
				{

					/* Si  l'on  essaie  de  clipper  une  zone  totalement
					exterieure    à   la   zone   précédente   (supérieure)
					l'opération  échoue  puisque   soit   le   rendue   est
					totalement  à  l'extérieur  de la zone (non visible, ou
					zone protégée), soit l'objet est partiellement dans  la
					zone précédente et dans ce cas, seule la partie commune
					sera modifiable. */

					if (Msg->Region->bounds.MinX <= region->bounds.MinX &&
						Msg->Region->bounds.MaxX >= region->bounds.MaxX &&
						Msg->Region->bounds.MinY <= region->bounds.MinY &&
						Msg->Region->bounds.MaxY >= region->bounds.MaxY)
					{
						goto __error;
					}
				}
			}
			break;
		}
	}

	if (!trace_lay)
	{
		if ((trace_lay = IFEELIN F_New(sizeof (struct FeelinTraceLayer))) != NULL)
		{
			trace_lay->Layer = lay;

			if (CUD->TraceLayers)
			{
				struct FeelinTraceLayer *prev = CUD->TraceLayers;

				while (prev->Next) prev = prev->Next;
				prev->Next = trace_lay;
			}
			else
			{
				CUD->TraceLayers = trace_lay;
			}
		}
	}

	if (!trace_lay)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to allocate TraceLayer"); goto __error;
	}

	trace_clip = IFEELIN F_New(sizeof (struct FeelinTraceClip));

	if (!trace_clip)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to allocate TraceClip"); goto __error;
	}

	if ((trace_clip->Region = IGRAPHICS NewRegion()) != NULL)
	{
		if (!IGRAPHICS OrRegionRegion(Msg->Region,trace_clip->Region))
		{
			IGRAPHICS DisposeRegion(trace_clip->Region); trace_clip->Region = NULL;
		}
	}

	if (!trace_clip->Region)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to duplicate region"); goto __error;
	}

	if (trace_lay->TraceClipList.Tail)
	{
		IGRAPHICS AndRegionRegion(((struct FeelinTraceClip *)(trace_lay->TraceClipList.Tail))->Region,trace_clip->Region);
	}

	if (win && !(win->Flags & WFLG_SIMPLE_REFRESH))
	{
		ILAYERS LockLayerInfo(&win->WScreen->LayerInfo);

		trace_clip->PreviousRegion = ILAYERS InstallClipRegion(lay,trace_clip->Region);

		ILAYERS UnlockLayerInfo(&win->WScreen->LayerInfo);
	}
	else
	{
		trace_clip->PreviousRegion = ILAYERS InstallClipRegion(lay,trace_clip->Region);
	}

	IFEELIN F_LinkTail(&trace_lay->TraceClipList,(FNode *) trace_clip);

	#ifdef DB_ADDCLIP
	IFEELIN F_Log(0,"TClip 0x%08lx - TLayer 0x%08lx (0x%08lx) - Region 0x%08lx (0x%08lx)",trace_clip,trace_lay,trace_lay->Layer,trace_clip->Region,trace_clip->PreviousRegion);
	#endif

	#ifdef DB_SEMAPHORE
	IFEELIN F_Log(0,"RENDER_UNLOCK");
	#endif

	F_UNLOCK_ARBITER;

	return trace_clip;

 /*** error handling ********************************************************/

 __error:

	if (trace_clip)
	{
		IFEELIN F_Dispose(trace_clip);
	}

	if (trace_lay)
	{
		if (!trace_lay->TraceClipList.Head)
		{
			struct FeelinTraceLayer *node;
			struct FeelinTraceLayer *prev=NULL;

			for (node = CUD->TraceLayers ; node ; node = node->Next)
			{
				if (node == trace_lay)
				{
					if (prev)
					{
						prev->Next = node->Next;
					}
					else
					{
						CUD->TraceLayers = node->Next;
					}

					IFEELIN F_Dispose(node);

					break;
				}
				prev = node;
			}
		}
	}

	#ifdef DB_SEMAPHORE
	IFEELIN F_Log(0,"RENDER_UNLOCK");
	#endif

	F_UNLOCK_ARBITER;

	return NULL;
}
//+
///Render_AddClip
F_METHODM(struct FeelinTraceClip *,Render_AddClip,FS_Render_AddClip)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct FeelinTraceClip *trace_clip = NULL;

	if (Msg->Rect->x1 <= Msg->Rect->x2 &&
		Msg->Rect->y1 <= Msg->Rect->y2 && LOD->Public.RPort)
	{
		struct Region *region;

		if ((region = IGRAPHICS NewRegion()) != NULL)
		{
			IGRAPHICS OrRectRegion(region,(struct Rectangle *) Msg->Rect);

			trace_clip = (struct FeelinTraceClip *) IFEELIN F_Do(Obj,FM_Render_AddClipRegion,region);

			IGRAPHICS DisposeRegion(region);
		}
	}
	return trace_clip;
}
//+
///Render_RemClip
F_METHODM(void,Render_RemClip,FS_Render_RemClip)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct Window *win;
	struct Layer *lay;

	win = (APTR) IFEELIN F_Get(LOD->Public.Window, FA_Window_System);

	if (win)
	{
		lay = win->WLayer;
	}
	else if (LOD->Public.RPort)
	{
		lay = LOD->Public.RPort->Layer;
	}
	else
	{
		lay = NULL;
	}

	#ifdef DB_SEMAPHORE
	IFEELIN F_Log(0,"RENDER_LOCK");
	#endif

	F_LOCK_ARBITER;

	if (Msg->Handle)
	{
		struct FeelinTraceLayer *trace_layer;
		struct FeelinTraceClip  *trace_clip  = NULL;

		for (trace_layer = CUD->TraceLayers ; trace_layer ; trace_layer = trace_layer->Next)
		{
			if (!lay || trace_layer->Layer == lay)
			{
				for (trace_clip = (struct FeelinTraceClip *) trace_layer->TraceClipList.Tail ; trace_clip ; trace_clip = trace_clip->Prev)
				{
					if (trace_clip == Msg->Handle) break;
				}
			}
			if (trace_clip) break;
		}

		#ifdef DB_REMCLIP
		IFEELIN F_Log(0,"trace_lay 0x%08lx - trace_clip 0x%08lx",trace_layer,trace_clip);
		#endif

		if (trace_clip)
		{
			IFEELIN F_LinkRemove(&trace_layer->TraceClipList,(FNode *) trace_clip);

			if (!trace_layer->TraceClipList.Head)
			{
				struct FeelinTraceLayer *node;
				struct FeelinTraceLayer *prev=NULL;

				for (node = CUD->TraceLayers ; node ; node = node->Next)
				{
					if (node == trace_layer)
					{
						if (prev)
						{
							prev->Next = node->Next;
						}
						else
						{
							CUD->TraceLayers = node->Next;
						}

						IFEELIN F_Dispose(node);

						break;
					}
					prev = node;
				}
			}

			if (win && !(win->Flags & WFLG_SIMPLE_REFRESH))
			{
				ILAYERS LockLayerInfo(&win->WScreen->LayerInfo);
				ILAYERS InstallClipRegion(lay,trace_clip->PreviousRegion);
				ILAYERS UnlockLayerInfo(&win->WScreen->LayerInfo);
			}
			else
			{
				ILAYERS InstallClipRegion(lay,trace_clip->PreviousRegion);
			}

			IGRAPHICS DisposeRegion(trace_clip->Region);

			#ifdef DB_CLIP
			IFEELIN F_Log(0,"trace_clip 0x%08lx - trace_layer 0x%08lx",trace_clip,trace_layer);
			#endif

			IFEELIN F_Dispose(trace_clip);
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV,"Unknown Handle 0x%08lx",Msg->Handle);
		}
	}
	else if (lay)
	{
	   ILAYERS InstallClipRegion(lay,NULL);
	}

	#ifdef DB_SEMAPHORE
	IFEELIN F_Log(0,"RENDER_UNLOCK");
	#endif

	F_UNLOCK_ARBITER;
}
//+

/*  CreateBuffer()  and  DeleteBuffer()  are  not  really  what  I  want...
FC_Render needs a serious update !! */
 
///Render_CreateBuffer
F_METHODM(FObject,Render_CreateBuffer,FS_Render_CreateBuffer)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct RastPort *rp;

	if ((rp = IFEELIN F_New(sizeof (struct RastPort))) != NULL)
	{
		struct BitMap *bmfrd;
		uint32 depth;

		if (LOD->Public.RPort)
		{
			bmfrd = LOD->Public.RPort->BitMap;
			depth = IGRAPHICS GetBitMapAttr(bmfrd,BMA_DEPTH);
		}
		else
		{
			IFEELIN F_Do(LOD->Public.Display,FM_Get,

				"FA_Display_BitMap",   &bmfrd,
				"FA_Display_Depth",    &depth,
			   
				TAG_DONE);
		}

		#ifdef DB_CREATEBUFFER
		IFEELIN F_Log(0,"fiend_bitmap 0x%08lx - depth %ld",bmfrd,depth);
		#endif

		IGRAPHICS InitRastPort(rp);

		if ((rp->BitMap = IGRAPHICS AllocBitMap(Msg->Width,Msg->Height,depth,BMF_INTERLEAVED,bmfrd)) != NULL)
		{
			FRender *render = RenderObject,

				FA_Render_Friend,   Obj,
				FA_Render_RPort,    rp,
			   
				End;

			if (render)
			{
				#ifdef DB_CREATEBUFFER
				IFEELIN F_Log(0,"render 0x%08lx",render);
				#endif

   //            IFEELIN F_DisposeObj(render);

				return render;
			}
			else
			{
				IGRAPHICS FreeBitMap(rp->BitMap);
			}
		}
	
		#ifdef __AROS__
		DeinitRastPort(rp);
		#endif

		IFEELIN F_Dispose(rp);
	}
	return NULL;
}
//+
///Render_DeleteBuffer
F_METHODM(void,Render_DeleteBuffer,FS_Render_DeleteBuffer)
{
	#ifdef DB_DELETEBUFFER
	IFEELIN F_Log(0,"buffer 0x%08lx",Msg->Buffer);
	#endif

	if (Msg->Buffer)
	{
		if (IFEELIN F_Get(Msg->Buffer,FA_Render_RPort))
		{
			if (Msg->Buffer->RPort->BitMap)
			{
				IGRAPHICS FreeBitMap(Msg->Buffer->RPort->BitMap);
			}
		
			#ifdef __AROS__
			DeinitRastPort(Msg->Buffer->RPort);
			#endif
 
			IFEELIN F_Dispose(Msg->Buffer->RPort);
		}

		IFEELIN F_DisposeObj(Msg->Buffer);
	}
}
//+
