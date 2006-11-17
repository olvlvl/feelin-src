/*

$VER: 05.00 (2006/05/28)

	Widget subclass.

$VER: 04.00 (2005/01/05)

	Removed a bug when  centering  text  that  was  already  centered  using
	preparse.

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Text_New);
F_METHOD_PROTO(void,Text_Dispose);
F_METHOD_PROTO(void,Text_Set);
F_METHOD_PROTO(void,Text_Get);
F_METHOD_PROTO(void,Text_Setup);
F_METHOD_PROTO(void,Text_Cleanup);
F_METHOD_PROTO(void,Text_CreateDecodedStyle);
F_METHOD_PROTO(void,Text_AskMinMax);
F_METHOD_PROTO(void,Text_Layout);
F_METHOD_PROTO(void,Text_Draw);
F_METHOD_PROTO(void,Text_SetState);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_STATIC("Contents",      FV_TYPE_STRING,  FA_Text_Contents),
				F_ATTRIBUTES_ADD_STATIC("Shortcut",      FV_TYPE_BOOLEAN, FA_Text_Shortcut),
				F_ATTRIBUTES_ADD_STATIC("HCenter",       FV_TYPE_BOOLEAN, FA_Text_HCenter),
				F_ATTRIBUTES_ADD_STATIC("VCenter",       FV_TYPE_BOOLEAN, FA_Text_VCenter),
				F_ATTRIBUTES_ADD_STATIC("Static",        FV_TYPE_BOOLEAN, FA_Text_Static),
				
				F_ATTRIBUTES_ADD_STATIC("PreParse",      FV_TYPE_STRING,  FA_Text_PreParse),
				#if 0
				F_ATTRIBUTES_ADD_STATIC("AltPreParse",   FV_TYPE_STRING,  FA_Text_AltPreParse),
				#endif

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Text_New,        FM_New),
				F_METHODS_ADD_STATIC(Text_Dispose,    FM_Dispose),
				F_METHODS_ADD_STATIC(Text_Get,        FM_Get),
				F_METHODS_ADD_STATIC(Text_Set,        FM_Set),

				F_METHODS_ADD_STATIC(Text_Setup,      FM_Element_Setup),
				F_METHODS_ADD_STATIC(Text_Cleanup,    FM_Element_Cleanup),
				F_METHODS_ADD_STATIC(Text_CreateDecodedStyle, FM_Element_CreateDecodedStyle),

				F_METHODS_ADD_STATIC(Text_AskMinMax,  FM_Area_AskMinMax),
				F_METHODS_ADD_STATIC(Text_Draw,       FM_Area_Draw),
				F_METHODS_ADD_STATIC(Text_SetState, 	FM_Area_SetState),
				
				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD(F_AREA_STATE_TOUCH),
				F_ATOMS_ADD(F_AREA_STATE_FOCUS),
				F_ATOMS_ADD(F_AREA_STATE_GHOST),

				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("preparse"),
				F_PROPERTIES_ADD("text-position"),

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
				F_TAGS_ADD_PROPERTIES_SIZE(FV_AREA_STATE_COUNT),
				
				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
