#include "Private.h"

//#define DB_CLEANUP
//#define DB_CREATEDECODEDSTYLE

#include <feelin/support/f_parse_values.c>
#include <feelin/support/f_decode_color.c>
#include <feelin/support/f_decode_padding.c>

/* PROPERTIES *
***************

	DIMENSIONS
	----------

		width, height, max-width, max-height, min-width, min-height

		margin, margin-top, margin-right, margin-bottom, margin-left

		padding, padding-top, padding-right, padding-bottom, padding-left

	PALETTE
	-------

		[13] palette-shine
		[12] palette-fill
		[12] palette-dark
		[12] palette-text
		[17] palette-highlight
		[16] palette-contrast
		[18] palette-saturation

*/

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///palette_set_entry
STATIC bool32 palette_set_entry(struct in_PaletteProperty *Palette, uint32 Entry, STRPTR Value, uint32 ValueLength)
{

/* FIXME-061018

	the 'inherit' keyword if not handled correctly:  ghost  palette  inherit
	its  colors from the nutral palette, but if inherit is defined it should
	inherit its value from the parent element.

*/

	if ((Value == NULL) || ((ValueLength == 7) && (IUTILITY Strnicmp("inherit", Value, 7) == 0)))
	{
		Palette->flags &= ~(1 << Entry);
	}
	else
	{
		Palette->flags |= (1 << Entry);

		Palette->entries[Entry].type = f_decode_color(Value, ValueLength, &(Palette->entries[Entry].value));
	}

//	  IFEELIN F_Log(0, "entry (%ld) type (0x%08lx) value (0x%08lx)", Entry, Palette->entries[Entry].type, Palette->entries[Entry].value);

	return TRUE;
}
//+
///palette_set_contrast
STATIC bool32 palette_set_contrast(struct in_PaletteProperty *PP, STRPTR Spec, uint32 NumericValue)
{
	if (Spec == NULL)
	{
		PP->flags &= ~FF_PALETTE_HAS_CONTRAST;
	}
	else
	{
		PP->flags |= FF_PALETTE_HAS_CONTRAST;

		PP->contrast = NumericValue;
	}

	return TRUE;
}
//+
///palette_set_luminance
STATIC bool32 palette_set_luminance(struct in_PaletteProperty *PP, STRPTR Spec, uint32 NumericValue)
{
	if (Spec == NULL)
	{
		PP->flags &= ~FF_PALETTE_HAS_LUMINANCE;
	}
	else
	{
		PP->flags |= FF_PALETTE_HAS_LUMINANCE;

		PP->luminance = NumericValue;
	}

	return TRUE;
}
//+
///palette_set_saturation
STATIC bool32 palette_set_saturation(struct in_PaletteProperty *PP, STRPTR Spec, uint32 NumericValue)
{
	if (Spec == NULL)
	{
		PP->flags &= ~FF_PALETTE_HAS_SATURATION;
	}
	else
	{
		PP->flags |= FF_PALETTE_HAS_SATURATION;

		PP->saturation = NumericValue;
	}

	return TRUE;
}
//+
///palette_init
STATIC void palette_init(struct in_PaletteProperty *PP, uint32 State)
{
	switch (State)
	{
		case FV_Area_State_Ghost:
		{
			palette_set_entry(PP, FV_PALETTE_SHINE, "halfshine", ALL);
			palette_set_entry(PP, FV_PALETTE_DARK, "shadow", ALL);
			palette_set_entry(PP, FV_PALETTE_TEXT, "halfshadow", ALL);
			palette_set_entry(PP, FV_PALETTE_FILL, "fill", ALL);
		}
		break;

		/* FIXME-061013

		As there is a default less-contrasted 'ghost' palette, there  should
		a darker 'touch' palette and a lighter 'focus' one.

		*/

		#if 0

		case FV_Area_State_Touch:
		{
			style_palette_set_fill(PT, "halfshadow");
		}
		break;

		case FV_Area_State_Focus:
		{
			style_palette_set_shine(PT, "highlight");
		}
		break;

		#endif
	}
}
//+
///palette_parse
STATIC bool32 palette_parse(STRPTR Value, uint32 ValueLength, struct in_PaletteProperty *Result)
{
	// palette-contrast palette-saturation (percentages)

	if (Value[ValueLength - 1] == '%')
	{
		if ((FF_PALETTE_HAS_CONTRAST & Result->flags) == 0)
		{
			palette_set_contrast(Result, Value, atol(Value));
		}
		else if ((FF_PALETTE_HAS_LUMINANCE & Result->flags) == 0)
		{
			palette_set_luminance(Result, Value, atol(Value));
		}
		else if ((FF_PALETTE_HAS_SATURATION & Result->flags) == 0)
		{
			palette_set_saturation(Result, Value, atol(Value));
		}
		else
		{
			return FALSE;
		}

		return TRUE;
	}

	// palette-fill palette-shine palette-dark palette-text palette-highlight (values)

	if ((FF_PALETTE_HAS_FILL & Result->flags) == 0)
	{
		palette_set_entry(Result, FV_PALETTE_FILL, Value, ValueLength);
	}
	else if ((FF_PALETTE_HAS_SHINE & Result->flags) == 0)
	{
		palette_set_entry(Result, FV_PALETTE_SHINE, Value, ValueLength);
	}
	else if ((FF_PALETTE_HAS_DARK & Result->flags) == 0)
	{
		palette_set_entry(Result, FV_PALETTE_DARK, Value, ValueLength);
	}
	else if ((FF_PALETTE_HAS_HIGHLIGHT & Result->flags) == 0)
	{
		palette_set_entry(Result, FV_PALETTE_HIGHLIGHT, Value, ValueLength);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}
//+
///palette_create
STATIC FPalette *palette_create(struct in_PaletteProperty *Palette, FPalette *Reference, FRender *Render)
{
	bits32 flags = Palette->flags;

	struct in_ColorProperty *entries = Palette->entries;
	struct in_ColorProperty *entry;

	uint32 argb[FV_PEN_COUNT];

	/* compose colors */

	argb[FV_Pen_Text]       = 0x00000000;
	argb[FV_Pen_Shine]      = 0x00FFFFFF;
	argb[FV_Pen_HalfShine]  = -1;
	argb[FV_Pen_Fill]       = 0x00BEBEBE;
	argb[FV_Pen_HalfShadow] = -1;
	argb[FV_Pen_Shadow]     = -1;
	argb[FV_Pen_HalfDark]   = -1;
	argb[FV_Pen_Dark]       = 0x00000000;
	argb[FV_Pen_Highlight]  = 0x00FF0000;

	if (Reference != NULL)
	{
		IEXEC CopyMem(Reference->ARGB, argb, sizeof (argb));
	}
	else
	{
		struct ColorMap *cm = NULL;
		struct DrawInfo *di = NULL;

		IFEELIN F_Do(Render->Display, FM_Get,

			"ColorMap", &cm,
			"DrawInfo", &di,

			TAG_DONE);

		if ((cm != NULL) && (di != NULL))
		{
			uint32 rgb[3];
			uint16 *di_pens = di->dri_Pens;

			IGRAPHICS GetRGB32(cm, di_pens[3], 1, rgb);
			argb[FV_Pen_Shine] = (0x00FF0000 & rgb[0]) | (0x0000FF00 & rgb[1]) | (0x000000FF & rgb[2]);

			IGRAPHICS GetRGB32(cm, di_pens[7], 1, rgb);
			argb[FV_Pen_Fill] = (0x00FF0000 & rgb[0]) | (0x0000FF00 & rgb[1]) | (0x000000FF & rgb[2]);

			IGRAPHICS GetRGB32(cm, di_pens[4], 1, rgb);
			argb[FV_Pen_Dark] = (0x00FF0000 & rgb[0]) | (0x0000FF00 & rgb[1]) | (0x000000FF & rgb[2]);

			IGRAPHICS GetRGB32(cm, di_pens[2], 1, rgb);
			argb[FV_Pen_Text] = (0x00FF0000 & rgb[0]) | (0x0000FF00 & rgb[1]) | (0x000000FF & rgb[2]);

			IGRAPHICS GetRGB32(cm, di_pens[8], 1, rgb);
			argb[FV_Pen_Highlight] = (0x00FF0000 & rgb[0]) | (0x0000FF00 & rgb[1]) | (0x000000FF & rgb[2]);
		}
	}

	/* decode pens */

	entry = &entries[FV_PALETTE_FILL];

	if (entry->type)
	{
		if ((entry->type == FV_COLORTYPE_PALETTE) && (Reference != NULL))
		{
			argb[FV_Pen_Fill] = Reference->ARGB[entry->value];
		}
		else
		{
			argb[FV_Pen_Fill] = entry->value;
		}
	}

	entry = &entries[FV_PALETTE_SHINE];

	if (entry->type)
	{
		if ((entry->type == FV_COLORTYPE_PALETTE) && (Reference != NULL))
		{
			argb[FV_Pen_Shine] = Reference->ARGB[entry->value];
		}
		else
		{
			argb[FV_Pen_Shine] = entry->value;
		}
	}

	entry = &entries[FV_PALETTE_DARK];

	if (entry->type)
	{
		if ((entry->type == FV_COLORTYPE_PALETTE) && (Reference != NULL))
		{
			argb[FV_Pen_Dark] = Reference->ARGB[entry->value];
		}
		else
		{
			argb[FV_Pen_Dark] = entry->value;
		}
	}

	entry = &entries[FV_PALETTE_TEXT];

	if (entry->type)
	{
		if ((entry->type == FV_COLORTYPE_PALETTE) && (Reference != NULL))
		{
			argb[FV_Pen_Text] = Reference->ARGB[entry->value];
		}
		else
		{
			argb[FV_Pen_Text] = entry->value;
		}
	}

	entry = &entries[FV_PALETTE_HIGHLIGHT];

	if (entry->type)
	{
		if ((entry->type == FV_COLORTYPE_PALETTE) && (Reference != NULL))
		{
			argb[FV_Pen_Highlight] = Reference->ARGB[entry->value];
		}
		else
		{
			argb[FV_Pen_Highlight] = entry->value;
		}
	}

	/* adjust colors */

	if ((FF_PALETTE_HAS_CONTRAST | FF_PALETTE_HAS_LUMINANCE | FF_PALETTE_HAS_SATURATION) & flags)
	{
		uint32 con = 100 + Palette->contrast;

		uint8 fr = (0x00FF0000 & argb[FV_Pen_Fill]) >> 16;
		uint8 fg = (0x0000FF00 & argb[FV_Pen_Fill]) >>  8;
		uint8 fb = (0x000000FF & argb[FV_Pen_Fill]);
		uint8 fm = MAX(fr,MAX(fg,fb));

		uint8 sr = fr + ((255 - fr) * con / 200);
		uint8 sg = fg + ((255 - fg) * con / 200);
		uint8 sb = fb + ((255 - fb) * con / 200);
		uint8 sm = MAX(sr, MAX(sb, sg));

		uint8 dr = fr - (fr * con / 200);
		uint8 dg = fg - (fg * con / 200);
		uint8 db = fb - (fb * con / 200);
		uint8 dm = MAX(dr,MAX(db,dg));

		if (FF_PALETTE_HAS_LUMINANCE & flags)
		{
			int32 lum = 255 * Palette->luminance / 200;

			fr = MAX(0, MIN(255, fr + lum));
			fg = MAX(0, MIN(255, fg + lum));
			fb = MAX(0, MIN(255, fb + lum));

			sr = MAX(0, MIN(255, sr + lum));
			sg = MAX(0, MIN(255, sg + lum));
			sb = MAX(0, MIN(255, sb + lum));

			dr = MAX(0, MIN(255, dr + lum));
			dg = MAX(0, MIN(255, dg + lum));
			db = MAX(0, MIN(255, db + lum));
		}

		if (FF_PALETTE_HAS_SATURATION & flags)
		{
			uint32 sat = Palette->saturation;

			sr = sm - ((sm - sr) * sat / 100);
			sg = sm - ((sm - sg) * sat / 100);
			sb = sm - ((sm - sb) * sat / 100);

			dr = dm - ((dm - dr) * sat / 100);
			dg = dm - ((dm - dg) * sat / 100);
			db = dm - ((dm - db) * sat / 100);

			fr = fm - ((fm - fr) * sat / 100);
			fg = fm - ((fm - fg) * sat / 100);
			fb = fm - ((fm - fb) * sat / 100);
		}

		argb[FV_Pen_Shine] = (sr << 16) | (sg << 8) | sb;
		argb[FV_Pen_Dark]  = (dr << 16) | (dg << 8) | db;
		argb[FV_Pen_Fill]  = (fr << 16) | (fg << 8) | fb;
	}

	/* create half colors */

	argb[FV_Pen_Shadow]     = F_ARGB_AVERAGE(argb[FV_Pen_Fill],   argb[FV_Pen_Dark]);
	argb[FV_Pen_HalfShine]  = F_ARGB_AVERAGE(argb[FV_Pen_Shine],  argb[FV_Pen_Fill]);
	argb[FV_Pen_HalfShadow] = F_ARGB_AVERAGE(argb[FV_Pen_Fill],   argb[FV_Pen_Shadow]);
	argb[FV_Pen_HalfDark]   = F_ARGB_AVERAGE(argb[FV_Pen_Shadow], argb[FV_Pen_Dark]);

	#if 1

	return (FPalette *) IFEELIN F_Do(Render->Display, FM_Display_AddPalette, FV_PEN_COUNT, &argb);

	#else
	{
		FPalette *palette = (FPalette *) IFEELIN F_Do(Render->Display, FM_Display_AddPalette, FV_PEN_COUNT, &argb);

		if (palette)
		{
			uint32 i;

			IFEELIN F_Log(0, "palette (0x%08lx) pens (%ld)(0x%08lx) argb (0x%08lx) colors (0x%08lx)",

				palette,
				palette->Count,
				palette->Pens,
				palette->ARGB,
				palette->Colors);

			#if 0

			for (i = 0 ; i < FV_PEN_COUNT ; i++)
			{
				IFEELIN F_Log(0, "[%ld] pen (%3ld) rgb (0x%06lx) color (0x%08lx)(%3ld)(0x%06lx)",

					i,
					palette->Pens[i],
					palette->ARGB[i],
					palette->Colors[i],
					palette->Colors[i]->Pen, palette->Colors[i]->ARGB

				);
			}

			#endif
		}

		return NULL;
	}
	#endif
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Area_Setup
F_METHODM(bool32,Area_Setup,FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct LocalPropertiesData *properties;

	STRPTR value;

	struct in_PaletteProperty *palette;

	struct LocalObjectData *parent_lod;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	_area_render = Msg->Render;

	LOD->properties = properties = F_LPD(Class, (APTR) IFEELIN F_Get(Obj, FA_Element_DecodedStyle));

/************************************************************************************************
*** properties **********************************************************************************
************************************************************************************************/

	/* Properties not defined are inherited for the parent  object.  If  the
	parent  is  a  subclass  of  the Area class we can obtain values easily,
	otherwise we need to create them. */

	#ifdef F_NEW_GLOBALCONNECT
	if (IFEELIN F_Get(_element_parent, FA_Area_PublicData) != 0)
	{
		parent_lod = F_LOD(Class, _element_parent);
	}
	#else
	if (IFEELIN F_Get(_area_parent, FA_Area_PublicData) != 0)
	{
		parent_lod = F_LOD(Class, _area_parent);
	}
	#endif
	else
	{
		parent_lod = NULL;
	}

	LOD->parent_lod = parent_lod;

/*** color scheme ******************************************************************************/

	// neutral

	palette = &properties[FV_Area_State_Neutral].palette;

	if ((palette->flags == 0) && (parent_lod != NULL))
	{
		LOD->flags |= FF_AREA_INHERITED_PALETTE;
		LOD->palette_neutral = parent_lod->palette_neutral;
	}
	else
	{
		LOD->flags &= ~FF_AREA_INHERITED_PALETTE;
		LOD->palette_neutral = palette_create(palette, parent_lod ? parent_lod->palette_neutral : NULL, _area_render);

		if (LOD->palette_neutral == NULL)
		{
			IFEELIN F_Log(0,"unable to create neutral palette");

			return FALSE;
		}
	}

	// focus

	palette = &properties[FV_Area_State_Focus].palette;

	if ((palette->flags == 0) && (parent_lod != NULL))
	{
		LOD->flags |= FF_AREA_INHERITED_PALETTE_FOCUS;
		LOD->palette_focus = parent_lod->palette_focus;
	}
	else
	{
		LOD->flags &= ~FF_AREA_INHERITED_PALETTE_FOCUS;
		LOD->palette_focus = palette_create(palette, LOD->palette_neutral, _area_render);

		if (LOD->palette_focus == NULL)
		{
			IFEELIN F_Log(0,"unable to create ghost palette");

			return FALSE;
		}
	}

	// touch

	palette = &properties[FV_Area_State_Touch].palette;

	if ((palette->flags == 0) && (parent_lod != NULL))
	{
		LOD->flags |= FF_AREA_INHERITED_PALETTE_TOUCH;
		LOD->palette_touch = parent_lod->palette_touch;
	}
	else
	{
		LOD->flags &= ~FF_AREA_INHERITED_PALETTE_TOUCH;
		LOD->palette_touch = palette_create(palette, LOD->palette_neutral, _area_render);

		if (LOD->palette_touch == NULL)
		{
			IFEELIN F_Log(0,"unable to create touch palette");

			return FALSE;
		}
	}

	// ghost

	palette = &properties[FV_Area_State_Ghost].palette;

	if ((palette->flags == 0) && (parent_lod != NULL))
	{
		LOD->flags |= FF_AREA_INHERITED_PALETTE_GHOST;
		LOD->palette_ghost = parent_lod->palette_ghost;
	}
	else
	{
		LOD->flags &= ~FF_AREA_INHERITED_PALETTE_GHOST;
		LOD->palette_ghost = palette_create(palette, LOD->palette_neutral, _area_render);

		if (LOD->palette_ghost == NULL)
		{
			IFEELIN F_Log(0,"unable to create ghost palette");

			return FALSE;
		}
	}

/*** font **************************************************************************************/

	value = properties[FV_Area_State_Neutral].font;

	if ((value == NULL) && (parent_lod != NULL))
	{
		struct TextAttr ta;

		ta.ta_Name  = parent_lod->public.Font->tf_Message.mn_Node.ln_Name;
		ta.ta_YSize = parent_lod->public.Font->tf_YSize;
		ta.ta_Style = parent_lod->public.Font->tf_Style;
		ta.ta_Flags = parent_lod->public.Font->tf_Flags;

		_area_font = IGRAPHICS OpenFont(&ta);
	}

	if (_area_font == NULL)
	{
		_area_font = (struct TextFont *) IFEELIN F_Do(_area_app, FM_Application_OpenFont, Obj, value);
	}

/*** background ********************************************************************************/

	if ((properties[FV_Area_State_Neutral].background != NULL) ||
		(properties[FV_Area_State_Touch].background != NULL) ||
		(properties[FV_Area_State_Focus].background != NULL) ||
		(properties[FV_Area_State_Touch].background != NULL))
	{
		LOD->background = ImageDisplayObject,

			FA_ImageDisplay_Origin, &_area_box,
			FA_ImageDisplay_Owner, Obj,

			End;

		IFEELIN F_Do(LOD->background, FM_ImageDisplay_Setup, _area_render);
	}

	LOD->background_inherited = parent_lod ? parent_lod->public.Background : NULL;

	//IFEELIN F_Log(0, "background (0x%08lx) inherited (0x%08lx)", LOD->background, LOD->background_inherited);


	/*
	LOD->flags &= ~(FF_AREA_INHERITED_BACK | FF_AREA_INHERITED_BACK_FOCUS | FF_AREA_INHERITED_BACK_TOUCH | FF_AREA_INHERITED_BACK_GHOST);

	// neutral

	value = properties[FV_Area_State_Neutral].background;

	if ((value == NULL) && (parent_lod != NULL))
	{
		LOD->flags |= FF_AREA_INHERITED_BACK;

		#ifdef F_CODE_USE_ERASER

		#else

		LOD->background = parent_lod->background;

		#endif
	}
	else
	{
		if (value == NULL)
		{
			value = "fill";
		}

		LOD->background_neutral = ImageDisplayObject,

			FA_ImageDisplay_Spec, value,
			FA_ImageDisplay_Origin, &_area_box,
			FA_ImageDisplay_Owner, Obj,

			End;

		if (LOD->background_neutral == NULL)
		{
			return FALSE;
		}

		IFEELIN F_Do(LOD->background_neutral, FM_ImageDisplay_Setup, Msg->Render);
	}

	// touch

	value = properties[FV_Area_State_Touch].background;

	if (value != NULL)
	{
		LOD->background_touch = ImageDisplayObject,

			FA_ImageDisplay_Spec, value,
			FA_ImageDisplay_Origin, &_area_box,
			FA_ImageDisplay_Owner, Obj,

			End;

		IFEELIN F_Do(LOD->background_touch, FM_ImageDisplay_Setup, Msg->Render);
	}

	if (LOD->background_touch == NULL)
	{
		LOD->flags |= FF_AREA_INHERITED_BACK_TOUCH;

		#ifdef F_CODE_USE_ERASER

		#else

		if (parent_lod != NULL)
		{
			if (parent_lod->background != parent_lod->background_touch)
			{
				LOD->background_touch = parent_lod->background_touch;
			}
		}

		if (LOD->background_touch == NULL)
		{
			LOD->background_touch = LOD->background;
		}

		#endif
	}

	// focus

	value = properties[FV_Area_State_Focus].background;

	if (value != NULL)
	{
		LOD->background_focus = ImageDisplayObject,

			FA_ImageDisplay_Spec, value,
			FA_ImageDisplay_Origin, &_area_box,
			FA_ImageDisplay_Owner, Obj,

			End;

		IFEELIN F_Do(LOD->background_focus, FM_ImageDisplay_Setup, Msg->Render);
	}

	if (LOD->background_focus == NULL)
	{
		LOD->flags |= FF_AREA_INHERITED_BACK_FOCUS;

		#ifdef F_CODE_USE_ERASER

		#else

		if (parent_lod != NULL)
		{
			if (parent_lod->background != parent_lod->background_focus)
			{
				LOD->background_focus = parent_lod->background_focus;
			}
		}

		if (LOD->background_focus == NULL)
		{
			LOD->background_focus = LOD->background;
		}

		#endif
	}

	// ghost

	value = properties[FV_Area_State_Ghost].background;

	if (value != NULL)
	{
		LOD->background_ghost = ImageDisplayObject,

			FA_ImageDisplay_Spec, value,
			FA_ImageDisplay_Origin, &_area_box,
			FA_ImageDisplay_Owner, Obj,

			End;

		IFEELIN F_Do(LOD->background_ghost, FM_ImageDisplay_Setup, Msg->Render);
	}

	if (LOD->background_ghost == NULL)
	{
		LOD->flags |= FF_AREA_INHERITED_BACK_GHOST;

		#ifdef F_CODE_USE_ERASER

		#else

		if (parent_lod != NULL)
		{
			if (parent_lod->background != parent_lod->background_ghost)
			{
				LOD->background_ghost = parent_lod->background_ghost;
			}
		}

		if (LOD->background_ghost == NULL)
		{
			LOD->background_ghost = LOD->background;
		}

		#endif
	}
	*/

/*** set the values for the current state ******************************************************/

	area_set_state(Class, Obj);

	return TRUE;
}
//+
///Area_Cleanup
F_METHOD(bool32,Area_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_area_flags &= ~FF_Area_Drawable;

	if (_area_render != NULL)
	{
		_area_palette = NULL;

		// palette

		if (FF_AREA_INHERITED_PALETTE & LOD->flags)
		{
			LOD->flags &= ~FF_AREA_INHERITED_PALETTE;
		}
		else if (LOD->palette_neutral != NULL)
		{
			#ifdef DB_CLEANUP
			IFEELIN F_Log(0, "remove palette (0x%08lx)", LOD->palette_neutral);
			#endif
			
			IFEELIN F_Do(_area_display, FM_Display_RemPalette, LOD->palette_neutral);
		}

		LOD->palette_neutral = NULL;

		// palette:focus

		if (FF_AREA_INHERITED_PALETTE_FOCUS & LOD->flags)
		{
			LOD->flags &= ~FF_AREA_INHERITED_PALETTE_FOCUS;
		}
		else if (LOD->palette_focus)
		{
			#ifdef DB_CLEANUP
			IFEELIN F_Log(0, "remove palette:focus (0x%08lx)", LOD->palette_focus);
			#endif

			IFEELIN F_Do(_area_display, FM_Display_RemPalette, LOD->palette_focus);
		}

		LOD->palette_focus = NULL;

		// palette:touch

		if (FF_AREA_INHERITED_PALETTE_TOUCH & LOD->flags)
		{
			#ifdef DB_CLEANUP
			IFEELIN F_Log(0, "inherited palette:touch (0x%08lx)", LOD->palette_touch);
			#endif

			LOD->flags &= ~FF_AREA_INHERITED_PALETTE_TOUCH;
		}
		else if (LOD->palette_touch)
		{
			#ifdef DB_CLEANUP
			IFEELIN F_Log(0, "remove palette:touch (0x%08lx)", LOD->palette_touch);
			#endif

			IFEELIN F_Do(_area_display, FM_Display_RemPalette, LOD->palette_touch);
		}

		LOD->palette_touch = NULL;

		// palette:ghost

		if (FF_AREA_INHERITED_PALETTE_GHOST & LOD->flags)
		{
			LOD->flags &= ~FF_AREA_INHERITED_PALETTE_GHOST;
		}
		else if (LOD->palette_ghost)
		{
			#ifdef DB_CLEANUP
			IFEELIN F_Log(0, "remove palette:ghost (0x%08lx)", LOD->palette_ghost);
			#endif
			
			IFEELIN F_Do(_area_display, FM_Display_RemPalette, LOD->palette_ghost);
		}

		LOD->palette_ghost = NULL;

/** backgrounds ********************************************************************************/

		_area_background = NULL;

		if (LOD->background != NULL)
		{
			IFEELIN F_Do(LOD->background, FM_ImageDisplay_Cleanup);
			IFEELIN F_DisposeObj(LOD->background);

			LOD->background = NULL;
		}

		F_SUPERDO();

		_area_render = NULL;
	}

	if (_area_font != NULL)
	{
		IGRAPHICS CloseFont(_area_font);

		_area_font = NULL;
	}

	return 0;
}
//+
///Area_CreateDecodedStyle
F_METHODM(bool32, Area_CreateDecodedStyle, FS_Element_CreateDecodedStyle)
{
	struct LocalPropertiesData *local = F_LPD(Class, Msg->DecodedSpace);

	FPreferenceStyle *style;

	for (style = Msg->Style ; style ; style = style->Next)
	{
		struct LocalPropertiesData *state;
		struct in_PaletteProperty *palette;

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

		palette = &state->palette;
		palette_init(palette, i);

		if (i == FV_Area_State_Ghost)
		{
			state->background = DEF_AREA_BACKGROUND_GHOST;
		}

/*** parse properties **************************************************************************/

		for (properties_a = style->Properties ; *properties_a ; properties_a++)
		{
			FPreferenceProperty *prop = *properties_a;

//            IFEELIN F_Log(0, "%s: %s;", prop->Atom->Key, prop->Value);

			switch(prop->Atom->KeyLength)
			{
				case 4:
				{
					if (prop->Atom == F_PROPERTY_ATOM(FONT))
					{
						state->font = prop->Value;
					}
				}
				break;

				case 5:
				{
					// dimensions

					if (prop->Atom == F_PROPERTY_ATOM(WIDTH))
					{
						if (IFEELIN F_StrCmp("auto", prop->Value, ALL) == 0)
						{
							state->flags |= FF_AREA_AUTO_W;
						}
						else if (prop->NumericType == FV_TYPE_PIXEL)
						{
			                state->width = prop->NumericValue;

							state->flags |= FF_AREA_DEFINED_W;
						}
						else
						{
							IFEELIN F_Log(FV_LOG_DEV, "type (0x%08lx) of (%s) is invalid for the 'width' property", prop->NumericType, prop->Value);
						}
					}
					
					// palette

					else if (prop->Atom == F_PROPERTY_ATOM(COLOR))
					{
						palette_set_entry(palette, FV_PALETTE_TEXT, prop->Value, ALL);
					}
				}
				break;

				case 6:
				{
					// dimensions

					if (prop->Atom == F_PROPERTY_ATOM(HEIGHT))
					{
						if (IFEELIN F_StrCmp("auto", prop->Value, ALL) == 0)
						{
							state->flags |= FF_AREA_AUTO_H;
						}
						else if (prop->NumericType == FV_TYPE_PIXEL)
						{
			                state->height = prop->NumericValue;

							state->flags |= FF_AREA_DEFINED_H;
						}
						else
						{
							IFEELIN F_Log(FV_LOG_DEV, "type (0x%08lx) of (%s) is invalid for the 'width' property", prop->NumericType, prop->Value);
						}
					}
					
					// marings

					else if (prop->Atom == F_PROPERTY_ATOM(MARGIN))
					{
						f_decode_padding(prop->Value, &state->margin);
					}
				}
				break;

				case 7:
				{
					if (prop->Atom == F_PROPERTY_ATOM(PADDING))
					{
						f_decode_padding(prop->Value, &state->padding);
					}

					// palette

					else if (prop->Atom == F_PROPERTY_ATOM(PALETTE))
					{
						f_parse_values(prop->Value, (f_parse_values_func) palette_parse, palette);
					}
				}
				break;

				case 9:
				{
					// limits

					if (prop->Atom == F_PROPERTY_ATOM(MIN_WIDTH))
					{
						if (prop->NumericType == FV_TYPE_PIXEL)
						{
			                state->minmax.MinW = prop->NumericValue;

							state->flags |= FF_AREA_DEFINED_MIN_W;
						}
						else
						{
							IFEELIN F_Log(FV_LOG_DEV, "type (0x%08lx) of (%s) is invalid for the 'min-width' property", prop->NumericType, prop->Value);
						}
					}
					else if (prop->Atom == F_PROPERTY_ATOM(MAX_WIDTH))
					{
						if (prop->NumericType == FV_TYPE_PIXEL)
						{
			                state->minmax.MaxW = prop->NumericValue;

							state->flags |= FF_AREA_DEFINED_MAX_W;
						}
						else
						{
							IFEELIN F_Log(FV_LOG_DEV, "type (0x%08lx) of (%s) is invalid for the 'max-width' property", prop->NumericType, prop->Value);
						}
					}
				}
				break;

				case 10:
				{
					if (prop->Atom == F_PROPERTY_ATOM(MIN_HEIGHT))
					{
						if (prop->NumericType == FV_TYPE_PIXEL)
						{
			                state->minmax.MinH = prop->NumericValue;

							state->flags |= FF_AREA_DEFINED_MIN_H;
						}
						else
						{
							IFEELIN F_Log(FV_LOG_DEV, "type (0x%08lx) of (%s) is invalid for the 'min-height' property", prop->NumericType, prop->Value);
						}
					}
					else if (prop->Atom == F_PROPERTY_ATOM(MAX_HEIGHT))
					{
						if (prop->NumericType == FV_TYPE_PIXEL)
						{
			                state->minmax.MaxH = prop->NumericValue;

							state->flags |= FF_AREA_DEFINED_MAX_H;
						}
						else
						{
							IFEELIN F_Log(FV_LOG_DEV, "type (0x%08lx) of (%s) is invalid for the 'max-height' property", prop->NumericType, prop->Value);
						}
					}
					
					// background

					else if (prop->Atom == F_PROPERTY_ATOM(BACKGROUND))
					{
						state->background = prop->Value;
					}
				}
				break;

				case 11:
				{
					if (prop->Atom == F_PROPERTY_ATOM(MARGIN_TOP))
					{
						state->margin.t = prop->NumericValue;
					}
					else if (prop->Atom == F_PROPERTY_ATOM(PADDING_TOP))
					{
						state->padding.t = prop->NumericValue;
					}
				}
				break;

				case 12:
				{
					if (prop->Atom == F_PROPERTY_ATOM(MARGIN_LEFT))
					{
						state->margin.l = prop->NumericValue;
					}
					else if (prop->Atom == F_PROPERTY_ATOM(PADDING_LEFT))
					{
						state->padding.l = prop->NumericValue;
					}
					
					// color-scheme >> DEPRECATED

					else if (prop->Atom == F_PROPERTY_ATOM(COLOR_SCHEME))
					{
						IFEELIN F_Log(FV_LOG_DEV, "'color-scheme' property is deprectaed, please, use the 'palette' property instead");
						//state->palette = prop->Value;
					}

					// palette

					else if (prop->Atom == F_PROPERTY_ATOM(PALETTE_FILL))
					{
						palette_set_entry(palette, FV_PALETTE_FILL, prop->Value, ALL);
					}
					else if (prop->Atom == F_PROPERTY_ATOM(PALETTE_DARK))
					{
						palette_set_entry(palette, FV_PALETTE_DARK, prop->Value, ALL);
					}
				}
				break;

				case 13:
				{
					if (prop->Atom == F_PROPERTY_ATOM(MARGIN_RIGHT))
					{
						state->margin.r = prop->NumericValue;
					}
					else if (prop->Atom == F_PROPERTY_ATOM(PADDING_RIGHT))
					{
						state->padding.r = prop->NumericValue;
					}

					// palette

					else if (prop->Atom == F_PROPERTY_ATOM(PALETTE_SHINE))
					{
						palette_set_entry(palette, FV_PALETTE_SHINE, prop->Value, ALL);
					}
				}
				break;

				case 14:
				{
					if (prop->Atom == F_PROPERTY_ATOM(MARGIN_BOTTOM))
					{
						state->margin.b = prop->NumericValue;
					}
					else if (prop->Atom == F_PROPERTY_ATOM(PADDING_BOTTOM))
					{
						state->padding.b = prop->NumericValue;
					}
				}
				break;

				case 16:
				{
					// palette

					if (prop->Atom == F_PROPERTY_ATOM(PALETTE_CONTRAST))
					{
						palette_set_contrast(palette, prop->Value, prop->NumericValue);
					}
				}
				break;

				case 17:
				{
					// palette

					if (prop->Atom == F_PROPERTY_ATOM(PALETTE_HIGHLIGHT))
					{
						palette_set_entry(palette, FV_PALETTE_HIGHLIGHT, prop->Value, ALL);
					}
					else if (prop->Atom == F_PROPERTY_ATOM(PALETTE_LUMINANCE))
					{
						palette_set_luminance(palette, prop->Value, prop->NumericValue);
					}
				}
				break;

				case 18:
				{
					// palette

					if (prop->Atom == F_PROPERTY_ATOM(PALETTE_SATURATION))
					{
						palette_set_saturation(palette, prop->Value, prop->NumericValue);
					}

				}
				break;
			}
		}

/*** clone neutral *****************************************************************************/

		#if 0
		{
			struct TagItem *item, *Tags = palette_tags;

			while ((item = IUTILITY NextTagItem(&Tags)) != NULL)
			{
				IFEELIN F_Log(0, "TAG (0x%08lx) DATA (0x%08lx)", item->ti_Tag, item->ti_Data);
			}

		}
		#endif

		if (i == FV_Area_State_Neutral)
		{
			IEXEC CopyMem(state, &local[FV_Area_State_Touch], sizeof (struct LocalPropertiesData));
			IEXEC CopyMem(state, &local[FV_Area_State_Focus], sizeof (struct LocalPropertiesData));
			IEXEC CopyMem(state, &local[FV_Area_State_Ghost], sizeof (struct LocalPropertiesData));
		}
	}

	return TRUE;
}
//+
