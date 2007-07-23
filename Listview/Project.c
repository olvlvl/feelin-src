/*

$VER: 02.02 (2007/07/24)

	The SetState method is now forwarded to the List object.

$VER: 02.00 (2006/05/03)

	The 'List' attribute is  deprecated.  The  class  support  the  FA_Child
	attribute instead.

$VER: 01.02 (2005/12/11)
 
	Portability update
 
$VER: 01.00 (2004/12/18)

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void, Listview_New);
F_METHOD_PROTO(void, Listview_SetState);
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
				F_METHODS_OVERRIDE_STATIC(Listview_New, FM_New),
				F_METHODS_OVERRIDE_STATIC(Listview_SetState, FM_Area_SetState),
			
				F_ARRAY_END
			};

			STATIC F_AUTOS_ARRAY =
			{
				F_AUTOS_ADD("FA_Scrollbar_Entries"),
				F_AUTOS_ADD("FA_Scrollbar_Visible"),
				F_AUTOS_ADD("FA_Scrollbar_First"),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Group),
				F_TAGS_ADD_LOD,
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
