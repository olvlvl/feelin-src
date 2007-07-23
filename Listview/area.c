#include "Private.h"

///Listview_SetState
F_METHODM(bool32,Listview_SetState,FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	F_SUPERDO();

	F_OBJDO(LOD->list);

	return TRUE;
}
//+

