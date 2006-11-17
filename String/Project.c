/*

$VER: 07.00 (2006/06/04)

	Added minus sign (-) in the special FV_String_Decimal.

	Widget subclass.

	Styles support.

$VER: 06.00 (2005/12/11)
 
	Added the FV_String_Decimal and FV_String_Hexadecimal specials values to
	the FA_String_Accept and FA_String_Reject attributes.
	
	FA_String_Max no longer includes the NULL character in count.
	
	FM_List_FindString support.
	
	Added FA_String_Displayable.
 
*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,String_New);
F_METHOD_PROTO(void,String_Dispose);
F_METHOD_PROTO(void,String_Get);
F_METHOD_PROTO(void,String_Set);
F_METHOD_PROTO(void,String_Setup);
F_METHOD_PROTO(void,String_Cleanup);
F_METHOD_PROTO(void,String_AskMinMax);
F_METHOD_PROTO(void,String_Draw);
F_METHOD_PROTO(void,String_SetState);
F_METHOD_PROTO(void,String_HandleEvent);

F_METHOD_PROTO(void,Prefs_New);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(Justify) =
			{
				F_VALUES_ADD("left",     FV_String_Left),
				F_VALUES_ADD("center",   FV_String_Center),
				F_VALUES_ADD("right",    FV_String_Right),

				F_ARRAY_END
			};

			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD("Accept",         FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("AdvanceOnCR",    FV_TYPE_BOOLEAN),

				F_ATTRIBUTES_ADD("Changed",        FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("Contents",       FV_TYPE_STRING),
				F_ATTRIBUTES_ADD_VALUES("Justify",FV_TYPE_INTEGER, Justify),
				F_ATTRIBUTES_ADD("Integer",        FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Max",            FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Reject",         FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("Secret",         FV_TYPE_BOOLEAN),
				F_ATTRIBUTES_ADD("AttachedList",   FV_TYPE_OBJECT),
				F_ATTRIBUTES_ADD("Displayable",    FV_TYPE_INTEGER),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(String_New,         	FM_New),
				F_METHODS_ADD_STATIC(String_Dispose,     	FM_Dispose),
				F_METHODS_ADD_STATIC(String_Get,         	FM_Get),
				F_METHODS_ADD_STATIC(String_Set,         	FM_Set),

				F_METHODS_ADD_STATIC(String_Setup,       	FM_Element_Setup),
				F_METHODS_ADD_STATIC(String_Cleanup,     	FM_Element_Cleanup),

				F_METHODS_ADD_STATIC(String_AskMinMax,   	FM_Area_AskMinMax),
			    F_METHODS_ADD_STATIC(String_Draw,           FM_Area_Draw),
			    F_METHODS_ADD_STATIC(String_SetState,  	    FM_Area_SetState),

				F_METHODS_ADD_STATIC(String_HandleEvent, 	FM_Widget_HandleEvent),

				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("cursor"),

				F_PROPERTIES_ADD("text-active"),
				F_PROPERTIES_ADD("text-inactive"),
				F_PROPERTIES_ADD("text-block"),

				F_PROPERTIES_ADD("blink-speed"),
				F_PROPERTIES_ADD("blink-image"),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Widget),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_PROPERTIES,

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
}

