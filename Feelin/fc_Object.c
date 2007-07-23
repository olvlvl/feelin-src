/*
**    FC_Object
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************

$VER: 05.00 (2007/07/22)

	The FA_ID attribute is no longer defined by the  Object  class.  Because
	only  Elements objects were using it, the attribute as been moved to the
	Element class and renamed as FA_Element_Id.

	Same goes for the FA_Parent attribute.

	Ajout des méthodes  AddListener()  and  RemoveListener().  Une  nouvelle
	façon de réagir à la modification des attributs.

	The object's arbiter is now created in a safer way  then  before  during
	the Lock method.

	Add the virtual method GetObjectById.

$VER: 04.01 (2005/09/29)

	Added AmigaOS 4 support.
	
	Removed FM_Area_Draw support from notification, it was a bad idea !

	Every FC_Object can be turned to semaphore on demand. FC_Semaphore thus
	now deprecated.
	
	The new FM_SetAs method can be used instead of FM_SetAsString (which  is
	now considered deprecated) to set a formated string. It can also be used
	to set a decimal number, an hexadecimal number or a 4CC number converted
	from a given string.
	
	FM_CallHook and FM_CallHookEntry no longer call code with FeelinBase in
	a6. Using hooks should be avoided as much as possible.
	
************************************************************************************************/

#include "Private.h"

//#define F_ENABLE_LOCK_NOTIFY
//#define CS_WARN_NOTIFICATION_LOOP
//#define DB_NOTIFY
//#define DB_HANDLENOTIFY
//#define DB_PERSIST

///Header
struct FeelinNotifyHandler
{
	struct FeelinNotifyHandler *	next;
	bits32                          flags;
	uint32                          attribute;
	uint32                          value;
	FObject                         target;
	uint32                          method;
	uint32                          count;
// uint32 Msg[]
// uint32 MsgCopy[]
};

#define FF_NOTIFY_PERFORMING                    (1 << 0)

#ifdef F_NEW_LISTENERS

struct in_Object_Listener
{
	struct in_Object_Listener * 	next;
	FClassAttribute	*				attribute;
	struct Hook	*					callback;
	bits32							flags;
};

#define FF_LISTENER_PERFORMING                  (1 << 31)

#endif

struct LocalObjectData
{
	APTR                            userdata;
	struct FeelinNotifyHandler *    handlers;
	struct SignalSemaphore *       	arbiter;

	#ifdef F_NEW_LISTENERS
	struct in_Object_Listener *		listeners;
	#endif
};

//+

#ifdef F_NEW_LISTENERS

///_listeners_walk
static void _listeners_walk(FObject Obj, struct in_Object_Listener *Listeners, struct TagItem *Tags)
{
	struct TagItem item;

	IFEELIN F_Do(Obj, FM_Lock, 0);

	while (IFEELIN F_DynamicNTI(&Tags, &item, NULL))
	{
		struct in_Object_Listener *l;

		for (l = Listeners ; l ; l = l->next)
		{
			struct FS_Listener_Trigger msg;

			FClassAttribute *attribute = l->attribute;

			if (FF_LISTENER_PERFORMING & l->flags)
			{
				IFEELIN F_Log(FV_LOG_DEV,"!!LISTENER LOOP!! Attribute (0x%08lx.%s)", attribute->ID, attribute->Name);

				continue;
			}

			if (attribute->ID != item.ti_Tag)
			{
				continue;
			}

			//
			//
			//

			msg.Id = attribute->ID;
			msg.Value = item.ti_Data;
			msg.Attribute = attribute;

//			  IFEELIN F_Log(0, "call (0x%08lx) for attribute '%s'", l->callback, attribute->Name);

			l->flags |= FF_LISTENER_PERFORMING;

			IUTILITY CallHookPkt(l->callback, Obj, &msg);

			l->flags &= ~FF_LISTENER_PERFORMING;
		}
	}

	IFEELIN F_Do(Obj, FM_Unlock);
}
//+
///_listeners_remove_all
static void _listeners_remove_all(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_Object_Listener *l;
	struct in_Object_Listener *next;

	for (l = LOD->listeners ; l ; l = next)
	{
		next = l->next;

		IFEELIN F_Dispose(l);
	}

	LOD->listeners = NULL;
}
//+

