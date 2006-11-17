#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Slider_GlobalConnect
F_METHOD(uint32, Slider_GlobalConnect)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	return F_OBJDO(LOD->knob);
}
//+
///Slider_GlobalDisconnect
F_METHOD(uint32, Slider_GlobalDisconnect)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	return F_OBJDO(LOD->knob);
}
//+
///Slider_Setup
F_METHOD(bool32,Slider_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_BUTTON, 0);

	if (FF_Slider_Buffer & LOD->flags)
	{
		if ((LOD->own_rport = IFEELIN F_New(sizeof (struct RastPort))) != NULL)
		{
			IGRAPHICS InitRastPort(LOD->own_rport);

			LOD->own_render = RenderObject,

				FA_Render_Friend,  _area_render,
				FA_Render_RPort,   LOD->own_rport,

				End;
					
			if (LOD->own_render)
			{
				return IFEELIN F_Do(LOD->knob,Method,LOD->own_render);
			}

			#ifdef __AROS__
			DeinitRastPort(LOD->own_rport);
			#endif
				
			IFEELIN F_Dispose(LOD->own_rport);
		}
	}

	return F_OBJDO(LOD->knob);
}
//+
///Slider_Cleanup
F_METHOD(uint32,Slider_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

//   F_DebugOut("Slider.Cleanup\n");

	IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_BUTTON);

	if (LOD->own_render)
	{
		IFEELIN F_Do(LOD->knob,Method,LOD->own_render);
		IFEELIN F_DisposeObj(LOD->own_render); LOD->own_render = NULL;
		
		#ifdef __AROS__
		DeinitRastPort(LOD->own_rport);
		#endif

		if (LOD->own_rport != NULL)
		{
			IFEELIN F_Dispose(LOD->own_rport);

			LOD->own_rport = NULL;
		}

		if (LOD->own_bitmap)
		{
			IGRAPHICS FreeBitMap(LOD->own_bitmap); LOD->own_bitmap = NULL;
		}
	}
	else
	{
		F_OBJDO(LOD->knob);
	}

	return F_SUPERDO();
}
//+
