/*
**    Extension to the Dynamic ID system
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************
						
$VER: 08.00 (2004/12/16)
 
************************************************************************************************/

#include "Private.h"

//#define DB_AUTO_FINDANCHOR
//#define DB_AUTO_CREATEANCHOR
//#define DB_AUTO_FINDENTRY
//#define DB_AUTO_CREATEENTRY
//#define DB_AUTO_CREATESAVE
//#define DB_AUTO_DELETESAVE
//#define DB_AUTO_ADDTABLE
//#define DB_AUTO_RESOLVE

///Header

struct FeelinDynamicAutoAnchor
{
	struct FeelinDynamicAutoAnchor *next;
	struct FeelinDynamicAutoAnchor *prev;

	STRPTR 							name;
	FList                           attributes_list;
	FList                           methods_list;
};

struct FeelinDynamicAutoEntry
{
	struct FeelinDynamicAutoEntry   *next;
	struct FeelinDynamicAutoEntry   *prev;

	STRPTR							name;
	struct FeelinDynamicAutoAnchor  *anchor;
	FList                           *anchor_list;
	struct FeelinDynamicAutoSave    *saves;
};

struct FeelinDynamicAutoSave
{
	struct FeelinDynamicAutoSave    *next;
	uint32                          *save;
};

//+

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///f_dynamic_auto_str_cmp
STATIC int32 f_dynamic_auto_str_cmp(STRPTR Str1,STRPTR Str2)
{
	uint8 c1, c2;

	do
	{
		c1 = *Str1++; if (c1 == '_' || c1 == '.') c1 = 0;
		c2 = *Str2++; if (c2 == '_' || c2 == '.') c2 = 0;
	}
	while (c1 == c2 && c1 && c2);

	return (int32)(c1) - (int32)(c2);
}
//+

///f_dynamic_auto_find_anchor
/*

	'Name' e.g. "Text_Xxx"

	When the function is called, semaphore are already locked.

*/

STATIC struct FeelinDynamicAutoAnchor * f_dynamic_auto_find_anchor(STRPTR Name, struct in_FeelinBase *FeelinBase)
{
	struct FeelinDynamicAutoAnchor *anchor;

///DB_AUTO_FINDANCHOR
#ifdef DB_AUTO_FINDANCHOR
	IFEELIN F_Log("AutoFindAnchor - SEARCH (0x%lx) '%s'",Name,Name);
#endif
//+

	for (anchor = (struct FeelinDynamicAutoAnchor *) F_DYNAMIC_LIST.Head ; anchor ; anchor = anchor->next)
	{
		int32 cmp = f_dynamic_auto_str_cmp(Name, anchor->name);

///DB_AUTO_FINDANCHOR
#ifdef DB_AUTO_FINDANCHOR
		IFEELIN F_Log("AutoFindAnchor - CMP '%s' & '%s' = %ld",Name,anchor->name,cmp);
#endif
//+

		if (cmp == 0)
		{
			break;
		}
		else if (cmp < 0)
		{
			anchor = NULL;
			
			break;
		}
	}

///DB_AUTO_FINDANCHOR
#ifdef DB_AUTO_FINDANCHOR
	if (anchor)
	{
		IFEELIN F_Log("AutoFindAnchor >> AutoA 0x%lx - '%s'",anchor,anchor->name);
	}
#endif
//+

	return anchor;
}
//+
///f_dynamic_auto_create_anchor
/*

Name e.g. "Text_Xxx"

*/