#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Object_New
F_METHOD(FObject,Object_New)
{
	IFEELIN F_DoA(Obj, FM_Set, Msg);

	return Obj;
}
//+
///Object_Dispose
F_METHOD(uint32,Object_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	/*
	#ifndef F_NEW_GLOBALCONNECT

	FObject parent = (FObject) IFEELIN F_Get(Obj, FA_Parent);

	if (parent)
	{
		IFEELIN F_Do(parent, FM_RemMember, Obj);
	}

	#endif
	*/
 
	if (LOD->handlers)
	{
		IFEELIN F_Do(Obj, FM_UnNotify, ALL);
	}

	#ifdef F_NEW_LISTENERS

	if (LOD->listeners)
	{
		_listeners_remove_all(Class, Obj);
	}

	#endif

	if (LOD->arbiter)
	{
		IFEELIN F_Dispose(LOD->arbiter);

		LOD->arbiter = NULL;
	}

	return 0;
}
//+
///Object_Set
F_METHOD(uint32,Object_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	int32 notify = TRUE;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_UserData: LOD->userdata = (APTR)(item.ti_Data); break;
		case FA_NoNotify: notify = (item.ti_Data) ? FALSE : TRUE; break;
	}

	#ifdef F_NEW_LISTENERS

	if (!notify)
	{
		return 0;
	}

	if (LOD->listeners)
	{
		_listeners_walk(Obj, LOD->listeners, Msg);
	}

	if (!LOD->handlers)
	{
		return 0;
	}

	#else

	if (!notify || !LOD->handlers)
	{
		return 0;
	}

	#endif

	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Shared);

	Tags = Msg;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	{
		struct FeelinNotifyHandler *nh;

		for (nh = LOD->handlers ; nh ; nh = nh->next)
		{
			if (FF_NOTIFY_PERFORMING & nh->flags)
			{
				#ifdef CS_WARN_NOTIFICATION_LOOP
				IFEELIN F_Log(FV_LOG_DEV,"!!NOTIFY LOOP!! Attribute (0x%08lx.%s) - Target %s{%08lx}",nh->attribute,f_find_attribute_name(nh->attribute,FeelinBase),_object_classname(nh->target),nh->target);
				#endif

				continue;
			}
		
			#ifdef DB_HANDLENOTIFY
			
			if (nh->attribute == item.ti_Tag)
			{
				IFEELIN F_Log
				(
					0, "attr (%08lx.%s) value (0x%08lx ?= 0x%08lx)",
					
					nh->attribute,
					f_find_attribute_name(nh->attribute,FeelinBase),
					nh->value,
					item.ti_Data
				);
			}

			#endif 

			if ((nh->attribute == item.ti_Tag) &&
				((nh->value == item.ti_Data) || (nh->value == FV_Notify_Always)))
			{
				uint32 *cpy = NULL;
				FObject target = nh->target;

				nh->flags |= FF_NOTIFY_PERFORMING;

				switch ((uint32)(target))
				{
					case FV_Notify_Self:        target = Obj; break;
					case FV_Notify_Parent:      target = (FObject) IFEELIN F_Get(Obj, FA_Element_Parent); break;

					#ifdef F_NEW_GLOBALCONNECT
					
					case FV_Notify_Window:		target = (FObject) IFEELIN F_Get(Obj, FA_Element_Window); break;
					case FV_Notify_Application:	target = (FObject) IFEELIN F_Get(Obj, FA_Element_Application); break;

					#else

					case FV_Notify_Window:		target = (FObject) IFEELIN F_Get(Obj, FA_Window); break;
					case FV_Notify_Application:	target = (FObject) IFEELIN F_Get(Obj, FA_Application); break;

					#endif
				}
			
				if (nh->count)
				{
					uint32 *msg = (APTR)((uint32)(nh) + sizeof (struct FeelinNotifyHandler));
					cpy = (APTR)((uint32)(msg) + (nh->count + 1) * sizeof (uint32));

					if (nh->value == FV_Notify_Always)
					{
						uint32 i;
						
						for (i = 0 ; i < nh->count ; i++)
						{
							switch (msg[i])
							{
								case FV_Notify_Value:   cpy[i] = item.ti_Data; break;
								case FV_Notify_Toggle:  cpy[i] = (item.ti_Data) ? FALSE : TRUE; break;
								default:                cpy[i] = msg[i]; break;
							}
						}
					}
					else
					{
///DB_HANDLENOTIFY
#ifdef DB_HANDLENOTIFY
						IFEELIN F_Log(0,"NOTIFY_0x%08lx",nh->value);
#endif
//+

						IEXEC CopyMem(msg,cpy,nh->count * sizeof (uint32));
					}
				}

///DB_HANDLENOTIFY
#ifdef DB_HANDLENOTIFY
				IFEELIN F_Log
				(
					0,
					
					"Attribute (0x%08lx.%s) Target (%s{%08lx}) Method(0x%08lx) Msg (0x%08lx)",
					
					nh->attribute,
					f_find_attribute_name(nh->attribute, FeelinBase),
					_object_classname(target),
					target,
					nh->method,
					cpy
				);

				if (nh->count)
				{
					IFEELIN F_Log(0,"     >> Org 0x%08lx %08lx %08lx %08lx %08lx",(APTR)((uint32)(nh) + sizeof (struct FeelinNotifyHandler)));
					IFEELIN F_Log(0,"     >> Msg 0x%08lx %08lx %08lx %08lx %08lx",cpy);
				}
#endif
//+
					
				IFEELIN F_DoA(target,nh->method,cpy);

				nh->flags &= ~FF_NOTIFY_PERFORMING;
			}
		}
	}

	IFEELIN F_Do(Obj, FM_Unlock);

	return 0;
}
//+
///Object_Get
F_METHOD(uint32,Object_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_UserData:
		{
			F_STORE(LOD->userdata);
		}
		break;
	}

	return 0;
}
//+

