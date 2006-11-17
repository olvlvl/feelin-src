#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///String_Setup
F_METHOD(bool32,String_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	STRPTR data;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_BUTTON, 0);

	data = DEF_STRING_TEXT_ACTIVE;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(TEXT_ACTIVE), &data, NULL);

	LOD->APen = (FColor *) IFEELIN F_Do(_area_display, FM_Display_CreateColor, data, _area_palette);

	data = DEF_STRING_TEXT_INACTIVE;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(TEXT_INACTIVE), &data, NULL);

	LOD->IPen = (FColor *) IFEELIN F_Do(_area_display, FM_Display_CreateColor, data, _area_palette);

	data = DEF_STRING_TEXT_BLOCK;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(TEXT_BLOCK), &data, NULL);

	LOD->BPen = (FColor *) IFEELIN F_Do(_area_display, FM_Display_CreateColor, data, _area_palette);

	data = DEF_STRING_CURSOR;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(CURSOR), &data, NULL);

	if (data)
	{
		LOD->Cursor = ImageDisplayObject,

			FA_ImageDisplay_Spec, data,

			End;

		if (LOD->Cursor)
		{
			IFEELIN F_DoA(LOD->Cursor, FM_ImageDisplay_Setup, Msg);
		}
	}

	data = (STRPTR) DEF_STRING_BLINK_SPEED;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(BLINK_SPEED), NULL, &data);

	LOD->BlinkSpeed = (uint32) data;

	if (LOD->BlinkSpeed)
	{
		data = DEF_STRING_BLINK_IMAGE;

		IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(BLINK_IMAGE), NULL, &data);

		if (data)
		{
			LOD->Blink = ImageDisplayObject, FA_ImageDisplay_Spec,data, End;
 
			if (LOD->Blink)
			{
				IFEELIN F_DoA(LOD->Blink,FM_ImageDisplay_Setup,Msg);
			}
		}
	}

	return TRUE;
}
//+
///String_Cleanup
F_METHOD(uint32,String_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render)
	{
		if (LOD->Blink)  IFEELIN F_Do(LOD->Blink, FM_ImageDisplay_Cleanup,_area_render);
		if (LOD->Cursor) IFEELIN F_Do(LOD->Cursor,FM_ImageDisplay_Cleanup,_area_render);

		if (LOD->BPen) { IFEELIN F_Do(_area_display,FM_Display_RemColor,LOD->BPen); LOD->BPen = NULL; }
		if (LOD->IPen) { IFEELIN F_Do(_area_display,FM_Display_RemColor,LOD->IPen); LOD->IPen = NULL; }
		if (LOD->APen) { IFEELIN F_Do(_area_display,FM_Display_RemColor,LOD->APen); LOD->APen = NULL; }
	}

	IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,0,FF_EVENT_BUTTON);

	IFEELIN F_DisposeObj(LOD->Blink);    LOD->Blink  = NULL;
	IFEELIN F_DisposeObj(LOD->Cursor);   LOD->Cursor = NULL;

	return F_SUPERDO();
}
//+
