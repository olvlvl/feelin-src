/*

$VER: 04.00 (2006/08/09)

	Widget subclass.

	No built-in frame anymore.

$VER: 03.00 (2005/12/08)
 
	Support for the new "damaged" rendering technique.

$VER: 02.02 (2005/05/19)
 
	The method FM_RethinkLayout is used instead of custom  functions,  which
	results in better update and above all neat code :-)
 
$VER: 02.00 (2004/09/12)

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Balance_New);
F_METHOD_PROTO(void,Balance_Set);

F_METHOD_PROTO(void,Balance_Setup);
F_METHOD_PROTO(void,Balance_Cleanup);
F_METHOD_PROTO(void,Balance_LoadPersistentAttributes);
F_METHOD_PROTO(void,Balance_SavePersistentAttributes);

F_METHOD_PROTO(void,Balance_AskMinMax);
F_METHOD_PROTO(void,Balance_Draw);
F_METHOD_PROTO(void,Balance_SetState);

F_METHOD_PROTO(void,Balance_HandleEvent);
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
				F_ATTRIBUTES_ADD("QuickDraw", FV_TYPE_BOOLEAN),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Balance_New,         FM_New),
				F_METHODS_ADD_STATIC(Balance_Set,         FM_Set),

				F_METHODS_ADD_STATIC(Balance_Setup,       FM_Element_Setup),
				F_METHODS_ADD_STATIC(Balance_Cleanup,     FM_Element_Cleanup),
				F_METHODS_ADD_STATIC(Balance_LoadPersistentAttributes, FM_Element_LoadPersistentAttributes),
				F_METHODS_ADD_STATIC(Balance_SavePersistentAttributes, FM_Element_SavePersistentAttributes),

				F_METHODS_ADD_STATIC(Balance_AskMinMax,     FM_Area_AskMinMax),
				F_METHODS_ADD_STATIC(Balance_Draw,          FM_Area_Draw),
				F_METHODS_ADD_STATIC(Balance_SetState,      FM_Area_SetState),

				F_METHODS_ADD_STATIC(Balance_HandleEvent,   FM_Widget_HandleEvent),

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
	}
	return NULL;
}