///Object_Notify
F_METHODM(uint32,Object_Notify,FS_Notify)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct FeelinNotifyHandler *nh;

	if (Msg->Method < 255 || Msg->Count > 20)
	{
		IFEELIN F_Log(FV_LOG_DEV,"Suspicious Notify: Attribute 0x%08lx - Method 0x%08lx - Count 0x%08lx",_object_classname(Obj),Obj,Msg->Attribute,Msg->Method,Msg->Count);
		
		return 0;
	}

	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

	if ((nh = IFEELIN F_NewP(FeelinBase->NotifyPool,(Msg->Count) ? sizeof (struct FeelinNotifyHandler) + (Msg->Count + 1) * sizeof (uint32) * 2 : sizeof (struct FeelinNotifyHandler))) != NULL)
	{
		nh->attribute = Msg->Attribute;
		nh->value     = Msg->Value;
		nh->target    = Msg->Target;
		nh->method    = IFEELIN F_DynamicFindID((STRPTR)(Msg->Method));
		nh->count     = Msg->Count;

/*** try to resolve the attribute and its value ************************************************/

		if ((TAG_USER & Msg->Attribute) == 0)
		{
			FClassAttribute *attr = IFEELIN F_DynamicFindAttribute((STRPTR)(Msg->Attribute),_object_class(Obj),NULL);
				
/*

HOMBRE : Je me suis permis d'enlever le test TypeOfMem,  puisqu'il  ne  sert
pas à grand chose... pour OS4 en tout cas !

Pour faire facilement la différence entre une  méthode  et  une  adresse  de
chaine...  on  pourrait  se  baser sur le fait que les chaines sont créé (je
l'espère) de manière aligné sur des mots, donc à une adresse forcément PAIRE
!  Il suffirait alors de ne définir que des constante IMPAIRE et le tour est
joué ! Le test du type ( adresse de chaine <-> id_int ) devient très  facile
et rapide !

J'ai bon ?

Après vérification, 99% des adresse sont aligné sur un long, mais  il  reste
quand même 1% de récalcitrant ! Si on pouvait forcer l'alignement, ce serait
vraiment super !

GOFROMIEL: Que penses-tu de ce nouveau test ? C'est pas encore super génial,
mais  ça  devrait  passer  en  attendand  que  je crée une véritable méthode
FM_NotifyAlways.

*/


 
			if (attr)
			{
				nh->attribute = attr->ID;
		
				if (attr->Values && ((int32)(nh->value) > 0xFFFF) && (nh->value != FV_Notify_Always))
				{
					FClassAttributeValue *val;

					for (F_VALUES_EACH(attr->Values, val))
					{
						if (IFEELIN F_StrCmp((STRPTR)(nh->value), _value_name(val), ALL) == 0)
						{
							nh->value = val->Value; break;
						}
					}
				}
			}
		}

/*** try to resolve method *********************************************************************/
		
		IEXEC CopyMem((APTR)((uint32)(Msg) + sizeof (struct FS_Notify)),(APTR)((uint32)(nh) + sizeof (struct FeelinNotifyHandler)),Msg->Count * sizeof (uint32));

///DB_NOTIFY
		#ifdef DB_NOTIFY
		IFEELIN F_Log(0,"Obj 0x%08lx (%s) - Notify 0x%lx - Attribute 0x%lx (0x%08.lx) - Target 0x%lx (0x%lx) - Count %ld",Obj,_object_class(Obj)->Name,nh,nh->attribute,nh->value,nh->target,nh->method,nh->count);
		IFEELIN F_LogA(0,"   >> Msg 0x%08lx %08lx %08lx %08lx %08lx",(APTR)((uint32)(nh) + sizeof (struct FeelinNotifyHandler)));
		#endif
//+

		nh->next = LOD->handlers;
		LOD->handlers = nh;
	}
	else
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to allocate handler for attribute 0x%08lx",_object_classname(Obj),Obj,Msg->Attribute);
	}

	IFEELIN F_Do(Obj, FM_Unlock);

	return (uint32) nh;
}
//+
///Object_UnNotify
F_METHODM(uint32,Object_UnNotify,FS_UnNotify)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Handler)
	{
		struct FeelinNotifyHandler *nh;
		
		#if 0
		if ((uint32)(Msg->Handler) == ALL)
		{
			IFEELIN F_Log(0,"OPool(0x%08lx.%ld) - ALL HANDLERS",F_POOL(Owner),F_POOL(NestCount));
		}
		else
		{
			IFEELIN F_Log(0,"OPool(0x%08lx.%ld) - Handler 0x%08lx - Attribute 0x%08lx (0x%08lx) - Target %s{%08lx}",F_POOL(Owner),F_POOL(NestCount),((struct FeelinNotifyHandler *)(Msg->Handler))->Attribute,((struct FeelinNotifyHandler *)(Msg->Handler))->Value,_object_classname(((struct FeelinNotifyHandler *)(Msg->Handler))->Target),((struct FeelinNotifyHandler *)(Msg->Handler))->Target);
		}
		#endif

		IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

		if ((uint32)(Msg->Handler) == ALL)
		{
			while ((nh = LOD->handlers) != NULL)
			{
				LOD->handlers = nh->next;
				
				if (FF_NOTIFY_PERFORMING & nh->flags)
				{
					IFEELIN F_Log(FV_LOG_DEV,"Trying to remove a notify handler (0x%08lx) under process. The notify handler will not be disposed to ensure the end of the notify process. Please, check your code !!\n   >> Attribute (0x%08lx)(%s) - Target %s{%08lx} - Method (0x%08lx)(%s)",nh,nh->attribute,f_find_attribute_name(nh->attribute,FeelinBase),_object_classname(nh->target),nh->target,nh->method,f_find_method_name(nh->method,FeelinBase));
	 
					nh->next = NULL;
				}
				else
				{
					IFEELIN F_Dispose(nh);
				}
			}
		}
		else if (LOD->handlers)
		{
			struct FeelinNotifyHandler *prev=NULL;

			for (nh = LOD->handlers ; nh ; nh = nh->next)
			{
				if (nh == Msg->Handler)
				{
					if (prev)
					{
						prev->next = nh->next;
					}
					else
					{
						LOD->handlers = nh->next;
					}

					if (FF_NOTIFY_PERFORMING & nh->flags)
					{
						IFEELIN F_Log(FV_LOG_DEV,"Trying to remove a notify handler (0x%08lx) under process. The notify handler will not be disposed to ensure the end of the notify process. Check your source code !!\n   >> Attribute (0x%08lx)(%s) - Target %s{%08lx} - Method (0x%08lx)(%s)",nh,nh->attribute,f_find_attribute_name(nh->attribute,FeelinBase),_object_classname(nh->target),nh->target,nh->method,f_find_method_name(nh->method,FeelinBase));

						nh->next = NULL;
					}
					else
					{
						IFEELIN F_Dispose(nh);
					}

					break;
				}
				prev = nh;
			}

			if (!nh)
			{
				IFEELIN F_Log(FV_LOG_DEV,"Unknown Handler 0x%08lx",Msg->Handler);
			}
		}

		IFEELIN F_Do(Obj, FM_Unlock);
	}
	
	return 0;
}
//+

