/*

$VER: 03.00 (2006/10/20)

	Atoms support.

	The special variables "_self" is now replaced by  "this",  "_parent"  by
	"parent",   "_window"   by   "window"   and   finaly  "_application"  by
	"application".

$VER: 02.10 (2005/12/11)
 
	Portability update

$VER: 02.00 (2005/03/15)

	Subclass of FC_XMLObject

	Custom classes can now be used within XML files. All you have to  do  is
	define  your  custom  class in FXMLDefinition, then use the same name in
	the XML file.
 
$VER: 01.10 (2005/02/02)
 
	Improved object collection management. F_MakeObj() is now used  as  much
	as possible.
 
*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,XMLApplication_New);
F_METHOD_PROTO(void,XMLApplication_Dispose);
F_METHOD_PROTO(void,XMLApplication_Build);
F_METHOD_PROTO(void,XMLApplication_CreateNotify);
F_METHOD_PROTO(void,XMLApplication_Run);

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
				F_METHODS_ADD(XMLApplication_CreateNotify, "CreateNotify"),
				F_METHODS_ADD(XMLApplication_Run, "Run"),
				F_METHODS_OVERRIDE(XMLApplication_Build, "XMLObject", "Build"),

				F_METHODS_OVERRIDE_STATIC(XMLApplication_New, FM_New),
				F_METHODS_OVERRIDE_STATIC(XMLApplication_Dispose, FM_Dispose),

				F_ARRAY_END
			};

			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FM_Document_Resolve"),
				F_RESOLVEDS_ADD("FM_Document_Log"),
				#ifndef F_NEW_GETELEMENTBYID
				F_RESOLVEDS_ADD("FM_XMLObject_GetObjects"),
				#endif

				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD(F_XMLAPPLICATION_APPLICATION),
				F_ATOMS_ADD(F_XMLAPPLICATION_ATTRIBUTE),
				F_ATOMS_ADD(F_XMLAPPLICATION_MESSAGE),
				F_ATOMS_ADD(F_XMLAPPLICATION_METHOD),
				F_ATOMS_ADD(F_XMLAPPLICATION_NOTIFY),
				F_ATOMS_ADD(F_XMLAPPLICATION_OBJ),
				F_ATOMS_ADD(F_XMLAPPLICATION_ROOT),
				F_ATOMS_ADD(F_XMLAPPLICATION_TARGET),
				F_ATOMS_ADD(F_XMLAPPLICATION_VALUE),
				F_ATOMS_ADD(F_XMLAPPLICATION_STYLESHEET),

				F_ATOMS_ADD("href"),
				F_ATOMS_ADD("type"),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(XMLObject),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
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
