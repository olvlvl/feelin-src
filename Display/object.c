#include "Private.h"
 
///str_copy_length
uint32 str_copy_length(STRPTR Src,STRPTR Dst,uint32 Max)
{
   uint32 len = (uint32)(Dst);

   if (Src && Dst)
   {
	  for ( ; Max && *Src ; Max--)
	  {
		 *Dst++ = *Src++;
	  }
   }

   if (Dst)
   {
	  *Dst = 0;
   }

   return (uint32)(Dst) - len;
}
//+

/*

FM_New_______________________________________________________________________

   FA_Display_Screen [I..]

	  Crée une interface pour un écran existant.

	  FA_Display_Type [I..]

		 Spécifie le type de l'écran FA_Display_Screen.

		 FV_Display_Public - C'est un écran  public.  Normalement  il  doit
		 déjà  être  bloqué  avec  LockPubScreen(),  pour être sûr qu'il ne
		 disparaisse pas pendant la création de l'objet. L'écran public est
		 bloqué  de  nouveau lors de la méthode FM_New, il sera libéré lors
		 de la méthode FM_Dispose.  L'attribut  FA_Display_PublicName  doit
		 être fournis avec ce type d'écran.

		 FV_Display_Custom - C'est un écran privé. Vous êtes censé être  le
		 propriétaire  de  l'écran.  Pas moyen d'utiliser une fonction pour
		 bloquer l'écran.

*/

/************************************************************************************************
**** Methods ************************************************************************************
************************************************************************************************/

///D_New
F_METHOD(uint32,D_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_SCREEN:
		{
			LOD->Screen = (struct Screen *)(item.ti_Data);
		}
		break;

		case FV_ATTRIBUTE_NAME:
		{
			if ((LOD->PublicName = IFEELIN F_New(MAXPUBSCREENNAME + 1)) != NULL)
			{
				str_copy_length((STRPTR)(item.ti_Data),LOD->PublicName,MAXPUBSCREENNAME);
			}
		}
		break;
	}

	if (LOD->PublicName)
	{
		LOD->Screen = IINTUITION LockPubScreen(LOD->PublicName);

		if (LOD->Screen != NULL)
		{
			LOD->UserCount  = 1;
			LOD->Type       = FV_Display_Public;
		}
	}

	if (LOD->Screen)
	{
		LOD->ColorMap   = LOD->Screen->ViewPort.ColorMap;
		LOD->Width      = LOD->Screen->Width;
		LOD->Height     = LOD->Screen->Height;
		LOD->Depth      = IGRAPHICS GetBitMapAttr(LOD->Screen->RastPort.BitMap, BMA_DEPTH);

		if ((LOD->DrawInfo = IINTUITION GetScreenDrawInfo(LOD->Screen)) != NULL)
		{
			IFEELIN F_ClassDo(Class,NULL,FM_AddMember,Obj,FV_AddMember_Tail);

			return F_SUPERDO();
		}
	}
	else
	{
		IFEELIN F_Log(FV_LOG_USER,"Screen creation is not yet implemented");
	}

	return NULL;
}
//+
///D_Dispose
F_METHOD(void,D_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->DrawInfo)
	{
		IFEELIN F_ClassDo(Class,NULL,FM_RemMember,Obj);
	}

	IFEELIN F_Dispose(LOD->PublicName); LOD->PublicName = NULL;

	if (LOD->Screen)
	{
		if (LOD->DrawInfo)
		{
			struct in_Color *col;
			struct in_Palette *palette;

			while ((palette = LOD->palettes) != NULL)
			{
				IFEELIN F_Log(FV_LOG_DEV, "left behind palette (0x%08lx) references (%ld)", F_PALETTE_PUBLICIZE(palette), palette->references);

				palette->references = 1;

				IFEELIN F_Do(Obj, FM_Display_RemPalette, F_PALETTE_PUBLICIZE(palette));
			}

			while ((col = (struct in_Color *)(LOD->ColorList.Head)) != NULL)
			{
				IFEELIN F_Log(FV_LOG_DEV, "left behind color (0x%08lx) references (%ld)", col, col->UserCount);

				col->UserCount = 1;

				IFEELIN F_Do(Obj,FM_Display_RemColor,col);
			}

			IINTUITION FreeScreenDrawInfo(LOD->Screen,LOD->DrawInfo); LOD->DrawInfo = NULL;
		}

		if (LOD->Type == FV_Display_Public)
		{
			IINTUITION UnlockPubScreen(NULL,LOD->Screen);
		}

		if (FF_Display_Created & LOD->Flags)
		{
			IINTUITION CloseScreen(LOD->Screen);
		}

		LOD->Screen = NULL;
	}

	F_SUPERDO();
}
//+
///D_Get
F_METHOD(void,D_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;
	
	bool32 up = FALSE;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_COLORMAP:     F_STORE(LOD->ColorMap);    break;
		case FV_ATTRIBUTE_DEPTH:        F_STORE(LOD->Depth);       break;
		case FV_ATTRIBUTE_BITMAP:       F_STORE(LOD->Screen->RastPort.BitMap);  break;
		case FV_ATTRIBUTE_DRAWINFO:     F_STORE(LOD->DrawInfo);    break;
		case FV_ATTRIBUTE_HEIGHT:       F_STORE(LOD->Height);      break;
		case FV_ATTRIBUTE_NAME:         F_STORE(LOD->PublicName);  break;
		case FV_ATTRIBUTE_SCREEN:       F_STORE(LOD->Screen);      break;
		case FV_ATTRIBUTE_TITLE:        F_STORE(LOD->ScreenName);  break;
		case FV_ATTRIBUTE_WIDTH:        F_STORE(LOD->Width);       break;
		default:                      up = TRUE;
	}

	if (up) F_SUPERDO();
}
//+

