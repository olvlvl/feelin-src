#include "Private.h"

#include <feelin/support/f_parse_values.c>
#include <feelin/support/f_decode_color.c>
 
/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///frame_get_widths
STATIC void frame_get_widths(uint32 ID, FPadding *Borders)
{
	FPadding *a;

	/* filter deprecated */

	switch (ID)
	{
		case 1:
		case 2:
		case 3:
		case 4:
		{
			IFEELIN F_Log(0, "please, use the 'border' property");
		}
		break;

		case 5:
		case 6:
		{
			IFEELIN F_Log(0, "please, use 'broke' with the 'border-style' property");
		}
		break;

		case 33: // classic shadow thick
		case 34: // classic shadow thick in
		case 46:
		case 47:
		{
			IFEELIN F_Log(0, "frame (%ld) is deprecated", ID);
		}
		break;

		case 37:
		case 38:
		{
			IFEELIN F_Log(0, "please, use the 'palette-contrast' property with the 'xen' or 'xen-in' properties");
		}
		break;

	 	case 39:
		case 40:
		case 44:
		case 45:
		{
			IFEELIN F_Log(0, "please, use the 'border-image' property");
		}
		break;
	}

	switch (ID)
	{
		case  1: case  2: case  3: case  4: case  5: case  6:
		case 55: case 56:
		{
			STATIC FPadding borders = { 1, 1, 1, 1 };

			a = &borders;
		}
		break;

		case  7: case  8: case  9: case 10: case 11: case 12:
		case 13: case 14: case 18: case 23: case 27: case 28:
		case 31: case 32: case 37: case 38: case 39: case 40:
		case 42: case 43: case 46: case 47:
		{
			STATIC FPadding borders = { 2, 2, 2, 2 };

			a = &borders;
		}
		break;

		case 15: case 16: case 17: case 21: case 25: case 29:
		case 30: case 33: case 34: case 35: case 48: case 49:
		{
			STATIC FPadding borders = { 3, 3, 3, 3 };

			a = &borders;
		}
		break;

		case 19:
		{
			STATIC FPadding borders = { 4, 2, 4, 2 };

			a = &borders;
		}
		break;

		case 41:
		{
			STATIC FPadding borders = { 4, 4, 4, 4 };

			a = &borders;
		}
		break;

		case 20:
		{
			STATIC FPadding borders = { 5, 5, 5, 5 };

			a = &borders;
		}
		break;

		case 44:
		case 54:
		{
			STATIC FPadding borders = { 2, 3, 2, 3 };

			a = &borders;
		}
		break;

		case 24:
		{
			STATIC FPadding borders = { 3, 1, 3, 1 };

			a = &borders;
		}
		break;

		case 36: case 45:
		{
			STATIC FPadding borders = { 3, 2, 3, 2 };

			a = &borders;
		}
		break;

		case 26:
		{
			STATIC FPadding borders = { 4, 1, 4, 1 };

			a = &borders;
		}
		break;

		case 22:
		{
			STATIC FPadding borders = { 6, 3, 6, 3 };

			a = &borders;
		}
		break;

		case 50:
		{
			STATIC FPadding borders = { 2, 0, 2, 0 };

			a = &borders;
		}
		break;

		case 51:
		{
			STATIC FPadding borders = { 0, 2, 2, 0 };

			a = &borders;
		}
		break;

		case 52:
		{
			STATIC FPadding borders = { 0, 2, 0, 2 };

			a = &borders;
		}
		break;

		case 53:
		{
			STATIC FPadding borders = { 2, 0, 0, 2 };

			a = &borders;
		}
		break;

		default:
		{
			STATIC FPadding borders = { 0, 0, 0, 0 };

			a = &borders;
		}
		break;
	}

	Borders->l = a->l;
	Borders->r = a->r;
	Borders->t = a->t;
	Borders->b = a->b;
}
//+
///parse_caption_position
STATIC bool32 parse_caption_position(STRPTR Value, uint32 ValueLength, struct LocalPropertiesData *LPD)
{
	switch (ValueLength)
	{
		case 3:
		{
			if (IFEELIN F_StrCmp(Value, "top", 3) == 0)
			{
				LPD->caption_position_bottom = FALSE;
			}
		}
		break;

		case 4:
		{
			if (IFEELIN F_StrCmp(Value, "left", 4) == 0)
			{
				LPD->caption_position = FV_CAPTION_POSITION_LEFT;
			}
		}
		break;

		case 5:
		{
			if (IFEELIN F_StrCmp(Value, "right", 5) == 0)
			{
				LPD->caption_position = FV_CAPTION_POSITION_RIGHT;
			}
		}
		break;

		case 6:
		{
			if (IFEELIN F_StrCmp(Value, "bottom", 6) == 0)
			{
				LPD->caption_position_bottom = TRUE;
			}
			else if (IFEELIN F_StrCmp(Value, "center", 6) == 0)
			{
				LPD->caption_position = FV_CAPTION_POSITION_CENTER;
			}
		}
		break;
	}

	return TRUE;
}
//+

