/*

$VER: 04.00 (2006/06/14)

	Removed the FindName and ObtainName methods, they are  now  replaced  by
	atoms pools.

$VER: 03.00 (2006/02/10)

	Moved the 'Entity' attributes and the 'AddEntities',  'RemEntities'  and
	'FindEntity' methods to FC_XMLDocument.

	A special case is used when 'Source' and  'SourceType'  are  defined  at
	creation time, which doesn't invoke the 'Clear' method. Thus, subclasses
	may obtain the pool and allocate things from it like FC_XMLDocument does
	when its 'Entities' attribute is defined at creation time.

	Added FV_TYPE_POINTER support to FM_Document_Numerify.

$VER: 02.00 (2005/12/10)

	FM_Adjust_Resolve gives the kind of data resolved instead of TRUE.
 
	DuLockFromFH()  didn't  work  with  WinUAE   (01.00)   vitual   volumes,
	FM_Document_Read has been modified and now use Lock() instead.
	
	Added FM_Document_Merge, FM_Document_Add, FM_Document_Rem
	FM_Document_Find and FM_Document_Write and FM_Document_AddEntities,
	FM_Document_RemEntities and FM_Document_FontEntity methods.

	"entities" support, with the new FDOCEntity type, the
	FM_Document_AddEntities, FM_Document_RemEntities and
	FM_Document_FindEntity methods.

	Replaced the FM_Document_Resolve method  with  the  FM_Document_Numerify
	one.

$VER: 01.00 (2005/05/01)
 
	This is the base class of document classes.

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Document_New);
F_METHOD_PROTO(void,Document_Dispose);
F_METHOD_PROTO(void,Document_Get);
F_METHOD_PROTO(void,Document_Set);

F_METHOD_PROTO(void,Document_Read);
F_METHOD_PROTO(void,Document_Merge);
F_METHOD_PROTO(void,Document_Resolve);
#if F_CODE_DEPRECATED
F_METHOD_PROTO(void,Document_FindName);
F_METHOD_PROTO(void,Document_ObtainName);
#endif
F_METHOD_PROTO(void,Document_Log);
F_METHOD_PROTO(void,Document_Clear);

F_METHOD_PROTO(void,Document_Numerify);
F_METHOD_PROTO(void,Document_Stringify);
F_METHOD_PROTO(void,Document_CreatePool);

F_METHOD_PROTO(void,Document_Dummy);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(SourceType) =
			{
				F_VALUES_ADD("file", FV_Document_SourceType_File),
				F_VALUES_ADD("memory", FV_Document_SourceType_Memory),
			 
				F_ARRAY_END
			};

			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD("Pool", FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Source", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD_VALUES("SourceType", FV_TYPE_INTEGER, SourceType),
				F_ATTRIBUTES_ADD("Version", FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Revision", FV_TYPE_INTEGER),

				F_ARRAY_END
			};
 
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD       (Document_Read,         "Read"),
				F_METHODS_ADD       (Document_Merge,        "Merge"),
				F_METHODS_ADD       (Document_Resolve,      "Resolve"),
				#if F_CODE_DEPRECATED
				F_METHODS_ADD       (Document_FindName,     "FindName"),
				F_METHODS_ADD       (Document_ObtainName,   "ObtainName"),
				#endif
				F_METHODS_ADD       (Document_Log,          "Log"),
				F_METHODS_ADD       (Document_Clear,        "Clear"),
				
				F_METHODS_ADD       (Document_Numerify,     "Numerify"),
				F_METHODS_ADD       (Document_Stringify,    "Stringify"),
				F_METHODS_ADD       (Document_CreatePool,   "CreatePool"),
				
				F_METHODS_ADD       (Document_Dummy,        "Parse"),

				F_METHODS_ADD_STATIC(Document_New,           FM_New),
				F_METHODS_ADD_STATIC(Document_Dispose,       FM_Dispose),
				F_METHODS_ADD_STATIC(Document_Get,           FM_Get),
				F_METHODS_ADD_STATIC(Document_Set,           FM_Set),
				
				F_METHODS_ADD       (Document_Dummy,        "Add"),
				F_METHODS_ADD       (Document_Dummy,        "Remove"),
				F_METHODS_ADD       (Document_Dummy,        "Find"),
				F_METHODS_ADD       (Document_Dummy,        "Write"),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Object),
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