/* The linking mecanism is deprecated. I should use  a  FC_Family  object...
waiting for a DisplayServer */

///D_AddMember
F_METHODM(int32,D_AddMember,FS_AddMember)
{
	struct LocalObjectData *new_lod = F_LOD(Class, Msg->Orphan);

	IFEELIN F_Do(CUD->Semaphore,FM_Lock,FF_Lock_Exclusive);

	if (Msg->Orphan)
	{
		FObject cur;
		struct LocalObjectData *cur_lod;

		for (cur = CUD->DisplayList.Head ; cur ; cur = cur_lod->Next)
		{
			cur_lod = F_LOD(Class,cur);

			if (cur == Msg->Orphan)
			{
				IFEELIN F_Log(FV_LOG_DEV,"%s{%08lx} already member",_object_classname(Msg->Orphan),Msg->Orphan);

				goto __error;
			}
		}
  /*
		if (!F_Do(Msg->Orphan,FM_Connect,Obj))
		{
		   IFEELIN F_Log(FV_LOG_DEV,"Unable to connect %s{%08lx}",_object_classname(Msg->Orphan),Msg->Orphan);

		   return FALSE;
		}
  */
		if (CUD->DisplayList.Tail)
		{
			struct LocalObjectData *nxt_lod;
			struct LocalObjectData *prv_lod; /* needed to be defined here because to goto */

			switch (Msg->Position)
			{
				case FV_AddMember_Head:
				{
__add_head:
					nxt_lod = F_LOD(Class,CUD->DisplayList.Head);

					nxt_lod->Prev      = Msg->Orphan;
					new_lod->Next      = CUD->DisplayList.Head;
					CUD->DisplayList.Head = Msg->Orphan;
				}
				break;

				case FV_AddMember_Tail:
				{
__add_tail:
					prv_lod = F_LOD(Class,CUD->DisplayList.Tail);

					prv_lod->Next      = Msg->Orphan;
					new_lod->Prev      = CUD->DisplayList.Tail;
					CUD->DisplayList.Tail = Msg->Orphan;
				}
				break;

				case FV_AddMember_Insert:
				{
					if (Msg->Previous)
					{
						prv_lod = F_LOD(Class,Msg->Previous);

						if (prv_lod->Next)
						{
							nxt_lod = F_LOD(Class,prv_lod->Next);

							new_lod->Next = prv_lod->Next;
							new_lod->Prev = Msg->Previous;
							prv_lod->Next = Msg->Orphan;
							nxt_lod->Prev = Msg->Orphan;
						}
						else goto __add_tail;
					}
					else goto __add_head;
				}
				break;

				default:
				{
					IFEELIN F_Log(0,"Invalid Position %ld",Msg->Position); goto __error;
				};
			}
		}
		else
		{
			CUD->DisplayList.Head = Msg->Orphan;
			CUD->DisplayList.Tail = Msg->Orphan;
		}

		IFEELIN F_Do(CUD->Semaphore,FM_Unlock);

		return TRUE;
	}

 __error:

	IFEELIN F_Do(CUD->Semaphore,FM_Unlock);

	return FALSE;
}
//+
///D_RemMember
F_METHODM(int32,D_RemMember,FS_RemMember)
{
	struct LocalObjectData *old_lod = F_LOD(Class,Msg->Member);

	#ifdef DB_REMMEMBER
	IFEELIN F_Log(0,"head (0x%08lx) tail (0x%08lx) new (0x%08lx)(%s)",CUD->DisplayList.Head,CUD->DisplayList.Tail,Msg->Member,_object_classname(Msg->Member));
	#endif

	IFEELIN F_Do(CUD->Semaphore,FM_Lock,FF_Lock_Exclusive);

	if (Msg->Member)
	{
		FObject cur;
		struct LocalObjectData *cur_lod;

		for (cur = CUD->DisplayList.Head ; cur ; cur = cur_lod->Next)
		{
			cur_lod = F_LOD(Class,cur);

			if (cur == Msg->Member)
			{
				break;
			}
		}

		if (!cur)
		{
			IFEELIN F_Log(FV_LOG_DEV,"Unknown member: %s{%08lx}",_object_classname(Msg->Member),Msg->Member);

			goto __error;
		}

//        IFEELIN F_Do(Msg->Member,FM_Disconnect,Obj);

		if (CUD->DisplayList.Head == Msg->Member)         // [OO,??] --OO??
		{
			if (CUD->DisplayList.Tail == Msg->Member)      // [OO,OO] --OO--
			{
				CUD->DisplayList.Head = NULL;
				CUD->DisplayList.Tail = NULL;
			}
			else                                            // [OO,oo] --OOoo
			{
				struct LocalObjectData *nxt_lod = F_LOD(Class,old_lod->Next);

				CUD->DisplayList.Head = old_lod->Next;
				nxt_lod->Prev = NULL;
			}
		}
		else if (CUD->DisplayList.Tail == Msg->Member)    // [oo,OO] ooOO--
		{
			struct LocalObjectData *prv_lod = F_LOD(Class,old_lod->Prev);

			CUD->DisplayList.Tail = old_lod->Prev;
			prv_lod->Next = NULL;
		}
		else                                               // [oo,oo] ooOOoo
		{
			struct LocalObjectData *prv_lod = F_LOD(Class,old_lod->Prev);
			struct LocalObjectData *nxt_lod = F_LOD(Class,old_lod->Next);

			prv_lod->Next = old_lod->Next;
			nxt_lod->Prev = old_lod->Prev;
		}

		old_lod->Next = NULL;
		old_lod->Prev = NULL;

		#ifdef DB_REMMEMBER
		IFEELIN F_Log(0,"head (0x%08lx) tail (0x%08lx) new (0x%08lx)",CUD->DisplayList.Head,CUD->DisplayList.Tail,Msg->Member);
		#endif

		IFEELIN F_Do(CUD->Semaphore,FM_Unlock);

		return TRUE;
	}

__error:

	IFEELIN F_Do(CUD->Semaphore,FM_Unlock);

	return FALSE;
}
//+

