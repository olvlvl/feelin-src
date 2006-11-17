/*

$VER: 01.00 (2006/05/27)
 
	This class add input as well as DnD capabilities to the Border class.

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Widget_New);
F_METHOD_PROTO(void,Widget_Get);
F_METHOD_PROTO(void,Widget_Set);

F_METHOD_PROTO(void,Widget_Setup);
F_METHOD_PROTO(void,Widget_Cleanup);
F_METHOD_PROTO(void,Widget_LoadPersistentAttributes);
F_METHOD_PROTO(void,Widget_SavePersistentAttributes);

F_METHOD_PROTO(void,Widget_AskMinMax);
F_METHOD_PROTO(void,Widget_Show);
F_METHOD_PROTO(void,Widget_Hide);
F_METHOD_PROTO(void,Widget_SetState);

F_METHOD_PROTO(void,Widget_HandleEvent);
F_METHOD_PROTO(void,Widget_ModifyEvents);
F_METHOD_PROTO(void,Widget_WhichObject);
F_METHOD_PROTO(void,Widget_BuildContextMenu);
F_METHOD_PROTO(void,Widget_BuildContextHelp);
F_METHOD_PROTO(void,Widget_DnDDo);
F_METHOD_PROTO(void,Widget_DnDQuery);
F_METHOD_PROTO(void,Widget_DnDBegin);
F_METHOD_PROTO(void,Widget_DnDFinish);
F_METHOD_PROTO(void,Widget_DnDReport);
F_METHOD_PROTO(void,Widget_DnDDrop);

F_METHOD_PROTO(void,Prefs_New);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(Mode) =
			{
				F_VALUES_ADD("inert",       FV_Widget_Mode_Inert),
				F_VALUES_ADD("immediate",   FV_Widget_Mode_Immediate),
				F_VALUES_ADD("release",     FV_Widget_Mode_Release),
				F_VALUES_ADD("toggle",      FV_Widget_Mode_Toggle),

				F_ARRAY_END
			};

			STATIC F_VALUES_ARRAY(Align) =
			{
				F_VALUES_ADD("center",      FV_Widget_Align_Center),
				F_VALUES_ADD("start",       FV_Widget_Align_Start),
				F_VALUES_ADD("end",         FV_Widget_Align_End),

				F_ARRAY_END
			};

			STATIC F_VALUES_ARRAY(SetMinMax) =
			{
				F_VALUES_ADD("none",     FV_Widget_SetNone),
				F_VALUES_ADD("both",     FV_Widget_SetBoth),
				F_VALUES_ADD("width",    FV_Widget_SetWidth),
				F_VALUES_ADD("height",   FV_Widget_SetHeight),

				F_ARRAY_END
			};

			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_STATIC("PublicData",       FV_TYPE_POINTER, FA_Widget_PublicData),
				F_ATTRIBUTES_ADD_STATIC_VALUES("Align",     FV_TYPE_INTEGER, FA_Widget_Align, Align),
				F_ATTRIBUTES_ADD_STATIC_VALUES("Mode",      FV_TYPE_INTEGER, FA_Widget_Mode, Mode),
				F_ATTRIBUTES_ADD_STATIC("Weight",           FV_TYPE_INTEGER, FA_Widget_Weight),
				F_ATTRIBUTES_ADD_STATIC("Chainable",        FV_TYPE_BOOLEAN, FA_Widget_Chainable),
				F_ATTRIBUTES_ADD_STATIC("Active",           FV_TYPE_BOOLEAN, FA_Widget_Active),
				F_ATTRIBUTES_ADD_STATIC("Disabled",         FV_TYPE_BOOLEAN, FA_Widget_Disabled),
				F_ATTRIBUTES_ADD_STATIC("Hidden",           FV_TYPE_BOOLEAN, FA_Widget_Hidden),
				F_ATTRIBUTES_ADD_STATIC("Pressed",          FV_TYPE_BOOLEAN, FA_Widget_Pressed),
				F_ATTRIBUTES_ADD_STATIC("PressedTick",      FV_TYPE_INTEGER, FA_Widget_PressedTick),
				F_ATTRIBUTES_ADD_STATIC("Selected",         FV_TYPE_BOOLEAN, FA_Widget_Selected),
				F_ATTRIBUTES_ADD_STATIC("Draggable",        FV_TYPE_BOOLEAN, FA_Widget_Draggable),
				F_ATTRIBUTES_ADD_STATIC("Dropable",         FV_TYPE_BOOLEAN, FA_Widget_Dropable),
				F_ATTRIBUTES_ADD_STATIC("Accel",            FV_TYPE_INTEGER, FA_Widget_Accel),
				F_ATTRIBUTES_ADD_STATIC_VALUES("SetMin", 	FV_TYPE_INTEGER, FA_Widget_SetMin, SetMinMax),
				F_ATTRIBUTES_ADD_STATIC_VALUES("SetMax", 	FV_TYPE_INTEGER, FA_Widget_SetMax, SetMinMax),

				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD("selected"),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Widget_New,          FM_New),
				F_METHODS_ADD_STATIC(Widget_Get,          FM_Get),
				F_METHODS_ADD_STATIC(Widget_Set,          FM_Set),

				F_METHODS_ADD_STATIC(Widget_Setup,                    FM_Element_Setup),
				F_METHODS_ADD_STATIC(Widget_Cleanup,                  FM_Element_Cleanup),
				F_METHODS_ADD_STATIC(Widget_LoadPersistentAttributes, FM_Element_LoadPersistentAttributes),
				F_METHODS_ADD_STATIC(Widget_SavePersistentAttributes, FM_Element_SavePersistentAttributes),

				F_METHODS_ADD_STATIC(Widget_AskMinMax,  	FM_Area_AskMinMax),
				F_METHODS_ADD_STATIC(Widget_Show,         	FM_Area_Show),
				F_METHODS_ADD_STATIC(Widget_Hide,         	FM_Area_Hide),

				F_METHODS_ADD_BOTH(Widget_HandleEvent,        "HandleEvent",      FM_Widget_HandleEvent),
				F_METHODS_ADD_BOTH(Widget_ModifyEvents,       "ModifyEvents",     FM_Widget_ModifyEvents),
/*
				F_METHODS_ADD_BOTH(Widget_BuildContextMenu,   "BuildContextMenu", FM_BuildContextMenu),
				F_METHODS_ADD_BOTH(Widget_BuildContextHelp,   "BuildContextHelp", FM_BuildContextHelp),
*/
				F_METHODS_ADD_BOTH(Widget_DnDDo,              "DnDDo",            FM_Widget_DnDDo),
				F_METHODS_ADD_BOTH(Widget_DnDQuery,           "DnDQuery",         FM_Widget_DnDQuery),
				F_METHODS_ADD_BOTH(Widget_DnDBegin,           "DnDBegin",         FM_Widget_DnDBegin),
				F_METHODS_ADD_BOTH(Widget_DnDFinish,          "DnDFinish",        FM_Widget_DnDFinish),
				F_METHODS_ADD_BOTH(Widget_DnDReport,          "DnDReport",        FM_Widget_DnDReport),
				
				F_METHODS_ADD_BOTH(Widget_DnDDrop,            "DnDDrop",          FM_Widget_DnDDrop),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Border),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_ATOMS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
   }
   return NULL;
}