///frame_parse_border
STATIC bool32 frame_parse_border(STRPTR Value, uint32 ValueLength, struct in_Border *Result)
{
	// border-width

	if ((Value[ValueLength - 2] == 'p') && (Value[ValueLength - 1] == 'x'))
	{
		Result->width = atol(Value);

		return TRUE;
	}

	// border-style

	switch (ValueLength)
	{
		case 4:
		{
			if (IUTILITY Strnicmp(Value, "none", 4) == 0)
			{
				Result->style = FV_BORDER_STYLE_NONE;

				return TRUE;
			}
		}
		break;

		case 5:
		{
			if (IUTILITY Strnicmp(Value, "broke", 5) == 0)
			{
				Result->style = FV_BORDER_STYLE_BROKE;

				return TRUE;
			}
			else if (IUTILITY Strnicmp(Value, "round", 5) == 0)
			{
				Result->style = FV_BORDER_STYLE_ROUND;

				return TRUE;
			}
			else if (IUTILITY Strnicmp(Value, "solid", 5) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}

			/* FIXME-061016

			Because the 'ridge' style is not implemented  yet,  the  'solid'
			style is used instead.

			*/

			if (IUTILITY Strnicmp(Value, "ridge", 5) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}
			/* FIXME-061016

			Because the 'inset' style is not implemented  yet,  the  'solid'
			style is used instead.

			*/

			if (IUTILITY Strnicmp(Value, "inset", 5) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}
		}
		break;

		case 6:
		{
			/* FIXME-061016

			Because the 'dotted' style is not implemented yet,  the  'solid'
			style is used instead.

			*/

			if (IUTILITY Strnicmp(Value, "dotted", 6) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}
			/* FIXME-061016

			Because the 'groove' style is not implemented yet,  the  'solid'
			style is used instead.

			*/

			else if (IUTILITY Strnicmp(Value, "groove", 6) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}
			
			/* FIXME-061016

			Because the 'outset' style is not implemented yet,  the  'solid'
			style is used instead.

			*/

			else if (IUTILITY Strnicmp(Value, "groove", 6) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}
		 
		   /* FIXME-061016

			Because the 'groove' style is not implemented yet,  the  'solid'
			style is used instead.

			*/

			else if (IUTILITY Strnicmp(Value, "outset", 6) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}
		}
		break;

		case 7:
		{
			/* FIXME-061016

			Because the 'dashed' style is not implemented yet,  the  'solid'
			style is used instead.

			*/

			if (IUTILITY Strnicmp(Value, "dashed", 7) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}
			
			/* FIXME-061016

			Because the 'groove' style is not implemented yet,  the  'solid'
			style is used instead.

			*/

			else if (IUTILITY Strnicmp(Value, "double", 7) == 0)
			{
				Result->style = FV_BORDER_STYLE_SOLID;

				return TRUE;
			}
		}
		break;
	}

	// border-color

	Result->color_type = f_decode_color(Value, ValueLength, &Result->color);

	//IFEELIN F_Log(0, "(%s)[%ld] >> (0x%08lx)(%ld)", Value, ValueLength, Result->color, Result->color_type);

	if (Result->color_type != FV_COLORTYPE_NONE)
	{
		return TRUE;
	}

	IFEELIN F_Log(0, "invalid token (%s)[%ld] for the 'border' property", Value, ValueLength);

	return FALSE;
}
//+
///frame_decode_border
STATIC bool32 frame_decode_border(STRPTR Spec, struct in_Border *Result)
{
	bool32 rc;

	/* reset values */

	Result->color = 0;
	Result->color_type = FV_COLORTYPE_NONE;
	Result->style = FV_BORDER_STYLE_NONE;
	Result->width = 0;

	rc = f_parse_values(Spec, (f_parse_values_func) frame_parse_border, Result);

	#if 0

	IFEELIN F_Log(0, "color (0x%08lx)(%ld) style (%ld) width (%ld)",

		Result->color,
		Result->color_type,
		Result->style,
		Result->width);

	#endif

	return rc;
}
//+

