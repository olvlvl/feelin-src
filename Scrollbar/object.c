#include "Private.h"

//#define DB_SET

struct FS_REFLECT                               { FClass *Class; uint32 Value; };
struct FS_HIDE                                  { FClass *Class; uint32 Value; };

///code_reflect
F_HOOKM(void,code_reflect,FS_REFLECT)
{
	struct FeelinClass *Class = Msg->Class;

	IFEELIN F_SuperDo(Class, Obj, FM_Set, F_ATTRIBUTE_ID(FIRST), Msg->Value, TAG_DONE);
}
//+
///code_hide
F_HOOKM(void,code_hide,FS_HIDE)
{
	if (Msg->Value)
	{
		IFEELIN F_Set(Obj, FA_Widget_Hidden, FF_Hidden_Check | TRUE);
	}
	else
	{
		IFEELIN F_Set(Obj, FA_Widget_Hidden, FF_Hidden_Check | FALSE);
	}
}
//+

/* if the property is not defined, it implies 'symetric' layout */

/*** Methods ***********************************************************************************/

///SB_New
F_METHOD(uint32,SB_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	uint32 orientation = FV_Area_Orientation_Horizontal;

	uint32 prop_entries = 100;
	uint32 prop_first = 0;
	uint32 prop_visible = 100;

	F_SAVE_AREA_PUBLIC;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Area_Orientation:
		{
			orientation = item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_ENTRIES: prop_entries = item.ti_Data; break;
		case FV_ATTRIBUTE_FIRST:   prop_first = item.ti_Data; break;
		case FV_ATTRIBUTE_VISIBLE: prop_visible = item.ti_Data; break;
	}

	LOD->button1 = ImageObject,

		FA_Element_Class,       (orientation == FV_Area_Orientation_Horizontal) ? "scrollbar-arrow scrollbar-arrow-left" : "scrollbar-arrow scrollbar-arrow-top",
		FA_Widget_SetMax,         (orientation == FV_Area_Orientation_Horizontal) ? FV_Widget_SetWidth : FV_Widget_SetHeight,
		FA_Widget_Chainable,    FALSE,
		FA_Widget_Mode,         FV_Widget_Mode_Release,
	End;

	LOD->button2 = ImageObject,

		FA_Element_Class,       (orientation == FV_Area_Orientation_Horizontal) ? "scrollbar-arrow scrollbar-arrow-right" : "scrollbar-arrow scrollbar-arrow-bottom",
		FA_Widget_SetMax,         (orientation == FV_Area_Orientation_Horizontal) ? FV_Widget_SetWidth : FV_Widget_SetHeight,
		FA_Widget_Chainable,    FALSE,
		FA_Widget_Mode,         FV_Widget_Mode_Release,
	End;

	LOD->prop = PropObject,

		FA_Element_Class,       "scrollbar-prop",
		FA_Area_Orientation,	orientation,
		FA_Widget_Chainable,    FALSE,

		"Entries",              prop_entries,
		"First",                prop_first,
		"Visible",              prop_visible,
		
		Child, WidgetObject,
			FA_Element_Class,       "scrollbar-knob",
			FA_Area_Orientation,	orientation,
			FA_Widget_Chainable,    FALSE,
		End,
	End;

	if (!LOD->button1 || !LOD->button2 || !LOD->prop)
	{
		IFEELIN F_DisposeObj(LOD->button1); LOD->button1 = NULL;
		IFEELIN F_DisposeObj(LOD->button2); LOD->button2 = NULL;
		IFEELIN F_DisposeObj(LOD->prop); LOD->prop = NULL;

		return NULL;
	}

	IFEELIN F_Do(LOD->prop,FM_Notify,"Useless",FV_Notify_Always,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_hide),Class,FV_Notify_Value);
	IFEELIN F_Do(LOD->prop,FM_Notify,"First",FV_Notify_Always,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_reflect),Class,FV_Notify_Value);
	IFEELIN F_Do(LOD->button1,FM_Notify,FA_Widget_Pressed,TRUE,LOD->prop,"FM_Prop_Decrease",1,1);
	IFEELIN F_Do(LOD->button2,FM_Notify,FA_Widget_Pressed,TRUE,LOD->prop,"FM_Prop_Increase",1,1);
	IFEELIN F_Do(LOD->button1,FM_Notify,FA_Widget_PressedTick,FV_Notify_Always,LOD->prop,"FM_Prop_Decrease",1,1);
	IFEELIN F_Do(LOD->button2,FM_Notify,FA_Widget_PressedTick,FV_Notify_Always,LOD->prop,"FM_Prop_Increase",1,1);

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_VSpacing, 0,
		FA_Group_HSpacing, 0,

	TAG_MORE, Msg);
}
//+
///SB_Dispose
F_METHOD(uint32,SB_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_DisposeObj(LOD->prop);
	IFEELIN F_DisposeObj(LOD->button1);
	IFEELIN F_DisposeObj(LOD->button2);

	return F_SUPERDO();
}
//+
///SB_Get

