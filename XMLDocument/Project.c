/*

$VER: 04.00 (2006/02/08)

	<!ENTITY > markup support.

	The 'Entities' attributes can now be defined  multiple  time  at  object
	creation time, each of them resulting in more entities added.

	<? ?> processing instruction support.

$VER: 03.00 (2005/12/11)
 
	Portability update.
	
	Implement FM_Document_Clear.
	
	The FXMLAttribute structure has been extended to hold a numeric value of
	the  attribute  data and its type. A numeric value of the attribute data
	is now resolved while the FXMLAttribute structure is created.

	"Definitions" are no longer a trick, they are now 'real'  entities  e.g.
	"&label.quit;".  The  substitution is performed over every string copied
	e.g.

		<Button>&label.quit;</Button>
		<Window Title="Feelin : &window.title; (&application.version;.&application.revision;)" />

	The FXMLDefinition is replaced by FDOCEntity
 
*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,XMLDocument_New);
F_METHOD_PROTO(void,XMLDocument_Get);
F_METHOD_PROTO(void,XMLDocument_Parse);
F_METHOD_PROTO(void,XMLDocument_Clear);

F_METHOD_PROTO(void,XMLDocument_Push);
F_METHOD_PROTO(void,XMLDocument_Pop);
F_METHOD_PROTO(void,XMLDocument_Add);
F_METHOD_PROTO(void,XMLDocument_AddInt);
F_METHOD_PROTO(void,XMLDocument_AddEntity);
F_METHOD_PROTO(void,XMLDocument_AddEntities);
F_METHOD_PROTO(void,XMLDocument_FindEntity);

F_METHOD_PROTO(void,XMLDocument_Write);
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
				F_ATTRIBUTES_ADD("Markups", 		FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Attributes", 		FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Instructions", 	FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Entities", 		FV_TYPE_POINTER),
				
				F_ARRAY_END
			};
								
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD       (XMLDocument_Push,          "Push"),
				F_METHODS_ADD       (XMLDocument_Pop,           "Pop"),
				F_METHODS_ADD       (XMLDocument_Add,           "Add"),
				F_METHODS_ADD       (XMLDocument_AddInt,        "AddInt"),
				F_METHODS_ADD       (XMLDocument_AddEntity,     "AddEntity"),
				F_METHODS_ADD       (XMLDocument_AddEntities,   "AddEntities"),
				F_METHODS_ADD       (XMLDocument_FindEntity,    "FindEntity"),
				F_METHODS_ADD       (XMLDocument_Write,         "FM_Document_Write"),
				
				F_METHODS_ADD       (XMLDocument_Parse,         "FM_Document_Parse"),
				F_METHODS_ADD       (XMLDocument_Clear,         "FM_Document_Clear"),
				F_METHODS_ADD_STATIC(XMLDocument_Get,            FM_Get),
				F_METHODS_ADD_STATIC(XMLDocument_New,            FM_New),
				
				F_ARRAY_END
			};
			
			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FA_Document_Pool"),
				F_RESOLVEDS_ADD("FA_Document_Source"),
				F_RESOLVEDS_ADD("FA_Document_SourceType"),
				F_RESOLVEDS_ADD("FA_Document_Version"),
				F_RESOLVEDS_ADD("FA_Document_Revision"),
				F_RESOLVEDS_ADD("FM_Document_Read"),
				F_RESOLVEDS_ADD("FM_Document_Resolve"),
				F_RESOLVEDS_ADD("FM_Document_Log"),
				F_RESOLVEDS_ADD("FM_Document_Numerify"),
				F_RESOLVEDS_ADD("FM_Document_CreatePool"),
				
				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD(F_XMLDOCUMENT_VERSION),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Document),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_RESOLVEDS,
				F_TAGS_ADD_ATOMS,
				
				F_ARRAY_END
			};
 
			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
