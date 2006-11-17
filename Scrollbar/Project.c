/*

$VER: 02.00 (2006/04/29)

    Styles support.

$VER: 01.10 (2005/08/10)
 
    Portability update
 
$VER: 01.02 (2005/04/27)
 
    The new attribute FA_Prop_Useless is now used to hide the object when it
    is useless.
 
$VER: 01.00 (2004/12/18)

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,SB_New);
F_METHOD_PROTO(void,SB_Dispose);
F_METHOD_PROTO(void,SB_Set);
F_METHOD_PROTO(void,SB_Get);
F_METHOD_PROTO(void,SB_Setup);
F_METHOD_PROTO(void,SB_Cleanup);

F_METHOD_PROTO(void,Prefs_New);
F_METHOD_PROTO(void,Prefs_Load);
F_METHOD_PROTO(void,Prefs_Save);
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
                F_ATTRIBUTES_ADD("Entries",  FV_TYPE_INTEGER),
                F_ATTRIBUTES_ADD("First",    FV_TYPE_INTEGER),
                F_ATTRIBUTES_ADD("Visible",  FV_TYPE_INTEGER),

                F_ARRAY_END
            };

            STATIC F_METHODS_ARRAY =
            {
                F_METHODS_ADD_STATIC(SB_New,       FM_New),
                F_METHODS_ADD_STATIC(SB_Dispose,   FM_Dispose),
                F_METHODS_ADD_STATIC(SB_Set,       FM_Set),
                F_METHODS_ADD_STATIC(SB_Get,       FM_Get),

                F_METHODS_ADD_STATIC(SB_Setup,     FM_Element_Setup),
                F_METHODS_ADD_STATIC(SB_Cleanup,   FM_Element_Cleanup),

                F_ARRAY_END
            };

            STATIC F_PROPERTIES_ARRAY =
            {
                F_PROPERTIES_ADD("type"),

                F_ARRAY_END
            };

            STATIC F_TAGS_ARRAY =
            {
                F_TAGS_ADD_LOD,
                F_TAGS_ADD_SUPER(Group),
                F_TAGS_ADD_METHODS,
                F_TAGS_ADD_ATTRIBUTES,
                F_TAGS_ADD_PROPERTIES,

                F_ARRAY_END
            };

            return F_TAGS_PTR;
        }
//+
///Prefs
        case FV_Query_PrefsTags:
        {
            STATIC F_METHODS_ARRAY =
            {
                F_METHODS_ADD(Prefs_Load, "FM_PreferenceGroup_Load"),
                F_METHODS_ADD(Prefs_Load, "FM_PreferenceGroup_Save"),

                F_METHODS_ADD_STATIC(Prefs_New, FM_New),

                F_ARRAY_END
            };

            STATIC F_TAGS_ARRAY =
            {
                F_TAGS_ADD_SUPER(PreferenceGroup),
                F_TAGS_ADD(LODSize, sizeof (struct LocalPreferenceData)),
                F_TAGS_ADD_METHODS,

                F_ARRAY_END
            };

            return F_TAGS_PTR;
        }
//+
    }
    return NULL;
}
