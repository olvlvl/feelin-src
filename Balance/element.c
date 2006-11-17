#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Balance_Setup
F_METHOD(bool32,Balance_Setup)
{
	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_BUTTON, 0);

	return TRUE;
}
//+
///Balance_Cleanup
F_METHOD(bool32,Balance_Cleanup)
{
	IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_BUTTON);

	return F_SUPERDO();
}
//+
///Balance_LoadPersistentAttributes
F_METHOD(FPDRHandle *, Balance_LoadPersistentAttributes)
{
	#warning Balance_LoadPersistentAttributes is DISABLED

	return (FPDRHandle *) F_SUPERDO();
}
//+
///Balance_SavePersistentAttributes
F_METHOD(FPDRHandle *, Balance_SavePersistentAttributes)
{
	#warning Balance_SavePersistentAttributes is DISABLED

	return (FPDRHandle *) F_SUPERDO();
}
//+