/*

DEPRECATED FRAME IDs
--------------------

	1= border: 1px solid dark;
	2= border: 1px solid shine;
	3= border: 1px solid halfdark;
	4= border: 1px solid halfshine;
	5= border: 1px broke halfdark;
	6= border: 1px broke halfshine;

	42= "border-frame: sweet"
*/

enum	{

		FV_BORDER_FRAME_XEN = 23,
		FV_BORDER_FRAME_XEN_IN = 24,
		FV_BORDER_FRAME_XEN2 = 25,
		FV_BORDER_FRAME_XEN2_IN = 26,

		FV_BORDER_FRAME_WIN = 17,
		FV_BORDER_FRAME_WIN_IN = 18,
		FV_BORDER_FRAME_WIN_IN_SHADOW = 19,
		FV_BORDER_FRAME_WIN2 = 20,
		FV_BORDER_FRAME_WIN2_IN = 21,
		FV_BORDER_FRAME_WIN2_IN_SHADOW = 22,

		FV_BORDER_FRAME_SWEET = 42,
		FV_BORDER_FRAME_SWEET_IN = 43,
		FV_BORDER_FRAME_SWEET_IN_NW = 50,
		FV_BORDER_FRAME_SWEET_IN_NE = 51,
		FV_BORDER_FRAME_SWEET_IN_SE = 52,
		FV_BORDER_FRAME_SWEET_IN_SW = 53,

		FV_BORDER_FRAME_CLASSIC = 55,
		FV_BORDER_FRAME_CLASSIC_IN = 56,
		FV_BORDER_FRAME_CLASSIC_SHADOW = 31,
		FV_BORDER_FRAME_CLASSIC_SHADOW_IN = 32,

		FV_BORDER_FRAME_LOTAN = 35,
		FV_BORDER_FRAME_LOTAN_IN = 36,

		FV_BORDER_FRAME_MELIS = 48,
		FV_BORDER_FRAME_MELIS_IN = 49,

		FV_BORDER_FRAME_GNOME = 54,

		FV_BORDER_FRAME_BE = 41,

		FV_BORDER_FRAME_KYT = 27,
		FV_BORDER_FRAME_KYT_IN = 28,
		FV_BORDER_FRAME_KYT_THICK = 29,
		FV_BORDER_FRAME_KYT_THICK_IN = 30,

		FV_BORDER_FRAME_GROOVE = 9,
		FV_BORDER_FRAME_GROOVE_IN = 10,
		FV_BORDER_FRAME_GROOVE_SOFT = 11,
		FV_BORDER_FRAME_GROOVE_SOFT_IN = 12,
		FV_BORDER_FRAME_GROOVE_ROUND = 15,
		FV_BORDER_FRAME_GROOVE_ROUND_IN = 16,

		FV_BORDER_FRAME_XP = 7,
		FV_BORDER_FRAME_XP_IN = 8

		};

