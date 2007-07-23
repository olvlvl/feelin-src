#include "Private.h"

/*************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Image_Setup
F_METHODM(bool32,Image_Setup,FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	STRPTR value;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	// render

//	  if (LOD->render == NULL)
	{
		value = LOD->render_spec;

		if (value == NULL)
		{
			IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(IMAGE), &value, NULL);
		}

		if (value != NULL)
		{
			LOD->render = ImageDisplayObject,

				FA_ImageDisplay_Spec, value,
				FA_ImageDisplay_Owner, Obj,
				FA_ImageDisplay_Origin, &_area_content,

				End;
	 	}

//		  IFEELIN F_Log(0, "render (%s) object (0x%08lx)", value, LOD->render);
	}
/*
	if (LOD->render == NULL)
	{
		return FALSE;
	}
*/
	IFEELIN F_Do(LOD->render, FM_ImageDisplay_Setup, Msg->Render);

	// select

	value = LOD->select_spec;

	if (value == NULL)
	{
		IFEELIN F_Do(Obj, FM_Element_GetProperty, F_ATOM(SELECTED), F_PROPERTY_ATOM(IMAGE), &value, NULL);
	}

	if (value != NULL)
	{
		LOD->select = ImageDisplayObject,

			FA_ImageDisplay_Spec, value,
			FA_ImageDisplay_Origin, &_area_content,

			End;
/*
		if (LOD->select == NULL)
		{
			return FALSE;
		}
*/
		IFEELIN F_Do(LOD->select, FM_ImageDisplay_Setup, Msg->Render);
	}

	return TRUE;
}
//+
///Image_Cleanup
F_METHOD(uint32,Image_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render != NULL)
	{
		if (LOD->render != NULL)
		{
			IFEELIN F_Do(LOD->render, FM_ImageDisplay_Cleanup, _area_render);
			IFEELIN F_DisposeObj(LOD->render);

			LOD->render = NULL;
		}

		if (LOD->select != NULL)
		{
			IFEELIN F_Do(LOD->select, FM_ImageDisplay_Cleanup, _area_render);
			IFEELIN F_DisposeObj(LOD->select);

			LOD->select = NULL;
		}
	}

	return F_SUPERDO();
}
//+
