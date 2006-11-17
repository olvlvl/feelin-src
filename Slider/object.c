#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Slider_New
F_METHOD(FObject,Slider_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	uint32 orientation = FV_Area_Orientation_Horizontal;

	FObject knob = NULL;

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif

	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;
	
	LOD->flags = FF_Slider_Buffer;
	
	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Area_Orientation:
		{
			orientation = item.ti_Data;
		}
		break;

		case FA_Child:
		{
			if (knob != NULL)
			{
				IFEELIN F_DisposeObj(knob);
			}

			knob = (APTR) item.ti_Data;

			if (knob == NULL)
			{
				return 0;
			}
		}
		break;
	}

	if (knob == NULL)
	{
		knob = TextObject,
					 
		  FA_Element_Class, "slider-knob",
		  FA_Area_Orientation, orientation,
		  FA_Widget_Chainable, FALSE,

	   End;
	}

	if (knob != NULL)
	{
		if (IFEELIN F_Do(Obj, FM_AddMember, knob, FV_AddMember_Tail) == FALSE)
		{
			IFEELIN F_DisposeObj(knob);

			return 0;
		}
	}

	if (LOD->knob != NULL)
	{
		if (IFEELIN F_SuperDo(Class,Obj,Method,

			FA_Area_Fillable, FALSE,
		
		TAG_MORE,Msg))
		{
			IFEELIN F_Do
			(
				LOD->knob, FM_Set,

				FA_Text_HCenter, TRUE,
				FA_Text_VCenter, TRUE,

				FA_Text_Static, TRUE,
				FA_Text_Contents, IFEELIN F_Get(Obj, F_RESOLVED_ID(BUFFER)),

				TAG_DONE);

			return Obj;
		}
	}

	return NULL;
}
//+
///Slider_Dispose
F_METHOD(uint32,Slider_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->own_bitmap)
	{
		IGRAPHICS FreeBitMap(LOD->own_bitmap);

		LOD->own_bitmap = NULL;
	}

	if (LOD->knob != NULL)
	{
		FObject knob = LOD->knob;

		IFEELIN F_Do(Obj, FM_RemMember, knob);

		IFEELIN F_DisposeObj(knob);
	}

	return F_SUPERDO();
}
//+
///Slider_AddMember
F_METHODM(bool32,Slider_AddMember,FS_AddMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FObject orphan = Msg->Orphan;
	FAreaPublic *ap;

	if (orphan == NULL)
	{
		return FALSE;
	}

	if (LOD->knob != NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Knob already defined:%s{%08lx}. Orphan %s{%08lx} refused", _object_classname(LOD->knob), LOD->knob, _object_classname(orphan), orphan);

		return FALSE;
	}

	ap = F_GET_AREA_PUBLIC(orphan);

	if (ap == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Orphan %s{%08lx} should be a subclass of Area", _object_classname(orphan), orphan);

		return FALSE;
	}

	if (IFEELIN F_Do(orphan, FM_Connect, Obj) == FALSE)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Orphan %s{%08lx} refused connection", _object_classname(orphan), orphan);

		return FALSE;
	}

	LOD->knob = orphan;
	LOD->knob_area_public = ap;

	#ifdef F_NEW_GLOBALCONNECT
	if (_element_application)
	{
		IFEELIN F_Do(Msg->Orphan, FM_Element_GlobalConnect, _element_application, _element_window);
	}
	#endif

	return TRUE;
}
//+
///Slider_RemMember
F_METHODM(bool32,Slider_RemMember,FS_RemMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if ((LOD->knob != NULL) && (LOD->knob == Msg->Member))
	{
		IFEELIN F_Do(Msg->Member, FM_Disconnect);

		LOD->knob = NULL;
		LOD->knob_area_public = NULL;

		return TRUE;
	}

	return FALSE;
}
//+

///Preferences
STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("slider",         "frame",            FV_TYPE_STRING, "Spec",     "<frame id='18' />" ),
	F_PREFERENCES_ADD("slider",         "back",             FV_TYPE_STRING, "Spec",     "halfshadow" ),
	F_PREFERENCES_ADD("slider",         "back-horizontal",  FV_TYPE_STRING, "Spec",     "halfshadow" ),
	F_PREFERENCES_ADD("slider",         "color-scheme",     FV_TYPE_STRING, "Spec",     NULL ),

	F_PREFERENCES_ADD("slider-knob",    "font",             FV_TYPE_STRING, "Contents", NULL ),
	F_PREFERENCES_ADD("slider-knob",    "preparse",         FV_TYPE_STRING, "Contents", NULL ),
	F_PREFERENCES_ADD("slider-knob",    "frame",            FV_TYPE_STRING, "Spec",     "<frame id='23' padding-width='4' /><frame id='24' />" ),
	F_PREFERENCES_ADD("slider-knob",    "back",             FV_TYPE_STRING, "Spec",     "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow" ),
	F_PREFERENCES_ADD("slider-knob",    "back-horizontal",  FV_TYPE_STRING, "Spec",     "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow" ),
	F_PREFERENCES_ADD("slider-knob",    "color-scheme",     FV_TYPE_STRING, "Spec",     NULL ),

	F_ARRAY_END
};

F_METHOD(uint32,Prefs_New)
{
	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Slider",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/slider.xml",

	TAG_MORE,Msg);
}
//+
