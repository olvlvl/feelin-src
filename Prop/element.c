#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Prop_GlobalConnect
F_METHOD(uint32, Prop_GlobalConnect)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	return F_OBJDO(LOD->knob);
}
//+
///Prop_GlobalDisconnect
F_METHOD(uint32, Prop_GlobalDisconnect)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	return F_OBJDO(LOD->knob);
}
//+
///Prop_Setup
F_METHOD(uint32,Prop_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_BUTTON, 0);

	return F_OBJDO(LOD->knob);
}
//+
///Prop_Cleanup
F_METHOD(uint32,Prop_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_OBJDO(LOD->knob);

	IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,0,FF_EVENT_BUTTON);

	return F_SUPERDO();
}
//+
