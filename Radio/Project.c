/*

$VER: 02.10 (2006/02/20)

	Uses Item objects instead of the deprecated FC_RadioItem

$VER: 02.00 (2005/12/24)
				
	Radio items are now FC_RadioItem objects, added as children.

		<Radio>
			<RadioEntry Label="Retro" Image="<image src=..." />
			<RadioEntry Label="Cold" Image="<image src=..." />
			<RadioEntry Label="Crystal" Image="<image src=..." />
		</Radio>
	
	The FA_Radio_Entries and FA_Radio_EntriesType attributes are deprecated.
	Many new features are available now.

	Added the special "Next", "Prev",  "Last"  and  "First"  values  to  the
	FA_Radio_Active attribute.
	
	Added a special layout mode for group in rows.
 
$VER: 01.00 ( 2004/12/03)

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Radio_New);
F_METHOD_PROTO(void,Radio_Dispose);
F_METHOD_PROTO(void,Radio_Set);
F_METHOD_PROTO(void,Radio_Get);
F_METHOD_PROTO(void,Radio_AddMember);
F_METHOD_PROTO(void,Radio_RemMember);
//F_METHOD_PROTO(void,Radio_SetState);
F_METHOD_PROTO(void,Radio_Activate);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(Active) =
			{
				F_VALUES_ADD("next", FV_Radio_Active_Next),
				F_VALUES_ADD("prev", FV_Radio_Active_Prev),
				F_VALUES_ADD("last", FV_Radio_Active_Last),
				F_VALUES_ADD("first", FV_Radio_Active_First),

				F_ARRAY_END
			};
			
			STATIC F_VALUES_ARRAY(Layout) =
			{
				F_VALUES_ADD("none", FV_Radio_Layout_None),
				F_VALUES_ADD("above", FV_Radio_Layout_Above),
				F_VALUES_ADD("below", FV_Radio_Layout_Below),
				
				F_ARRAY_END
			};

			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_VALUES("Active", FV_TYPE_INTEGER, Active),

				F_ATTRIBUTES_ADD("PreParse", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("AltPreParse", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD_VALUES("Layout", FV_TYPE_INTEGER, Layout),

				F_ARRAY_END
			};

			STATIC F_AUTOS_ARRAY =
			{
				F_AUTOS_ADD("FA_Item_Label"),
				F_AUTOS_ADD("FA_Item_Image"),
				F_AUTOS_ADD("FA_Item_AltImage"),
				F_AUTOS_ADD("FA_Item_PreParse"),
				F_AUTOS_ADD("FA_Item_AltPreParse"),

				F_ARRAY_END
			};
			
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(Radio_Activate, "Private"),

				F_METHODS_ADD_STATIC(Radio_New,         FM_New),
				F_METHODS_ADD_STATIC(Radio_Dispose,     FM_Dispose),
				F_METHODS_ADD_STATIC(Radio_Set,         FM_Set),
				F_METHODS_ADD_STATIC(Radio_Get,         FM_Get),
				F_METHODS_ADD_STATIC(Radio_AddMember,   FM_AddMember),
				F_METHODS_ADD_STATIC(Radio_RemMember,   FM_RemMember),

//				  F_METHODS_ADD_STATIC(Radio_SetState,    FM_Area_SetState),

				F_ARRAY_END
			};
						 
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Group),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_AUTOS,

				F_ARRAY_END
			};
 
			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