STATIC APTR f_dynamic_auto_create_anchor(STRPTR Name,struct in_FeelinBase *FeelinBase)
{
	struct FeelinDynamicAutoAnchor *anchor;
	struct FeelinDynamicAutoAnchor *prev;

	uint32 len=0;

	F_DYNAMIC_LOCK;

	while (Name[len] != '_') len++;

	if ((anchor = IFEELIN F_NewP(F_DYNAMIC_POOL,sizeof (struct FeelinDynamicAutoAnchor) + len + 1)) != NULL)
	{
		anchor->name = (APTR)((ULONG)(anchor) + sizeof (struct FeelinDynamicAutoAnchor));

		IEXEC CopyMem(Name,anchor->name,len);

		for (prev = (APTR)(F_DYNAMIC_LIST.Tail) ; prev ; prev = prev->prev)
		{
			if (IFEELIN F_StrCmp(anchor->name,prev->name,ALL) > 0) break;
		}

		IFEELIN F_LinkInsert(&F_DYNAMIC_LIST,(FNode *) anchor,(FNode *) prev);
	}

	F_DYNAMIC_UNLOCK;

	return anchor;
}
//+
///f_dynamic_auto_delete_anchor
STATIC void f_dynamic_auto_delete_anchor(struct FeelinDynamicAutoAnchor *AutoA,struct in_FeelinBase *FeelinBase)
{
	if (AutoA)
	{
		F_DYNAMIC_LOCK;

///DB_AUTO_DELETEANCHOR
#ifdef DB_AUTO_DELETEANCHOR
		IFEELIN F_Log(0,"AutoDeleteAnchor - AutoA 0x%lx - '%s'",AutoA,AutoA->name);
#endif
//+

		if ((AutoA->attributes_list.Head == NULL) && (AutoA->methods_list.Head == NULL))
		{
			IFEELIN F_LinkRemove(&F_DYNAMIC_LIST,(FNode *) AutoA);

			IFEELIN F_Dispose(AutoA);
		}

		F_DYNAMIC_UNLOCK;
	}
}
//+

///f_dynamic_auto_find_entry
/*
	'Name' e.g. "FA_Text_Width"
*/
STATIC struct FeelinDynamicAutoEntry * f_dynamic_auto_find_entry(STRPTR Name,struct in_FeelinBase *FeelinBase)
{
	struct FeelinDynamicAutoAnchor *anchor;
	struct FeelinDynamicAutoEntry  *autoe = NULL;

	if ((anchor = f_dynamic_auto_find_anchor(Name+3, FeelinBase)) != NULL)
	{
		APTR first = (Name[1] == 'A') ? anchor->attributes_list.Head : anchor->methods_list.Head;

		Name += 3; while (*Name != '_') Name++; Name++;

///DB_AUTO_FINDENTRY
#ifdef DB_AUTO_FINDENTRY
		IFEELIN F_Log(0,"AutoFindEntry - Anchor '%s' - Item '%s'",anchor->name,Name);
#endif
//+

		for (autoe = first ; autoe ; autoe = autoe->next)
		{
			int32 cmp = IFEELIN F_StrCmp(Name,autoe->name,ALL);

			if (cmp == 0)     break;
			else if (cmp < 0) { autoe = NULL; break; }
		}
///DB_AUTO_FINDENTRY
#ifdef DB_AUTO_FINDENTRY
		if (autoe)
		{
			IFEELIN F_Log(0,"AutoFindEntry - Entry 0x%lx - Name '%s'",autoe,autoe->name);
		}
		else
		{
			IFEELIN F_Log(0,"AutoFindEntry - NOT FOUND");
		}
#endif
//+
	}

	return autoe;
}
//+
///f_dynamic_auto_create_entry
/* ///DOC

	'Name' e.g. "FM_Text_Draw"

	This function create a new struct FeelinDynamicAutoEntry  that  will  be
	linked to the approriate list of its appropriate anchor.

	Before calling this function you must be sure that no  other  Auto  Entry
	with  the  same name already exists, and the name passed as argument is a
	valid one.

	Only     the     function     DynamicAutoCreateSave     should      call
	DynamicAutoCreateEntry.  The  Dynamic  semaphore  must  be locked before
	calling the function because the function will not lock it to save time.

*/ //+
STATIC struct FeelinDynamicAutoEntry * f_dynamic_auto_create_entry(STRPTR Name,struct in_FeelinBase *FeelinBase)
{
	struct FeelinDynamicAutoAnchor  *anchor;
	struct FeelinDynamicAutoEntry   *autoe = NULL;

	anchor = f_dynamic_auto_find_anchor(Name+3,FeelinBase);
	if (!anchor) anchor = f_dynamic_auto_create_anchor(Name+3,FeelinBase);

	if (anchor)
	{
		FList *list = (Name[1] == 'A') ? &anchor->attributes_list : &anchor->methods_list;

		struct FeelinDynamicAutoEntry *prev;
		uint32 len;

		Name += 3 ; while (*Name != '_') Name++; Name++; len = IFEELIN F_StrLen(Name);
			
		if ((autoe = IFEELIN F_NewP(F_DYNAMIC_POOL,sizeof (struct FeelinDynamicAutoEntry) + len + 1)) != NULL)
		{
			autoe->anchor = anchor;
			autoe->anchor_list = list;
			autoe->name = (APTR)((ULONG)(autoe) + sizeof (struct FeelinDynamicAutoEntry));

			IEXEC CopyMem(Name,autoe->name,len);

			for (prev = (APTR)(list->Tail) ; prev ; prev = prev->prev)
			{
				if (IFEELIN F_StrCmp(autoe->name,prev->name,ALL) > 0) break;
			}

			IFEELIN F_LinkInsert(list,(FNode *) autoe,(FNode *) prev);

///DB_AUTO_CREATEENTRY
#ifdef DB_AUTO_CREATEENTRY
			IFEELIN F_Log(0,"AutoCreateEntry - Anchor 0x%lx '%s' - AutoE 0x%lx - '%s'",anchor,anchor->name,autoe,autoe->name);
#endif
//+
		}
	}

	return autoe;
}
//+
///f_dynamic_auto_delete_entry
STATIC void f_dynamic_auto_delete_entry(struct FeelinDynamicAutoEntry *AutoE, struct in_FeelinBase *FeelinBase)
{
	if (AutoE)
	{
		struct FeelinDynamicAutoAnchor *anchor = AutoE->anchor;

///DB_AUTO_DELETEENTRY
#ifdef DB_AUTO_DELETEENTRY
		IFEELIN F_Log(0,"AutoDeleteEntry - AutoE 0x%lx - AutoA 0x%lx (0x%lx) '%s'",AutoE,AutoE->anchor,AutoE->anchor_list);
#endif
//+

		IFEELIN F_LinkRemove(AutoE->anchor_list, (FNode *) AutoE);

		IFEELIN F_Dispose(AutoE);

		if ((anchor->attributes_list.Head == NULL) && (anchor->methods_list.Head == NULL))
		{
			f_dynamic_auto_delete_anchor(anchor, FeelinBase);
		}
	}
}
//+