///border_decode_frame
STATIC bool32 border_decode_frame(STRPTR Spec, struct in_Frame *Result)
{
	uint32 len = IFEELIN F_StrLen(Spec);
	uint32 id = 0;

	switch (len)
	{
		case 2:
		{
			if (IUTILITY Strnicmp(Spec, "be", 2) == 0)
			{
				id = FV_BORDER_FRAME_BE;
			}
			else if (IUTILITY Strnicmp(Spec, "xp", 2) == 0)
			{
				id = FV_BORDER_FRAME_XP;
			}
		}
		break;

		case 3:
		{
			if (IUTILITY Strnicmp(Spec, "xen", 3) == 0)
			{
				id = FV_BORDER_FRAME_XEN;
			}
			else if (IUTILITY Strnicmp(Spec, "win", 3) == 0)
			{
				id = FV_BORDER_FRAME_WIN;
			}
			else if (IUTILITY Strnicmp(Spec, "kyt", 3) == 0)
			{
				id = FV_BORDER_FRAME_KYT;
			}
		}
		break;

		case 4:
		{
			if (IUTILITY Strnicmp(Spec, "xen2", 4) == 0)
			{
				id = FV_BORDER_FRAME_XEN2;
			}
			else if (IUTILITY Strnicmp(Spec, "win2", 4) == 0)
			{
				id = FV_BORDER_FRAME_WIN2;
			}
		}
		break;

		case 5:
		{
			if (IUTILITY Strnicmp(Spec, "sweet", 5) == 0)
			{
				id = FV_BORDER_FRAME_SWEET;
			}
			else if (IUTILITY Strnicmp(Spec, "lotan", 5) == 0)
			{
				id = FV_BORDER_FRAME_LOTAN;
			}
			else if (IUTILITY Strnicmp(Spec, "melis", 5) == 0)
			{
				id = FV_BORDER_FRAME_MELIS;
			}
			else if (IUTILITY Strnicmp(Spec, "gnome", 5) == 0)
			{
				id = FV_BORDER_FRAME_GNOME;
			}
			else if (IUTILITY Strnicmp(Spec, "xp-in", 5) == 0)
			{
				id = FV_BORDER_FRAME_XP_IN;
			}
		}
		break;

		case 6:
		{
			if (IUTILITY Strnicmp(Spec, "xen-in", 6) == 0)
			{
				id = FV_BORDER_FRAME_XEN_IN;
			}
			else if (IUTILITY Strnicmp(Spec, "win-in", 6) == 0)
			{
				id = FV_BORDER_FRAME_WIN_IN;
			}
			else if (IUTILITY Strnicmp(Spec, "kyt-in", 6) == 0)
			{
				id = FV_BORDER_FRAME_KYT_IN;
			}
			else if (IUTILITY Strnicmp(Spec, "groove", 6) == 0)
			{
				id = FV_BORDER_FRAME_GROOVE;
			}
		}
		break;

		case 7:
		{
			if (IUTILITY Strnicmp(Spec, "xen2-in", 7) == 0)
			{
				id = FV_BORDER_FRAME_XEN2_IN;
			}
			else if (IUTILITY Strnicmp(Spec, "win2-in", 7) == 0)
			{
				id = FV_BORDER_FRAME_WIN2_IN;
			}
			else if (IUTILITY Strnicmp(Spec, "classic", 7) == 0)
			{
				id = FV_BORDER_FRAME_CLASSIC;
			}
		}
		break;

		case 8:
		{
			if (IUTILITY Strnicmp(Spec, "sweet-in", 8) == 0)
			{
				id = FV_BORDER_FRAME_SWEET_IN;
			}
			else if (IUTILITY Strnicmp(Spec, "lotan-in", 8) == 0)
			{
				id = FV_BORDER_FRAME_LOTAN_IN;
			}
			else if (IUTILITY Strnicmp(Spec, "melis-in", 8) == 0)
			{
				id = FV_BORDER_FRAME_MELIS_IN;
			}
		}
		break;

		case 9:
		{
			if (IUTILITY Strnicmp(Spec, "kyt-thick", 9) == 0)
			{
				id = FV_BORDER_FRAME_KYT_THICK;
			}
			else if (IUTILITY Strnicmp(Spec, "groove-in", 9) == 0)
			{
				id = FV_BORDER_FRAME_GROOVE_IN;
			}
		}
		break;

		case 10:
		{
			if (IUTILITY Strnicmp(Spec, "classic-in", 10) == 0)
			{
				id = FV_BORDER_FRAME_CLASSIC_IN;
			}
		}
		break;

		case 11:
		{
			if (IUTILITY Strnicmp(Spec, "sweet-in-nw", 11) == 0)
			{
				id = FV_BORDER_FRAME_SWEET_IN_NW;
			}
			else if (IUTILITY Strnicmp(Spec, "sweet-in-ne", 11) == 0)
			{
				id = FV_BORDER_FRAME_SWEET_IN_NE;
			}
			else if (IUTILITY Strnicmp(Spec, "sweet-in-se", 11) == 0)
			{
				id = FV_BORDER_FRAME_SWEET_IN_SE;
			}
			else if (IUTILITY Strnicmp(Spec, "sweet-in-sw", 11) == 0)
			{
				id = FV_BORDER_FRAME_SWEET_IN_SW;
			}
			else if (IUTILITY Strnicmp(Spec, "groove-soft", 11) == 0)
			{
				id = FV_BORDER_FRAME_GROOVE_SOFT;
			}
		}
		break;

		case 12:
		{
			if (IUTILITY Strnicmp(Spec, "kyt-thick-in", 12) == 0)
			{
				id = FV_BORDER_FRAME_KYT_THICK_IN;
			}
			else if (IUTILITY Strnicmp(Spec, "groove-round", 12) == 0)
			{
				id = FV_BORDER_FRAME_GROOVE_ROUND;
			}
		}
		break;

		case 13:
		{
			if (IUTILITY Strnicmp(Spec, "win-in-shadow", 13) == 0)
			{
				id = FV_BORDER_FRAME_WIN_IN_SHADOW;
			}
		}
		break;
		
		case 14:
		{
			if (IUTILITY Strnicmp(Spec, "win2-in-shadow", 14) == 0)
			{
				id = FV_BORDER_FRAME_WIN2_IN_SHADOW;
			}
			else if (IUTILITY Strnicmp(Spec, "classic-shadow", 14) == 0)
			{
				id = FV_BORDER_FRAME_CLASSIC_SHADOW;
			}
			else if (IUTILITY Strnicmp(Spec, "groove-soft-in", 14) == 0)
			{
				id = FV_BORDER_FRAME_GROOVE_SOFT_IN;
			}
		}
		break;

		case 15:
		{
			if (IUTILITY Strnicmp(Spec, "groove-round-in", 15) == 0)
			{
				id = FV_BORDER_FRAME_GROOVE_ROUND_IN;
			}
		}
		break;

		case 17:
		{
			if (IUTILITY Strnicmp(Spec, "classic-shadow-in", 17) == 0)
			{
				id = FV_BORDER_FRAME_CLASSIC_SHADOW_IN;
			}
		}
		break;
	}

	if (id != 0)
	{
		FPadding border;

	    Result->id = id;

		frame_get_widths(id, &border);

	    Result->border_top.width = border.t;
		Result->border_right.width = border.r;
	    Result->border_bottom.width = border.b;
		Result->border_left.width = border.l;

	    Result->type = FV_FRAME_TYPE_FRAME;
	}
	else
	{
	    Result->border_top.width =
		Result->border_right.width =
	    Result->border_bottom.width =
		Result->border_left.width = 0;
	}

	return TRUE;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Border_Setup
F_METHODM(bool32,Border_Setup,FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	LOD->properties = F_LPD(Class, (APTR) IFEELIN F_Get(Obj, FA_Element_DecodedStyle));
	LOD->frame = &LOD->properties->frames[_area_state];

	_area_border.l = LOD->frame->border_left.width;
	_area_border.r = LOD->frame->border_right.width;
	_area_border.t = LOD->frame->border_top.width;
	_area_border.b = LOD->frame->border_bottom.width;

/*** title *************************************************************************************/

	if ((LOD->caption != NULL) && ((_area_border.t != 0) || (_area_border.b != 0)))
	{
		struct in_Caption *caption = LOD->caption;

		caption->preparse = LOD->properties->caption_preparse;
		caption->position = LOD->properties->caption_position;

		if (LOD->properties->caption_position_bottom)
		{
			caption->position |= FF_CAPTION_POSITION_BOTTOM;
		}

		if (LOD->properties->caption_font == NULL)
		{
			struct TextAttr ta;

			ta.ta_Name  = _area_font->tf_Message.mn_Node.ln_Name;
			ta.ta_YSize = _area_font->tf_YSize;
			ta.ta_Style = _area_font->tf_Style;
			ta.ta_Flags = _area_font->tf_Flags;

			caption->font = IGRAPHICS OpenFont(&ta);
		}
		else
		{
			caption->font = (struct TextFont *) IFEELIN F_Do(_area_app, FM_Application_OpenFont, Obj, LOD->properties->caption_font);
		}

		#ifdef DB_SETUP
		IFEELIN F_Log(0, "preparse (%s) position (%ld) font (0x%08lx)", caption->preparse, caption->position, caption->font);
		#endif

		caption->display = TextDisplayObject,

			FA_TextDisplay_Contents,  caption->label,
			FA_TextDisplay_PreParse,  caption->preparse,
			FA_TextDisplay_Font,      caption->font,
			FA_TextDisplay_Shortcut,  FALSE,

		End;

		if (IFEELIN F_Do(caption->display, FM_TextDisplay_Setup, Msg->Render) == FALSE)
		{
			IFEELIN F_DisposeObj(caption->display);

			caption->display = NULL;
		}

		if (caption->display != NULL)
		{
			caption->width  = IFEELIN F_Get(caption->display, FA_TextDisplay_Width);
			caption->height = IFEELIN F_Get(caption->display, FA_TextDisplay_Height);
		}

		if ((FF_CAPTION_POSITION_BOTTOM & caption->position) != 0)
		{
			caption->border = _area_border.b;
			_area_border.b = LOD->caption->height;
		}
		else
		{
			caption->border = _area_border.t;
			_area_border.t = LOD->caption->height;
		}
	}

	return TRUE;
}
//+
///Border_Cleanup
F_METHOD(uint32,Border_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render != NULL)
	{
		if (LOD->caption != NULL)
		{
			IFEELIN F_Do(LOD->caption->display, FM_TextDisplay_Cleanup);
			IFEELIN F_DisposeObj(LOD->caption->display);

			LOD->caption->display = NULL;

			if (LOD->caption->font != NULL)
			{
				IGRAPHICS CloseFont(LOD->caption->font);

				LOD->caption->font = NULL;
			}
		}
	}

	return F_SUPERDO();
}
//+
///Border_CreateDecodedStyle
F_METHODM(bool32, Border_CreateDecodedStyle, FS_Element_CreateDecodedStyle)
{
	struct LocalPropertiesData *local = F_LPD(Class, Msg->DecodedSpace);

	FPreferenceStyle *style;

	for (style = Msg->Style ; style ; style = style->Next)
	{
		struct in_Frame *state;

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

		state = &local->frames[i];

/*** parse properties **************************************************************************/

		for (properties_a = style->Properties ; *properties_a ; properties_a++)
		{
			FPreferenceProperty *prop = *properties_a;

			switch (prop->Atom->KeyLength)
			{
				case 6:
				{
					// border

					if (prop->Atom == F_PROPERTY_ATOM(BORDER))
					{
						if (FV_FRAME_TYPE_BORDER >= state->type)
						{
							frame_decode_border(prop->Value, &state->border_top);

							IEXEC CopyMem(&state->border_top, &state->border_right, sizeof (struct in_Border));
							IEXEC CopyMem(&state->border_top, &state->border_bottom, sizeof (struct in_Border));
							IEXEC CopyMem(&state->border_top, &state->border_left, sizeof (struct in_Border));

							state->type = FV_FRAME_TYPE_BORDER;
						}
					}
				}
				break;

				case 10:
				{
					// border-top

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_TOP))
					{
						if (FV_FRAME_TYPE_BORDER >= state->type)
						{
							frame_decode_border(prop->Value, &state->border_top);

							state->type = FV_FRAME_TYPE_BORDER;
						}
					}
				}
				break;

				case 11:
				{
					// border-left

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_LEFT))
					{
						if (FV_FRAME_TYPE_BORDER >= state->type)
						{
							frame_decode_border(prop->Value, &state->border_left);

							state->type = FV_FRAME_TYPE_BORDER;
						}
					}
				}
				break;

				case 12:
				{
					// border-color

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_COLOR))
					{
						uint32 color;
						uint32 color_type;

						color_type = f_decode_color(prop->Value, ALL, &color);

					  	state->border_top.color =
					  	state->border_right.color =
					  	state->border_bottom.color =
					  	state->border_left.color = color;

					  	state->border_top.color_type =
					  	state->border_right.color_type =
					  	state->border_bottom.color_type =
					  	state->border_left.color_type = color_type;
					}

					// border-width

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_WIDTH))
					{
						if (prop->NumericType == FV_TYPE_PIXEL)
						{
						  	state->border_top.width =
						  	state->border_right.width =
						  	state->border_bottom.width =
						  	state->border_left.width = prop->NumericValue;
						}
						else
						{
							IFEELIN F_Log(0, "only 'px' value are currently supported for the '%s' property", prop->Atom->Key);
						}
					}

					// border-style

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_STYLE))
					{
						uint32 style = FV_BORDER_STYLE_SOLID;

						IFEELIN F_Log(0, "border-style: (%s)(%ld)", prop->Value, style);

					  	state->border_top.style =
					  	state->border_right.style =
					  	state->border_bottom.style =
					  	state->border_left.style = style;
					}

					// border-right

					else if (prop->Atom == F_PROPERTY_ATOM(BORDER_RIGHT))
					{
						if (FV_FRAME_TYPE_BORDER >= state->type)
						{
							frame_decode_border(prop->Value, &state->border_right);

							state->type = FV_FRAME_TYPE_BORDER;
						}
					}

					// border-frame

					else if (prop->Atom == F_PROPERTY_ATOM(BORDER_FRAME))
					{
						if (FV_FRAME_TYPE_FRAME >= state->type)
						{
							border_decode_frame(prop->Value, state);
						}
					}

					// caption-font

					else if (prop->Atom == F_PROPERTY_ATOM(CAPTION_FONT))
					{
						local->caption_font = prop->Value;
					}
				}
				break;

				case 13:
				{
					// border-bottom

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_BOTTOM))
					{
						if (FV_FRAME_TYPE_BORDER >= state->type)
						{
							frame_decode_border(prop->Value, &state->border_bottom);

							state->type = FV_FRAME_TYPE_BORDER;
						}
					}
				}
				break;

				case 16:
				{
					// border-top-color

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_TOP_COLOR))
					{
						state->border_top.color_type = f_decode_color(prop->Value, ALL, &state->border_top.color);
					}

					// border-top-width

					else if (prop->Atom == F_PROPERTY_ATOM(BORDER_TOP_WIDTH))
					{
						if (prop->NumericType == FV_TYPE_PIXEL)
						{
	                        state->border_top.width = prop->NumericValue;
						}
						else
						{
							IFEELIN F_Log(0, "only 'px' value are currently supported for the '%s' property", prop->Atom->Key);
						}
					}

					// caption-position

					else if (prop->Atom == F_PROPERTY_ATOM(CAPTION_POSITION))
					{
						f_parse_values(prop->Value, (f_parse_values_func) parse_caption_position, local);
					}

					// caption-preparse

					else if (prop->Atom == F_PROPERTY_ATOM(CAPTION_PREPARSE))
					{
						local->caption_preparse = prop->Value;
					}
				}
				break;

				case 17:
				{
					// border-left-color

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_LEFT_COLOR))
					{
						state->border_left.color_type = f_decode_color(prop->Value, ALL, &state->border_left.color);
					}
				}
				break;

				case 18:
				{
					// border-right-color

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_RIGHT_COLOR))
					{
						state->border_right.color_type = f_decode_color(prop->Value, ALL, &state->border_right.color);
					}
				}
				break;

				case 19:
				{
					// border-bottom-color

					if (prop->Atom == F_PROPERTY_ATOM(BORDER_BOTTOM_COLOR))
					{
						state->border_bottom.color_type = f_decode_color(prop->Value, ALL, &state->border_bottom.color);
					}
				}
				break;
			}





			/* THE FOLLOWING ARE DEPRECATED */

			// frame

			if (prop->Atom == F_PROPERTY_ATOM(FRAME))
			{
				IFEELIN F_Log(0, "the '%s' property is deprecated !", prop->Atom->Key);
			}
			else if (prop->Atom == F_PROPERTY_ATOM(FRAME_ID))
			{
				IFEELIN F_Log(0, "the '%s' property is deprecated !", prop->Atom->Key);

				if (FV_FRAME_TYPE_FRAME >= state->type)
				{
					state->id = prop->NumericValue;

					state->type = FV_FRAME_TYPE_FRAME;
				}
			}
			else if (style->Atom == NULL)
			{
				if (prop->Atom == F_PROPERTY_ATOM(FRAME_PREPARSE))
				{
					IFEELIN F_Log(0, "the 'frame-preparse' property is deprecated, please use the 'caption-prepearse' property");
				}
				else if (prop->Atom == F_PROPERTY_ATOM(FRAME_POSITION))
				{
					IFEELIN F_Log(0, "the 'frame-position' property is deprecated, please use the 'caption-position' property");
				}
				else if (prop->Atom == F_PROPERTY_ATOM(FRAME_FONT))
				{
					IFEELIN F_Log(0, "the 'frame-font' property is deprecated, please use the 'caption-font' property");
				}
			}
		}

