/*
 
*************************************************************************************************
 
$VER: 02.00 (2007/07/08)

	The FA_ID attribute is now defined by the Element class. It was  formaly
	defined  by  the  Object  class but that was a waste since only elements
	need an Id. The attribute as be renamed as FA_Element_Id (mind the  case
	of the 'Id').

************************************************************************************************/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Element_New);
F_METHOD_PROTO(void,Element_Dispose);
F_METHOD_PROTO(void,Element_Get);
F_METHOD_PROTO(void,Element_Connect);
F_METHOD_PROTO(void,Element_Disconnect);
#ifdef F_NEW_GETELEMENTBYID
F_METHOD_PROTO(void,Element_GetElementById);
#endif

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
				F_ATTRIBUTES_ADD_STATIC("Id", FV_TYPE_STRING, FA_Element_Id),
				F_ATTRIBUTES_ADD_STATIC("Class", FV_TYPE_STRING, FA_Element_Class),
				F_ATTRIBUTES_ADD_STATIC("Style", FV_TYPE_STRING, FA_Element_Style),
				F_ATTRIBUTES_ADD_STATIC("Persist", FV_TYPE_STRING, FA_Element_Persist),
				F_ATTRIBUTES_ADD_STATIC("Parent", FV_TYPE_OBJECT, FA_Element_Parent),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_OVERRIDE_STATIC(Element_New,   		 FM_New),
				F_METHODS_OVERRIDE_STATIC(Element_Dispose,   	 FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(Element_Get,   		 FM_Get),
				F_METHODS_OVERRIDE_STATIC(Element_Connect,		 FM_Connect),
				F_METHODS_OVERRIDE_STATIC(Element_Disconnect,	 FM_Disconnect),
				#ifdef F_NEW_GETELEMENTBYID
				F_METHODS_OVERRIDE_STATIC(Element_GetElementById, FM_GetElementById),
				#endif

				#ifdef F_NEW_GLOBALCONNECT
				F_METHODS_ADD_STATIC(Element_GlobalConnect, "GlobalConnect", FM_Element_GlobalConnect),
				F_METHODS_ADD_STATIC(Element_GlobalDisconnect, "GlobalDisconnect", FM_Element_GlobalDisconnect),
				#endif

				F_METHODS_ADD_STATIC(Element_Setup, "Setup", FM_Element_Setup),
				F_METHODS_ADD_STATIC(Element_Cleanup, "Cleanup", FM_Element_Cleanup),
				F_METHODS_ADD_STATIC(Element_CreateDecodedStyle, "CreateDecodedStyle", FM_Element_CreateDecodedStyle),
				F_METHODS_ADD_STATIC(Element_DeleteDecodedStyle, "DeleteDecodedStyle", FM_Element_DeleteDecodedStyle),
				F_METHODS_ADD_STATIC(Element_LoadPersistentAttributes, "LoadPersistentAttributes", FM_Element_LoadPersistentAttributes),
				F_METHODS_ADD_STATIC(Element_SavePersistentAttributes, "SavePersistentAttributes", FM_Element_SavePersistentAttributes),
				F_METHODS_ADD_STATIC(Element_GetProperty, "GetProperty", FM_Element_GetProperty),

				F_ARRAY_END
			};

			STATIC F_AUTOS_ARRAY =
			{
				F_AUTOS_ADD("FM_PDRDocument_FindHandle"),
				F_AUTOS_ADD("FM_PDRDocument_CreateHandle"),

				F_AUTOS_ADD("FM_Preference_ObtainStyle"),
				F_AUTOS_ADD("FM_Preference_ReleaseStyle"),

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
