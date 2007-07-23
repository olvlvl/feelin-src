#include "Private.h"

#if 0

STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("checkbox",   "back",     FV_TYPE_STRING, "Spec", NULL),
	F_PREFERENCES_ADD("checkbox",   "frame",    FV_TYPE_STRING, "Spec", NULL),
	F_PREFERENCES_ADD("checkbox",   "color-scheme",   FV_TYPE_STRING, "Spec", NULL),
	F_PREFERENCES_ADD("checkbox",   "image",    FV_TYPE_STRING, "Spec", "<image type='brush' src='defaults/checkmark.fb0' />" F_IMAGEDISPLAY_SEPARATOR "<image type='brush' src='defaults/checkmark.fb1' />"),

	F_PREFERENCES_ADD("radio-button",   "back",     FV_TYPE_STRING, "Spec", NULL),
	F_PREFERENCES_ADD("radio-button",   "color-scheme",   FV_TYPE_STRING, "Spec", NULL),
	F_PREFERENCES_ADD("radio-button",   "image",    FV_TYPE_STRING, "Spec", "<image type='brush' src='defaults/radio.fb0' />" F_IMAGEDISPLAY_SEPARATOR "<image type='brush' src='defaults/radio.fb1' />"),
		
	F_ARRAY_END
};

F_METHOD(uint32,Prefs_New)
{
	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Images",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/images.xml",

	TAG_MORE,Msg);
}

#endif
