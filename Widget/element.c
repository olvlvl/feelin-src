#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Widget_Setup
F_METHODM(bool32,Widget_Setup,FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	bits32 events = 0;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	_area_render = Msg->Render;

/** chainable **********************************************************************************/

	if ((FF_WIDGET_CHAINABLE & LOD->flags) != 0)
	{
		IFEELIN F_Do(_area_win, FM_Window_AddChainable, Obj);
	}

/*** input handler ******************************************************************************

	FM_Widget_ModifyEvents only adds the handler if the object is not disabled and
	can be shown

*/

	if (LOD->mode != 0)
	{
		events |= FF_EVENT_KEY | FF_EVENT_BUTTON;
	}
	else if (_widget_is_draggable)
	{
		events |= FF_EVENT_BUTTON;
	}

	if (LOD->context_help != NULL)
	{
		events |= FF_EVENT_HELP;
	}

	if (events != 0)
	{
		IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, events, 0);
	}

	return TRUE;
}
//+
///Widget_Cleanup
F_METHOD(bool32,Widget_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render != NULL)
	{
		/* remove the object from its window's cycle chain */
 
		if (_widget_is_chainable)
		{
			IFEELIN F_Do(_area_win, FM_Window_RemChainable, Obj);
		}
	
		/* remove its events handler */

		if (LOD->handler != NULL)
		{
			IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, ALL);
		}
	}

	return F_SUPERDO();
}
//+

