/*

$VER: 01.02 (2005/12/08)
 
	Portability update
 
$VER: 01.00 (2005/05/05)
 
	Base class for dialog window

*/

#include "Project.h"
#include "_locale/table.h"

///METHODS
F_METHOD_PROTO(void,Dialog_New);
F_METHOD_PROTO(void,Dialog_Open);
F_METHOD_PROTO(void,Dialog_Close);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(Buttons) =
			{
				F_VALUES_ADD("none",FV_Dialog_Buttons_None),
				F_VALUES_ADD("ok",FV_Dialog_Buttons_Ok),
				F_VALUES_ADD("boolean",FV_Dialog_Buttons_Boolean),
				F_VALUES_ADD("confirm",FV_Dialog_Buttons_Confirm),
				F_VALUES_ADD("always",FV_Dialog_Buttons_Always),
				F_VALUES_ADD("preference",FV_Dialog_Buttons_Preference),
				F_VALUES_ADD("preference-test",FV_Dialog_Buttons_PreferenceTest),

				F_ARRAY_END
			};

			STATIC F_VALUES_ARRAY(Response) =
			{
				F_VALUES_ADD("none",FV_Dialog_Response_None),
				F_VALUES_ADD("ok",FV_Dialog_Response_Ok),
				F_VALUES_ADD("cancel",FV_Dialog_Response_Cancel),
				F_VALUES_ADD("apply",FV_Dialog_Response_Apply),
				F_VALUES_ADD("yes",FV_Dialog_Response_Yes),
				F_VALUES_ADD("no",FV_Dialog_Response_No),
				F_VALUES_ADD("all",FV_Dialog_Response_All),
				F_VALUES_ADD("save",FV_Dialog_Response_Save),
				F_VALUES_ADD("use",FV_Dialog_Response_Use),
				
				F_ARRAY_END
			};
			
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_VALUES("Buttons", FV_TYPE_INTEGER, Buttons),
				F_ATTRIBUTES_ADD("Separator", FV_TYPE_BOOLEAN),
				F_ATTRIBUTES_ADD_VALUES("Response", FV_TYPE_INTEGER, Response),
				F_ATTRIBUTES_ADD("ApplyText", FV_TYPE_STRING),
				
				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Dialog_New, FM_New),
				F_METHODS_ADD_STATIC(Dialog_Open, FM_Window_Open),
				F_METHODS_ADD_STATIC(Dialog_Close, FM_Window_Close),
			
				F_ARRAY_END
			};
			
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Window),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_CATALOG,
				
				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
