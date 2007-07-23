/*

$VER: 04.00 (2006/05/29)

	Widget subclass.

	Style support.

$VER: 03.20 (2005/12/10)
 
	Portability update.
	
	FM_Window_RethinkRequest is invoked when the image spec is modified  and
	the object setuped.
 
*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Image_New);
F_METHOD_PROTO(void,Image_Dispose);
F_METHOD_PROTO(void,Image_Set);
F_METHOD_PROTO(void,Image_Get);
F_METHOD_PROTO(void,Image_Setup);
F_METHOD_PROTO(void,Image_Cleanup);
F_METHOD_PROTO(void,Image_AskMinMax);
F_METHOD_PROTO(void,Image_Draw);
F_METHOD_PROTO(void,Prefs_New);
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
				F_ATTRIBUTES_ADD("Spec", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("AltSpec", FV_TYPE_STRING),
			
				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD("selected"),

				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("image"),

				F_ARRAY_END
			};
								
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_OVERRIDE_STATIC(Image_New,        FM_New),
				F_METHODS_OVERRIDE_STATIC(Image_Dispose,    FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(Image_Set,        FM_Set),
				F_METHODS_OVERRIDE_STATIC(Image_Get,        FM_Get),
				F_METHODS_OVERRIDE_STATIC(Image_Setup,      FM_Element_Setup),
				F_METHODS_OVERRIDE_STATIC(Image_Cleanup,    FM_Element_Cleanup),
				F_METHODS_OVERRIDE_STATIC(Image_AskMinMax,  FM_Area_AskMinMax),
				F_METHODS_OVERRIDE_STATIC(Image_Draw,       FM_Area_Draw),
				
				F_ARRAY_END
			};
			
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Widget),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_ATOMS,
				F_TAGS_ADD_METHODS,
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

