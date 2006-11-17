/*
 
*************************************************************************************************
 
$VER: 01.00 (2006/03/31)

************************************************************************************************/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Element_New);
F_METHOD_PROTO(void,Element_Dispose);
F_METHOD_PROTO(void,Element_Get);
F_METHOD_PROTO(void,Element_Connect);
F_METHOD_PROTO(void,Element_Disconnect);

#ifdef F_NEW_GLOBALCONNECT
F_METHOD_PROTO(void,Element_GlobalConnect);
F_METHOD_PROTO(void,Element_GlobalDisconnect);
#endif
F_METHOD_PROTO(void,Element_Setup);
F_METHOD_PROTO(void,Element_Cleanup);
F_METHOD_PROTO(void,Element_CreateDecodedStyle);
F_METHOD_PROTO(void,Element_DeleteDecodedStyle);
F_METHOD_PROTO(void,Element_LoadPersistentAttributes);
F_METHOD_PROTO(void,Element_SavePersistentAttributes);
F_METHOD_PROTO(void,Element_GetProperty);
//F_METHOD_PROTO(void,Element_GetProperties);
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
				#ifdef F_NEW_ELEMENT_ID
				F_ATTRIBUTES_ADD_STATIC("ID", FV_TYPE_STRING, FA_Element_ID),
				#endif

				F_ATTRIBUTES_ADD_STATIC("Class", FV_TYPE_STRING, FA_Element_Class),
				F_ATTRIBUTES_ADD_STATIC("Style", FV_TYPE_STRING, FA_Element_Style),
				F_ATTRIBUTES_ADD_STATIC("Persist", FV_TYPE_STRING, FA_Element_Persist),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Element_New,   		FM_New),
				F_METHODS_ADD_STATIC(Element_Dispose,   	FM_Dispose),
				F_METHODS_ADD_STATIC(Element_Get,   		FM_Get),
				F_METHODS_ADD_STATIC(Element_Connect,		FM_Connect),
				F_METHODS_ADD_STATIC(Element_Disconnect,	FM_Disconnect),

				#ifdef F_NEW_GLOBALCONNECT
				F_METHODS_ADD_BOTH(Element_GlobalConnect, "GlobalConnect", FM_Element_GlobalConnect),
				F_METHODS_ADD_BOTH(Element_GlobalDisconnect, "GlobalDisconnect", FM_Element_GlobalDisconnect),
				#endif

				F_METHODS_ADD_BOTH(Element_Setup, "Setup", FM_Element_Setup),
				F_METHODS_ADD_BOTH(Element_Cleanup, "Cleanup", FM_Element_Cleanup),
				F_METHODS_ADD_BOTH(Element_CreateDecodedStyle, "CreateDecodedStyle", FM_Element_CreateDecodedStyle),
				F_METHODS_ADD_BOTH(Element_DeleteDecodedStyle, "DeleteDecodedStyle", FM_Element_DeleteDecodedStyle),

				F_METHODS_ADD_BOTH(Element_LoadPersistentAttributes, "LoadPersistentAttributes", FM_Element_LoadPersistentAttributes),
				F_METHODS_ADD_BOTH(Element_SavePersistentAttributes, "SavePersistentAttributes", FM_Element_SavePersistentAttributes),
				
				F_METHODS_ADD_BOTH(Element_GetProperty, "GetProperty", FM_Element_GetProperty),

				F_ARRAY_END
			};

			STATIC F_AUTOS_ARRAY =
			{
				F_AUTOS_ADD("FM_PDRDocument_FindHandle"),
				F_AUTOS_ADD("FM_PDRDocument_CreateHandle"),

				F_AUTOS_ADD("FM_Preference_ObtainStyle"),
				F_AUTOS_ADD("FM_Preference_ReleaseStyle"),
				F_AUTOS_ADD("FM_Preference_ObtainStyleByName"),

				F_ARRAY_END
			};
			
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Object),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_AUTOS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
