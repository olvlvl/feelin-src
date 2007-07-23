/*
**    fc_Class.c
**
**    Class class :-)
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************

$VER: 09.01 (2005/09/29)

	Added AOS4 support.

	GROG: Modifications for the new design. Morphos port and Mr Proper.

	It's now possible, forced in fact since FA_Class_Init and FA_Class_Exit
	are considered deprecated, to create metaclasses (classes for classes).
	
	Class name is now allocated  from  F_DYNAMIC_POOL  instead  of
	default memory pool.
  
************************************************************************************************/

#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///f_localize
STATIC struct Catalog * f_localize
(
	STRPTR CatalogName,
	struct FeelinCatalogEntry *Table,
	struct in_FeelinBase *FeelinBase
)
/*

	This function is used to replace default class strings by localized ones

*/
{
	struct FeelinCatalogEntry *en;

	for (en = Table ; en->ID ; en++)
	{
		en->String = en->Default;
	}

//    IFEELIN F_Log(0,"catalog (%s) locale (0x%08lx)", CatalogName, LocaleBase);

	if (CatalogName)
	{
		if (LocaleBase)
		{
			struct Catalog *cat = ILOCALE OpenCatalog(NULL,CatalogName,TAG_DONE);

			if (cat)
			{
				for (en = Table ; en->ID ; en++)
				{
					STRPTR message = ILOCALE GetCatalogStr(cat,en->ID,en->Default);

					if (message)
					{
						en->String = message;

//                        IFEELIN F_Log(0,"default (%s) locale (%s)", en->Default, en->String);
					}
				}

				return cat;
			}
		}
	}

	return NULL;
}
//+