F_METHOD(uint32, SB_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ENTRIES:
		{
			F_STORE(IFEELIN F_Get(LOD->prop,(uint32) "FA_Prop_Entries"));
		}
		break;

		case FV_ATTRIBUTE_FIRST:
		{
			F_STORE(IFEELIN F_Get(LOD->prop,(uint32) "FA_Prop_First"));
		}
		break;

		case FV_ATTRIBUTE_VISIBLE:
		{
			F_STORE(IFEELIN F_Get(LOD->prop,(uint32) "FA_Prop_Visible"));
		}
		break;
	}

	return F_SUPERDO();
}
//+
///SB_Set

#define FF_FORWARD_ENTRIES                      (1 << 0)
#define FF_FORWARD_FIRST                        (1 << 1)
#define FF_FORWARD_VISIBLE                      (1 << 2)

F_METHOD(uint32,SB_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	bits32 flags=0;
	uint32 value_entries=0;
	uint32 value_first=0;
	uint32 value_visible=0;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ENTRIES:
		{
			flags |= FF_FORWARD_ENTRIES; value_entries = item.ti_Data;

			#ifdef DB_SET
			IFEELIN F_Log(0,"entires (%ld)", value_entries);
			#endif
		}
		break;

		case FV_ATTRIBUTE_FIRST:
		{
			flags |= FF_FORWARD_FIRST; value_first = item.ti_Data;

			#ifdef DB_SET
			IFEELIN F_Log(0,"first (%ld)", value_first);
			#endif
		}
		break;

		case FV_ATTRIBUTE_VISIBLE:
		{
			flags |= FF_FORWARD_VISIBLE; value_visible = item.ti_Data;

			#ifdef DB_SET
			IFEELIN F_Log(0,"visible (%ld)", value_visible);
			#endif
		}
		break;
	}

	if (flags)
	{
		IFEELIN F_Do(LOD->prop,FM_Set,
			
			FA_NoNotify,TRUE,
			(FF_FORWARD_ENTRIES & flags) ? (uint32) "FA_Prop_Entries" : TAG_IGNORE, value_entries,
			(FF_FORWARD_FIRST   & flags) ? (uint32) "FA_Prop_First"   : TAG_IGNORE, value_first,
			(FF_FORWARD_VISIBLE & flags) ? (uint32) "FA_Prop_Visible" : TAG_IGNORE, value_visible,
			
			TAG_DONE);
	}

	return F_SUPERDO();
}
//+

///SB_Setup
F_METHODM(bool32,SB_Setup,FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	STRPTR value = NULL;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}
		
	LOD->type = FV_Scrollbar_Type_Symetric;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(TYPE), &value, NULL);

	if (value)
	{
		if (IFEELIN F_StrCmp(value, "start", ALL) == 0)
		{
			LOD->type = FV_Scrollbar_Type_Start;
		}
		else if (IFEELIN F_StrCmp(value, "end", ALL) == 0)
		{
			LOD->type = FV_Scrollbar_Type_End;
		}
		else if (IFEELIN F_StrCmp(value, "clear", ALL) == 0)
		{
			LOD->type = FV_Scrollbar_Type_Clear;
		}
		else if (IFEELIN F_StrCmp(value, "symetric", ALL) == 0)
		{
			LOD->type = FV_Scrollbar_Type_Symetric;
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV, "invalid value (%s) for property 'type'", value);
		}
	}

	switch (LOD->type)
	{
		case FV_Scrollbar_Type_Start:
		{
			IFEELIN F_Do(Obj, FM_AddMember, LOD->button1, FV_AddMember_Tail);
			IFEELIN F_Do(Obj, FM_AddMember, LOD->button2, FV_AddMember_Tail);
			IFEELIN F_Do(Obj, FM_AddMember, LOD->prop, FV_AddMember_Tail);
		}
		break;

		case FV_Scrollbar_Type_End:
		{
			IFEELIN F_Do(Obj, FM_AddMember, LOD->prop, FV_AddMember_Tail);
			IFEELIN F_Do(Obj, FM_AddMember, LOD->button1, FV_AddMember_Tail);
			IFEELIN F_Do(Obj, FM_AddMember, LOD->button2, FV_AddMember_Tail);
		}
		break;

		case FV_Scrollbar_Type_Clear:
		{
			IFEELIN F_Do(Obj, FM_AddMember, LOD->prop, FV_AddMember_Tail);
		}
		break;

		default: // symetric
		{
			IFEELIN F_Do(Obj, FM_AddMember, LOD->button1, FV_AddMember_Tail);
			IFEELIN F_Do(Obj, FM_AddMember, LOD->prop, FV_AddMember_Tail);
			IFEELIN F_Do(Obj, FM_AddMember, LOD->button2, FV_AddMember_Tail);
		}
		break;
	}

	return TRUE;
}
//+
///SB_Cleanup
F_METHOD(bool32,SB_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	bool32 rc = F_SUPERDO();

	if (IFEELIN F_Get(LOD->prop, FA_Parent) != 0)
	{
		IFEELIN F_Do(Obj, FM_RemMember, LOD->prop);
	}

	if (IFEELIN F_Get(LOD->button1, FA_Parent) != 0)
	{
		IFEELIN F_Do(Obj, FM_RemMember, LOD->button1);
	}

	if (IFEELIN F_Get(LOD->button2, FA_Parent) != 0)
	{
		IFEELIN F_Do(Obj, FM_RemMember, LOD->button2);
	}

	return rc;
}
//+

