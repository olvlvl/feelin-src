#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///String_New
F_METHOD(uint32,String_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;

	LOD->displayable = DEF_STRING_DISPLAYABLE;
	LOD->Flags = FF_String_AdvanceOnCR;
	LOD->Max   = 0;
	LOD->Sel   = 1;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_MAX:
		{
			LOD->Max = item.ti_Data ? item.ti_Data + 1 : 0;
		}
		break;

		case FV_ATTRIBUTE_SECRET:
		{
			if (item.ti_Data)
			{
				LOD->Flags |= FF_String_Secret;
			}
			else
			{
				LOD->Flags &= ~FF_String_Secret;
			}
		}
		break;

		case FV_ATTRIBUTE_DISPLAYABLE:
		{
			LOD->displayable = item.ti_Data;
		}
		break;
	}

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Widget_SetMax, FV_Widget_SetHeight,

	TAG_MORE,Msg);
}
//+
///String_Dispose
F_METHOD(void,String_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	IFEELIN F_Dispose(LOD->String);  LOD->String = NULL;
	IFEELIN F_Dispose(LOD->Undo);    LOD->Undo = NULL;

	F_SUPERDO();
}
//+
///String_Set
F_METHOD(void,String_Set)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item,*tag;
	
	while ((tag = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ACCEPT:
		{
			switch (item.ti_Data)
			{
				case FV_String_Decimal:
				{
					LOD->Accept = "-0123456789";
				}
				break;
			   
				case FV_String_Hexadecimal:
				{
					LOD->Accept = "0123456789ABCDEFabcdef";
				}
				break;
			   
				default:
				{
					LOD->Accept = (STRPTR)(item.ti_Data);
				}
				break;
			}
		}
		break;

		case FV_ATTRIBUTE_ADVANCEONCR:
		{
			if (item.ti_Data) LOD->Flags |=  FF_String_AdvanceOnCR;
			else              LOD->Flags &= ~FF_String_AdvanceOnCR;
		}
		break;

		#ifndef F_NEW_STYLES

		case FV_ATTRIBUTE_BLINKSPEED:
		{
			LOD->BlinkSpeed   = item.ti_Data;
			LOD->BlinkElapsed = 0;
		}
		break;

		#endif

		case FV_ATTRIBUTE_CONTENTS:
		{
			if (item.ti_Data != (uint32)(LOD->String))
			{
				LOD->Pos = 0; LOD->Len = 0;
				LOD->Cur = 0; LOD->Sel = 1;

				if (item.ti_Data)
				{
					STRPTR str = (STRPTR)(item.ti_Data);

					while (*str)
					{
						String_Insert(Class,Obj,*str++);
					   
						if (LOD->Max)
						{
							if (LOD->Len + 1 == LOD->Max) break;
						}
					}
					
					IFEELIN F_Dispose(LOD->Undo);
					LOD->Undo = IFEELIN F_StrNew(NULL,"%s",LOD->String);
				}
				else if (LOD->String)
				{
					if (LOD->Max)
					{
						LOD->String[0] = 0;
					}
					else
					{
						IFEELIN F_Dispose(LOD->String); LOD->String = NULL; LOD->Allocated = 0;
					}
				}
			 
				#ifdef DB_SET
				IFEELIN F_Log(0,"len (%ld)(%s)(%ld)",LOD->Len,LOD->String,LOD->String[LOD->Len]);
				#endif

				LOD->Pos = 0;
				LOD->Cur = 0;
				
				IFEELIN F_Draw(Obj,FF_Draw_Object);
				
				tag->ti_Data = (uint32) LOD->String;
			}
		}
		break;

		case FV_ATTRIBUTE_JUSTIFY:
		{
			LOD->Justify = item.ti_Data;
			
			IFEELIN F_Draw(Obj,FF_Draw_Object);
		}
		break;

		case FV_ATTRIBUTE_INTEGER:
		{
			STRPTR spec;

			spec = IFEELIN F_StrNew(NULL,"%ld",item.ti_Data);

			IFEELIN F_Set(Obj,F_ATTRIBUTE_ID(CONTENTS),(uint32)(spec));

			IFEELIN F_Dispose(spec);
		}
		break;

		case FV_ATTRIBUTE_REJECT:
		{
			switch (item.ti_Data)
			{
				case FV_String_Decimal:     LOD->Reject = "0123456789"; break;
				case FV_String_Hexadecimal: LOD->Reject = "0123456789ABCDEFabcdef"; break;
				default:                    LOD->Reject = (STRPTR)(item.ti_Data);
			}
		}
		break;

		case FV_ATTRIBUTE_ATTACHEDLIST:
		{
			LOD->AttachedList = (FObject)(item.ti_Data);
		}
		break;
	}

	F_SUPERDO();
}
//+
///String_Get
F_METHOD(void,String_Get)
{
   struct LocalObjectData *LOD  = F_LOD(Class,Obj);
   struct TagItem         *Tags = Msg,item;

   while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
   switch (item.ti_Tag)
   {
		case FV_ATTRIBUTE_ADVANCEONCR:   F_STORE(NULL != (LOD->Flags & FF_String_AdvanceOnCR)); break;
		case FV_ATTRIBUTE_ATTACHEDLIST:  F_STORE(LOD->AttachedList); break;
		case FV_ATTRIBUTE_CONTENTS:      F_STORE((LOD->String && *LOD->String) ? LOD->String : NULL); break;
		case FV_ATTRIBUTE_JUSTIFY:       F_STORE(LOD->Justify); break;
		case FV_ATTRIBUTE_INTEGER:       F_STORE(atol(LOD->String));  break;
		case FV_ATTRIBUTE_MAX:           F_STORE(LOD->Max);    break;
   }
   F_SUPERDO();
}
//+