///f_dynamic_auto_create_save
STATIC struct FeelinDynamicAutoSave * f_dynamic_auto_create_save(FDynamicEntry *entry,struct in_FeelinBase *FeelinBase)
{
	struct FeelinDynamicAutoSave  *autos = NULL;
	struct FeelinDynamicAutoEntry *autoe;
	
	STRPTR name = entry->Name;

	if (!name || name[0] != 'F' || (name[1] != 'A' && name[1] != 'M') || name[2] != '_')
	{
		IFEELIN F_Log(FV_LOG_CLASS,"F_DynamicCreateAutoSave() - Incorrect name '%s' (0x%08.lx)",name,name);
		
		return NULL;
	}

///DB_AUTO_CREATESAVE
#ifdef DB_AUTO_CREATESAVE
	IFEELIN F_Log(0,"[33mAutoCreateSave[0m - [1mEntry[0m 0x%lx - [1mStore[0m 0x%lx - '%s'",entry,&entry->ID,entry->Name);
#endif
//+

	autoe = f_dynamic_auto_find_entry(name,FeelinBase);
	if (!autoe) autoe = f_dynamic_auto_create_entry(name,FeelinBase);

	if (autoe)
	{
		if ((autos = IFEELIN F_NewP(F_DYNAMIC_POOL,sizeof (struct FeelinDynamicAutoSave))) != NULL)
		{
			autos->save = &entry->ID;

			/*** link ***/

			if (autoe->saves)
			{
				struct FeelinDynamicAutoSave *prev = autoe->saves;

				while (prev->next) prev = prev->next;
				prev->next = autos;
			}
			else
			{
				autoe->saves = autos;
			}

/* if the correct class is loaded we can already resolve some IDs */

			if (f_dynamic_find_name(name+3,FeelinBase))
			{
				*autos->save = IFEELIN F_DynamicFindID(name);
			}
		}
///DB_AUTO_CREATESAVE
#ifdef DB_AUTO_CREATESAVE
		IFEELIN F_Log(0,"AutoCreateSave - AutoS 0x%lx - Store 0x%lx - AutoE 0x%lx '%s' - AutoA 0x%lx '%s'",autos,autos->save,autoe,autoe->name,autoe->anchor,autoe->anchor->name);
#endif
//+
	}

	return autos;
}
//+
///f_dynamic_auto_delete_save
STATIC void f_dynamic_auto_delete_save(struct FeelinDynamicEntry *entry,struct in_FeelinBase *FeelinBase)
{
	struct FeelinDynamicAutoSave  *autos;
	struct FeelinDynamicAutoEntry *autoe;
	
	STRPTR name = entry->Name;

	if (!name || name[0] != 'F' || (name[1] != 'A' && name[1] != 'M') || name[2] != '_')
	{
		IFEELIN F_Log(FV_LOG_CLASS,"F_DynamicCreateAutoSave() - Incorrect name '%s' (0x%08.lx)",name,name);
		
		return;
	}

///DB_AUTO_DELETESAVE
#ifdef DB_AUTO_DELETESAVE
	IFEELIN F_Log(0,"Auto_DeleteSave - Entry 0x%lx - IDAddr 0x%lx - '%s'",entry,&entry->ID,entry->Name);
#endif
//+

	if ((autoe = f_dynamic_auto_find_entry(name,FeelinBase)) != NULL)
	{
		struct FeelinDynamicAutoSave *prev = NULL;

///DB_AUTO_DELETESAVE
#ifdef DB_AUTO_DELETESAVE
		IFEELIN F_Log(0,"Auto_DeleteSave - AutoE 0x%lx '%s'",autoe,autoe->name);
#endif
//+

		for (autos = autoe->saves ; autos ; autos = autos->next)
		{
			if (autos->save == &entry->ID)
			{
///DB_AUTO_DELETESAVE
#ifdef DB_AUTO_DELETESAVE
				IFEELIN F_Log(0,"Auto_DeleteSave - FOUND - Save 0x%lx",autos);
#endif
//+
				if (prev)   prev->next = autos->next;
				else        autoe->saves = autos->next;

				*autos->save = 0;

				IFEELIN F_Dispose(autos);

				break;
			}
			prev = autos;
		}

		if (!autoe->saves)
		{
			f_dynamic_auto_delete_entry(autoe,FeelinBase);
		}
	}
}
//+

