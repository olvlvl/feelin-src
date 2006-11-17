#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Element_New
F_METHOD(uint32,Element_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;

	STRPTR style = NULL;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		#ifdef F_NEW_ELEMENT_ID
		case FA_Element_ID:
		#else
		case FA_ID:
		#endif
		{
			LOD->id = (STRPTR) item.ti_Data;
		}
		break;

		case FA_Element_Class:
		{
			LOD->styleclass = (STRPTR) item.ti_Data;
		}
		break;

		case FA_Element_Style:
		{
			if (style == NULL)
			{
				style = (STRPTR) item.ti_Data;
			}
			else
			{
				IFEELIN F_Log(0,"'Style' has already been defined in the tag list (%s)", style);
			}
		}
		break;

		case FA_Element_Persist:
		{
			LOD->persist = (STRPTR) item.ti_Data;
		}
		break;
	}

	if (style)
	{
		#ifdef F_NEW_STYLES_EXTENDED

		LOD->style = IFEELIN F_StrNew(NULL, "%s", style);

		#else

		element_style_create(Class, Obj, style);

		#endif
	}

	return F_SUPERDO();
}
//+
///Element_Dispose
F_METHOD(uint32,Element_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

//	  IFEELIN F_Log(0, "parent (0x%08lx) window (0x%08lx) application (0x%08lx)", _element_parent, _element_window, _element_application);

	#ifdef F_NEW_STYLES_EXTENDED

	if (LOD->style)
	{
		IFEELIN F_Dispose(LOD->style);

		LOD->style = NULL;
	}

	#else

	if (LOD->properties != NULL)
	{
		FPreferenceProperty **array;

		for (array = LOD->properties ; *array ; array++)
		{
			IFEELIN F_AtomRelease((*array)->Atom);
			IFEELIN F_Dispose((*array)->Value);
		}

		IFEELIN F_Dispose(LOD->properties);

		LOD->properties = NULL;
	}

	#endif

	#ifdef F_NEW_GLOBALCONNECT

	if (_element_parent != NULL)
	{
//		  IFEELIN F_Log(0, "parent %s{%lx}", _object_classname(_element_parent), _element_parent);

		IFEELIN F_Do(_element_parent, FM_RemMember, Obj);

		if (_element_parent)
		{
			IFEELIN F_Log(0, "parent %s{%lx} NOT DISCONNECTED !!!", _object_classname(_element_parent), _element_parent);
		}
	}

	#endif

	return F_SUPERDO();
}
//+
///Element_Get
F_METHOD(uint32,Element_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		#ifdef F_NEW_ELEMENT_ID

		case FA_Element_ID:
		{
			F_STORE(LOD->id);
		}
		break;

		#endif

		#ifdef F_NEW_GLOBALCONNECT

		case FA_Element_PublicData:
		{
			F_STORE(&LOD->public);
		}
		break;

		case FA_Element_Application:
		{
			F_STORE(_element_application);
		}
		break;

		case FA_Element_Window:
		{
			F_STORE(_element_window);
		}
		break;

		case FA_Parent:
		{
			F_STORE(_element_parent);
		}
		break;

		#else

		case FA_Application:
		{
			F_STORE(LOD->render ? LOD->render->Application : NULL);
		}
		break;

		case FA_Window:
		{
			F_STORE(LOD->render ? LOD->render->Window : NULL);
		}
		break;

		#endif

		case FA_Element_Persist:
		{
			F_STORE(LOD->persist);
		}
		break;

		case FA_Element_Class:
		{
			F_STORE(LOD->styleclass);
		}
		break;

		case FA_Element_Style:
		{
			F_STORE(LOD->style);
		}
		break;

		case FA_Element_DecodedStyle:
		{
			if ((LOD->composed_style == NULL) || ((LOD->composed_style != NULL) && (LOD->composed_style->DecodedPropertiesSpace == NULL)))
			{
				IFEELIN F_Log(0, "DecodedStyle is NULL !!");
			}

			F_STORE(LOD->composed_style ? LOD->composed_style->DecodedPropertiesSpace : NULL);
		}
		break;

		case FA_Element_ComposedStyle:
		{
			F_STORE(LOD->composed_style);
		}
		break;
	}

	return F_SUPERDO();
}
//+
///Element_Connect
F_METHODM(bool32, Element_Connect, FS_Connect)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_element_parent = Msg->Parent;

	return TRUE;
}
//+
///Element_Disconnect
F_METHOD(bool32, Element_Disconnect)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_element_parent = NULL;

	IFEELIN F_Do(Obj, FM_Element_GlobalDisconnect);

	return TRUE;
}
//+

