/*

$VER: 05.00 (2005/12/10)
 
	Added FA_Gauge_Image attribute, which can now be used to  customize  the
	gauge look.

$VER: 04.00 (2005/04/06)
 
	Uses an XMLObject for preferences GUI.
	
	Added $gauge-back-vertical preference item, which should be used for
	background of vertical objects. Default is $gauge-back.

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Gauge_New);
F_METHOD_PROTO(void,Gauge_Setup);
F_METHOD_PROTO(void,Gauge_Cleanup);
F_METHOD_PROTO(void,Gauge_AskMinMax);
F_METHOD_PROTO(void,Gauge_Draw);

/*** Preferences ***/

F_METHOD_PROTO(void,Prefs_New);
F_METHOD_PROTO(void,Prefs_Show);
F_METHOD_PROTO(void,Prefs_Hide);
F_METHOD_PROTO(void,Prefs_Update);
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
				F_ATTRIBUTES_ADD("Info", FV_TYPE_STRING),
				
				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_OVERRIDE_STATIC(Gauge_New, FM_New),
				F_METHODS_OVERRIDE_STATIC(Gauge_Setup, FM_Element_Setup),
				F_METHODS_OVERRIDE_STATIC(Gauge_Cleanup, FM_Element_Cleanup),
				F_METHODS_OVERRIDE_STATIC(Gauge_AskMinMax, FM_Area_AskMinMax),
				F_METHODS_OVERRIDE_STATIC(Gauge_Draw, FM_Area_Draw),
				
				F_ARRAY_END
			};
			 
			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("image"),

				F_ARRAY_END
			};

			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FA_Numeric_Value"),
				F_RESOLVEDS_ADD("FA_Numeric_Min"),
				F_RESOLVEDS_ADD("FA_Numeric_Max"),
				
				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Numeric),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_PROPERTIES,
				F_TAGS_ADD_RESOLVEDS,
				
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
				F_METHODS_ADD_STATIC(Prefs_New,     FM_New),
				F_METHODS_ADD_STATIC(Prefs_Show,    FM_Area_Show),
				F_METHODS_ADD_STATIC(Prefs_Hide,    FM_Area_Hide),
				F_METHODS_ADD_STATIC(Prefs_Update,  FM_Gauge_Update),
				
				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(PreferenceGroup),
				F_TAGS_ADD(LODSize,  sizeof (struct p_LocalObjectData)),
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

