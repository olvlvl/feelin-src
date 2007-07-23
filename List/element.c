#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///List_Setup
F_METHOD(bool32, List_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	STRPTR spec;
	FLine *line;

	uint32 value;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	LOD->MaximumH = 0;

	LOD->Scheme = _area_palette;

	value = DEF_LIST_STEPS;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(STEPS), NULL, &value);

	LOD->steps = value;

	value = DEF_LIST_SPACING;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(SPACING), NULL, &value);

	LOD->spacing = value;

	spec = NULL;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(CURSOR_ACTIVE), &spec, NULL);

	if (spec != NULL)
	{
		LOD->CursorActive = ImageDisplayObject,
				
			FA_ImageDisplay_Spec, spec,
			
		TAG_DONE);
			
		if (LOD->CursorActive != NULL)
		{
			if (IFEELIN F_Do(LOD->CursorActive, FM_ImageDisplay_Setup, _area_render) == FALSE)
			{
				IFEELIN F_DisposeObj(LOD->CursorActive); LOD->CursorActive = NULL;
			}
		}
	}

	IFEELIN F_Set(LOD->TDObj,FA_TextDisplay_Font,(uint32) _area_font);
	IFEELIN F_Do(LOD->TDObj,FM_TextDisplay_Setup,_area_render);
		
	titlebar_setup(Class,Obj);

	for (line = (FLine *)(LOD->LineList.Head) ; line ; line = line->Next)
	{
		line_compute_dimensions(Class,Obj,line);
	}

	IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,FF_EVENT_BUTTON | FF_EVENT_KEY,0);

	return TRUE;
}
//+
///List_Cleanup
F_METHOD(void,List_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FLine *line;

	for (line = (FLine *)(LOD->LineList.Head) ; line ; line = line->Next)
	{
		line->Flags &= ~FF_LINE_COMPUTED;
	}

	titlebar_cleanup(Class,Obj);
	
	if (LOD->pop_window)
	{
		list_popstring_delete(Class,Obj);
	}

	IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,0,FF_EVENT_KEY | FF_EVENT_BUTTON);
	IFEELIN F_Do(LOD->TDObj,FM_TextDisplay_Cleanup);
	IFEELIN F_Do(LOD->CursorActive,FM_ImageDisplay_Cleanup,_area_render);
	IFEELIN F_DisposeObj(LOD->CursorActive);

	F_SUPERDO();
}
//+