/* The following methods  should  only  be  invoked  on  the  class  itself.
Displays  are shared objects, but due to Feelin OOS limitation these methods
need to be called to create (or share) a FC_Display object. Thus, the  'Obj'
variable  has  no use. These methods are only valid until a DisplayServer is
created. */

struct FS_Display_Find                       { STRPTR Name; };
struct FS_Display_Delete                     { FObject Display; };

///D_Find
F_METHODM(FObject,D_Find,FS_Display_Find)
{
	struct LocalObjectData *LOD;

	F_LOCK_ARBITER;

	for (Obj = CUD->DisplayList.Head ; Obj ; Obj = LOD->Next)
	{
		LOD = F_LOD(Class,Obj);

		if (IFEELIN F_StrCmp(Msg->Name,LOD->PublicName,ALL) == 0)
		{
			break;
		}
	}

	F_UNLOCK_ARBITER;

	return Obj;
}
//+
///D_Create
F_METHOD(FObject,D_Create)
{
	STRPTR name = IFEELIN F_New(MAXPUBSCREENNAME + 1);

	if (name != NULL)
	{
		IINTUITION GetDefaultPubScreen(name);
	}

	Obj = (FObject) IFEELIN F_ClassDo(Class, NULL, F_METHOD_ID(FIND), name);

	if (Obj != NULL)
	{
		struct LocalObjectData *LOD = F_LOD(Class,Obj);

		LOD->UserCount++;
	}
	else
	{
		struct Screen *scr = IINTUITION LockPubScreen(name);

		if (scr != NULL)
		{
			Obj = IFEELIN F_NewObj
			(
				Class->Name,

				F_ATTRIBUTE_ID(NAME), name,

				TAG_DONE
			);

			IINTUITION UnlockPubScreen(NULL,scr);
		}
	}

	IFEELIN F_Dispose(name);

	return Obj;
}
//+
///D_Delete
F_METHODM(void,D_Delete,FS_Display_Delete)
{
   struct LocalObjectData *LOD;

   F_LOCK_ARBITER;

   for (Obj = CUD->DisplayList.Head ; Obj ; Obj = LOD->Next)
   {
	   LOD = F_LOD(Class,Obj);

	   if (Msg->Display == Obj)
	   {
		   if (--LOD->UserCount == 0)
		   {
			   IFEELIN F_DisposeObj(Obj);
		   }

		   break;
	   }
   }

   if (!Obj)
   {
	   IFEELIN F_Log(FV_LOG_DEV,"Unknown Display 0x%08lx",Obj);
   }

   F_UNLOCK_ARBITER;
}
//+

