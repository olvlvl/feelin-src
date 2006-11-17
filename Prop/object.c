#include "Private.h"

//#define DB_SET

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Prop_New
F_METHOD(uint32,Prop_New)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	uint32 orientation = FV_Area_Orientation_Horizontal;

	FObject knob = NULL;

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif

	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;

	LOD->entries = 1;
	LOD->visible = 1;
	LOD->first   = 0;
	LOD->step    = 3;

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
		knob = WidgetObject,

			FA_Element_Class, "prop-knob",
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
		return IFEELIN F_SuperDo(Class,Obj,Method,

			FA_Area_Fillable, FALSE,

			TAG_MORE,Msg);
	}
	return 0;
}
//+
///Prop_Dispose
F_METHOD(uint32,Prop_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->knob != NULL)
	{
		FObject knob = LOD->knob;

		IFEELIN F_Do(Obj, FM_RemMember, knob);

		IFEELIN F_DisposeObj(knob);
	}

	return F_SUPERDO();
}
//+
///Prop_Get
F_METHOD(uint32,Prop_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	bool32 up = FALSE;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ENTRIES:
		{
			F_STORE(LOD->entries);
		}
		break;

		case FV_ATTRIBUTE_VISIBLE:
		{
			F_STORE(LOD->visible);
		}
		break;

		case FV_ATTRIBUTE_FIRST:
		{
			F_STORE(LOD->first);
		}
		break;

		case FV_ATTRIBUTE_USELESS:
		{
			F_STORE((LOD->entries == 0 || LOD->visible == LOD->entries) ? TRUE : FALSE);
		}
		break;

		default: up = TRUE;
	}

	if (up)
	{
		return F_SUPERDO();
	}

	return 0;
}
//+
///Prop_Set

#define FF_UPDATE_ENTRIES                       (1 << 0)
#define FF_UPDATE_VISIBLE                       (1 << 1)
#define FF_UPDATE_FIRST                         (1 << 2)

F_METHOD(uint32,Prop_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;
	
	bits32 update = 0;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ENTRIES:
		{
			update |= FF_UPDATE_ENTRIES;
			
			LOD->entries = item.ti_Data;
		}
		break;
		
		case FV_ATTRIBUTE_VISIBLE:
		{
			update |= FF_UPDATE_VISIBLE;
		   
			LOD->visible = MAX(1, (int32) item.ti_Data);
		}
		break;
		
		case FV_ATTRIBUTE_FIRST:
		{
			update |= FF_UPDATE_FIRST;
		   
			LOD->first = MAX(0, (int32) item.ti_Data);
		}
		break;
	}
		  
	if (update)
	{
		#ifdef DB_SET
		IFEELIN F_Log(0,"entries (%ld) visible (%ld) first (%ld)",LOD->entries,LOD->visible,LOD->first);
		#endif
   
		if ((FF_UPDATE_ENTRIES | FF_UPDATE_VISIBLE) & update)
		{
			if ((int32)(LOD->entries) < (int32)(LOD->visible))
			{
				LOD->visible = LOD->entries;
			}
		}
		
		if ((int32)(LOD->first)  > (int32)(LOD->entries - LOD->visible))
		{
			LOD->first = LOD->entries - LOD->visible;
		}
		   
		if ((FF_UPDATE_ENTRIES | FF_UPDATE_VISIBLE) & update)
		{
			if (LOD->entries == 0 || LOD->visible == LOD->entries)
			{
				#ifdef DB_SET
				IFEELIN F_Log(0,"prop is useless");
				#endif

				IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(USELESS), TRUE);
			}
			else
			{
				#ifdef DB_SET
				IFEELIN F_Log(0,"prop is not useless");
				#endif

				IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(USELESS), FALSE);
			}
		}

		IFEELIN F_Draw(Obj, FF_Draw_Damaged | ((FF_UPDATE_VISIBLE & update) ? FF_Draw_Object : FF_Draw_Update));
	}

	return F_SUPERDO();
}
//+
///Prop_AddMember
F_METHODM(bool32,Prop_AddMember,FS_AddMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->knob == NULL)
	{
		FAreaPublic *apd = F_GET_AREA_PUBLIC(Msg->Orphan);
		FWidgetPublic *wpd = F_GET_WIDGET_PUBLIC(Msg->Orphan);

		if ((apd != NULL) && (wpd != NULL))
		{
			if (IFEELIN F_Do(Msg->Orphan, FM_Connect, Obj) != FALSE)
			{
				LOD->knob = Msg->Orphan;
				LOD->knob_area_public = apd;

				#ifdef F_NEW_GLOBALCONNECT
				if (_element_application)
				{
					IFEELIN F_Do(Msg->Orphan, FM_Element_GlobalConnect, _element_application, _element_window);
				}
				#endif

				return TRUE;
			}
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV, "Knob %s{%08lx} is not a subclass of Widget", _object_classname(Msg->Orphan), Msg->Orphan);
		}
	}

	return FALSE;
}
//+
///Prop_RemMember
F_METHODM(bool32,Prop_RemMember,FS_RemMember)
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
	F_PREFERENCES_ADD("prop",   "frame",                FV_TYPE_STRING, "Spec", "<frame id='18' />" ),
	F_PREFERENCES_ADD("prop",   "back-horizontal",      FV_TYPE_STRING, "Spec", "halfshadow" ),
	F_PREFERENCES_ADD("prop",   "back",                 FV_TYPE_STRING, "Spec", "halfshadow" ),
	F_PREFERENCES_ADD("prop",   "color-scheme",         FV_TYPE_STRING, "Spec", NULL ),

	F_PREFERENCES_ADD("prop-knob",  "frame",            FV_TYPE_STRING, "Spec", "<frame id='23' /><frame id='24' />" ),
	F_PREFERENCES_ADD("prop-knob",  "back",             FV_TYPE_STRING, "Spec", "fill" ),
	F_PREFERENCES_ADD("prop-knob",  "back-horizontal",  FV_TYPE_STRING, "Spec", "fill" ),
	F_PREFERENCES_ADD("prop-knob",  "color-scheme",     FV_TYPE_STRING, "Spec", NULL ),
	
	F_ARRAY_END
};

F_METHOD(uint32,Prefs_New)
{
	STATIC F_ENTITIES_ARRAY =
	{
		F_ENTITIES_ADD("locale.container",   "Container"),
		F_ENTITIES_ADD("locale.frame",       "Frame"),
		F_ENTITIES_ADD("locale.back",        "Back"),
		F_ENTITIES_ADD("locale.scheme",      "Scheme"),
		F_ENTITIES_ADD("locale.knob",        "Knob"),
		F_ENTITIES_ADD("locale.examples",    "Examples"),

		F_ARRAY_END
	};

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Prop",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/prop.xml",
		"Entities", F_ENTITIES_PTR,

	TAG_MORE,Msg);
}
//+