///class_add_atoms
STATIC bool32 class_add_atoms(struct in_FeelinClass *LOD, struct in_FeelinBase *FeelinBase)
{
	FClassAtom *ca;

	for (ca = LOD->public.Atoms ; ca->Name ; ca++)
	{
		ca->Atom = IFEELIN F_AtomObtain(ca->Name, ALL);

		if (ca->Atom == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}
//+
///class_rem_atoms
STATIC void class_rem_atoms(struct in_FeelinClass *LOD, struct in_FeelinBase *FeelinBase)
{
	FClassAtom *ca;

	for (ca = LOD->public.Atoms ; ca->Name ; ca++)
	{
		if (ca->Atom)
		{
			IFEELIN F_AtomRelease(ca->Atom);

			ca->Atom = NULL;
		}
	}
}
//+

///class_add_properties
STATIC bool32 class_add_properties(struct in_FeelinClass *LOD, struct in_FeelinBase *FeelinBase)
{
	FClassProperty *property;

	for (F_CLASS_EACH_PROPERTY(LOD->public.Properties, property))
	{
		property->Atom = IFEELIN F_AtomObtain(property->Name, ALL);

		if (property->Atom == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}
//+
///class_rem_properties
STATIC void class_rem_properties(struct in_FeelinClass *LOD, struct in_FeelinBase *FeelinBase)
{
	FClassProperty *property;

	for (F_CLASS_EACH_PROPERTY(LOD->public.Properties, property))
	{
		if (property->Atom != NULL)
		{
			IFEELIN F_AtomRelease(property->Atom);

			property->Atom = NULL;
		}
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Class_New
F_METHOD(FObject,Class_New)
{
	struct in_FeelinClass *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,*item;

	FCatalogEntry *cat_table=NULL;

	STRPTR classname=NULL;
	STRPTR supername=NULL;
	STRPTR cat_name=NULL;

	bool32 haspool=TRUE;
  
	while ((item = IUTILITY NextTagItem(&Tags)))
	switch (item->ti_Tag)
	{
		case FA_Class_Name:             classname = (STRPTR) item->ti_Data; break;
		case FA_Class_Super:            supername = (STRPTR) item->ti_Data; break;
		case FA_Class_LocalSize:        LOD->public.LocalSize    = (((item->ti_Data + 3) >> 2) << 2); break;
		case FA_Class_Dispatcher:       LOD->dispatcher        = (FMethod) item->ti_Data; break;
		case FA_Class_UserData:         LOD->public.UserData   = (APTR) item->ti_Data; break;
		case FA_Class_Pool:             haspool = item->ti_Data; break;
		case FA_Class_Methods:          LOD->public.Methods         = (APTR) item->ti_Data; break;
		case FA_Class_Attributes:       LOD->public.Attributes      = (APTR) item->ti_Data; break;
		case FA_Class_Resolveds:        LOD->public.Resolveds     = (APTR) item->ti_Data; break;
		case FA_Class_Autos:            LOD->public.Autos = (APTR) item->ti_Data; break;
		case FA_Class_CatalogName:      cat_name = (STRPTR) item->ti_Data; break;
		case FA_Class_CatalogTable:     cat_table = (APTR) item->ti_Data; break;
		case FA_Class_Module:           LOD->module = (struct Library *)(item->ti_Data); break;

		case FA_Class_Atoms:
		{
			LOD->public.Atoms = (FClassAtom *) item->ti_Data;
		}
		break;

		case FA_Class_Properties:
		{
			LOD->public.Properties = (FClassProperty *) item->ti_Data;
		}
		break;

		case FA_Class_PropertiesLocalSize:
		{
			LOD->public.PropertiesLocalSize = item->ti_Data;
		}
		break;

		default:
		{
		   IFEELIN F_Log(0,"Unknow Attribute : 0x%08lx (0x%08lx) class (%s) super (%s)",item->ti_Tag,item->ti_Data,classname,supername);
		}
	}

/** copy/create class name *********************************************************************/

	if (classname)
	{
		LOD->public.Atom = IFEELIN F_AtomObtain(classname, ALL);
	}
	else
	{
		uint32 len;
		STRPTR temp = IFEELIN F_StrNew(&len, ":cc:%08lx%02lx", LOD, LOD->public.LocalSize);

		if (temp)
		{
			LOD->public.Atom = IFEELIN F_AtomObtain(temp, len);

			IFEELIN F_Dispose(temp);
		}
	}

	if (LOD->public.Atom == NULL)
	{
		IFEELIN F_Log(FV_LOG_USER, "unable to obtain atom");

		return NULL;
	}

	LOD->public.Name = LOD->public.Atom->Key;

/** inheritence ********************************************************************************/

	if (supername)
	{
		FClass *super = IFEELIN F_OpenClass(supername);

		if (super)
		{
			LOD->public.Super  = super;
			LOD->public.Offset = super->Offset + super->LocalSize;
			LOD->public.PropertiesOffset = super->PropertiesOffset + super->PropertiesLocalSize;
		}
		else
		{
			return NULL;
		}
	}

/*** memory pool setup *************************************************************************/
  
	if (haspool && classname && LOD->public.LocalSize)
	{
		LOD->pool = IFEELIN F_CreatePool(sizeof (APTR) + LOD->public.Offset + LOD->public.LocalSize,FA_Pool_Name,LOD->public.Name,TAG_DONE);
	}
	
	if (!LOD->pool)
	{
		if (LOD->public.Super)
		{
			LOD->pool = ((struct in_FeelinClass *)(LOD->public.Super))->pool;
		  
			LOD->flags |= FF_CLASS_INHERITED_POOL;
		}
		else
		{
			LOD->pool = FeelinBase->DefaultPool;
		  
			LOD->flags |= FF_CLASS_INHERITED_POOL;
		}
	}

/*** localization ******************************************************************************/
  
	if (cat_table)
	{
		STRPTR str;

		if (cat_name)
		{
			str = IFEELIN F_StrNew(NULL,"Feelin/%s.catalog",cat_name);
		}
		else
		{
			str = IFEELIN F_StrNew(NULL,"Feelin/FC_%s.catalog",LOD->public.Name);
		}

		if (str)
		{
			LOD->catalog = f_localize(str, cat_table, FeelinBase);

			IFEELIN F_Dispose(str);
		}
	}

/** tables *************************************************************************************/

	if (LOD->public.Atoms)
	{
		if (class_add_atoms(LOD, FeelinBase) == FALSE)
		{
			return NULL;
		}
	}

	if (LOD->public.Properties != NULL)
	{
		if (class_add_properties(LOD, FeelinBase) == FALSE)
		{
			return NULL;
		}
	}

/*** linking ***********************************************************************************/

	#ifdef F_NEW_SINGLELIST_CLASSES

	LOD->public.Next = F_CLASSES_INDEX;
	F_CLASSES_INDEX = (FClass *) LOD;

	#else

	IFEELIN F_LinkTail(&F_CLASSES_LIST, (FNode *) LOD);

	#endif

/*** dynamic IDs setup *************************************************************************/

	if (IFEELIN F_DynamicCreate((FClass *) LOD) == FALSE)
	{
	   IFEELIN F_Log(FV_LOG_USER,"(%s) Unable to create Dynamic entries", LOD->module ? LOD->module->lib_Node.ln_Name : "custom");
	   
	   return NULL;
	}

	return Obj;
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
	struct in_FeelinClass *LOD = F_LOD(Class,Obj);

	F_CLASSES_LOCK;

	/* Check User Count */

	if (LOD->public.UserCount)
	{
		IFEELIN F_Alert("Class.Dispose()","!! WARNING !!\nOpen count of '%s' at %ld\nClass will be removed anyway",LOD->public.Name,LOD->public.UserCount);
	}

	if (LOD->catalog)
	{
		ILOCALE CloseCatalog(LOD->catalog); LOD->catalog = NULL;
	}

	if (LOD->public.Properties)
	{
		class_rem_properties(LOD, FeelinBase);

		LOD->public.Properties = NULL;
	}

	if (LOD->public.Atoms)
	{
		class_rem_atoms(LOD, FeelinBase);

		LOD->public.Atoms = NULL;
	}

	/* unlink class */

	#ifdef F_NEW_SINGLELIST_CLASSES

	{
		FClass *prev = NULL;
		FClass *node;

		for (F_CLASSES_EACH(node))
		{
			if (node == (FClass *) LOD)
			{
				if (prev)
				{
					prev->Next = node->Next;
				}
				else
				{
					F_CLASSES_INDEX = node->Next;
				}

				break;
			}

			prev = node;
		}
	}

	#else

	if (IFEELIN F_LinkMember(&F_CLASSES_LIST,(FNode *) LOD))
	{
		IFEELIN F_LinkRemove(&F_CLASSES_LIST,(FNode *) LOD);
	}

	#endif

	/* remove dynamic entries */

	IFEELIN F_DynamicDelete((FClass *) LOD);
	
	if (!(FF_CLASS_INHERITED_POOL & LOD->flags))
	{
		IFEELIN F_DeletePool(LOD->pool); LOD->pool = NULL;
	}

/** release class name atom ********************************************************************/

	if (LOD->public.Atom)
	{
		IFEELIN F_AtomRelease(LOD->public.Atom);

		LOD->public.Atom = NULL;
		LOD->public.Name = NULL;
	}

	/* automatically close library module, if class was built from a library
	*/

	if (LOD->module)
	{
		#ifdef __amigaos4__
		IEXEC DropInterface((struct Interface *) LOD->module_iface); LOD->module_iface = NULL;
		#endif
	   
		IEXEC CloseLibrary(LOD->module); LOD->module = NULL;
	}

	/* Free SuperClass */

	IFEELIN F_CloseClass(LOD->public.Super);
  
	F_CLASSES_UNLOCK;

	return 0;
}
//+
///Class_Get
F_METHOD(uint32,Class_Get)
{
	struct in_FeelinClass *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class) != NULL)
	switch (item.ti_Tag)
	{
		case FA_Class_Version:
		{
			F_STORE(LOD->module ? LOD->module->lib_Version : 0);
		}
		break;

		case FA_Class_Revision:
		{
			F_STORE(LOD->module ? LOD->module->lib_Revision : 0);
		}
		break;

		case FA_Class_LocalSize:
		{
			F_STORE(LOD->public.LocalSize);
		}
		break;
	}

	return 0;
}
//+

/************************************************************************************************
*** Constructor *********************************************************************************
************************************************************************************************/

STATIC F_ATTRIBUTES_ARRAY =
{
	F_ATTRIBUTES_ADD_STATIC("Revision",   FV_TYPE_INTEGER,    FA_Class_Revision),
	F_ATTRIBUTES_ADD_STATIC("Version",    FV_TYPE_INTEGER,    FA_Class_Version),

	F_ARRAY_END
};

STATIC F_METHODS_ARRAY =
{
	F_METHODS_ADD_STATIC(Class_New, 	"New",      FM_New),
	F_METHODS_ADD_STATIC(Class_Dispose, "Dispose",  FM_Dispose),
	F_METHODS_ADD_STATIC(Class_Get,  	"Get",  	FM_Get),
   
	F_ARRAY_END
};

///fc_class_create
FClass * fc_class_create(struct in_FeelinBase *FeelinBase)
{
	FClass *cl = IFEELIN F_NewP(F_CLASSES_POOL,sizeof (FClass *) + sizeof (struct in_FeelinClass));

	if (cl)
	{
		*(FClass **)(cl) = (FClass *)((uint32)(cl) + sizeof (FClass *));
		cl = (FClass *)((uint32)(cl) + sizeof (FClass *));

		((struct in_FeelinClass *) cl)->pool = F_CLASSES_POOL;
		cl->Atom = IFEELIN F_AtomObtain(FC_Class, ALL);

		if (cl->Atom == NULL)
		{
			return NULL;
		}

		cl->Name = cl->Atom->Key;

		cl->LocalSize = sizeof (struct in_FeelinClass);
		cl->UserCount = 1;
		cl->Attributes = F_ATTRIBUTES_PTR;
		cl->Methods = F_METHODS_PTR;

		#ifdef F_NEW_SINGLELIST_CLASSES

		cl->Next = F_CLASSES_INDEX;
		F_CLASSES_INDEX = (FClass *) cl;

		#else

		IFEELIN F_LinkTail(&F_CLASSES_LIST,(FNode *) cl);

		#endif
	}

	return cl;
}
//+

