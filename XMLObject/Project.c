/*

$VER: 04.00 (2006/08/10)

	Atoms support.

	The 'References' attribute can be used multiple time at object  creation
	time, each reference is proceed.

	Replaced the 'Find' method by the more handy 'GetObjects'.

$VER: 03.00 (2005/12/11)

	Since  FC_Document  introduced  entities,  FA_XMLObject_Definitions   is
	deprecated.

	The value's type of the attributes are now  checked.  Feelin  is  finaly
	becoming 'typed' (it's XML part at least).
   
*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,XMLObject_New);
F_METHOD_PROTO(void,XMLObject_Dispose);
F_METHOD_PROTO(void,XMLObject_Get);
F_METHOD_PROTO(void,XMLObject_Read);

F_METHOD_PROTO(void,XMLObject_GetObjects);

F_METHOD_PROTO(void,XMLObject_Build);
F_METHOD_PROTO(void,XMLObject_Create);
F_METHOD_PROTO(void,XMLObject_Resolve);
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
				F_ATTRIBUTES_ADD("References", FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Tags", FV_TYPE_POINTER),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(XMLObject_Read, "FM_Document_Read"),
				F_METHODS_ADD(XMLObject_Resolve, "FM_Document_Resolve"),
				F_METHODS_ADD(XMLObject_GetObjects, "GetObjects"),
				F_METHODS_ADD(XMLObject_Build, "Build"),
				F_METHODS_ADD(XMLObject_Create, "Create"),

				F_METHODS_ADD_STATIC(XMLObject_Get, FM_Get),
				F_METHODS_ADD_STATIC(XMLObject_Dispose, FM_Dispose),
				F_METHODS_ADD_STATIC(XMLObject_New, FM_New),

				F_ARRAY_END
			};

			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FM_Document_Log"),

				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD(F_XMLOBJECT_ID),
				F_ATOMS_ADD(F_XMLOBJECT_OBJECTS),

				#ifdef F_NEW_STYLES
				F_ATOMS_ADD(F_XMLOBJECT_STYLE),
				F_ATOMS_ADD(F_XMLOBJECT_CLASS),
				#endif

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(XMLDocument),
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