///Object_MultiSet
F_METHODM(void,Object_MultiSet,FS_MultiSet)
{
	FObject *list;

	for (list = (FObject *)((uint32)(Msg) + sizeof (struct FS_MultiSet)) ; *list ; list++)
	{
		IFEELIN F_Set(*list,Msg->Attribute,Msg->Value);
	}
}
//+
///Object_SetAs
F_METHODM(void,Object_SetAs,FS_SetAs)
{
	int32 nonotify = (FF_SetAs_Notify & Msg->Flags) ? FALSE : TRUE;

	#ifdef DB_SETAS
	IFEELIN F_Log(0,"flags (0x%08lx) >> %ld",Msg->Flags,Msg->Flags & 0x0000FFFF);
	#endif
 
	switch (Msg->Flags & 0x0000FFFF)
	{
		case FV_SetAs_String:
		{
			STRPTR str = IFEELIN F_StrNewA(NULL,Msg->Data,(APTR)((uint32)(Msg) + sizeof (struct FS_SetAs)));
			
			#ifdef DB_SETAS
			IFEELIN F_Log(0,"notify (%ld) string (0x%08lx)(%s) >> (%s)",!nonotify,Msg->Data,Msg->Data,str);
			#endif
 
			if (str)
			{
				IFEELIN F_Do(Obj, FM_Set, FA_NoNotify, nonotify, Msg->Attribute, (uint32) str, TAG_DONE);

				IFEELIN F_Dispose(str);
			}
		}
		break;
		
		case FV_SetAs_Decimal:
		{
			int32 val=0;
			
			if (Msg->Data)
			{
				f_stcd(Msg->Data, &val);
			}
			
			#ifdef DB_SETAS
			IFEELIN F_Log(0,"notify (%ld) decimal (0x%08lx)(%s) >> (%ld)",!nonotify,Msg->Data,Msg->Data,val);
			#endif
			
			IFEELIN F_Do(Obj,FM_Set, FA_NoNotify,nonotify, Msg->Attribute,val, TAG_DONE);
		}
		break;
	
		case FV_SetAs_Hexadecimal:
		{
			int32 val=0;
			
			if (Msg->Data)
			{
				f_stch(Msg->Data, &val);
			}
			
			#ifdef DB_SETAS
			IFEELIN F_Log(0,"notify (%ld) hexadecimal (0x%08lx)(%s) >> (%ld)",!nonotify,Msg->Data,Msg->Data,val);
			#endif

			IFEELIN F_Do(Obj,FM_Set, FA_NoNotify,nonotify, Msg->Attribute,val, TAG_DONE);
		}
		break;
	
		case FV_SetAs_4CC:
		{
			int32 val=0;
			
			if (Msg->Data)
			{
				val = *((uint32 *)(Msg->Data));
			}
			
			#ifdef DB_SETAS
			IFEELIN F_Log(0,"notify (%ld) constant (0x%08lx)(%s) >> (%08lx)",Msg->Notify,Msg->Data,Msg->Data,*((uint32 *)(Msg->Data)));
			#endif
 
			IFEELIN F_Do(Obj,FM_Set, FA_NoNotify,nonotify, Msg->Attribute,val, TAG_DONE);
		}
		break;

		#if 0
		default:
		{
			IFEELIN F_Log(0,"INVALID TYPE (%ld)",Msg->Type);
		}
		break;
		#endif
	}
}
//+
///Object_Null
F_METHOD(uint32,Object_Null)
{
	return 0;
}
//+
///Object_CallHook
F_METHODM(uint32,Object_CallHook,FS_CallHook)
{
	if (Msg->Hook && Msg->Hook->h_Entry)
	{
		return IUTILITY CallHookPkt(Msg->Hook,Obj,++Msg);
	}
	return 0;
}
//+
///Object_CallHookEntry
F_METHODM(uint32,Object_CallHookEntry,FS_CallHookEntry)
{
	if (Msg->Entry)
	{
		struct Hook hook;

		hook.h_MinNode.mln_Succ = NULL;
		hook.h_MinNode.mln_Pred = NULL;
		hook.h_Entry            = (HOOKFUNC) Msg->Entry;
		hook.h_SubEntry         = NULL;
		hook.h_Data             = NULL;
 
		return IUTILITY CallHookPkt(&hook,Obj,++Msg);
	}
	return 0;
}
//+

