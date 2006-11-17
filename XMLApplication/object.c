#include "Private.h"

//#define DB_ADDSTYLESHEET

///xmlapplication_addstylesheet
STATIC void xmlapplication_addstylesheet(FClass *Class, FObject Obj, FXMLPI *pi)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FXMLAttribute *href = NULL;
	FXMLAttribute *type = NULL;
	FXMLAttribute *attr;

	for (attr = (FXMLAttribute *) pi->AttributesList.Head ; attr ; attr = attr->Next)
	{
		if (attr->Atom == F_ATOM(HREF))
		{
			href = attr;
		}
		else if (attr->Atom == F_ATOM(TYPE))
		{
			type = attr;
		}
	}

	if (href == NULL)
	{
		IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG),

			pi->Line, NULL, "'href' is REQUIRED for processing instruction 'xml-stylesheet'");
	}

	if (type == NULL)
	{
		IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG),

			pi->Line, NULL, "'type' is REQUIRED for processing instruction 'xml-stylesheet'");
	}
	else if (href != NULL)
	{
		if (IFEELIN F_StrCmp(type->Value, "text/css", ALL) != 0)
		{
			IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG),

				type->Line, NULL, "type (%s) is not yet support xml-stylesheet", type->Value);

		}
		else
		{
			STRPTR buf = IFEELIN F_StrNew(NULL, "%-1024.1024s", IFEELIN F_Get(Obj, (uint32) "Source"));

			if (buf)
			{
				STRPTR file = IDOS_ FilePart(buf);

				*file = 0;

				IDOS_ AddPart(buf, href->Value, 1024);

				#ifdef DB_ADDSTYLESHEET
				IFEELIN F_Log(0, "href (%s) type (%s) url(%s) ", href->Value, type->Value, buf);
				#endif

				IFEELIN F_Do(LOD->Application, FM_Application_AddStyleSheet, buf, FV_Document_SourceType_File);

				IFEELIN F_Dispose(buf);
			}
		}
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///XMLApplication_New
F_METHOD(FObject,XMLApplication_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == 0)
	{
		return NULL;
	}
	
	if (LOD->Application == NULL)
	{
		return NULL;
	}
	
	return Obj;
}
//+
///XMLApplication_Dispose
F_METHOD(uint32,XMLApplication_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_DisposeObj(LOD->Application);

	LOD->Application = NULL;

	return F_SUPERDO();
}
//+
///XMLApplication_Build
F_METHOD(LONG,XMLApplication_Build)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	APTR pool=NULL;

	FXMLMarkup *markup=NULL;
	FXMLMarkup *markup_application=NULL;
	FXMLMarkup *markup_notify=NULL;

	FXMLPI *pi=NULL;

	if (LOD->Application != NULL)
	{
		IFEELIN F_Log(0,"Application already built (0x%08lx)",LOD->Application); return FALSE;
	}

	IFEELIN F_Do(Obj,FM_Get,

		"FA_Document_Pool",         	&pool,
		"FA_XMLDocument_Markups",   	&markup,
		"FA_XMLDocument_Instructions",  &pi,
		
		TAG_DONE);

/*** searching for markup 'feelin:application' *************************************************/

	for ( ; markup ; markup = markup->Next)
	{
		if (markup->Atom == F_ATOM(ROOT))
		{
			break;
		}
	}

	if (markup == NULL)
	{
		IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG), 0, NULL, "Unable to locate '%s' markup", F_XMLAPPLICATION_ROOT);

		return FALSE;
	}
		
	for (markup = (FXMLMarkup *)(markup->ChildrenList.Head) ; markup ; markup = markup->Next)
	{
		if (markup->Atom == F_ATOM(APPLICATION))
		{
			markup_application = markup;
		}
		else if (markup->Atom == F_ATOM(NOTIFY))
		{
			markup_notify = markup;
		}
	}

	if (markup_application == NULL)
	{
		return FALSE;
	}

	LOD->Application = (FObject) IFEELIN F_Do(Obj, (uint32) "FM_XMLObject_Create", markup_application, pool);
		
	if (LOD->Application == NULL)
	{
		return FALSE;
	}
	
	if (markup_notify != NULL)
	{
		if (IFEELIN F_Do(Obj, F_METHOD_ID(CREATENOTIFY), markup_notify,pool) == FALSE)
		{
			return FALSE;
		}
	}

/*** xml-stylesheet ****************************************************************************/

	if (LOD->Application == NULL)
	{
		return FALSE;
	}
		
	for ( ; pi ; pi = pi->Next)
	{
		if (pi->Atom == F_ATOM(STYLESHEET))
		{
			xmlapplication_addstylesheet(Class, Obj, pi);
		}
	}

	return TRUE;
}
//+
///XMLApplication_Run
F_METHOD(uint32,XMLApplication_Run)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	return IFEELIN F_Do(LOD->Application,FM_Application_Run);
}
//+

