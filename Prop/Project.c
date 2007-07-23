/*

$VER: 06.00 (2006/05/02)

	Style support.

	The custom knob object is added with the commun FA_Child  attribute  and
	the FM_AddMember method. The 'Knob' attribute is deprecated.

	Layout method rewrote.

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Prop_New);
F_METHOD_PROTO(void,Prop_Dispose);
F_METHOD_PROTO(void,Prop_Get);
F_METHOD_PROTO(void,Prop_Set);
F_METHOD_PROTO(void,Prop_AddMember);
F_METHOD_PROTO(void,Prop_RemMember);
F_METHOD_PROTO(void,Prop_Set);

#ifdef F_NEW_GLOBALCONNECT
F_METHOD_PROTO(void,Prop_GlobalConnect);
F_METHOD_PROTO(void,Prop_GlobalDisconnect);
#endif

F_METHOD_PROTO(void,Prop_Setup);
F_METHOD_PROTO(void,Prop_Cleanup);
F_METHOD_PROTO(void,Prop_Show);
F_METHOD_PROTO(void,Prop_Hide);
F_METHOD_PROTO(void,Prop_AskMinMax);
F_METHOD_PROTO(void,Prop_Layout);
F_METHOD_PROTO(void,Prop_Draw);
F_METHOD_PROTO(void,Prop_SetState);
F_METHOD_PROTO(void,Prop_HandleEvent);
F_METHOD_PROTO(void,Prop_Decrease);
F_METHOD_PROTO(void,Prop_Increase);
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
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD("Entries",  FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Visible",  FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("First",    FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Step",     FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Useless",  FV_TYPE_BOOLEAN),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(Prop_Decrease,    "Decrease"),
				F_METHODS_ADD(Prop_Increase,    "Increase"),

				F_METHODS_OVERRIDE_STATIC(Prop_New,          FM_New),
				F_METHODS_OVERRIDE_STATIC(Prop_Dispose,      FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(Prop_Get,          FM_Get),
				F_METHODS_OVERRIDE_STATIC(Prop_Set,          FM_Set),
				F_METHODS_OVERRIDE_STATIC(Prop_AddMember,    FM_AddMember),
				F_METHODS_OVERRIDE_STATIC(Prop_RemMember,    FM_RemMember),

				#ifdef F_NEW_GLOBALCONNECT
				F_METHODS_OVERRIDE_STATIC(Prop_GlobalConnect,	 FM_Element_GlobalConnect),
				F_METHODS_OVERRIDE_STATIC(Prop_GlobalDisconnect, FM_Element_GlobalDisconnect),
				#endif

				F_METHODS_OVERRIDE_STATIC(Prop_Setup,        FM_Element_Setup),
				F_METHODS_OVERRIDE_STATIC(Prop_Cleanup,      FM_Element_Cleanup),
				F_METHODS_OVERRIDE_STATIC(Prop_Show,         FM_Area_Show),
				F_METHODS_OVERRIDE_STATIC(Prop_Hide,         FM_Area_Hide),
				F_METHODS_OVERRIDE_STATIC(Prop_AskMinMax,    FM_Area_AskMinMax),
				F_METHODS_OVERRIDE_STATIC(Prop_Layout,       FM_Area_Layout),
				F_METHODS_OVERRIDE_STATIC(Prop_Draw,         FM_Area_Draw),
				F_METHODS_OVERRIDE_STATIC(Prop_SetState,     FM_Area_SetState),
				F_METHODS_OVERRIDE_STATIC(Prop_HandleEvent,  FM_Widget_HandleEvent),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Widget),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,

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

