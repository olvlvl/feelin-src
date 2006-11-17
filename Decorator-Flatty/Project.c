/*

$VER: Decorator-Flatty 01.00 (2005/12/21)
 
*/

#include "Project.h"

struct ClassUserData				CUD;

///METHODS
F_METHOD_PROTO(void, Deco_New);
F_METHOD_PROTO(void, Deco_Setup);
F_METHOD_PROTO(void, Deco_Cleanup);
F_METHOD_PROTO(void, Deco_Show);
F_METHOD_PROTO(void, Deco_Hide);
F_METHOD_PROTO(void, Deco_AskMinMax);
//F_METHOD_PROTO(void, Deco_Layout);
F_METHOD_PROTO(void, Deco_Draw);
F_METHOD_PROTO(void, Deco_SetState);

F_METHOD_PROTO(void, Prefs_New);
//+

///Class_New
F_METHOD(uint32,Class_New)
{
	CUD.layout_hook.h_Entry = (HOOKFUNC) F_FUNCTION_GATE(code_layout);
	CUD.layout_hook.h_Data = Obj;
	
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
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
///Class
		case FV_Query_ClassTags:
		{
			#ifdef F_NEW_STYLES

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("preparse-active"),
				F_PROPERTIES_ADD("preparse-inactive"),
				F_PROPERTIES_ADD("sizebar-background"),
				F_PROPERTIES_ADD("sizebar-height"),

				F_ARRAY_END
			};

			#endif

			STATIC F_METHODS_ARRAY =
			{
			   F_METHODS_ADD_STATIC(Deco_New,          FM_New),

			   F_METHODS_ADD_STATIC(Deco_Setup,        FM_Element_Setup),
			   F_METHODS_ADD_STATIC(Deco_Cleanup,      FM_Element_Cleanup),

			   F_METHODS_ADD_STATIC(Deco_Show,         FM_Area_Show),
			   F_METHODS_ADD_STATIC(Deco_Hide,         FM_Area_Hide),
			   F_METHODS_ADD_STATIC(Deco_AskMinMax,    FM_Area_AskMinMax),
//			     F_METHODS_ADD_STATIC(Deco_Layout,       FM_Area_Layout),
			   F_METHODS_ADD_STATIC(Deco_Draw,         FM_Area_Draw),
			   F_METHODS_ADD_STATIC(Deco_SetState,     FM_Area_SetState),
			   
			   F_ARRAY_END
			};
			 
			STATIC F_TAGS_ARRAY =
			{
			   F_TAGS_ADD_SUPER(Decorator),
			   F_TAGS_ADD_LOD,
			   F_TAGS_ADD_METHODS,

			   #ifdef F_NEW_STYLES
			   F_TAGS_ADD_PROPERTIES,
			   #endif
			
			   F_ARRAY_END
			};
	
			return F_TAGS_PTR;
		}
//+
///DecoratorPrefs
		case FV_Query_DecoratorPrefsTags:
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