///Object_Lock
F_METHODM(bool32,Object_Lock,FS_Lock)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (!LOD->arbiter)
	{
		F_OBJECTS_LOCK();

		//
		// the semaphore might have been created while we where
		// locking the objects' arbiter !
		//

		if (!LOD->arbiter)
		{
			LOD->arbiter = IFEELIN F_New(sizeof (struct SignalSemaphore));

			#ifdef DB_LOCK
			IFEELIN F_Log(0,"NEW ATOMIC 0x%08lx",LOD->arbiter);
			#endif

			if (LOD->arbiter)
			{
				IEXEC InitSemaphore(LOD->arbiter);
			}
			else
			{
				IFEELIN F_Log(FV_LOG_USER, "Unable to create arbiter !");
			}
		}

		F_OBJECTS_UNLOCK();
	}

	/*
	IEXEC Forbid();
 
	if (!LOD->atomic)
	{
		LOD->atomic = IFEELIN F_New(sizeof (struct SignalSemaphore));

		#ifdef DB_LOCK
		IFEELIN F_Log(0,"NEW ATOMIC 0x%08lx",LOD->Atomic);
		#endif
 
		if (LOD->atomic)
		{
			IEXEC InitSemaphore(LOD->atomic);
		}
	}

	IEXEC Permit();
	*/
	
	if (!LOD->arbiter)
	{
		return FALSE;
	}

	//
	// lock the arbiter
	//
	
	if (FF_Lock_Attempt & Msg->Flags)
	{
		if (FF_Lock_Shared & Msg->Flags)
		{
			#ifdef DB_LOCK
			IFEELIN F_Log(0,"AttemptSemaphoreShared 0x%08lx", LOD->arbiter);
			#endif
				 
			return IEXEC AttemptSemaphoreShared(LOD->arbiter);
		}
		else
		{
			#ifdef DB_LOCK
			IFEELIN F_Log(0,"AttemptSemaphore 0x%08lx", LOD->arbiter);
			#endif
				 
			return IEXEC AttemptSemaphore(LOD->arbiter);
		}
	}
	else if (FF_Lock_Shared & Msg->Flags)
	{
		#ifdef DB_LOCK
		IFEELIN F_Log(0,"ObtainSemaphoreShared 0x%08lx", LOD->arbiter);
		#endif
			 
		IEXEC ObtainSemaphoreShared(LOD->arbiter);
	}
	else
	{
		#ifdef DB_LOCK
		IFEELIN F_Log(0,"ObtainSemaphore 0x%08lx", LOD->arbiter);
		#endif
			 
		IEXEC ObtainSemaphore(LOD->arbiter);
	}

	return TRUE;
}
//+
///Object_Unlock
F_METHOD(bool32,Object_Unlock)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (!LOD->arbiter)
	{
		IFEELIN F_Log(FV_LOG_DEV,"No Semaphore created yet !");

		return FALSE;
	}

	#ifdef DB_UNLOCK
	IFEELIN F_Log(0,"Owner (%s)(0x%08lx)", LOD->arbiter->ss_Owner->tc_Node.ln_Name, LOD->arbiter->ss_Owner);
	#endif
		
	IEXEC ReleaseSemaphore(LOD->arbiter);

	return TRUE;
}
//+

