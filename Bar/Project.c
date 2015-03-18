/*

$VER: 04.00 (2006/05/30)

    Widget subclass.

    CSS support.

$VER: 03.10 (2005/12/08)
 
    Bar_New() was using the end of the taglist with TAG_MORE.
 
$VER: 03.00 (2005/05/13)
 
    Added preference item  $bar-font,  $bar-preparse,  $bar-color-shine  and
    $bar-color-shadow.
    
$VER: 02.00 (2004/09/12)
 
*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,Bar_New);
F_METHOD_PROTO(void,Bar_Get);
F_METHOD_PROTO(void,Bar_Setup);
F_METHOD_PROTO(void,Bar_Cleanup);
F_METHOD_PROTO(void,Bar_AskMinMax);
F_METHOD_PROTO(void,Bar_Draw);
F_METHOD_PROTO(void,Prefs_New);
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
                F_ATTRIBUTES_ADD("Title", FV_TYPE_STRING),

                F_ARRAY_END
            };

            STATIC F_PROPERTIES_ARRAY =
            {
                F_PROPERTIES_ADD("preparse"),
                F_PROPERTIES_ADD("color-shine"),
                F_PROPERTIES_ADD("color-shadow"),

                F_ARRAY_END
            };

            STATIC F_METHODS_ARRAY =
            {
                F_METHODS_ADD_STATIC(Bar_New,         FM_New),
                F_METHODS_ADD_STATIC(Bar_Get,         FM_Get),
                F_METHODS_ADD_STATIC(Bar_Setup,       FM_Element_Setup),
                F_METHODS_ADD_STATIC(Bar_Cleanup,     FM_Element_Cleanup),
                F_METHODS_ADD_STATIC(Bar_AskMinMax,   FM_Area_AskMinMax),
                F_METHODS_ADD_STATIC(Bar_Draw,        FM_Area_Draw),

                F_ARRAY_END
            };

            STATIC F_TAGS_ARRAY =
            {
                F_TAGS_ADD_SUPER(Widget),
                F_TAGS_ADD_LOD,
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
                F_METHODS_ADD_STATIC(Prefs_New, FM_New),

                F_ARRAY_END
            };

            STATIC F_TAGS_ARRAY =
            {
                F_TAGS_ADD_SUPER(PreferenceGroup),
                F_TAGS_ADD_METHODS,

                F_ARRAY_END
            };

            return F_TAGS_PTR;
        }
//+
    }
    return NULL;
}
//+

