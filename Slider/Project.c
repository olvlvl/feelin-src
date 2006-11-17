/*

$VER: 05.00 (2006/05/02)

	Style support.

	The custom knob object is added with the commun FA_Child  attribute  and
	the FM_AddMember method. The 'Knob' attribute is deprecated.

	Layout method rewrote.

$VER: 04.02 (2005/12/11)
 
	Portability update.
	
	Enhance minmax computing.
 
$VER: 04.00 (2005/05/06)
 
	Improved  rendering  and  knob  dimensions  computing.  New  preferences
	support.  Added  preferences  for  vertical back, knob vertical back and
	knob preparse.
 
$VER: 03.00 (2004/12/28)

	FA_AreaData support.

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Slider_New);
F_METHOD_PROTO(void,Slider_Dispose);
F_METHOD_PROTO(void,Slider_AddMember);
F_METHOD_PROTO(void,Slider_RemMember);

#ifdef F_NEW_GLOBALCONNECT
F_METHOD_PROTO(void,Slider_GlobalConnect);
F_METHOD_PROTO(void,Slider_GlobalDisconnect);
#endif

F_METHOD_PROTO(void,Slider_Setup);
F_METHOD_PROTO(void,Slider_Cleanup);
F_METHOD_PROTO(void,Slider_Show);
F_METHOD_PROTO(void,Slider_Hide);
F_METHOD_PROTO(void,Slider_AskMinMax);
F_METHOD_PROTO(void,Slider_Layout);
F_METHOD_PROTO(void,Slider_Draw);
F_METHOD_PROTO(void,Slider_SetState);
F_METHOD_PROTO(void,Slider_HandleEvent);
F_METHOD_PROTO(void,Prefs_New);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Slider_New,            FM_New),
				F_METHODS_ADD_STATIC(Slider_Dispose,        FM_Dispose),
				F_METHODS_ADD_STATIC(Slider_AddMember,      FM_AddMember),
				F_METHODS_ADD_STATIC(Slider_RemMember,      FM_RemMember),

				#ifdef F_NEW_GLOBALCONNECT
				F_METHODS_ADD_STATIC(Slider_GlobalConnect,	  FM_Element_GlobalConnect),
				F_METHODS_ADD_STATIC(Slider_GlobalDisconnect, FM_Element_GlobalDisconnect),
				#endif

				F_METHODS_ADD_STATIC(Slider_Setup,          FM_Element_Setup),
				F_METHODS_ADD_STATIC(Slider_Cleanup,        FM_Element_Cleanup),

				F_METHODS_ADD_STATIC(Slider_Show,           FM_Area_Show),
				F_METHODS_ADD_STATIC(Slider_Hide,           FM_Area_Hide),
				F_METHODS_ADD_STATIC(Slider_AskMinMax,      FM_Area_AskMinMax),
				F_METHODS_ADD_STATIC(Slider_Layout,         FM_Area_Layout),
				F_METHODS_ADD_STATIC(Slider_Draw,           FM_Area_Draw),
				F_METHODS_ADD_STATIC(Slider_SetState,       FM_Area_SetState),

/*
				F_METHODS_ADD_STATIC(Slider_GoActive,       FM_GoActive),
				F_METHODS_ADD_STATIC(Slider_GoInactive,     FM_GoInactive),
				F_METHODS_ADD_STATIC(Slider_GoEnabled,      FM_GoEnabled),
				F_METHODS_ADD_STATIC(Slider_GoDisabled,     FM_GoDisabled),
*/
				F_METHODS_ADD_STATIC(Slider_HandleEvent,    FM_Widget_HandleEvent),

				F_ARRAY_END
			};

			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FM_Numeric_Reset"),
				F_RESOLVEDS_ADD("FM_Numeric_Stringify"),
				F_RESOLVEDS_ADD("FA_Numeric_Value"),
				F_RESOLVEDS_ADD("FA_Numeric_Min"),
				F_RESOLVEDS_ADD("FA_Numeric_Max"),
				F_RESOLVEDS_ADD("FA_Numeric_Buffer"),
				F_RESOLVEDS_ADD("FA_Numeric_StringArray"),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Numeric),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_RESOLVEDS,
				
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