/************************************************************************************************
*** Shared **************************************************************************************
************************************************************************************************/

///f_dynamic_auto_add_class
void f_dynamic_auto_add_class
(
	struct FeelinClass *Class,
	struct in_FeelinBase *FeelinBase
)
{
	struct FeelinDynamicAutoAnchor *autoa;
	struct FeelinDynamicAutoEntry  *autoe;
	struct FeelinDynamicAutoSave   *autos;

	if (!Class) return;

	F_CLASSES_LOCK;
	F_DYNAMIC_LOCK;

	if ((autoa = f_dynamic_auto_find_anchor(Class->Name, FeelinBase)) != NULL)
	{
		if (autoa->attributes_list.Head)
		{
			if (Class->Attributes)
			{
				for (autoe = (struct FeelinDynamicAutoEntry *)(autoa->attributes_list.Head) ; autoe ; autoe = autoe->next)
				{
					FClassAttribute *entry;
					int32 found = FALSE;

					for (F_ATTRIBUTES_EACH(Class->Attributes, entry))
					{
						if (IFEELIN F_StrCmp(autoe->name, _attribute_name(entry), ALL) == 0)
						{
							found = TRUE;

							for (autos = autoe->saves ; autos ; autos = autos->next)
							{
								*autos->save = entry->ID;
///DB_AUTO_RESOLVE
#ifdef DB_AUTO_RESOLVE
								IFEELIN F_Log(0,"AutoResolve - F%lc_%s_%s 0x%lx",(autoe->anchor_list == &autoe->anchor->attributes_list) ? 'A' : 'M',autoe->anchor->name,autoe->name,*autos->save);
#endif
//+
							}
						}
					}

					if (found == FALSE)
					{
						IFEELIN F_Log(FV_LOG_CLASS,"F_AutoDynamic() - 'F%lc_%s_%s' not defined !!",(autoe->anchor_list == &autoe->anchor->attributes_list) ? 'A' : 'M',autoe->anchor->name,autoe->name);
					}
				}
			}
			else
			{
				struct FeelinDynamicAutoEntry *autoe;

				IFEELIN F_Log(FV_LOG_CLASS,"%s'%s' does not define any %s","F_DynamicAuto() - ",Class->Name,"attribute");

				for (autoe = (struct FeelinDynamicAutoEntry *)(autoa->attributes_list.Head) ; autoe ; autoe = autoe->next)
				{
					IFEELIN F_Log(FV_LOG_CLASS,"%sUnable to resolve 'F%lc_%s_%s'","F_DynamicAuto() - ",'A',autoa->name,autoe->name);
				}
			}
		}

		if (autoa->methods_list.Head)
		{
			if (Class->Methods)
			{
				for (autoe = (struct FeelinDynamicAutoEntry *) autoa->methods_list.Head ; autoe ; autoe = autoe->next)
				{
					FClassMethod *entry;
					BOOL found = FALSE;

					for (F_METHODS_EACH(Class->Methods, entry))
					{
						if (_method_name(entry))
						{
							if (IFEELIN F_StrCmp(autoe->name, _method_name(entry), ALL) == 0)
							{
								found = TRUE;

								for (autos = autoe->saves ; autos ; autos = autos->next)
								{
									*autos->save = entry->ID;
///DB_AUTO_RESOLVE
#ifdef DB_AUTO_RESOLVE
									IFEELIN F_Log(0,"AutoResolve - F%lc_%s_%s 0x%lx",(autoe->anchor_list == &autoe->anchor->attributes_list) ? 'A' : 'M',autoe->anchor->name,autoe->name,*autos->save);
#endif
//+
								}
							}
						}
					}

					if (found == FALSE)
					{
						IFEELIN F_Log(FV_LOG_CLASS,"F_AutoDynamic() - 'F%lc_%s_%s' not defined !!",(autoe->anchor_list == &autoe->anchor->attributes_list) ? 'A' : 'M',autoe->anchor->name,autoe->name);
					}
				}
			}
			else
			{
				struct FeelinDynamicAutoEntry *autoe;

				IFEELIN F_Log(FV_LOG_CLASS,"%s'%s' does not define any %s","F_DynamicAuto() - ",Class->Name,"methods");

				for (autoe = (struct FeelinDynamicAutoEntry *) autoa->methods_list.Head ; autoe ; autoe = autoe->next)
				{
					IFEELIN F_Log(FV_LOG_CLASS,"%sUnable to resolve 'F%lc_%s_%s'","F_DynamicAuto() - ",'M',autoa->name,autoe->name);
				}
			}
		}
	}

	F_DYNAMIC_UNLOCK;
	F_CLASSES_UNLOCK;
}
//+

