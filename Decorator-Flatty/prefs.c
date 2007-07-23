#include "Private.h"

#if 0

STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("decorator-flatty",   "sizebar-height", FV_TYPE_INTEGER, "Value", (STRPTR) DEF_SIZEBAR_HEIGHT),
	F_PREFERENCES_ADD("decorator-flatty",   "sizebar-back", FV_TYPE_STRING, "Spec", DEF_SIZEBAR_BACK),

	F_PREFERENCES_ADD("decorator-flatty-bar",   "back", FV_TYPE_STRING, "Spec", DEF_DRAGBAR_BACK),
	F_PREFERENCES_ADD("decorator-flatty-bar",   "spacing-horizontal", FV_TYPE_INTEGER, "Value", (STRPTR) DEF_SPACING_HORIZONTAL),
	F_PREFERENCES_ADD("decorator-flatty-bar",   "spacing-vertical", FV_TYPE_INTEGER, "Value", (STRPTR) DEF_SPACING_VERTICAL),
//    F_PREFERENCES_ADD("decorator-flatty-bar",       "frame", FV_TYPE_STRING, "Spec",
	F_PREFERENCES_ADD("decorator-flatty-close",     "image", FV_TYPE_STRING, "Spec", DEF_IMAGE_CLOSE),
	F_PREFERENCES_ADD("decorator-flatty-zoom",      "image", FV_TYPE_STRING, "Spec", DEF_IMAGE_ZOOM),
	F_PREFERENCES_ADD("decorator-flatty-depth",     "image", FV_TYPE_STRING, "Spec", DEF_IMAGE_DEPTH),

	F_ARRAY_END
};

///Prefs_New
F_METHOD(uint32,Prefs_New)
{
	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Flatty",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/decorator-flatty.xml",

	TAG_MORE,Msg);
}
//+

#endif
