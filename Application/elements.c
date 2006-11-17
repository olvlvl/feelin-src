#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///app_create_name
STATIC STRPTR app_create_name(struct LocalObjectData *LOD, STRPTR Name)
{
	STRPTR buf = NULL;

	if (((uint32)(Name) == FV_Application_ENV) ||
		((uint32)(Name) == FV_Application_ENVARC))
	{
		if (LOD->title)
		{
			buf = IFEELIN F_StrNewP(LOD->pool, NULL, "%s/%s." F_PDRDOCUMENT_EXT, (Name == FV_Application_ENV) ? "ENV:Feelin" : "ENVARC:Feelin", LOD->title);
		}
	}
	else
	{
		buf = IFEELIN F_StrNewP(LOD->pool, NULL, "%s", Name);
	}

	return buf;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

#ifdef F_NEW_PERSISTENT

///App_Load
F_METHODM(bool32,App_Load,FS_Application_Load)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	bool32 rc = FALSE;

	STRPTR name = app_create_name(LOD, (STRPTR) FV_Application_ENV);

	if (name)
	{
		FObject storage = PDRDocumentObject,

			"FA_Document_Source",       name,
			"FA_Document_SourceType",   FV_Document_SourceType_File,

			End;

		if (storage)
		{
			FFamilyNode *node;

			for (node = (FFamilyNode *) IFEELIN F_Get(Obj,FA_Family_Head) ; node ; node = node->Next)
			{
				IFEELIN F_Do(node->Object, FM_Element_LoadPersistentAttributes, storage);
			}

			rc = TRUE;

			IFEELIN F_DisposeObj(storage);
		}

		IFEELIN F_Dispose(name);
	}

	return rc;
}
//+
///App_Save
F_METHODM(bool32, App_Save, FS_Application_Save)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	bool32 rc = FALSE;

	if (LOD->title)
	{
		FObject storage = PDRDocumentObject, End;

		if (storage)
		{
			FFamilyNode *node;

			for (node = (FFamilyNode *) IFEELIN F_Get(Obj,FA_Family_Head) ; node ; node = node->Next)
			{
				IFEELIN F_Do(node->Object, FM_Element_SavePersistentAttributes, storage);
			}

			if ((uint32)(Msg->Name) == FV_Application_BOTH)
			{
				STRPTR name;

				name = app_create_name(LOD, (STRPTR) FV_Application_ENV);

				if (name)
				{
					IFEELIN F_Do(storage, (uint32) "FM_Document_Write", name, FV_Document_SourceType_File);

					IFEELIN F_Dispose(name);
				}

				name = app_create_name(LOD, (STRPTR) FV_Application_ENVARC);

				if (name)
				{
					IFEELIN F_Do(storage, (uint32) "FM_Document_Write", name, FV_Document_SourceType_File);

					IFEELIN F_Dispose(name);
				}
			}
			else
			{
				STRPTR name;

				name = app_create_name(LOD, FV_Application_ENV);

				if (name)
				{
					IFEELIN F_Do(storage, (uint32) "FM_Document_Write", name, FV_Document_SourceType_File);

					IFEELIN F_Dispose(name);
				}
			}

			rc = TRUE;

			IFEELIN F_DisposeObj(storage);
		}
	}

	return rc;
}
//+

#endif

#ifdef F_NEW_APPLICATION_PUBLICIZE

struct in_Element
{
	struct in_Element			   *next;
	FObject							element;
	FAtom						   *id;
};

///Application_PublicizeElement
F_METHODM(bool32, Application_PublicizeElement, FS_Application_PublicizeElement)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FObject element = Msg->Element;
	STRPTR id = Msg->Id;

	FAtom *atom;
	struct in_Element *node;

	if (!element || !id)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Missing one of Element (0x%08lx) Id (0x%08lx)", element, id);

		return FALSE;
	}

	F_LOCK_ELEMENTS;

	/* obtain the atom of the element's id */

	atom = IFEELIN F_AtomObtainP(LOD->elements_atom_pool, id);

	if (!atom)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Unable to obtain atom (%s) from pool", id);

		F_UNLOCK_ELEMENTS;

		return FALSE;
	}

	/* check if another element has the same id */

	for (node = LOD->elements ; node ; node = node->next)
	{
		if (node->id == atom)
		{
			IFEELIN F_Log(FV_LOG_DEV, "Id (%s) already used by Element %s{%lx}", id, _object_classname(node->element), node->element);

			IFEELIN F_AtomReleaseP(LOD->elements_atom_pool, atom);

			F_UNLOCK_ELEMENTS;

			return FALSE;
		}
	}

	/* allocate element's node */

	node = IFEELIN F_NewP(LOD->pool, sizeof (struct in_Element));

	if (!node)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Unable to allocate a node for the Element (%s)", id);

		IFEELIN F_AtomReleaseP(LOD->elements_atom_pool, atom);

		F_UNLOCK_ELEMENTS;

		return FALSE;
	}

	/* initiate the node */

	node->next = LOD->elements;
	node->element = element;
	node->id = atom;

	LOD->elements = node;

	F_UNLOCK_ELEMENTS;

	return TRUE;
}
//+
///Application_PrivatizeElement
F_METHODM(bool32, Application_PrivatizeElement, FS_Application_PrivatizeElement)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FObject element = Msg->Element;

	struct in_Element *prev = NULL;
	struct in_Element *node;

	if (element == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Missing Element");

		return FALSE;
	}

	F_LOCK_ELEMENTS;

	/* search element's node */

	for (node = LOD->elements ; node ; prev = node, node = node->next)
	{
		if (node->element == element)
		{
			break;
		}
	}

	if (!node)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Unknown Element %s{%lx}", _object_classname(element), element);

		F_UNLOCK_ELEMENTS;

		return FALSE;
	}

	if (prev)
	{
		prev->next = node->next;
	}
	else
	{
		LOD->elements = node->next;
	}

	IFEELIN F_AtomReleaseP(LOD->elements_atom_pool, node->id);

	IFEELIN F_Dispose(node);

	return TRUE;
}
//+
#endif
