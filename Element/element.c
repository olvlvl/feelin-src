#include "Private.h"

//#define DB_SETUP
//#define DB_VERBOSE_UNDEFINED

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

#ifdef F_NEW_GLOBALCONNECT
///Element_GlobalConnect
F_METHODM(bool32, Element_GlobalConnect, FS_Element_GlobalConnect)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if ((Msg->Application == NULL) || (Msg->Window == NULL))
	{
		IFEELIN F_Log(FV_LOG_DEV, "Missing one of application (0x%08lx) window (0x%08lx)", Msg->Application, Msg->Window);

		return FALSE;
	}

	_element_application = Msg->Application;
	_element_window = Msg->Window;

	#ifdef DB_GLOBALCONNECT

	IFEELIN F_Log(0, "%s{%lx} %s{%lx}",

		_object_classname(_element_application),
		_element_application,
		_object_classname(_element_window),
		_element_window);

	#endif

	return TRUE;
}
//+
///Element_GlobalDisconnect
F_METHODM(bool32, Element_GlobalDisconnect, FS_Element_GlobalDisconnect)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	_element_application = NULL;
	_element_window = NULL;

	return TRUE;
}
//+
#endif
///Element_Setup
F_METHODM(bool32, Element_Setup, FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT

	if ((_element_application == NULL) || (_element_window == NULL) || (_element_parent == NULL))
	{
		IFEELIN F_Log(FV_LOG_DEV, "Missing one of application (0x%08lx) window (0x%08lx) parent (0x%08lx)", _element_application, _element_window, _element_parent);

		return FALSE;
	}

	#endif

	if (Msg->Render == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Render is NULL !");

		return FALSE;
	}

	LOD->render = Msg->Render;

	/* Get the Preference object of the application */

	LOD->preferences = (FObject) IFEELIN F_Get(Msg->Render->Application, FA_Application_Preference);

	if (LOD->preferences == NULL)
	{
		return FALSE;
	}

	/* Obtain the compased style for our object */

	LOD->composed_style = (FPreferenceStyle *) IFEELIN F_Do(LOD->preferences, F_AUTO_ID(OBTAINSTYLE), Obj);

	if (LOD->composed_style == NULL)
	{
		if (LOD->styleclass != NULL)
		{
			IFEELIN F_Log(0, "style failed for (%s) of class (%s)", _object_classname(Obj), LOD->styleclass);
		}
		else
		{
			IFEELIN F_Log(0, "style failed for (%s)", _object_classname(Obj));
		}

		return FALSE;
	}

	return TRUE;
}
//+
///Element_Cleanup
F_METHODM(bool32, Element_Cleanup, FS_Element_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->preferences == NULL)
	{
		return FALSE;
	}

	if (LOD->composed_style == NULL)
	{
		return FALSE;
	}

	IFEELIN F_Do(LOD->preferences, F_AUTO_ID(RELEASESTYLE), Obj, LOD->composed_style);

	LOD->composed_style = NULL;
	LOD->preferences = NULL;
	LOD->render = NULL;

	return TRUE;
}
//+
///Element_CreateDecodedStyle
F_METHOD(bool32,Element_CreateDecodedStyle)
{
	return TRUE;
}
//+
///Element_DeleteDecodedStyle
F_METHOD(bool32,Element_DeleteDecodedStyle)
{
	return TRUE;
}
//+
///Element_LoadPersistentAttributes
F_METHODM(FPDRHandle *, Element_LoadPersistentAttributes, FS_Element_LoadPersistentAttributes)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->id && LOD->persist)
	{
		return (FPDRHandle *) IFEELIN F_Do(Msg->Storage, F_AUTO_ID(FINDHANDLE), LOD->id, LOD->persist);
	}

	return NULL;
}
//+
///Element_SavePersistentAttributes
F_METHODM(FPDRHandle *, Element_SavePersistentAttributes, FS_Element_SavePersistentAttributes)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->id && LOD->persist)
	{
		return (FPDRHandle *) IFEELIN F_Do(Msg->Storage, F_AUTO_ID(CREATEHANDLE), LOD->id, LOD->persist);
	}

	return NULL;
}
//+

///Element_GetProperty
F_METHODM(bool32,Element_GetProperty,FS_Element_GetProperty)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	bool32 rc = FALSE;

	if (LOD->composed_style == NULL)
	{
		IFEELIN F_Log(0,"*** OBJECT HAS NOT BEEN SETUP YET !!");
	}

	if (Msg->Atom == NULL)
	{
		IFEELIN F_Log(0, "suspicious message: speudo atom (0x%08lx) property atom (0x%08lx) value ptr (0x%08lx) numeric ptr (0x%08lx)", Msg->PseudoAtom, Msg->Atom, Msg->ValuePtr, Msg->NumericValuePtr);

		return FALSE;
	}

	#ifdef DB_GETPROPERTY
	IFEELIN F_Log(0, "speudo atom (0x%08lx) property atom (0x%08lx)(%s) value ptr (0x%08lx) numeric ptr (0x%08lx)", Msg->PseudoAtom, Msg->Atom, Msg->Atom->Key, Msg->ValuePtr, Msg->NumericValuePtr);
	#endif

	if (Msg->Atom != NULL)
	{
		FPreferenceStyle *style;

		for (style = LOD->composed_style ; style ; style = style->Next)
		{
			if (Msg->PseudoAtom == style->Atom)
			{
				FPreferenceProperty **array;

				for (array = style->Properties ; *array ; array++)
				{
					if ((*array)->Atom == Msg->Atom)
					{
						if (Msg->ValuePtr)
						{
							*Msg->ValuePtr = (*array)->Value;
						}

						if (Msg->NumericValuePtr)
						{
							*Msg->NumericValuePtr = (*array)->NumericValue;
						}

						rc = TRUE;

						//return TRUE;
					}
				}

				//break;
			}
		}

		#ifdef DB_VERBOSE_UNDEFINED
		IFEELIN F_Log(FV_LOG_DEV, "undefined property (%s) for class (%s)", Msg->Atom->Key, _object_classname(Obj));
		#endif
	}

	return rc;
}
//+