///Preferences

STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("scrollbar",              "type",             FV_TYPE_INTEGER, NULL,   (STRPTR) FV_Scrollbar_Type_Symetric ),

	F_PREFERENCES_ADD("scrollbar",              "frame",            FV_TYPE_STRING,  "Spec",     "<frame id='1' />" ),
	F_PREFERENCES_ADD("scrollbar",              "back-horizontal",  FV_TYPE_STRING,  "Spec",     NULL ),
	F_PREFERENCES_ADD("scrollbar",              "back",             FV_TYPE_STRING,  "Spec",     NULL ),
	F_PREFERENCES_ADD("scrollbar",              "color-scheme",     FV_TYPE_STRING,  "Spec",     NULL ),

	F_PREFERENCES_ADD("scrollbar-prop",         "frame",            FV_TYPE_STRING,  "Spec",     "<frame id='0' />" ),
	F_PREFERENCES_ADD("scrollbar-prop",         "back-horizontal",  FV_TYPE_STRING,  "Spec",     "halfshadow" ),
	F_PREFERENCES_ADD("scrollbar-prop",         "back",             FV_TYPE_STRING,  "Spec",     "halfshadow" ),
	F_PREFERENCES_ADD("scrollbar-prop",         "color-scheme",     FV_TYPE_STRING,  "Spec",     NULL ),

	F_PREFERENCES_ADD("scrollbar-knob",         "frame",            FV_TYPE_STRING,  "Spec",     "<frame id='42' /><frame id='40' />" ),
	F_PREFERENCES_ADD("scrollbar-knob",         "back-horizontal",  FV_TYPE_STRING,  "Spec",     "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow" ),
	F_PREFERENCES_ADD("scrollbar-knob",         "back",             FV_TYPE_STRING,  "Spec",     "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow" ),
	F_PREFERENCES_ADD("scrollbar-knob",         "color-scheme",     FV_TYPE_STRING,  "Spec",     NULL ),

	F_PREFERENCES_ADD("scrollbar-arrow-left",   "frame",            FV_TYPE_STRING,  "Spec",     "<frame id='42' padding-width='2' /><frame id='40' padding-width='2' />" ),
	F_PREFERENCES_ADD("scrollbar-arrow-left",   "image",            FV_TYPE_STRING,  "Spec",     "<image type='brush' src='defaults/arrow_l.fb0' />" F_IMAGEDISPLAY_SEPARATOR "<image type='brush' src='defaults/arrow_l.fb1' />" ),
	F_PREFERENCES_ADD("scrollbar-arrow-left",   "back",             FV_TYPE_STRING,  "Spec",     "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow" ),
	F_PREFERENCES_ADD("scrollbar-arrow-left",   "color-scheme",     FV_TYPE_STRING,  "Spec",     NULL ),

	F_PREFERENCES_ADD("scrollbar-arrow-right",  "frame",            FV_TYPE_STRING,  "Spec",     "<frame id='42' padding-width='2' /><frame id='40' padding-width='2' />" ),
	F_PREFERENCES_ADD("scrollbar-arrow-right",  "image",            FV_TYPE_STRING,  "Spec",     "<image type='brush' src='defaults/arrow_r.fb0' />" F_IMAGEDISPLAY_SEPARATOR "<image type='brush' src='defaults/arrow_r.fb1' />" ),
	F_PREFERENCES_ADD("scrollbar-arrow-right",  "back",             FV_TYPE_STRING,  "Spec",     "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow" ),
	F_PREFERENCES_ADD("scrollbar-arrow-right",  "color-scheme",     FV_TYPE_STRING,  "Spec",     NULL ),

	F_PREFERENCES_ADD("scrollbar-arrow-top",    "frame",            FV_TYPE_STRING,  "Spec",     "<frame id='42' padding-height='2' /><frame id='40' padding-height='2' />" ),
	F_PREFERENCES_ADD("scrollbar-arrow-top",    "image",            FV_TYPE_STRING,  "Spec",     "<image type='brush' src='defaults/arrow_t.fb0' />" F_IMAGEDISPLAY_SEPARATOR "<image type='brush' src='defaults/arrow_t.fb1' />" ),
	F_PREFERENCES_ADD("scrollbar-arrow-top",    "back",             FV_TYPE_STRING,  "Spec",     "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow" ),
	F_PREFERENCES_ADD("scrollbar-arrow-top",    "color-scheme",     FV_TYPE_STRING,  "Spec",     NULL ),

	F_PREFERENCES_ADD("scrollbar-arrow-bottom", "frame",            FV_TYPE_STRING,  "Spec",     "<frame id='42' padding-height='2' /><frame id='40' padding-height='2' />" ),
	F_PREFERENCES_ADD("scrollbar-arrow-bottom", "image",            FV_TYPE_STRING,  "Spec",     "<image type='brush' src='defaults/arrow_b.fb0' />" F_IMAGEDISPLAY_SEPARATOR "<image type='brush' src='defaults/arrow_b.fb1' />" ),
	F_PREFERENCES_ADD("scrollbar-arrow-bottom", "back",             FV_TYPE_STRING,  "Spec",     "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow" ),
	F_PREFERENCES_ADD("scrollbar-arrow-bottom", "color-scheme",     FV_TYPE_STRING,  "Spec",     NULL ),
 
	F_ARRAY_END
};

