/*

$VER: 01.00 (2005/12/08)

	This class is used to decode CSS source. Currently the class is  limited
	to reading.
	
*/

#include "Project.h"

///METHODS
//F_METHOD_PROTO(void,CSSDocument_Dispose);
F_METHOD_PROTO(void,CSSDocument_Get);
F_METHOD_PROTO(void,CSSDocument_Parse);
F_METHOD_PROTO(void,CSSDocument_Clear);
F_METHOD_PROTO(void,CSSDocument_Write);
F_METHOD_PROTO(void,CSSDocument_GetProperty);
F_METHOD_PROTO(void,CSSDocument_AddProperty);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(CSSDocument_GetProperty, "GetProperty"),
				F_METHODS_ADD(CSSDocument_AddProperty, "AddProperty"),

				F_METHODS_OVERRIDE(CSSDocument_Parse, "Document", "Parse"),
				F_METHODS_OVERRIDE(CSSDocument_Clear, "Document", "Clear"),
				F_METHODS_OVERRIDE(CSSDocument_Write, "Document", "Write"),

				F_METHODS_OVERRIDE_STATIC(CSSDocument_Get, FM_Get),
 
				F_ARRAY_END
			};
			
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD("Declarations", FV_TYPE_POINTER),
				
				F_ARRAY_END
			};


			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FM_Document_Numerify"),
				F_RESOLVEDS_ADD("FM_Document_Stringify"),

				F_ARRAY_END
			};

 
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Document),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_RESOLVEDS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
