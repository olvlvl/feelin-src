/*

$VER: 03.02 (2005/12/11)
 
	Portability update
	
	Characters can now be used to navigation (or jump)  through  entries.  A
	FC_String object is even popuped to show what's going on. The activation
	of the object is not mandatory  to  use  this  feature,  which  is  also
	available if the mouse pointer is over the object.
	
*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,List_New);
F_METHOD_PROTO(void,List_Dispose);
F_METHOD_PROTO(void,List_Set);
F_METHOD_PROTO(void,List_Get);
F_METHOD_PROTO(void,List_Setup);
F_METHOD_PROTO(void,List_Cleanup);
F_METHOD_PROTO(void,List_AskMinMax);
F_METHOD_PROTO(void,List_Layout);
F_METHOD_PROTO(void,List_Draw);
F_METHOD_PROTO(void,List_SetState);
F_METHOD_PROTO(void,List_HandleEvent);

F_METHOD_PROTO(void,List_Construct);
F_METHOD_PROTO(void,List_Destruct);
F_METHOD_PROTO(void,List_Compare);
F_METHOD_PROTO(void,List_Display);
F_METHOD_PROTO(void,List_GetEntry);
F_METHOD_PROTO(void,List_Insert);
F_METHOD_PROTO(void,List_InsertSingle);
F_METHOD_PROTO(void,List_Remove);
F_METHOD_PROTO(void,List_FindString);

#if 0
F_METHOD_PROTO(void,Prefs_New);
#endif
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(Active) =
			{
				F_VALUES_ADD("page-down", FV_List_Active_PageDown),
				F_VALUES_ADD("page-up", FV_List_Active_PageUp),
				F_VALUES_ADD("down", FV_List_Active_Down),
				F_VALUES_ADD("up", FV_List_Active_Up),
				F_VALUES_ADD("bottom", FV_List_Active_Bottom),
				F_VALUES_ADD("top", FV_List_Active_Top),
				F_VALUES_ADD("none", FV_List_Active_None),
				
				F_ARRAY_END
			};
 
			STATIC F_VALUES_ARRAY(Activation) =
			{
				F_VALUES_ADD("click", FV_List_Activation_Click),
				F_VALUES_ADD("double-click", FV_List_Activation_DoubleClick),
				F_VALUES_ADD("key", FV_List_Activation_Key),
				F_VALUES_ADD("external", FV_List_Activation_External),

				F_ARRAY_END
			};
			
			STATIC F_VALUES_ARRAY(String) =
			{
				F_VALUES_ADD("string", FV_List_Hook_String),

				F_ARRAY_END
			};
			 
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_VALUES("Active",          FV_TYPE_INTEGER, Active),
				F_ATTRIBUTES_ADD_VALUES("Activation",      FV_TYPE_INTEGER, Activation),
				F_ATTRIBUTES_ADD_VALUES("CompareHook",     FV_TYPE_POINTER, String),
				F_ATTRIBUTES_ADD_VALUES("ConstructHook",   FV_TYPE_POINTER, String),
				F_ATTRIBUTES_ADD_VALUES("DestructHook",    FV_TYPE_POINTER, String),
				F_ATTRIBUTES_ADD_VALUES("DisplayHook",     FV_TYPE_POINTER, String),

				F_ATTRIBUTES_ADD("Entries",         FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Visible",         FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("First",           FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Last",            FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Format",          FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("Pool",            FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("PoolItemNumber",  FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("PoolItemSize",    FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Quiet",           FV_TYPE_BOOLEAN),
				F_ATTRIBUTES_ADD("SortMode",        FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("SourceArray",     FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Title",           FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("ReadOnly",        FV_TYPE_BOOLEAN),

				F_ATTRIBUTES_ADD("Steps", FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Spacing", FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("CursorActive", FV_TYPE_STRING),
			
				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD("bar"),
				F_ATOMS_ADD("col"),
				F_ATOMS_ADD("fixed"),
				F_ATOMS_ADD("max"),
				F_ATOMS_ADD("min"),
				F_ATOMS_ADD("padding"),
				F_ATOMS_ADD("span"),
				F_ATOMS_ADD("weight"),
				F_ATOMS_ADD("width"),

				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("steps"),
				F_PROPERTIES_ADD("spacing"),
				F_PROPERTIES_ADD("cursor-active"),
				F_PROPERTIES_ADD("title-color-scheme"),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(List_Compare,       "Compare"),
				F_METHODS_ADD(List_Construct,     "Construct"),
				F_METHODS_ADD(List_Destruct,      "Destruct"),
				F_METHODS_ADD(List_Display,       "Display"),
				F_METHODS_ADD(List_GetEntry,      "GetEntry"),
				F_METHODS_ADD(List_Insert,        "Insert"),
				F_METHODS_ADD(List_InsertSingle,  "InsertSingle"),
				F_METHODS_ADD(List_Remove,        "Remove"),
				F_METHODS_ADD(List_FindString,    "FindString"),
				
				F_METHODS_OVERRIDE_STATIC(List_New,          FM_New),
				F_METHODS_OVERRIDE_STATIC(List_Dispose,      FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(List_Get,          FM_Get),
				F_METHODS_OVERRIDE_STATIC(List_Set,          FM_Set),
				F_METHODS_OVERRIDE_STATIC(List_Setup,        FM_Element_Setup),
				F_METHODS_OVERRIDE_STATIC(List_Cleanup,      FM_Element_Cleanup),
				F_METHODS_OVERRIDE_STATIC(List_AskMinMax,    FM_Area_AskMinMax),
				F_METHODS_OVERRIDE_STATIC(List_Layout,       FM_Area_Layout),
				F_METHODS_OVERRIDE_STATIC(List_Draw,         FM_Area_Draw),
				F_METHODS_OVERRIDE_STATIC(List_SetState,     FM_Area_SetState),
				F_METHODS_OVERRIDE_STATIC(List_HandleEvent,  FM_Widget_HandleEvent),

				F_ARRAY_END
			};
						
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Widget),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_ATOMS,
				F_TAGS_ADD_PROPERTIES,
				
				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
#if 0
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
#endif
	}
	return NULL;
}

