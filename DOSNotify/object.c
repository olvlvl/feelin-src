#include "Private.h"

///DN_New
F_METHOD(uint32,DN_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class) != NULL)
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_NAME:
		{
			LOD->Notify.nr_Name = (STRPTR)(item.ti_Data);
		}
		break;
	}
	 
	if (LOD->Notify.nr_Name != NULL)
	{
		if (IFEELIN F_Do(CUD->thread, CUD->id_Send, FV_Thread_AddNotify, &LOD->Notify, Obj) != 0)
		{
			return F_SUPERDO();
		}
		else
		{
			IFEELIN F_Log(FV_LOG_USER, "Unable to create DOS notification '%s'", LOD->Notify.nr_Name);
		}
	}

	 return NULL;
}
//+
///DN_Dispose
F_METHOD(void,DN_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_Do(CUD->thread, CUD->id_Send, FV_Thread_RemNotify, &LOD->Notify);

	F_SUPERDO();
}
//+