/************************************************************************************************
*** API *****************************************************************************************
************************************************************************************************/

///f_dynamic_add_auto_table
F_LIB_DYNAMIC_ADD_AUTO_TABLE
{
	F_DYNAMIC_LOCK;

	if (Table)
	{
		FDynamicEntry *entry = Table;

		while (entry->Name)
		{
			if (f_dynamic_auto_create_save(entry, FeelinBase))
			{
				entry++;
			}
			else
			{
				break;
			}
		}

		/* if we failed to get through the  table  we  need  to  delete  the
		structures we created */

		if (entry->Name)
		{
			FDynamicEntry *stop = entry;

			entry = Table;

			while (entry->Name)
			{
				if (entry == stop) break;

				f_dynamic_auto_delete_save(entry++, FeelinBase);
			}
		}
		else
		{
			F_DYNAMIC_UNLOCK;

			return TRUE;
		}
	}

	F_DYNAMIC_UNLOCK;

	return FALSE;
}
//+
///f_dynamic_rem_auto_table
F_LIB_DYNAMIC_REM_AUTO_TABLE
{
	if (Table)
	{
		F_DYNAMIC_LOCK;

		while (Table->Name)
		{
			f_dynamic_auto_delete_save(Table++, FeelinBase);
		}

		F_DYNAMIC_UNLOCK;
	}
}
//+

