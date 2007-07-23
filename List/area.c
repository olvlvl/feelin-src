#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///List_AskMinMax
F_METHOD(uint32,List_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_area_minh += (((LOD->MaximumH) ? LOD->MaximumH : _area_font->tf_YSize) + LOD->spacing) * (_widget_is_setminh ? LOD->LineCount : MIN(5,LOD->LineCount)) - LOD->spacing + LOD->TitleBarH;
	_area_minh += (((LOD->MaximumH) ? LOD->MaximumH : _area_font->tf_YSize) + LOD->spacing) * MIN(5,LOD->LineCount) - LOD->spacing + LOD->TitleBarH;
 
	if (_widget_is_setminw)
	{
		uint32 minw = 0;
		uint32 j;
		
		if (LOD->Columns)
		{
			for (j = 0 ; j < LOD->ColumnCount ; j++)
			{
				minw += LOD->Columns[j].Width;
			}
		}

		if (_widget_is_setminw)
		{
			_area_minw += minw;
		}
	}

	return F_SUPERDO();
}
//+
///List_Layout
F_METHOD(bool32,List_Layout)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	LOD->LastFirst = NULL;

	list_update(Class,Obj,TRUE);

	if (LOD->First && LOD->Last && LOD->Active)
	{
		if (LOD->Active->Position < LOD->First->Position ||
			 LOD->Active->Position > LOD->Last->Position)
		{
			LOD->First = LOD->Active;

			list_update(Class,Obj,TRUE);
		}
	}

	return TRUE;
}
//+
///List_SetState
F_METHODM(uint32,List_SetState,FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	switch (Msg->State)
	{
		case FV_Area_State_Neutral:
		{
			LOD->Flags &= ~FF_LIST_ACTIVE;
		}
		break;

		case FV_Area_State_Focus:
		{
			LOD->Flags |= FF_LIST_ACTIVE;
		}
		break;
	}

	return F_SUPERDO();
}
//+
