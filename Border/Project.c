/*

$VER: 01.00 (2006/10/29)

	This class was formely known as the Frame class. It was renamed "Border"
	because  frames  are  now one feature among others to add borders around
	objects contents.

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Border_New);
F_METHOD_PROTO(void,Border_Dispose);
F_METHOD_PROTO(void,Border_Get);
F_METHOD_PROTO(void,Border_Set);

F_METHOD_PROTO(void,Border_Setup);
F_METHOD_PROTO(void,Border_Cleanup);
F_METHOD_PROTO(void,Border_CreateDecodedStyle);

F_METHOD_PROTO(void,Border_Draw);
F_METHOD_PROTO(void,Border_SetState);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD("touch"),
				F_ATOMS_ADD("focus"),
				F_ATOMS_ADD("ghost"),

				/* frame-position */

				F_ATOMS_ADD("up"),
				F_ATOMS_ADD("down"),
				F_ATOMS_ADD("left"),
				F_ATOMS_ADD("right"),
				F_ATOMS_ADD("center"),

				F_ARRAY_END
			};

			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_STATIC("Caption", FV_TYPE_STRING, FA_Border_Caption),

				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				// border

				F_PROPERTIES_ADD("border"),
				F_PROPERTIES_ADD("border-color"),
				F_PROPERTIES_ADD("border-style"),
				F_PROPERTIES_ADD("border-width"),

				F_PROPERTIES_ADD("border-top"),
				F_PROPERTIES_ADD("border-top-color"),
				F_PROPERTIES_ADD("border-top-style"),
				F_PROPERTIES_ADD("border-top-width"),

				F_PROPERTIES_ADD("border-right"),
				F_PROPERTIES_ADD("border-right-color"),
				F_PROPERTIES_ADD("border-right-style"),
				F_PROPERTIES_ADD("border-right-width"),

				F_PROPERTIES_ADD("border-bottom"),
				F_PROPERTIES_ADD("border-bottom-color"),
				F_PROPERTIES_ADD("border-bottom-style"),
				F_PROPERTIES_ADD("border-bottom-width"),

				F_PROPERTIES_ADD("border-left"),
				F_PROPERTIES_ADD("border-left-color"),
				F_PROPERTIES_ADD("border-left-style"),
				F_PROPERTIES_ADD("border-left-width"),

				// frame

				F_PROPERTIES_ADD("border-frame"),

				// caption

				F_PROPERTIES_ADD("caption-position"),
				F_PROPERTIES_ADD("caption-preparse"),
				F_PROPERTIES_ADD("caption-font"),

				/* THE FOLLOWING ARE DEPRECATED */

				F_PROPERTIES_ADD("frame"),
				F_PROPERTIES_ADD("frame-id"),
				F_PROPERTIES_ADD("frame-preparse"),
				F_PROPERTIES_ADD("frame-position"),
				F_PROPERTIES_ADD("frame-font"),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Border_New,     FM_New),
				F_METHODS_ADD_STATIC(Border_Dispose, FM_Dispose),
				F_METHODS_ADD_STATIC(Border_Get,     FM_Get),
				F_METHODS_ADD_STATIC(Border_Set,     FM_Set),

				F_METHODS_ADD_STATIC(Border_Setup,               FM_Element_Setup),
				F_METHODS_ADD_STATIC(Border_Cleanup,             FM_Element_Cleanup),
				F_METHODS_ADD_STATIC(Border_CreateDecodedStyle,  FM_Element_CreateDecodedStyle),

//				  F_METHODS_ADD_STATIC(Border_AskMinMax,   FM_Area_AskMinMax),
				F_METHODS_ADD_STATIC(Border_Draw,        FM_Area_Draw),
				F_METHODS_ADD_STATIC(Border_SetState,    FM_Area_SetState),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Area),

				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_ATOMS,
				F_TAGS_ADD_PROPERTIES,
				F_TAGS_ADD_PROPERTIES_SIZE(0),

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
};
