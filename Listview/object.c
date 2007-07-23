#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Listview_New
F_METHOD(FObject, Listview_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,*tag,item;

	F_SAVE_AREA_PUBLIC;

	while ((tag = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
	switch (item.ti_Tag)
	{
		case FA_Child:
		{
			LOD->list = (FObject) item.ti_Data;

			tag->ti_Tag = TAG_IGNORE;
			tag->ti_Data = NULL;
		}
		break;
	}

	if (LOD->list == NULL)
	{
		return NULL;
	}

	if (IFEELIN F_SuperDo(Class,Obj,Method,

//		  FA_Area_NoFill,         TRUE,
		FA_Widget_Chainable,    TRUE,

		FA_Group_HSpacing, 0,
			
		Child, LOD->list,

		Child, LOD->vert = ScrollbarObject,
			
			FA_Element_Style,		"width: auto",
			FA_Area_Orientation,	FV_Area_Orientation_Vertical,
			FA_Widget_Chainable,    FALSE,
		   "Entries",               IFEELIN F_Get(LOD->list, (uint32) "FA_List_Entries"),
			
			End,

		TAG_MORE, Msg) == 0)
	{
		return NULL;
	}

	// update list's first according to prop's first

	IFEELIN F_Do
	(
		LOD->vert, FM_Notify,

		F_AUTO_ID(FIRST), FV_Notify_Always,

		LOD->list, FM_Set, 4,

		FA_NoNotify,TRUE, "First",FV_Notify_Value
	);


	// update prop's entries according to list's entries

	IFEELIN F_Do
	(
		LOD->list, FM_Notify,

		"FA_List_Entries", FV_Notify_Always,

		LOD->vert, FM_Set, 2,

		F_AUTO_ID(ENTRIES), FV_Notify_Value
	);

	// update prop's first according to list's first

	IFEELIN F_Do
	(
		LOD->list, FM_Notify,

		"FA_List_First", FV_Notify_Always,

		LOD->vert, FM_Set, 4,
		FA_NoNotify, TRUE, F_AUTO_ID(FIRST), FV_Notify_Value
	);

	// update prop's visible according to list's visible

	IFEELIN F_Do
	(
		LOD->list, FM_Notify,

		"FA_List_Visible", FV_Notify_Always,

		LOD->vert, FM_Set, 2,

		F_AUTO_ID(VISIBLE), FV_Notify_Value
	);

	return Obj;
}
//+
