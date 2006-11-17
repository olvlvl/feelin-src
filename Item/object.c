#include "Private.h"

#define DB_DISPOSE
 
/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///RadioItem_New
F_METHOD(FObject, RadioItem_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;
	
	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_LABEL:
		{
			LOD->label = (STRPTR) item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_IMAGE:
		{
			LOD->image = (STRPTR) item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_ALTIMAGE:
		{
			LOD->altimage = (STRPTR) item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_PREPARSE:
		{
			LOD->preparse = (STRPTR) item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_ALTPREPARSE:
		{
			LOD->altpreparse = (STRPTR) item.ti_Data;
		}
		break;
		
		case FA_Child:
		{
			if (item.ti_Data != 0)
			{
				LOD->child = (FObject) item.ti_Data;
			}
			else
			{
				return NULL;
			}
		}
		break;
	}

	return Obj;
}
//+
///RadioItem_Dispose
F_METHOD(uint32, RadioItem_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	if (LOD->child != NULL)
	{
		FObject parent = (FObject) IFEELIN F_Get(LOD->child, FA_Parent);

		if ((parent != NULL) && (parent != Obj))
		{
			#ifdef DB_DISPOSE
			IFEELIN F_Log(0,"remove child %s{%08lx} from parent %s{%08lx}",_object_classname(LOD->child),LOD->child,_object_classname(parent),parent);
			#endif
			
			IFEELIN F_Do(parent, FM_RemMember, LOD->child);
		}
 
		IFEELIN F_DisposeObj(LOD->child);

		LOD->child = NULL;
	}

	return 0;
}
//+
///RadioItem_Get
F_METHOD(uint32, RadioItem_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_LABEL:        F_STORE(LOD->label); break;
		case FV_ATTRIBUTE_IMAGE:        F_STORE(LOD->image); break;
		case FV_ATTRIBUTE_ALTIMAGE:     F_STORE(LOD->altimage); break;
		case FV_ATTRIBUTE_PREPARSE:     F_STORE(LOD->preparse); break;
		case FV_ATTRIBUTE_ALTPREPARSE:  F_STORE(LOD->altpreparse); break;
		case FA_Child:                  F_STORE(LOD->child); break;
	}

	return 0;
}
//+
