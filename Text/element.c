#include "Private.h"

#include <feelin/support/f_parse_values.c>

///text_parse_position
STATIC bool32 text_parse_position(STRPTR Value, uint32 ValueLength, struct LocalPropertiesData *Result)
{
	switch (ValueLength)
	{
		case 3:
		{
			if (IUTILITY Strnicmp("top", Value, 3) == 0)
			{
				Result->position_y = 0;
				Result->position_y_type = FV_TYPE_PIXEL;

				return TRUE;
			}
		}
		break;

		case 4:
		{
			if (IUTILITY Strnicmp("left", Value, 4) == 0)
			{
				if (Result->position_x_type != 0)
				{
					return FALSE;
				}

				Result->position_x = 0;
				Result->position_x_type = FV_TYPE_PIXEL;

				return TRUE;
			}
		}
		break;

		case 5:
		{
			if (IUTILITY Strnicmp("right", Value, 5) == 0)
			{
				if (Result->position_x_type != 0)
				{
					return FALSE;
				}

				Result->position_x = 100;
				Result->position_x_type = FV_TYPE_PERCENTAGE;

				return TRUE;
			}
		}
		break;

		case 6:
		{
			if (IUTILITY Strnicmp("bottom", Value, 6) == 0)
			{
				if (Result->position_y_type != 0)
				{
					return FALSE;
				}

				Result->position_y = 100;
				Result->position_y_type = FV_TYPE_PERCENTAGE;

				return TRUE;
			}
			else if (IUTILITY Strnicmp("center", Value, 6) == 0)
			{
				if (Result->position_x_type == 0)
				{
					Result->position_x = 50;
					Result->position_x_type = FV_TYPE_PERCENTAGE;
				}

				if (Result->position_y_type == 0)
				{
					Result->position_y = 50;
					Result->position_y_type = FV_TYPE_PERCENTAGE;
				}

				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Text_Setup
F_METHOD(bool32,Text_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	//STRPTR prep;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	LOD->properties = F_LPD(Class, IFEELIN F_Get(Obj, FA_Element_DecodedStyle));

	if (LOD->preparse_user != NULL)
	{
		LOD->preparse = LOD->preparse_user;
	}
	else
	{
		LOD->preparse = LOD->properties[_area_state].preparse;
	}

	#if 1

	LOD->TextDisplay = TextDisplayObject,

		FA_TextDisplay_Font,       _area_font,
		FA_TextDisplay_PreParse,   LOD->preparse,
		FA_TextDisplay_Contents,   LOD->text,
		FA_TextDisplay_Shortcut,   (0 != (FF_Text_Shortcut & LOD->Flags)),

		End;

	#else

	prep = LOD->Prep[_widget_is_selected ? 1 : 0];
	if (!prep) prep = LOD->Prep[0];

	LOD->TextDisplay = TextDisplayObject,

		FA_TextDisplay_Font,       _area_font,
		FA_TextDisplay_PreParse,   prep,
		FA_TextDisplay_Contents,   LOD->Text,
		FA_TextDisplay_Shortcut,   (0 != (FF_Text_Shortcut & LOD->Flags)),

		End;

	#endif

	if (IFEELIN F_Do(LOD->TextDisplay, FM_TextDisplay_Setup, _area_render) == FALSE)
	{
		return FALSE;
	}

	IFEELIN F_Set(Obj,FA_Widget_Accel, IFEELIN F_Get(LOD->TextDisplay, FA_TextDisplay_Shortcut));

	return TRUE;
}
//+
///Text_Cleanup
F_METHOD(uint32,Text_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render)
	{
		if (LOD->TextDisplay != NULL)
		{
			IFEELIN F_Do(LOD->TextDisplay, FM_TextDisplay_Cleanup, _area_render);
			IFEELIN F_DisposeObj(LOD->TextDisplay);

			LOD->TextDisplay = NULL;
		}
	}

	return F_SUPERDO();
}
//+
///Text_CreateDecodedStyle
F_METHODM(bool32, Text_CreateDecodedStyle, FS_Element_CreateDecodedStyle)
{
	struct LocalPropertiesData *local = F_LPD(Class, Msg->DecodedSpace);

	FPreferenceStyle *style;

	for (style = Msg->Style ; style ; style = style->Next)
	{
		struct LocalPropertiesData *state;

		uint32 i;

		FPreferenceProperty **properties_a;

/*** select state ******************************************************************************/

		if (style->Atom == NULL)
		{
			i = FV_Area_State_Neutral;
		}
		else if (style->Atom == F_ATOM(TOUCH))
		{
			i = FV_Area_State_Touch;
		}
		else if (style->Atom == F_ATOM(FOCUS))
		{
			i = FV_Area_State_Focus;
		}
		else if (style->Atom == F_ATOM(GHOST))
		{
			i = FV_Area_State_Ghost;
		}
		else continue;

		state = &local[i];

/*** parse properties **************************************************************************/

		for (properties_a = style->Properties ; *properties_a ; properties_a++)
		{
			FPreferenceProperty *prop = *properties_a;

			// preparse

			if (prop->Atom == F_PROPERTY_ATOM(PREPARSE))
			{
				state->preparse = prop->Value;
			}

			// text-position

			else if (prop->Atom == F_PROPERTY_ATOM(TEXT_POSITION))
			{
				f_parse_values(prop->Value, (f_parse_values_func) text_parse_position, state);
			}
		}

/*** clone neutral *****************************************************************************/

		if (i == FV_Area_State_Neutral)
		{
			IEXEC CopyMem(state, &local[FV_Area_State_Touch], sizeof (struct LocalPropertiesData));
			IEXEC CopyMem(state, &local[FV_Area_State_Focus], sizeof (struct LocalPropertiesData));
			IEXEC CopyMem(state, &local[FV_Area_State_Ghost], sizeof (struct LocalPropertiesData));
		}
	}

	return F_SUPERDO();
}
//+
