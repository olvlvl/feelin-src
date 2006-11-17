#include "Private.h"

#include <feelin/preference.h>

STATIC F_PREFERENCES_ARRAY =
{
    F_PREFERENCES_ADD("button", "back",         FV_TYPE_STRING, "Spec", "fill" F_IMAGEDISPLAY_SEPARATOR "halfshadow"),
    F_PREFERENCES_ADD("button", "font",         FV_TYPE_STRING, "Contents", NULL),
    F_PREFERENCES_ADD("button", "frame",        FV_TYPE_STRING, "Spec",     "<frame id='#23' padding-width='6' /><frame id='#24' padding-left='7' padding-right='6' padding-top='1' />"),
    F_PREFERENCES_ADD("button", "color-scheme", FV_TYPE_STRING, "Spec",     NULL),
    
    F_PREFERENCES_ADD("textinfo", "back",           FV_TYPE_STRING, "Spec",     "halfshadow"),
    F_PREFERENCES_ADD("textinfo", "font",           FV_TYPE_STRING, "Contents", NULL),
    F_PREFERENCES_ADD("textinfo", "frame",          FV_TYPE_STRING, "Spec",     "<frame id='#28' padding-width='2' padding-height='1' /><frame id='#27' padding-width='2' padding-height='1' />"),
    F_PREFERENCES_ADD("textinfo", "color-scheme",   FV_TYPE_STRING, "Spec",     NULL),
 
    F_ARRAY_END
};

F_METHOD(uint32,Prefs_New)
{
    F_ENTITIES_ARRAY =
    {
        F_ENTITIES_ADD("locale.button",     "Button"),
        F_ENTITIES_ADD("locale.font",       "Font"),
        F_ENTITIES_ADD("locale.frame",      "Frame"),
        F_ENTITIES_ADD("locale.back",       "Back"),
        F_ENTITIES_ADD("locale.scheme",     "Scheme"),
        F_ENTITIES_ADD("locale.example",    "Example"),
        F_ENTITIES_ADD("locale.text",       "Text"),
                        
        F_ARRAY_END
    };
 
    return IFEELIN F_SuperDo(Class,Obj,Method,

        FA_Group_Name, "Area",

        "Script", F_PREFERENCES_PTR,
        "Source", "feelin/preference/area.xml",
        "Entities", F_ENTITIES_PTR,
          
    TAG_MORE,Msg);
}

