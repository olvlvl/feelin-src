#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///family_search
STATIC FFamilyNode * family_search(FClass *Class,FObject Obj,FObject Member)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FFamilyNode *node;

	for (node = (FFamilyNode *)(LOD->list.Head) ; node ; node = node->Next)
	{
		if (node->Object == Member)
		{
			break;
		}
	}
	
	return node;
}
//+
///family_kill
STATIC uint32 family_kill(FClass *Class, FObject Obj, struct TagItem *Tags)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	/* one child in not valid (== NULL),  we  must  remove  objects  already
	member  of the family, then dispose all valid objects in the taglist and
	return NULL */

	FFamilyNode *node;
	struct TagItem item, *real;

	FObject owner = LOD->owner;

 	while ((node = (FFamilyNode *) LOD->list.Tail) != NULL)
 	{
 		IFEELIN F_Do(owner ? owner : Obj, FM_RemMember, node->Object);

 		if (node == (FFamilyNode *) LOD->list.Tail)
 		{
 			IFEELIN F_Log(FV_LOG_DEV, "!! LINK ERROR !!");

			return 0;
 		}
 	}

 	while ((real = IFEELIN F_DynamicNTI(&Tags, &item, Class)) != NULL)
 	{
 		if (item.ti_Tag == FA_Child)
 		{
			if (item.ti_Data != 0)
			{
 				IFEELIN F_DisposeObj((FObject) item.ti_Data);
			}

			real->ti_Tag = TAG_IGNORE;
			real->ti_Data = NULL;
 		}
 	}

 	IFEELIN F_Set(owner, FA_Family, 0);
 	
	LOD->owner = NULL;

 	return 0;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Family_New
F_METHOD(uint32,Family_New)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Child:
		{
			FObject owner = LOD->owner;

			if (item.ti_Data == 0)
			{
				if (owner)
				{
					IFEELIN F_Log(FV_LOG_USER,"found a NULL object in Msg (0x%08lx) for %s{%08lx}", Msg, _object_classname(owner), owner);
				}
				else
				{
					IFEELIN F_Log(FV_LOG_USER,"found a NULL object in Msg (0x%08lx)", Msg);
				}

				return family_kill(Class, Obj, Msg);
			}

			if (!IFEELIN F_Do(owner ? owner : Obj, FM_AddMember, item.ti_Data, FV_AddMember_Tail))
			{
				if (owner)
				{
					IFEELIN F_Log(FV_LOG_DEV,"AddMember method failed with Child %s{%08lx} (Owner: %s{%08lx})",_object_classname(item.ti_Data),item.ti_Data, _object_classname(owner), owner);
				}
				else
				{
					IFEELIN F_Log(FV_LOG_DEV,"AddMember method failed with Child %s{%08lx}", _object_classname(item.ti_Data), item.ti_Data);
				}

				return family_kill(Class, Obj, Msg);
			}
		}
		break;

		case FA_Family_Owner:
		{
			LOD->owner = (FObject)(item.ti_Data);

			IFEELIN F_Set(LOD->owner,FA_Family,(ULONG)(Obj));
		}
		break;

		case FA_Family_CreateNodeHook:
		{
			LOD->create_node_hook = (struct Hook *)(item.ti_Data);
		}
		break;

		case FA_Family_DeleteNodeHook:
		{
			LOD->delete_node_hook = (struct Hook *)(item.ti_Data);
		}
		break;
	}
	
	return F_SUPERDO();
}
//+
///Family_Dispose
F_METHOD(void,Family_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FFamilyNode *node;

	#ifdef DB_DISPOSE
	IFEELIN F_Log(0,"BEGIN");
	#endif

	while ((node = (FFamilyNode *) LOD->list.Tail) != NULL)
	{
		/* remember that FM_RemMember disposes the node */

		#ifdef DB_DISPOSE
		FObject owner = LOD->owner ? LOD->owner : Obj;
		#endif
		 
		FObject object = node->Object;

		#ifdef DB_DISPOSE
		IFEELIN F_Log(0,"REMMEMBER owner %s{%08lx} object %s{%08lx} >> BEGIN",_object_classname(owner),owner,_object_classname(object),object);
		#endif

		IFEELIN F_Do((LOD->owner) ? LOD->owner : Obj,FM_RemMember,object);
		
		#ifdef DB_DISPOSE
		IFEELIN F_Log(0,"REMMEMBER owner %s{%08lx} object %s{%08lx} >> DONE",_object_classname(owner),owner,_object_classname(object),object);
		#endif
		
		#ifdef DB_DISPOSE
		IFEELIN F_Log(0,"DISPOSE %s{%08lx} >> BEGIN",_object_classname(object),object);
		#endif
		
		IFEELIN F_DisposeObj(object);
		
		#ifdef DB_DISPOSE
		F_Log(0,"DISPOSE %s{%08lx} >> DONE",_object_classname(object),object);
		#endif

		if (node == (FFamilyNode *) LOD->list.Tail)
		{
			IFEELIN F_Log(FV_LOG_DEV, "OBJECT (0x%08lx) HAS NOT BEEN REMOVED !! THE LIST IS ABANDONNED", object);

			break;
		}
	}

	LOD->owner = NULL;

	F_SUPERDO();
}
//+
///Family_Get
F_METHOD(void,Family_Get)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while  (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Family:      F_STORE(Obj); break;
		case FA_Family_Head: F_STORE(LOD->list.Head); break;
		case FA_Family_Tail: F_STORE(LOD->list.Tail); break;
	}

	F_SUPERDO();
}
//+

