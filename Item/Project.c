/*

*************************************************************************************************
 
$VER: 01.00 (2006/02/20)

	This class was created to easily create items  for  FC_Radio,  FC_Cycle,
	FC_Menu... objects.

************************************************************************************************/

#include "Project.h"

struct ClassUserData               *CUD;

///METHODS
F_METHOD_PROTO(void,RadioItem_New);
F_METHOD_PROTO(void,RadioItem_Dispose);
F_METHOD_PROTO(void,RadioItem_Get);
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
				F_ATTRIBUTES_ADD("Label", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("Image", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("AltImage", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("PreParse", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("AltPreParse", FV_TYPE_STRING),
				
				F_ATTRIBUTES_ADD_STATIC("Child", FV_TYPE_OBJECT, FA_Child),
				
				F_ARRAY_END
			};
				
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(RadioItem_New, FM_New),
				F_METHODS_ADD_STATIC(RadioItem_Dispose, FM_Dispose),
				F_METHODS_ADD_STATIC(RadioItem_Get, FM_Get),
				
				F_ARRAY_END
			};
			
			STATIC F_TAGS_ARRAY =
			{
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
