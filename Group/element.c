#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

#ifdef F_NEW_GLOBALCONNECT
///Group_GlobalConnect
F_METHODM(bool32, Group_GlobalConnect, FS_Element_GlobalConnect)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FWidgetNode *node;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	for (_each)
	{
		if (F_OBJDO(node->Widget) == FALSE)
		{
			IFEELIN F_Log(FV_LOG_USER, "global connection failed with %s{%lx}", _object_classname(node->Widget), node->Widget);

			return FALSE;
		}
	}

	return TRUE;
}
//+
///Group_GlobalDisconnect
F_METHOD(bool32, Group_GlobalDisconnect)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FWidgetNode *node;

	for (_each)
	{
	    F_OBJDO(node->Widget);
	}

	return F_SUPERDO();
}
//+
#endif
///Group_Setup
F_METHOD(bool32, Group_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *node;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	if ((FF_GROUP_USER_HSPACING & LOD->flags) == 0)
	{
		uint32 data = DEF_GROUP_HSPACING;

		IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(SPACING_HORIZONTAL), NULL, &data);

		LOD->hspacing = data;
	}

	if ((FF_GROUP_USER_VSPACING & LOD->flags) == 0)
	{
		uint32 data = DEF_GROUP_VSPACING;

		IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(SPACING_VERTICAL), NULL, &data);

		LOD->vspacing = data;
	}

	for (_each)
	{
		if (F_OBJDO(node->Widget) == FALSE)
		{
			IFEELIN F_Log(FV_LOG_USER, "setup of widget %s{%lx} failed !", _object_classname(node->Widget), node->Widget);

			return FALSE;
		}
	}

	return TRUE;
}
//+
///Group_Cleanup
F_METHOD(void,Group_Cleanup)
{
	Group_DoA(Class,Obj,Method,Msg);

	F_SUPERDO();
}
//+
