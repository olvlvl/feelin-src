/*

$VER: 01.00 (2005/12/30)

	Page mode has been separated from FC_Group to create this new class. The
	page design has been totaly rewritten and is now much more customizable.

*/

#include "Project.h"
#include "_locale/table.h"

struct ClassUserData *CUD;

///METHODS
F_METHOD_PROTO(void,Page_New);
F_METHOD_PROTO(void,Page_Get);
F_METHOD_PROTO(void,Page_Set);

F_METHOD_PROTO(void,Page_Setup);
F_METHOD_PROTO(void,Page_Cleanup);
F_METHOD_PROTO(void,Page_CreateDecodedStyle);

F_METHOD_PROTO(void,Page_Show);
F_METHOD_PROTO(void,Page_Hide);
F_METHOD_PROTO(void,Page_Draw);
F_METHOD_PROTO(void,Page_SetState);

F_METHOD_PROTO(void,Page_HandleEvent);
F_METHOD_PROTO(void,Page_TreeUp);

F_METHOD_PROTO(void,Prefs_New);
//+

///Class_New
F_METHOD(uint32,Class_New)
{
	CUD = F_LOD(Class,Obj);

	CUD->minmax_hook.h_Entry = (HOOKFUNC) F_FUNCTION_GATE(code_minmax);
	CUD->minmax_hook.h_Data  = Obj;
	CUD->layout_hook.h_Entry = (HOOKFUNC) F_FUNCTION_GATE(code_layout);
	CUD->layout_hook.h_Data  = Obj;

	return F_SUPERDO();
}
//+

F_QUERY()
{
	switch (Which)
	{
///Meta
		case FV_Query_MetaClassTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Class_New, FM_New),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Class),
				F_TAGS_ADD(LODSize, sizeof (struct ClassUserData)),
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(Active) =
			{
				F_VALUES_ADD("prev", FV_Page_Active_Prev),
				F_VALUES_ADD("next", FV_Page_Active_Next),
				F_VALUES_ADD("last", FV_Page_Active_Last),
				F_VALUES_ADD("first", FV_Page_Active_First),

				F_ARRAY_END
			};

			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_VALUES("Active", FV_TYPE_INTEGER, Active),

				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("padding-left"),
				F_PROPERTIES_ADD("padding-right"),
				F_PROPERTIES_ADD("padding-top"),
				F_PROPERTIES_ADD("padding-bottom"),
				F_PROPERTIES_ADD("active-padding-top"),
				F_PROPERTIES_ADD("active-padding-bottom"),
				F_PROPERTIES_ADD("inactive-padding-top"),
				F_PROPERTIES_ADD("inactive-padding-bottom"),
				F_PROPERTIES_ADD("inactive-margin-top"),
				F_PROPERTIES_ADD("preparse-active"),
				F_PROPERTIES_ADD("preparse-inactive"),
				F_PROPERTIES_ADD("tab-font"),
				F_PROPERTIES_ADD("tab-background-inactive"),
				F_PROPERTIES_ADD("tab-background-active"),
				F_PROPERTIES_ADD("font"),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Page_New,  FM_New),
				F_METHODS_ADD_STATIC(Page_Get,  FM_Get),
				F_METHODS_ADD_STATIC(Page_Set,  FM_Set),
 
				F_METHODS_ADD_STATIC(Page_Setup,                FM_Element_Setup),
				F_METHODS_ADD_STATIC(Page_Cleanup,              FM_Element_Cleanup),
				F_METHODS_ADD_STATIC(Page_CreateDecodedStyle,   FM_Element_CreateDecodedStyle),

				F_METHODS_ADD_STATIC(Page_Show,         FM_Area_Show),
				F_METHODS_ADD_STATIC(Page_Hide,         FM_Area_Hide),
				F_METHODS_ADD_STATIC(Page_Draw,         FM_Area_Draw),
				F_METHODS_ADD_STATIC(Page_SetState,     FM_Area_SetState),
				
				F_METHODS_ADD_STATIC(Page_HandleEvent,  FM_Widget_HandleEvent),
				F_METHODS_ADD_STATIC(Page_TreeUp,       FM_Widget_DnDQuery),
/*
				F_METHODS_ADD_STATIC(Page_TreeUp,       FM_BuildContextHelp),
				F_METHODS_ADD_STATIC(Page_TreeUp,       FM_BuildContextMenu),
*/
				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Group),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_PROPERTIES,
				F_TAGS_ADD_PROPERTIES_SIZE(0),

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
///Prefs
		case FV_Query_PrefsTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Prefs_New, FM_New),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(PreferenceGroup),
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
   }
   return NULL;
};
//+