/*** validation ********************************************************************************/

		if (state->id)
		{
			FPadding border;

			frame_get_widths(state->id, &border);

			state->border_top.width = border.t;
			state->border_right.width = border.r;
			state->border_bottom.width = border.b;
			state->border_left.width = border.l;
		}
		else
		{
			if ((state->border_top.width != 0) && ((state->border_top.style == FV_BORDER_STYLE_NONE) || (state->border_top.color_type == FV_COLORTYPE_NONE)))
			{
				state->border_top.width = 0;
			}

			if ((state->border_right.width != 0) && ((state->border_right.style == FV_BORDER_STYLE_NONE) || (state->border_right.color_type == FV_COLORTYPE_NONE)))
			{
				state->border_right.width = 0;
			}

			if ((state->border_bottom.width != 0) && ((state->border_bottom.style == FV_BORDER_STYLE_NONE) || (state->border_bottom.color_type == FV_COLORTYPE_NONE)))
			{
				state->border_bottom.width = 0;
			}

			if ((state->border_left.width != 0) && ((state->border_left.style == FV_BORDER_STYLE_NONE) || (state->border_left.color_type == FV_COLORTYPE_NONE)))
			{
				state->border_left.width = 0;
			}
		}

/*** clone neutral *****************************************************************************/

		if (i == FV_Area_State_Neutral)
		{
			IEXEC CopyMem(state, &local->frames[FV_Area_State_Touch], sizeof (struct in_Frame));
			IEXEC CopyMem(state, &local->frames[FV_Area_State_Focus], sizeof (struct in_Frame));
			IEXEC CopyMem(state, &local->frames[FV_Area_State_Ghost], sizeof (struct in_Frame));
		}
	}

	return F_SUPERDO();
}
//+