///Preferences
STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("string", "font",          FV_TYPE_STRING,  "Contents", NULL ),
	F_PREFERENCES_ADD("string", "frame",         FV_TYPE_STRING,  "Spec",     "<frame id='47' padding-width='3' padding-height='1' /><frame id='47' padding-width='3' padding-height='1' />" ),
	F_PREFERENCES_ADD("string", "back",          FV_TYPE_STRING,  "Spec",     "dark" F_IMAGEDISPLAY_SEPARATOR "shine" ),
	F_PREFERENCES_ADD("string", "color-scheme",  FV_TYPE_STRING,  "Spec",     NULL ),
	F_PREFERENCES_ADD("string", "cursor",        FV_TYPE_STRING,  "Spec",     DEF_STRING_CURSOR ),
	F_PREFERENCES_ADD("string", "blink-image",   FV_TYPE_STRING,  "Spec",     DEF_STRING_BLINK_IMAGE ),
	F_PREFERENCES_ADD("string", "blink-speed",   FV_TYPE_INTEGER, "Value",    DEF_STRING_BLINK_SPEED ),
	F_PREFERENCES_ADD("string", "text-active",   FV_TYPE_STRING,  "Spec",     DEF_STRING_TEXT_ACTIVE ),
	F_PREFERENCES_ADD("string", "text-inactive", FV_TYPE_STRING,  "Spec",     DEF_STRING_TEXT_INACTIVE ),
	F_PREFERENCES_ADD("string", "text-block",    FV_TYPE_STRING,  "Spec",     DEF_STRING_TEXT_BLOCK ),

	F_ARRAY_END
};

F_METHOD(uint32,Prefs_New)
{
	STATIC F_ENTITIES_ARRAY =
	{
		F_ENTITIES_ADD("locale.gadget",     "Gadget"),
		F_ENTITIES_ADD("locale.font",       "Font"),
		F_ENTITIES_ADD("locale.frame",      "Frame"),
		F_ENTITIES_ADD("locale.back",       "Back"),
		F_ENTITIES_ADD("locale.scheme",     "Scheme"),
		F_ENTITIES_ADD("locale.frame",      "Frame"),
		F_ENTITIES_ADD("locale.speed",      "Speed"),
		F_ENTITIES_ADD("locale.cursor",     "Cursor"),
		F_ENTITIES_ADD("locale.blink",      "Blink"),
		F_ENTITIES_ADD("locale.text",       "Text"),
		F_ENTITIES_ADD("locale.active",     "Active"),
		F_ENTITIES_ADD("locale.inactive",   "Inactive"),
		F_ENTITIES_ADD("locale.block",      "Block"),
		
		F_ARRAY_END
	};

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "String",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/string.xml",
		"Entities", F_ENTITIES_PTR,

	TAG_MORE,Msg);
}
//+
