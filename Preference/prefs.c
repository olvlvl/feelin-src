#include "Private.h"

//#define DB_CLEAR
//#define DB_WRITE

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Prefs_Clear
F_METHOD(uint32,Prefs_Clear)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_LOCK_ARBITER;

	if (LOD->styles != NULL)
	{
		style_flush(Class, Obj);

		#ifdef DB_CLEAR

		if (LOD->styles != NULL)
		{
			IFEELIN F_Log(0,"some styles remain !");
		}

		#endif
	}

	if (LOD->declaration_list.Head != NULL)
	{
		declaration_flush(Class, Obj);

		#ifdef DB_CLEAR

		if (LOD->declaration_list.Head != NULL)
		{
			IFEELIN F_Log(0,"some declarations remain !");
		}

		#endif
	}

	IFEELIN F_SuperDo(Class, Obj, F_RESOLVED_ID(CLEAR));

	F_UNLOCK_ARBITER;

	return 0;
}
//+
///Prefs_Read
F_METHODM(bool32,Prefs_Read,FS_Preference_Read)
{
	bool32 rc = FALSE;
	
	STRPTR name = preference_resolve_name(Class,Obj,Msg->Name);

	F_LOCK_ARBITER;

	IFEELIN F_Do(Obj, F_METHOD_ID(CLEAR));

/** read ****************************************************************************************

	Reading new preference items is done by clearing the document's contents
	and merging the new one.
 
*/
	
	if (name)
	{
		rc = IFEELIN F_Do(Obj, F_METHOD_ID(MERGE), name, FV_Document_SourceType_File);

		IFEELIN F_Dispose(name);

	}

	F_UNLOCK_ARBITER;

	return rc;
}
//+
///Prefs_Write
F_METHODM(bool32,Prefs_Write,FS_Preference_Write)
{
	bool32 rc = FALSE;
 
	STRPTR name = Msg->Name;

	if ((uint32)(name) == FV_Preference_BOTH)
	{
	   IFEELIN F_Do(Obj,Method,FV_Preference_ENVARC);

	   name = (STRPTR)(FV_Preference_ENV);
	}

	name = preference_resolve_name(Class,Obj,name);

	if (name)
	{
		IFEELIN F_Do(CUD->arbiter, FM_Lock, FF_Lock_Shared);

		rc = IFEELIN F_SuperDo(Class, Obj, F_RESOLVED_ID(WRITE), name);
 
		IFEELIN F_Do(CUD->arbiter, FM_Unlock);
		
		IFEELIN F_Dispose(name);
	}
	
	return rc;
}
//+

///Prefs_Merge
F_METHODM(uint32,Prefs_Merge,FS_Document_Merge)
{
	uint32 rc;
	
	F_LOCK_ARBITER;
		
/*** merge contents ****************************************************************************/
 
	rc = F_SUPERDO();

	if (rc != 0)
	{
		APTR pool = NULL;
		FCSSDeclaration *declaration = NULL;
	
/*** parse CSSDeclarations and create preference items *****************************************/
 
		IFEELIN F_Do
		(
			Obj, FM_Get,

			F_RESOLVED_ID(POOL), &pool,
			F_RESOLVED_ID(DECLARATIONS), &declaration,

			TAG_DONE
		);

		if ((pool != NULL) && (declaration != NULL))
		{
			for ( ; declaration ; declaration = declaration->Next)
			{
				declaration_create(Class, Obj, declaration, NULL);

				if (declaration->Pseudos != NULL)
				{
					FCSSDeclaration *pseudo;

					for (pseudo = declaration->Pseudos ; pseudo ; pseudo = pseudo->Next)
					{
						declaration_create(Class, Obj, pseudo, declaration);
					}
				}
			}

			rc = TRUE;
		}
	}

	F_UNLOCK_ARBITER;
	
	return rc;
}
//+