#ifdef F_NEW_LISTENERS

///Object_AddListener
F_METHODM(bool32, Object_AddListener, FS_AddListener)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	STRPTR attribute_name = Msg->Attribute;
	struct Hook *callback = Msg->Callback;

	struct in_Object_Listener *l;
	
	FClassAttribute *attribute;

	if (!attribute_name)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Attribute is NULL !");

		return FALSE;
	}

	if (!callback)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Callback is NULL !");

		return FALSE;
	}

	//
	// find the attribute
	//
	
	attribute = IFEELIN F_DynamicFindAttribute(attribute_name, _object_class(Obj), NULL);

	if (!attribute)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Attribute '%s' was not found in class '%s'", attribute_name, _object_classname(Obj));

		return FALSE;
	}

	//
	// create listener
	//

	l = IFEELIN F_NewP(FeelinBase->NotifyPool, sizeof (struct in_Object_Listener));

	if (!l)
	{
		return FALSE;
	}

	l->attribute = attribute;
	l->callback = callback;

	//
	// link the listener
	//

	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

	l->next = LOD->listeners;
	LOD->listeners = l;

	IFEELIN F_Do(Obj, FM_Unlock);

	//IFEELIN F_Log(FV_LOG_DEV, "Listener added for attribute '%s'", attribute_name);

	return TRUE;
}
//+
///Object_RemoveListerner
F_METHODM(uint32, Object_RemoveListener, FS_RemoveListener)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	STRPTR attribute_name = Msg->Attribute;
	struct Hook *callback = Msg->Callback;

	struct in_Object_Listener *node;
	struct in_Object_Listener *prev = NULL;

	FClassAttribute *attribute;

	if (!attribute_name)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Attribute is NULL !");

		return 0;
	}

	if (!callback)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Callback is NULL !");

		return 0;
	}

	//
	// does the object have any listener ?
	//

	if (!LOD->listeners)
	{
		IFEELIN F_Log(FV_LOG_DEV, "The object have no listeners");

		return 0;
	}

	//
	// find the attribute
	//

	attribute = IFEELIN F_DynamicFindAttribute(attribute_name, _object_class(Obj), NULL);

	if (!attribute)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Attribute '%s' was not found in class '%s'", attribute_name, _object_classname(Obj));

		return 0;
	}

	//
	// search and remove listener
	//

	IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

	for (node = LOD->listeners ; node ; node = node->next)
	{
		if ((node->attribute == attribute) && (node->callback == callback))
		{
			if (prev)
			{
				prev->next = node->next;
			}
			else
			{
				LOD->listeners = node->next;
			}

			if (FF_LISTENER_PERFORMING & node->flags)
			{
				IFEELIN F_Log(FV_LOG_DEV, "Removing a listener under process.");
			}
			
			IFEELIN F_Dispose(node);

			break;
		}
		
		prev = node;
	}

	if (!node)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Unknown listener for Attribute '%s' with Callback (0x%08lx)", attribute_name, callback);
	}

	IFEELIN F_Do(Obj, FM_Unlock);

	return 0;
}
//+

