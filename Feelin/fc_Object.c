/*
**    FC_Object
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************

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
	struct FeelinNotifyHandler     *next;
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

struct LocalObjectData
{
	STRPTR                          id;
	APTR                            userdata;
	struct FeelinNotifyHandler     *handlers;
	struct SignalSemaphore         *atomic;
};

//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Object_New
F_METHOD(FObject,Object_New)
{
	#ifndef F_NEW_ELEMENT_ID

	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_ID:
		{
			LOD->id = (STRPTR) item.ti_Data;
		}
		break;
	}

	#endif

	IFEELIN F_DoA(Obj, FM_Set, Msg);

	return Obj;
}
//+
///Object_Dispose
F_METHOD(uint32,Object_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifndef F_NEW_GLOBALCONNECT

	FObject parent = (FObject) IFEELIN F_Get(Obj, FA_Parent);

	if (parent)
	{
		IFEELIN F_Do(parent, FM_RemMember, Obj);
	}

	#endif
 
	if (LOD->handlers)
	{
		IFEELIN F_Do(Obj, FM_UnNotify, ALL);
	}

	if (LOD->atomic)
	{
		IFEELIN F_Dispose(LOD->atomic);

		LOD->atomic = NULL;
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

	if (!notify || !LOD->handlers)
	{
		return 0;
	}

	#ifdef F_ENABLE_LOCK_NOTIFY
	IFEELIN F_SPool(FeelinBase->NotifyPool);
	#endif

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
					case FV_Notify_Parent:      target = (FObject) IFEELIN F_Get(Obj, FA_Parent); break;

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
					
				#if F_CODE_DEPRECATED
				if (nh->method == FM_Area_Draw)
				{
					IFEELIN F_Draw(target,*cpy);
				}
				else
				#endif
				{
					IFEELIN F_DoA(target,nh->method,cpy);
				}

				nh->flags &= ~FF_NOTIFY_PERFORMING;
			}
		}
	}

	#ifdef F_ENABLE_LOCK_NOTIFY
	IFEELIN F_RPool(FeelinBase->NotifyPool);
	#endif

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
		#ifndef F_NEW_ELEMENT_ID
		case FA_ID:             F_STORE(LOD->id); break;
		#endif
		case FA_UserData:       F_STORE(LOD->userdata); break;
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

	#ifdef F_ENABLE_LOCK_NOTIFY
	IFEELIN F_OPool(FeelinBase->NotifyPool);
	#endif

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

	#ifdef F_ENABLE_LOCK_NOTIFY
	IFEELIN F_RPool(FeelinBase->NotifyPool);
	#endif

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

		#ifdef F_ENABLE_LOCK_NOTIFY
		IFEELIN F_OPool(FeelinBase->NotifyPool);
		#endif

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

		#ifdef F_ENABLE_LOCK_NOTIFY
		IFEELIN F_RPool(FeelinBase->NotifyPool);
		#endif
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
F_METHODM(uint32,Object_Lock,FS_Lock)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

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
	
	if (LOD->atomic)
	{
		if (FF_Lock_Attempt & Msg->Flags)
		{
			if (FF_Lock_Shared & Msg->Flags)
			{
				#ifdef DB_LOCK
				IFEELIN F_Log(0,"AttemptSemaphoreShared 0x%08lx",LOD->Atomic);
				#endif
				 
				return IEXEC AttemptSemaphoreShared(LOD->atomic);
			}
			else
			{
				#ifdef DB_LOCK
				IFEELIN F_Log(0,"AttemptSemaphore 0x%08lx",LOD->Atomic);
				#endif
				 
				return IEXEC AttemptSemaphore(LOD->atomic);
			}
		}
		else if (FF_Lock_Shared & Msg->Flags)
		{
			#ifdef DB_LOCK
			IFEELIN F_Log(0,"ObtainSemaphoreShared 0x%08lx",LOD->Atomic);
			#endif
			 
			IEXEC ObtainSemaphoreShared(LOD->atomic);
		}
		else
		{
			#ifdef DB_LOCK
			IFEELIN F_Log(0,"ObtainSemaphore 0x%08lx",LOD->Atomic);
			#endif
			 
			IEXEC ObtainSemaphore(LOD->atomic);
		}
	}
	return 0;
}
//+
///Object_Unlock
F_METHOD(void,Object_Unlock)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->atomic)
	{
		#ifdef DB_UNLOCK
		IFEELIN F_Log(0,"Owner (%s)(0x%08lx)",LOD->Atomic->ss_Owner->tc_Node.ln_Name,LOD->Atomic->ss_Owner);
		#endif
		
		IEXEC ReleaseSemaphore(LOD->atomic);
	}
	else
	{
		IFEELIN F_Log(FV_LOG_DEV,"No Semaphore created yet !");
	}
}
//+

/************************************************************************************************
*** Constructor *********************************************************************************
************************************************************************************************/

STATIC F_ATTRIBUTES_ARRAY =
{
	#ifndef F_NEW_ELEMENT_ID
	F_ATTRIBUTES_ADD_STATIC("ID",           FV_TYPE_STRING, 	FA_ID),
	#endif

	F_ATTRIBUTES_ADD_STATIC("UserData",     FV_TYPE_POINTER,    FA_UserData),
	F_ATTRIBUTES_ADD_STATIC("NoNotify",     FV_TYPE_BOOLEAN,    FA_NoNotify),
	F_ATTRIBUTES_ADD_STATIC("Child",        FV_TYPE_OBJECT,     FA_Child),
	F_ATTRIBUTES_ADD_STATIC("Parent",       FV_TYPE_OBJECT,     FA_Parent),
	
	#ifndef F_NEW_GLOBALCONNECT
	F_ATTRIBUTES_ADD_STATIC("Window",       FV_TYPE_OBJECT,     FA_Window),
	F_ATTRIBUTES_ADD_STATIC("Application",  FV_TYPE_OBJECT,     FA_Application),
	#endif
	
	F_ARRAY_END
};

STATIC F_METHODS_ARRAY =
{
	F_METHODS_ADD_BOTH(Object_Set,            "Set",            FM_Set),
	F_METHODS_ADD_BOTH(Object_Get,            "Get",            FM_Get),
	F_METHODS_ADD_BOTH(Object_Lock,           "Lock",           FM_Lock),
	F_METHODS_ADD_BOTH(Object_Unlock,         "Unlock",         FM_Unlock),
	F_METHODS_ADD_BOTH(Object_New,            "New",            FM_New),
	F_METHODS_ADD_BOTH(Object_Dispose,        "Dispose",        FM_Dispose),
	F_METHODS_ADD_BOTH(Object_Notify,         "Notify",         FM_Notify),
	F_METHODS_ADD_BOTH(Object_UnNotify,       "UnNotify",       FM_UnNotify),
	F_METHODS_ADD_BOTH(Object_SetAs,          "SetAs",          FM_SetAs),
	F_METHODS_ADD_BOTH(Object_CallHook,       "CallHook",       FM_CallHook),
	F_METHODS_ADD_BOTH(Object_CallHookEntry,  "CallHookEntry",  FM_CallHookEntry),
	F_METHODS_ADD_BOTH(Object_MultiSet,       "MultiSet",       FM_MultiSet),

	F_METHODS_ADD_BOTH(Object_Null,           "Connect",        FM_Connect),
	F_METHODS_ADD_BOTH(Object_Null,           "Disconnect",     FM_Disconnect),
	F_METHODS_ADD_BOTH(Object_Null,           "AddMember",      FM_AddMember),
	F_METHODS_ADD_BOTH(Object_Null,           "RemMember",      FM_RemMember),
	
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