///Family_Add
F_METHODM(FFamilyNode *,Family_Add,FS_AddMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Orphan == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Orphan in Msg (0x%08lx) is NULL !", Msg);

		return NULL;
	}

	if (family_search(Class,Obj,Msg->Orphan) != NULL)
	{
		if (LOD->owner != NULL)
		{
			IFEELIN F_Log(FV_LOG_DEV,"%s{%08lx} already member of %s{%08lx}",_object_classname(Msg->Orphan),Msg->Orphan,_object_classname(LOD->owner),LOD->owner);
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV,"%s{%08lx} already member",_object_classname(Msg->Orphan),Msg->Orphan);
		}
	}
	else
	{
		/* The object is added  to  the  family  *only*  if  the  method
		FM_Connect returns TRUE. */

		if (IFEELIN F_Do(Msg->Orphan, FM_Connect, LOD->owner ? LOD->owner : Obj, Obj) != FALSE)
		{
			FFamilyNode *node;

			if (LOD->create_node_hook != NULL)
			{
				node = (FFamilyNode *) IUTILITY CallHookPkt(LOD->create_node_hook, LOD->owner ? LOD->owner : Obj, Msg);
			}
			else
			{
				node = IFEELIN F_NewP(CUD->NodePool, sizeof (FFamilyNode));
			}

			if (node)
			{
				node->Next = NULL;
				node->Prev = NULL;
				node->Object = Msg->Orphan;

				switch (Msg->Position)
				{
					case FV_AddMember_Tail:
					{
						IFEELIN F_LinkTail(&LOD->list,(FNode *) node);
///db_AddMember
							#ifdef F_ENABLE_VERBOSE
							if (CUD->db_AddMember)
							{
								IFEELIN F_Log(0,"(TAIL) Node 0x%08lx - %s{%08lx}",node,_object_classname(node->Object),node->Object);
							}
							#endif
//+
						return node;
					}
					break;

					case FV_AddMember_Head:
					{
						IFEELIN F_LinkHead(&LOD->list,(FNode *) node);
///db_AddMember
							#ifdef F_ENABLE_VERBOSE
							if (CUD->db_AddMember)
							{
								IFEELIN F_Log(0,"(HEAD) Node 0x%08lx - %s{%08lx}",node,_object_classname(node->Object),node->Object);
							}
							#endif
//+
						return node;
					}
					break;

					case FV_AddMember_Insert:
					{
						FFamilyNode *prev;

						if (Msg->Previous != NULL)
						{
							IFEELIN F_LinkHead(&LOD->list,(FNode *) node);
///db_AddMember
							#ifdef F_ENABLE_VERBOSE
							if (CUD->db_AddMember)
							{
								IFEELIN F_Log(0,"(HEAD) Node 0x%08lx - %s{%08lx}",node,_object_classname(node->Object),node->Object);
							}
							#endif
//+
							return node;
						}
						if ((prev = family_search(Class,Obj,Msg->Previous)) != NULL)
						{
							IFEELIN F_LinkInsert(&LOD->list,(FNode *) node,(FNode *) prev);
///db_AddMember
								#ifdef F_ENABLE_VERBOSE
								if (CUD->db_AddMember)
								{
									IFEELIN F_Log(0,"(INSR) Node 0x%08lx - %s{%08lx}",node,_object_classname(node->Object),node->Object);
								}
								#endif
//+
							return node;
						}
						else
						{
							IFEELIN F_Log(FV_LOG_DEV,"%s{%08lx} (Previous) is not member of the family",_object_classname(Msg->Previous),Msg->Previous);
						}
					}
					break;

					default:
					{
					   if (LOD->owner != NULL)
					   {
						   IFEELIN F_Log(FV_LOG_DEV,"%ld is not a valid position - Object %s{%08lx} - Owner %s{%08lx}",Msg->Position,_object_classname(Msg->Orphan),Msg->Orphan,_object_classname(LOD->owner),LOD->owner);
					   }
					   else
					   {
						   IFEELIN F_Log(FV_LOG_DEV,"%ld is not a valid position - Object %s{%08lx}",Msg->Position,_object_classname(Msg->Orphan),Msg->Orphan);
					   }
					}
				}

/*** error *************************************************************************************/

				IFEELIN F_Dispose(node);
			}

		#ifndef F_NEW_CONNECT

			IFEELIN F_Do(Msg->Orphan, FM_Disconnect);
		}
		else
		{
			if (LOD->owner != NULL)
			{
				IFEELIN F_Log(FV_LOG_DEV,"Unable to connect %s{%08lx} to %s{%08lx}",_object_classname(Msg->Orphan),Msg->Orphan,_object_classname(LOD->owner),LOD->owner);
			}
			else
			{
				IFEELIN F_Log(FV_LOG_DEV,"Unable to connect %s{%08lx}",_object_classname(Msg->Orphan),Msg->Orphan);
			}

		#endif
		}
	}
	
	return NULL;
}
//+
///Family_Rem
F_METHODM(uint32,Family_Rem,FS_RemMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

///db_RemMember
	#ifdef F_ENABLE_VERBOSE
	if (CUD->db_RemMember)
	{
		if (LOD->owner)
		{
			IFEELIN F_Log(FV_LOG_DEV,"Member: %s{%08lx} - Owner: %s{%08lx}",_object_classname(Msg->Member),Msg->Member,_object_classname(LOD->owner),LOD->owner);
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV,"Member: %s{%08lx}",_object_classname(Msg->Member),Msg->Member);
		}
	}
	#endif