#endif

/************************************************************************************************
*** Constructor *********************************************************************************
************************************************************************************************/

STATIC F_ATTRIBUTES_ARRAY =
{
	F_ATTRIBUTES_ADD_STATIC("UserData",     FV_TYPE_POINTER,    FA_UserData),
	F_ATTRIBUTES_ADD_STATIC("NoNotify",     FV_TYPE_BOOLEAN,    FA_NoNotify),
	F_ATTRIBUTES_ADD_STATIC("Child",        FV_TYPE_OBJECT,     FA_Child),
	
	#ifndef F_NEW_GLOBALCONNECT
	F_ATTRIBUTES_ADD_STATIC("Window",       FV_TYPE_OBJECT,     FA_Window),
	F_ATTRIBUTES_ADD_STATIC("Application",  FV_TYPE_OBJECT,     FA_Application),
	#endif
	
	F_ARRAY_END
};

STATIC F_METHODS_ARRAY =
{
	F_METHODS_ADD_STATIC(Object_Set,            "Set",            FM_Set),
	F_METHODS_ADD_STATIC(Object_Get,            "Get",            FM_Get),
	F_METHODS_ADD_STATIC(Object_Lock,           "Lock",           FM_Lock),
	F_METHODS_ADD_STATIC(Object_Unlock,         "Unlock",         FM_Unlock),
	F_METHODS_ADD_STATIC(Object_New,            "New",            FM_New),
	F_METHODS_ADD_STATIC(Object_Dispose,        "Dispose",        FM_Dispose),
	F_METHODS_ADD_STATIC(Object_Notify,         "Notify",         FM_Notify),
	F_METHODS_ADD_STATIC(Object_UnNotify,       "UnNotify",       FM_UnNotify),
	F_METHODS_ADD_STATIC(Object_SetAs,          "SetAs",          FM_SetAs),
	F_METHODS_ADD_STATIC(Object_CallHook,       "CallHook",       FM_CallHook),
	F_METHODS_ADD_STATIC(Object_CallHookEntry,  "CallHookEntry",  FM_CallHookEntry),
	F_METHODS_ADD_STATIC(Object_MultiSet,       "MultiSet",       FM_MultiSet),
	#ifdef F_NEW_LISTENERS
	F_METHODS_ADD_STATIC(Object_AddListener,    "AddListener",  	FM_AddListener),
	F_METHODS_ADD_STATIC(Object_RemoveListener, "RemoveListener",	FM_RemoveListener),
	#endif
	F_METHODS_ADD_STATIC(Object_Null,           "Connect",        FM_Connect),
	F_METHODS_ADD_STATIC(Object_Null,           "Disconnect",     FM_Disconnect),
	F_METHODS_ADD_STATIC(Object_Null,           "AddMember",      FM_AddMember),
	F_METHODS_ADD_STATIC(Object_Null,           "RemMember",      FM_RemMember),
	#ifdef F_NEW_GETELEMENTBYID
	F_METHODS_ADD_STATIC(Object_Null,           "GetElementById",	FM_GetElementById),
	#endif
	
	F_ARRAY_END
};

STATIC F_TAGS_ARRAY =
{
	F_TAGS_ADD_LOD,
	F_TAGS_ADD_METHODS,
	F_TAGS_ADD_ATTRIBUTES,

	F_ARRAY_END
};

FClass * fc_object_create(struct in_FeelinBase *FeelinBase)
{
	return IFEELIN F_CreateClassA(FC_Object, F_TAGS_PTR);
}
