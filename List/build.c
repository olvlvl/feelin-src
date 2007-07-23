#include "Private.h"

/* Hooks are called with a pointer to the Object in A2 and a  pointer  to  a
FS_List_Xxx message in A1. */

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///List_Construct
F_METHODM(APTR,List_Construct,FS_List_Construct)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef DB_CONSTRUCT
	IFEELIN F_Log(0,"entry 0x%08lx - hook (0x%08lx)",Msg->Entry,LOD->Hook_Construct);
	#endif

	if ((uint32)(LOD->Hook_Construct) == FV_List_Hook_String)
	{
		uint32 len = (Msg->Entry) ? IFEELIN F_StrLen((STRPTR)(Msg->Entry)) : 0;
		uint32 *mem = IFEELIN F_NewP(Msg->Pool,len + 1);

		if (!mem) return NULL;

		if (Msg->Entry)
		{
		   IEXEC CopyMem(Msg->Entry,mem,len);
		}
		else
		{
		   *(STRPTR)(mem) = 0;
		}

		return mem;
	}
	else if (LOD->Hook_Construct)
	{
		return (APTR) IUTILITY CallHookPkt(LOD->Hook_Construct,Obj,Msg);
	}
	else
	{
		return Msg->Entry;
	}
}
//+
///List_Destruct
F_METHODM(void,List_Destruct,FS_List_Destruct)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef DB_DESTRUCT
	IFEELIN F_Log(0,"entry 0x%08lx",Msg->Entry);
	#endif

	if ((uint32)(LOD->Hook_Destruct) == FV_List_Hook_String)
	{
		IFEELIN F_Dispose(Msg->Entry);
	}
	else if (LOD->Hook_Destruct)
	{
		IUTILITY CallHookPkt(LOD->Hook_Destruct,Obj,Msg);
	}
}
//+
///List_Compare
F_METHODM(LONG,List_Compare,FS_List_Compare)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef DB_COMPARE
	IFEELIN F_Log(0,"entry 0x%08lx",Msg->Entry);
	#endif

	if ((uint32)(LOD->Hook_Compare) == FV_List_Hook_String)
	{
		return IUTILITY Stricmp(Msg->Entry,Msg->Other);
	}
	else if (LOD->Hook_Compare)
	{
		return (LONG) IUTILITY CallHookPkt(LOD->Hook_Compare,Obj,Msg);
	}
	else
	{
		return 0;
	}
}
//+
///List_Display
F_METHODM(uint32,List_Display,FS_List_Display)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->Hook_Display && ((uint32)(LOD->Hook_Display) != FV_List_Hook_String))
	{
		FListDisplay *ld = (FListDisplay *) IUTILITY CallHookPkt(LOD->Hook_Display,Obj,Msg);
		
		if (ld)
		{
			uint32 i;
			
			for (i = 0 ; i < LOD->ColumnCount ; i++, ld++)
			{
				Msg->Strings[i] = ld->String;
				Msg->PreParses[i] = ld->PreParse;
			}
		}
	}
	else
	{
		if (Msg->Entry)
		{
			*Msg->Strings = Msg->Entry;
		}
		else
		{
			*Msg->Strings = NULL;
		}

		return TRUE;
	}

	return 0;
}
//+