//+

	if (Msg->Member != NULL)
	{
		FFamilyNode *node = family_search(Class,Obj,Msg->Member);

		#ifdef DB_REMMEMBER
		IFEELIN F_Log(0,"%s{%08lx}",_object_classname(Msg->Member),Msg->Member);
		#endif

		if (node != NULL)
		{
			#ifdef DB_REMMEMBER
			IFEELIN F_Log(0,"Node 0x%08lx - %s{%08lx} - DISCONNECT",node,_object_classname(Msg->Member),Msg->Member);
			#endif

			IFEELIN F_Do(node->Object,FM_Disconnect);
			
			#ifdef DB_REMMEMBER
			IFEELIN F_Log(0,"Link remove");
			#endif
			 
			IFEELIN F_LinkRemove(&LOD->list,(FNode *) node);

			#ifdef DB_REMMEMBER
			IFEELIN F_Log(0,"dispose node - hook 0x%08lx - node 0x%08lx",LOD->delete_node_hook, node);
			#endif
 
			if (LOD->delete_node_hook != NULL)
			{
				IUTILITY CallHookPkt(LOD->delete_node_hook,((LOD->owner) ? LOD->owner : Obj),&node);
			}
			else
			{
				IFEELIN F_Dispose(node);
			}

			#ifdef DB_REMMEMBER
			IFEELIN F_Log(0,"DONE");
			#endif
			
			return TRUE;
		}
		else
		{
			if (LOD->owner != NULL)
			{
				IFEELIN F_Log(FV_LOG_DEV,"%s{%08lx} is not a member. Owner: %s{%08lx}",_object_classname(Msg->Member),Msg->Member,_object_classname(LOD->owner),LOD->owner);
			}
			else
			{
				IFEELIN F_Log(FV_LOG_DEV,"%s{%08lx} is not a member.",_object_classname(Msg->Member),Msg->Member);
			}
		}
	}
	return FALSE;
}
//+

#ifdef F_NEW_GETELEMENTBYID
///Family_GetElementById
F_METHODM(FObject, Family_GetElementById, FS_GetElementById)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FFamilyNode *node;

	FObject rc = NULL;

	IFEELIN F_Log(0, "search id '%s'", Msg->Id);

	for (node = (FFamilyNode *) LOD->list.Head ; node ; node = node->Next)
	{
		rc = (FObject) IFEELIN F_OBJDO(node->Object);

		if (rc)
		{
			break;
		}
	}

	return rc;
}
//+
#endif