enum    {

		FV_ADJUSTER_TYPE

		};

#define F_ADJUSTER(id)                          LPD->adjusters[FV_ADJUSTER_##id]

F_METHOD(FObject,Prefs_New)
{
	struct LocalPreferenceData *LPD = F_LOD(Class, Obj);

	STATIC F_ENTITIES_ARRAY =
	{
		F_ENTITIES_ADD("locale.container",  "Container"),
		F_ENTITIES_ADD("locale.knob",       "Knob"),
		F_ENTITIES_ADD("locale.scrollbar",  "Scrollbar"),
		F_ENTITIES_ADD("locale.frame",      "Frame"),
		F_ENTITIES_ADD("locale.back",       "Back"),
		F_ENTITIES_ADD("locale.scheme",     "Scheme"),
		F_ENTITIES_ADD("locale.image",      "Image"),
	
		F_ARRAY_END
	};

	if (IFEELIN F_SuperDo(Class, Obj, Method,

	   FA_Group_Name, "Scrollbar",

	   "Script", F_PREFERENCES_PTR,
	   "Source", "feelin/preference/scrollbar.xml",
	   "Entities", F_ENTITIES_PTR,

	TAG_MORE,Msg) != 0)
	{
		IFEELIN F_Do
		(
			Obj, (uint32) "GetObjects",

			"scrollbar:type", &LPD->type,

			NULL
		);

		return Obj;
	}

	return NULL;
}
//+

///Prefs_Load
F_METHODM(uint32,Prefs_Load,FS_PreferenceGroup_Load)
{
	struct LocalPreferenceData *LPD = F_LOD(Class,Obj);

	uint32 active = FV_Scrollbar_Type_Symetric;

	FCSSProperty *property = (FCSSProperty *) IFEELIN F_Do(Msg->Prefs, Msg->id_GetProperty, "scrollbar", "type");

	if (property)
	{
		if (IFEELIN F_StrCmp(property->Value, "start", ALL) == 0)
		{
			active = FV_Scrollbar_Type_Start;
		}
		else if (IFEELIN F_StrCmp(property->Value, "end", ALL) == 0)
		{
			active = FV_Scrollbar_Type_End;
		}
		else if (IFEELIN F_StrCmp(property->Value, "clear", ALL) == 0)
		{
			active = FV_Scrollbar_Type_Clear;
		}
	}

	IFEELIN F_Set(LPD->type, (uint32) "Active", active);

	return F_SUPERDO();
}
//+
///Prefs_Save
F_METHODM(uint32,Prefs_Save,FS_PreferenceGroup_Save)
{
	struct LocalPreferenceData *LPD = F_LOD(Class,Obj);

	uint32 active = IFEELIN F_Get(LPD->type, (uint32) "Active");

	STRPTR type = NULL;

	switch (active)
	{
		case FV_Scrollbar_Type_Start:   type = "start"; break;
		case FV_Scrollbar_Type_End:     type = "end"; break;
		case FV_Scrollbar_Type_Clear:   type = "clear"; break;
	}

	IFEELIN F_Do
	(
		Msg->Prefs, Msg->id_AddProperty,

		"scrollbar",
		"type",

		type, FV_TYPE_STRING
	);

	return F_SUPERDO();
}
//+
