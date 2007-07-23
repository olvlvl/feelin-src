/*

$VER: 10.00 (2006/08/10)

	The Area class is now a subclass of the Element class.  Most  important,
	the  class  is no longer the base class for gadgets. All events support,
	weight, alignement... has been moved to a the new Widget class. The Area
	class is now the base class for all UI element such as gadgets, but also
	windows.

	There are too many changes to be listed, the class is nearly a new one !

$VER: 09.00 (2005/12/08)
 
	Removed a longstanding bug in FM_Widget_HandleEvent.  The  FEventHandler
	was not added the first time it was created, but only when modified.
   
	Metaclass support.
   
	Added FF_Hidden_Check and FF_Hidden_NoNesting to set FA_Widget_Hidden.
	
	Added    FF_Disabled_Check    and    FF_Disabled_NoNesting    to     set
	FA_Widget_Disabled.
	
	A new technique is used to limit redraws to damaged objects.  An  object
	is  damaged  when at least one of its coordinate or its visual state was
	modified. A damaged object is marked with the FF_Area_Damaged flag. This
	flag  is then used by Feelin to decide if an object should be redrawn or
	not. This new technique is used widely and particuliarly when rethinking
	layout. The FM_RethinkLayout method is now deprecated.
	
	Added the FM_Area_Move method used to move an  object  to  abosolute  or
	relative  coordinates.  The purpose of this method is to support virtual
	groups. The FF_Area_Damaged flags is set when coordinates are modified.
	
	FA_Left, FA_Right,  FA_Top,  FA_Bottom,  FA_Area_Width,  FA_Area_Height,
	FA_Area_MinWidth,    FA_Area_MinHeight,    FA_MaxWidth,    FA_MaxHeight,
	FA_FixWidth and FA_FixHeight are now FC_Frame attributes, thus FC_Window
	can inherit them too.

	The FM_Get method is  no  longer  used  to  obtain  parent's  background
	object,  its  FFramePublic is used instead, which also work for the root
	object since FC_Window is now a subclass of FC_Frame.

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Area_New);
F_METHOD_PROTO(void,Area_Get);
F_METHOD_PROTO(void,Area_Set);
#ifndef F_NEW_GLOBALCONNECT
F_METHOD_PROTO(void,Area_Connect);
F_METHOD_PROTO(void,Area_Disconnect);
#endif

F_METHOD_PROTO(void,Area_Setup);
F_METHOD_PROTO(void,Area_Cleanup);
F_METHOD_PROTO(void,Area_CreateDecodedStyle);

F_METHOD_PROTO(void,Area_Show);
F_METHOD_PROTO(void,Area_Hide);
F_METHOD_PROTO(void,Area_Layout);
F_METHOD_PROTO(void,Area_AskMinMax);
F_METHOD_PROTO(void,Area_Erase);
F_METHOD_PROTO(void,Area_Draw);
F_METHOD_PROTO(void,Area_SetState);
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
			STATIC F_VALUES_ARRAY(Orientation) =
			{
				F_VALUES_ADD("horizontal", FV_Area_Orientation_Horizontal),
				F_VALUES_ADD("vertical", FV_Area_Orientation_Vertical),

				F_ARRAY_END
			};

			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_STATIC("PublicData", FV_TYPE_POINTER, FA_Area_PublicData),
				F_ATTRIBUTES_ADD_STATIC_VALUES("Orientation", FV_TYPE_INTEGER, FA_Area_Orientation, Orientation),
				F_ATTRIBUTES_ADD_STATIC("Fillable", FV_TYPE_BOOLEAN, FA_Area_Fillable),
				F_ATTRIBUTES_ADD_STATIC("Font", FV_TYPE_STRING,  FA_Area_Font),
				F_ATTRIBUTES_ADD_STATIC("Bufferize", FV_TYPE_BOOLEAN, FA_Area_Bufferize),

				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD("touch"),
				F_ATOMS_ADD("focus"),
				F_ATOMS_ADD("ghost"),

				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("width"),
				F_PROPERTIES_ADD("height"),

				F_PROPERTIES_ADD("min-width"),
				F_PROPERTIES_ADD("min-height"),
				F_PROPERTIES_ADD("max-width"),
				F_PROPERTIES_ADD("max-height"),

				F_PROPERTIES_ADD("margin"),
				F_PROPERTIES_ADD("margin-left"),
				F_PROPERTIES_ADD("margin-right"),
				F_PROPERTIES_ADD("margin-top"),
				F_PROPERTIES_ADD("margin-bottom"),

				F_PROPERTIES_ADD("padding"),
				F_PROPERTIES_ADD("padding-left"),
				F_PROPERTIES_ADD("padding-right"),
				F_PROPERTIES_ADD("padding-top"),
				F_PROPERTIES_ADD("padding-bottom"),

				F_PROPERTIES_ADD("font"),
				F_PROPERTIES_ADD("background"),
				F_PROPERTIES_ADD("color-scheme"),

				F_PROPERTIES_ADD("color"),

				F_PROPERTIES_ADD("palette"),
				F_PROPERTIES_ADD("palette-fill"),
				F_PROPERTIES_ADD("palette-shine"),
				F_PROPERTIES_ADD("palette-dark"),
				F_PROPERTIES_ADD("palette-highlight"),
				F_PROPERTIES_ADD("palette-contrast"),
				F_PROPERTIES_ADD("palette-luminance"),
				F_PROPERTIES_ADD("palette-saturation"),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_OVERRIDE_STATIC(Area_New,          FM_New),
				F_METHODS_OVERRIDE_STATIC(Area_Get,          FM_Get),
				F_METHODS_OVERRIDE_STATIC(Area_Set,          FM_Set),
				#ifndef F_NEW_GLOBALCONNECT
				F_METHODS_OVERRIDE_STATIC(Area_Connect,      FM_Connect),
				F_METHODS_OVERRIDE_STATIC(Area_Disconnect,	 FM_Disconnect),
				#endif

				F_METHODS_OVERRIDE_STATIC(Area_Setup,                FM_Element_Setup),
				F_METHODS_OVERRIDE_STATIC(Area_Cleanup,              FM_Element_Cleanup),
				F_METHODS_OVERRIDE_STATIC(Area_CreateDecodedStyle,	 FM_Element_CreateDecodedStyle),

				F_METHODS_ADD_STATIC(Area_AskMinMax,  "AskMinMax",    FM_Area_AskMinMax),
				F_METHODS_ADD_STATIC(Area_Layout,     "Layout",       FM_Area_Layout),
				F_METHODS_ADD_STATIC(Area_Show,       "Show",         FM_Area_Show),
				F_METHODS_ADD_STATIC(Area_Hide,       "Hide",         FM_Area_Hide),
				F_METHODS_ADD_STATIC(Area_Erase,      "Erase",        FM_Area_Erase),
				F_METHODS_ADD_STATIC(Area_Draw,       "Draw",         FM_Area_Draw),
				F_METHODS_ADD_STATIC(Area_SetState,   "SetState",	  FM_Area_SetState),
				//F_METHODS_ADD_STATIC(Area_Move,         "Move",             FM_Area_Move),
				
				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Element),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_ATOMS,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_PROPERTIES,
				F_TAGS_ADD_PROPERTIES_SIZE(FV_AREA_STATE_COUNT - 1),

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
