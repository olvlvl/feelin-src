#include "Private.h"

//#define DB_VERBOSE_BAD_COORDINATES

#include <feelin/support/f_parse_values.h>
#include <feelin/support/f_parse_values.c>
#include <feelin/support/f_decode_rgb.h>
#include <feelin/support/f_decode_rgb.c>

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

/*

	PICTURE & BRUSHES
	*****************

		url()

		no-repeat
		repeat
		repeat-x
		repeat-y

		no-scale
		scale
		scale-nearest
		scale-average
		scale-bilinear

		orientation-clockwise (add 90° when owner is horizontal)
		orientation-anticlockwise (remove 90° when owner is horizontal)

		>> supprimer les brosses: il faut un moyen d'identifier une image en
		tant que brosse... peut-être en fonction du nombre de couleurs et de
		la palette...

		>> ajouter un mot-clé qui  permettrait  d'imposer  les  couleurs  du
		scheme sur une image.

*/

///id_parse_func
bool32 id_parse_func(STRPTR Key, uint32 KeyLength, struct in_Parse *Parse)
{
	uint8 c = *Key;

	// hexadecimal

	if (c == '#')
	{
		int32 value;
		//uint32 len = stch_l(Key + 1, &value);
		uint32 len = f_stch(Key + 1, &value);

		switch (len)
		{
			case 3:
			{
				value = ((0x00000F00 & value) << 12) | ((0x00000F00 & value) << 8) |
						((0x000000F0 & value) <<  8) | ((0x000000F0 & value) << 4) |
						((0x0000000F & value) <<  4) | (0x0000000F & value);
			}
			case 6: break;

			default: return FALSE;
		}

		if (Parse->colors_count == FV_IMAGEDISPLAY_COLORS_MAX)
		{
			return FALSE;
		}

		Parse->colors_type[Parse->colors_count] = FV_COLORTYPE_RGB;
		Parse->colors_value[Parse->colors_count] = value;
		Parse->colors_count++;

		return TRUE;
	}

	// pointer

	if ((c == '0') && (Key[1] == 'x'))
	{
		int32 value;
		//uint32 len = stch_l(Key + 2, &value);
		uint32 len = f_stch(Key + 2, &value);

		if (len != 8)
		{
			return FALSE;
		}
		
		Parse->hook = (struct Hook *) value;

		return TRUE;
	}

	// integer

	else if ((c == '-') || ((c >= '0') && (c <= '9')))
	{
		int32 value;
		uint32 len = f_stcd(Key, &value);

		if (len == 0)
		{
			return FALSE;
		}
			
		if (Key[len] == 0xB0) // '°' for GCC
		{
			Parse->gradient_angle = value;
		}
		else if (Key[len] == '%')
		{
			if (Parse->bitmap_position_x_type == 0)
			{
				Parse->bitmap_position_x_type = FV_TYPE_PERCENTAGE;
				Parse->bitmap_position_x = value;
			}
			else if (Parse->bitmap_position_y_type == 0)
			{
				Parse->bitmap_position_y_type = FV_TYPE_PERCENTAGE;
				Parse->bitmap_position_y = value;
			}
			else
			{
				return FALSE;
			}
		}
		else if ((Key[len] == 'p') && (Key[len+1] == 'x'))
		{
			if (Parse->bitmap_position_x_type == 0)
			{
				Parse->bitmap_position_x_type = FV_TYPE_PIXEL;
				Parse->bitmap_position_x = value;
			}
			else if (Parse->bitmap_position_y_type == 0)
			{
				Parse->bitmap_position_y_type = FV_TYPE_PIXEL;
				Parse->bitmap_position_y = value;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}

		return TRUE;
	}

	// url (picture or brush)

	else if ((c == 'u') && (Key[1] == 'r') && (Key[2] == 'l') && (Key[3] == '('))
	{
		if (Key[KeyLength - 1] != ')')
		{
			return FALSE;
		}

		Parse->bitmap_url = IFEELIN F_NewP(CUD->pool, KeyLength - 4);

		if (Parse->bitmap_url == NULL)
		{
			return FALSE;
		}

		Parse->bitmap_url_length = KeyLength - 5;

		IEXEC CopyMem(Key + 4, Parse->bitmap_url, Parse->bitmap_url_length);

		return TRUE;
	}

	// rgb

	else if ((c == 'r') && (Key[1] == 'g') && (Key[2] == 'b') && (Key[3] == '('))
	{
		int32 value;

		if (f_decode_rgb(Key + 4, (uint32 *) &value))
		{
			if (Parse->colors_count == FV_IMAGEDISPLAY_COLORS_MAX)
			{
				return FALSE;
			}

			Parse->colors_type[Parse->colors_count] = FV_COLORTYPE_RGB;
			Parse->colors_value[Parse->colors_count] = value;
			Parse->colors_count++;

			return TRUE;
		}
	}

	// keywords

	else
	{
		bits32 type = 0;
		uint32 value = 0;

		switch (KeyLength)
		{
			case 3:
			{
				// colors

				if (IUTILITY Strnicmp("red", Key, 3) == 0)
				{
					value = 0xFF0000;
					type = FV_COLORTYPE_RGB;
				}

				// bitmap

				else if (IUTILITY Strnicmp("top", Key, 3) == 0)
				{
					Parse->bitmap_position_y = 0;
					Parse->bitmap_position_y_type = FV_TYPE_PIXEL;

					return TRUE;
				}
			}
			break;

			case 4:
			{
				// entries

				if (IUTILITY Strnicmp("text", Key, 4) == 0)
				{
					value = FV_Pen_Text;
					type = FV_COLORTYPE_ENTRY;
				}
				else if (IUTILITY Strnicmp("fill", Key, 4) == 0)
				{
					value = FV_Pen_Fill;
					type = FV_COLORTYPE_ENTRY;
				}
				else if (IUTILITY Strnicmp("dark", Key, 4) == 0)
				{
					value = FV_Pen_Dark;
					type = FV_COLORTYPE_ENTRY;
				}

				// colors

				else if (IUTILITY Strnicmp("gray", Key, 4) == 0)
				{
					value = 0x808080;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("lime", Key, 4) == 0)
				{
					value = 0x00FF00;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("blue", Key, 4) == 0)
				{
					value = 0x0000FF;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("navy", Key, 4) == 0)
				{
					value = 0x000080;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("teal", Key, 4) == 0)
				{
					value = 0x008080;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("aqua", Key, 4) == 0)
				{
					value = 0x00FFFF;
					type = FV_COLORTYPE_RGB;
				}
				
				// patterns

				else if (IUTILITY Strnicmp("dots", Key, 4) == 0)
				{
					Parse->pattern = FV_PATTERN_DOTS;

					return TRUE;
				}

				// bitmap

				else if (IUTILITY Strnicmp("left", Key, 4) == 0)
				{
					#if 0
					if (Parse->bitmap_position_x_type != 0)
					{
						return FALSE;
					}
					#endif

					Parse->bitmap_position_x = 0;
					Parse->bitmap_position_x_type = FV_TYPE_PIXEL;

					return TRUE;
				}
			}
			break;

			case 5:
			{
				// entries

				if (IUTILITY Strnicmp("shine", Key, 5) == 0)
				{
					value = FV_Pen_Shine;
					type = FV_COLORTYPE_ENTRY;
				}

				// colors

				if (IUTILITY Strnicmp("black", Key, 5) == 0)
				{
					value = 0x000000;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("white", Key, 5) == 0)
				{
					value = 0xFFFFFF;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("green", Key, 5) == 0)
				{
					value = 0x008000;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("olive", Key, 5) == 0)
				{
					value = 0x808000;
					type = FV_COLORTYPE_RGB;
				}

				// bitmap

				else if (IUTILITY Strnicmp("right", Key, 5) == 0)
				{
					#if 0
					if (Parse->bitmap_position_x_type != 0)
					{
						return FALSE;
					}
					#endif

					Parse->bitmap_position_x = 100;
					Parse->bitmap_position_x_type = FV_TYPE_PERCENTAGE;

					return TRUE;
				}

				// patterns

				else if (IUTILITY Strnicmp("small", Key, 5) == 0)
				{
					Parse->pattern_size = FF_PATTERN_SMALL;
				}
				else if (IUTILITY Strnicmp("large", Key, 5) == 0)
				{
					Parse->pattern_size = FF_PATTERN_LARGE;
				}
			}
			break;

			case 6:
			{
				// entries

				if (IUTILITY Strnicmp("shadow", Key, 6) == 0)
				{
					value = FV_Pen_Shadow;
					type = FV_COLORTYPE_ENTRY;
				}

				// colors

				else if (IUTILITY Strnicmp("silver", Key, 6) == 0)
				{
					value = 0x00C0C0;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("maroon", Key, 6) == 0)
				{
					value = 0x800000;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("purple", Key, 6) == 0)
				{
					value = 0x800080;
					type = FV_COLORTYPE_RGB;
				}
				else if (IUTILITY Strnicmp("yellow", Key, 6) == 0)
				{
					value = 0xFFFF00;
					type = FV_COLORTYPE_RGB;
				}

				// pattern

				else if (IUTILITY Strnicmp("groove", Key, 6) == 0)
				{
					Parse->pattern = FV_PATTERN_GROOVE;

					return TRUE;
				}
				else if (IUTILITY Strnicmp("xsmall", Key, 6) == 0)
				{
					Parse->pattern_size = FF_PATTERN_XSMALL;

					return TRUE;
				}
				else if (IUTILITY Strnicmp("medium", Key, 6) == 0)
				{
					Parse->pattern_size = FF_PATTERN_MEDIUM;

					return TRUE;
				}

				// bitmap

				else if (IUTILITY Strnicmp("repeat", Key, 6) == 0)
				{
					Parse->bitmap_repeat = FV_BITMAP_REPEAT;

					return TRUE;
				}
				else if (IUTILITY Strnicmp("bottom", Key, 6) == 0)
				{
					#if 0
					if (Parse->bitmap_position_y_type != 0)
					{
						return FALSE;
					}
					#endif

					Parse->bitmap_position_y = 100;
					Parse->bitmap_position_y_type = FV_TYPE_PERCENTAGE;

					return TRUE;
				}
				else if (IUTILITY Strnicmp("center", Key, 6) == 0)
				{
					if (Parse->bitmap_position_x_type == 0)
					{
						Parse->bitmap_position_x = 50;
						Parse->bitmap_position_x_type = FV_TYPE_PERCENTAGE;
					}

					if (Parse->bitmap_position_y_type == 0)
					{
						Parse->bitmap_position_y = 50;
						Parse->bitmap_position_y_type = FV_TYPE_PERCENTAGE;
					}

					return TRUE;
				}

			}
			break;

			case 7:
			{
				// colors

				if (IUTILITY Strnicmp("fuchsia", Key, 7) == 0)
				{
					value = 0xFF00FF;
					type = FV_COLORTYPE_RGB;
				}

				// patterns

				else if (IUTILITY Strnicmp("oblique", Key, 7) == 0)
				{
					Parse->pattern = FV_PATTERN_OBLIQUE;

					return TRUE;
				}
				else if (IUTILITY Strnicmp("stripes", Key, 7) == 0)
				{
					Parse->pattern = FV_PATTERN_STRIPES;

					return TRUE;
				}
			}
			break;
			
			case 8:
			{
				// entries

				if (IUTILITY Strnicmp("halfdark", Key, 8) == 0)
				{
					value = FV_Pen_HalfDark;
					type = FV_COLORTYPE_ENTRY;
				}

				// bitmap

				else if (IUTILITY Strnicmp("repeat-x", Key, 8) == 0)
				{
					Parse->bitmap_repeat = FV_BITMAP_REPEAT_X;

					return TRUE;
				}
				else if (IUTILITY Strnicmp("repeat-y", Key, 8) == 0)
				{
					Parse->bitmap_repeat = FV_BITMAP_REPEAT_Y;

					return TRUE;
				}
				else if (IUTILITY Strnicmp("multiple", Key, 8) == 0)
				{
					Parse->bitmap_multiple = TRUE;

					return TRUE;
				}
			}
			break;

			case 9:
			{
				if (IUTILITY Strnicmp("halfshine", Key, 9) == 0)
				{
					value = FV_Pen_HalfShine;
					type = FV_COLORTYPE_ENTRY;
				}
				else if (IUTILITY Strnicmp("highlight", Key, 9) == 0)
				{
					value = FV_Pen_Highlight;
					type = FV_COLORTYPE_ENTRY;
				}

				// bitmap

				else if (IUTILITY Strnicmp("no-repeat", Key, 9) == 0)
				{
					Parse->bitmap_repeat = FV_BITMAP_REPEAT_NO;

					return TRUE;
				}

				// gradient

				else if (IUTILITY Strnicmp("clockwise", Key, 9) == 0)
				{
					Parse->gradient_clockwise = TRUE;

					return TRUE;
				}

			}
			break;

			case 10:
			{
				// entries

				if (IUTILITY Strnicmp("halfshadow", Key, 10) == 0)
				{
					value = FV_Pen_HalfShadow;
					type = FV_COLORTYPE_ENTRY;
				}
			}
			break;
		}

		if ((type == FV_COLORTYPE_RGB) || (type == FV_COLORTYPE_ENTRY))
		{
			if (Parse->colors_count == FV_IMAGEDISPLAY_COLORS_MAX)
			{
				return FALSE;
			}

			Parse->colors_type[Parse->colors_count] = type;
			Parse->colors_value[Parse->colors_count] = value;
			Parse->colors_count++;

			return TRUE;
		}
	}

	return FALSE;
}
//+

///id_create
void id_create(FClass *Class, FObject Obj, STRPTR Spec)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_CodeTable *methods = NULL;

	struct in_Parse *parse;

	uint32 n;


	parse = IFEELIN F_NewP(CUD->pool, sizeof (struct in_Parse));

	if (parse == NULL)
	{
		return;
	}

	n = f_parse_values(Spec, (f_parse_values_func) &id_parse_func, parse);

	if (n == 0)
	{
		return;
	}

/*** adjust values *****************************************************************************/

	if ((parse->bitmap_position_x_type != 0) && (parse->bitmap_position_y_type == 0))
	{
		parse->bitmap_position_y_type = FV_TYPE_PERCENTAGE;
		parse->bitmap_position_y = 50;
	}

/*** create image ******************************************************************************/

	if (parse->bitmap_url != NULL)
	{
		methods = &id_picture_table;
	}
	else if (parse->hook != NULL)
	{
		methods = &id_hook_table;
	}
	else if (parse->colors_count != 0)
	{
		switch (parse->colors_count)
		{
			case 1:
			{
				methods = &id_color_table;
			}
			break;

			// pattern or gradient

			case 2:
			{
				if (parse->pattern)
				{
					methods = &id_pattern_table;

					break;
				}
			}

			// gradient

			case 3:
			case 4:
			{
				methods = &id_gradient_table;
			}
			break;
		}
	}

	if (methods != NULL)
	{
		LOD->image = methods->create(Obj, Class, parse, CUD->pool);

		if (LOD->image != NULL)
		{
			LOD->image_methods = methods;
		}
	}

/*** dispose parse data ************************************************************************/

	if (parse->bitmap_url != NULL)
	{
		IFEELIN F_Dispose(parse->bitmap_url);
	}

	IFEELIN F_Dispose(parse);

	#if 0

	STRPTR copy;

	uint32 count = 0;

	if (*Spec == '<')
	{
		IFEELIN F_Log(FV_LOG_DEV, "XML defined specifications are no longer supported: (%s)", Spec);

		return;
	}

	#if 0

	copy = IFEELIN F_StrNewP(CUD->pool, NULL, "fill");

	#else

	copy = IFEELIN F_StrNewP(CUD->pool, NULL, "%s", Spec);

	#endif

	if (copy == NULL)
	{
		return;
	}

	id_parse_values(Class, Obj, copy, id_parse_count_func, &count);

	if (count != 0)
	{
		struct in_Parse *parse = IFEELIN F_New(sizeof (struct in_Parse) + sizeof (struct in_Item) * (count + 1));

		if (parse != NULL)
		{
			uint32 i;
			struct in_CodeTable *methods = NULL;

			/* FIXME: default values ?? */

			parse->items = parse->cursor = (struct in_Item *) ((uint32)(parse) + sizeof (struct in_Parse));

			id_parse_values(Class, Obj, copy, id_parse_array_func, parse);

/*** adjust values *****************************************************************************/

			if ((parse->bitmap_position_x_type != 0) && (parse->bitmap_position_y_type == 0))
			{
				parse->bitmap_position_y_type = FV_TYPE_PERCENTAGE;
				parse->bitmap_position_y = 50;
			}

/*** create image ******************************************************************************/

			if (parse->bitmap_url != NULL)
			{
				methods = &id_picture_table;
			}
			else if (parse->hook != NULL)
			{
				methods = &id_hook_table;
			}
			else if (parse->colors_count != 0)
			{
				switch (parse->colors_count)
				{
					case 1:
					{
						methods = &id_color_table;
					}
					break;

					// pattern or gradient

					case 2:
					{
						if (parse->pattern)
						{
							methods = &id_pattern_table;

							break;
						}
					}

					// gradient

					case 3:
					case 4:
					{
						methods = &id_gradient_table;
					}
					break;
				}
			}

			if (methods != NULL)
			{
				LOD->image = methods->create(Obj, Class, parse, CUD->pool);

				if (LOD->image != NULL)
				{
					LOD->image_methods = methods;
				}
			}

			for (i = 0 ; i < count ; i++)
			{
				IFEELIN F_AtomRelease(parse->items[i].atom);
			}

			IFEELIN F_Dispose(parse);
		}
	}

	IFEELIN F_Dispose(copy);

	#endif
}
//+
///id_delete
STATIC void id_delete(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (LOD->image && LOD->image_methods)
	{
		if (LOD->image_methods->delete)
		{
			LOD->image_methods->delete(LOD->image);
		}
	}
}
//+

///id_draw
bool32 id_draw(APTR Image, in_Code_Draw *Func, FRender *Render, FBox *Origin, FPalette *Palette, FRect *Rect, bits32 Flags, FClass *Class)
{
	uint32 fill = Render->Palette->Pens[FV_Pen_Fill];

	struct in_DrawMessage msg;

	msg.Render  = Render;
	msg.Origin  = Origin;
	msg.Palette = Palette;
	msg.Flags   = Flags;

	if ((FF_ImageDisplay_Draw_Region & Flags) != 0)
	{
		struct RegionRectangle *rr;
		FRect r;
		msg.Rect = &r;

		for (rr = ((struct Region *)(Rect))->RegionRectangle ; rr ; rr = rr->Next)
		{
			r.x1 = ((struct Region *)(Rect))->bounds.MinX + rr->bounds.MinX;
			r.y1 = ((struct Region *)(Rect))->bounds.MinY + rr->bounds.MinY;
			r.x2 = ((struct Region *)(Rect))->bounds.MinX + rr->bounds.MaxX;
			r.y2 = ((struct Region *)(Rect))->bounds.MinY + rr->bounds.MaxY;

			if (r.x1 <= r.x2 && r.y1 <= r.y2)
			{
				if ((Func == NULL) || (Func(Image, &msg, Class) == FALSE))
				{
					IGRAPHICS SetAPen(Render->RPort, fill);
					IGRAPHICS RectFill(Render->RPort, r.x1, r.y1, r.x2, r.y2);
				}
			}
			#ifdef DB_VERBOSE_BAD_COORDINATES
			
			else
			{
				IFEELIN F_Log(0, "invalid coordinates: x (%ld, %ld) y (%ld, %ld)", r.x1, r.x2, r.y1, r.y2);
			}

			#endif
		}
	}
	else
	{
		if ((Rect->x1 <= Rect->x2) && (Rect->y1 <= Rect->y2))
		{
			msg.Rect = Rect;

			if ((Func == NULL) || (Func(Image, &msg, Class) == FALSE))
			{
				IGRAPHICS SetAPen(Render->RPort, fill);
				IGRAPHICS RectFill(Render->RPort, Rect->x1, Rect->y1, Rect->x2, Rect->y2);
			}
		}
		#ifdef DB_VERBOSE_BAD_COORDINATES

		else
		{
			IFEELIN F_Log(0, "invalid coordinates: x (%ld, %ld) y (%ld, %ld)", Rect->x1, Rect->x2, Rect->y1, Rect->y2);
		}

		#endif
	}

	return TRUE;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///ID_Dispose
F_METHOD(uint32,ID_Dispose)
{
/*
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->render)
	{
		IFEELIN F_Do(Obj, FM_ImageDisplay_Cleanup);
	}
*/
	id_delete(Class, Obj);

	return F_SUPERDO();
}
//+
///ID_Set
F_METHOD(uint32,ID_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	bool32 update = FALSE;

	STRPTR spec = NULL;
//	  STRPTR fallback = NULL;

	while (IFEELIN F_DynamicNTI(&Tags,&item,NULL))
	switch (item.ti_Tag)
	{
		case FA_ImageDisplay_Spec:
		{
			spec = (STRPTR) item.ti_Data;
			update = TRUE;
		}
		break;

		case FA_ImageDisplay_Origin:
		{
			LOD->origin = (FBox *) item.ti_Data;
		}
		break;

		case FA_ImageDisplay_Owner:
		{
			LOD->owner = (FObject) item.ti_Data;
			LOD->owner_public = F_GET_AREA_PUBLIC(LOD->owner);

			if ((LOD->owner != NULL) && (LOD->owner_public == NULL))
			{
				IFEELIN F_Log(FV_LOG_DEV, "Owner %s{%08lx} should be an Area subclass", _object_classname(LOD->owner), LOD->owner);
			}
		}
		break;
/*
		case FA_ImageDisplay_Fallback:
		{
			fallback = (STRPTR) item.ti_Data;
			update = TRUE;
		}
		break;
*/
	}

	if (update)
	{
		FRender *render = LOD->render;

		if (render != NULL)
		{
			IFEELIN F_Do(Obj, FM_ImageDisplay_Cleanup, render);
		}

		if (spec)
		{
			id_create(Class, Obj, spec);
		}
/*
		if ((LOD->image == NULL) && (fallback != NULL))
		{
			id_create(Class, Obj, fallback);
		}
*/
		if (render != NULL)
		{
			IFEELIN F_Do(Obj, FM_ImageDisplay_Setup, render);
		}
	}

	return F_SUPERDO();
}
//+
///ID_Get
F_METHOD(uint32,ID_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	APTR image = LOD->image;
	
	in_Code_Get *func = NULL;

	if (LOD->image_methods != NULL)
	{
		func = LOD->image_methods->get;
	}

	while (IFEELIN F_DynamicNTI(&Tags,&item,NULL))
	switch (item.ti_Tag)
	{
		case FA_ImageDisplay_Width:
		case FA_ImageDisplay_Height:
		{
			if (func != NULL)
			{
				F_STORE(func(image, Class, item.ti_Tag));
			}
			else F_STORE(8);
		}
		break;

		case FA_ImageDisplay_Mask:
		{
			if (func != NULL)
			{
				F_STORE(func(image, Class, item.ti_Tag));
			}
			else F_STORE(FALSE);
		}
		break;

		case FA_ImageDisplay_Owner:
		{
			F_STORE(LOD->owner);
		}
		break;
	}

	return F_SUPERDO();
}
//+

///ID_Setup
F_METHODM(bool32,ID_Setup,FS_ImageDisplay_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Render == NULL)
	{
		return FALSE;
	}

	LOD->render = Msg->Render;

	if ((LOD->image == NULL) || (LOD->image_methods == NULL))
	{
		return TRUE;
	}

	if (LOD->image_methods->setup == NULL)
	{
		return TRUE;
	}

	return LOD->image_methods->setup(Class, LOD->image, LOD->render);
}
//+
///ID_Cleanup
F_METHOD(void,ID_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->render)
	{
		if ((LOD->image != NULL) && (LOD->image_methods != NULL) && (LOD->image_methods->cleanup != NULL))
		{
			LOD->image_methods->cleanup(Class, LOD->image, LOD->render);
		}

		LOD->render = NULL;
	}
}
//+

///ID_Draw
F_METHODM(bool32,ID_Draw,FS_ImageDisplay_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	APTR data;

	in_Code_Draw *func;

	if (Msg->Render == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Render is NULL");

		return FALSE;
	}

	if (Msg->Render->RPort == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "RPort is NULL for Render (0x%08lx)", Msg->Render);

		return FALSE;
	}

	if (Msg->Rect == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Rectangle is NULL", Msg->Rect);

		return FALSE;
	}

	if (LOD->image && LOD->image_methods)
	{
		data = LOD->image;
		func = LOD->image_methods->draw;
	}
	else
	{
		data = NULL;
		func = NULL;
	}

	return id_draw
	(
		data,
		func,

		Msg->Render,
		LOD->origin,
		LOD->owner_public ? LOD->owner_public->Palette : Msg->Render->Palette,
		Msg->Rect,
		Msg->Flags,

		Class
	);
}
//+

#if 0

//#define DB_ASSOCIATED

///id_image_new
static APTR id_image_new(FClass *Class, STRPTR Spec, struct in_CodeTable **TablePtr, APTR Pool)
{
	APTR data=NULL;

	if (Spec == NULL)
	{
		return NULL;
	}
 
	if (*Spec == '<')
	{
		#if 1

		FXMLMarkup *markup;

		IFEELIN F_Do(CUD->XMLDocument,FM_Lock,FF_Lock_Exclusive);

		IFEELIN F_Do(CUD->XMLDocument,FM_Set,

			F_ID(CUD->IDs,FA_Document_Source), Spec,
			F_ID(CUD->IDs,FA_Document_SourceType), FV_Document_SourceType_Memory,

			TAG_DONE);

		for (markup = (FXMLMarkup *) IFEELIN F_Get(CUD->XMLDocument,F_ID(CUD->IDs,FA_XMLDocument_Markups)) ; markup ; markup = markup->Next)
		{
			if (markup->Atom == F_ATOM(IMAGE))
			{
				FXMLAttribute *attribute;

				for (attribute = (FXMLAttribute *) markup->AttributesList.Head ; attribute ; attribute = attribute->Next)
				{
					if (attribute->Atom == F_ATOM(TYPE))
					{
						STATIC FDOCValue image_type_values[] =
						{
							{ "raster",    (uint32) &id_raster_table    },
							{ "gradient",  (uint32) &id_gradient_table  },
							{ "brush",     (uint32) &id_brush_table     },
							{ "picture",   (uint32) &id_picture_table   },
							{ "hook",      (uint32) &id_hook_table      },

							F_ARRAY_END
						};
					   
						struct in_CodeTable *table = (struct in_CodeTable *) IFEELIN F_Do(CUD->XMLDocument,F_ID(CUD->IDs,FM_Document_Resolve),attribute->Value,0,image_type_values,NULL);
					   
						#ifdef DB_ASSOCIATED
						IFEELIN F_Log(0,"type (%s) table (0x%08lx)", attribute->Value, table);
						#endif
						
						if (table)
						{
							if ((data = table->create(Class, (STRPTR) markup->AttributesList.Head, Pool)) != NULL)
							{
								*TablePtr = table;
							}
						}
						else
						{
							IFEELIN F_Log(0,"Unsupported type (%s) in (%s)",attribute->Value,Spec);
						}
					}
				}
				break;
			}
		}
		
		IFEELIN F_Do(CUD->XMLDocument,FM_Unlock);

		#endif
	}
	else
	{
		if ((data = id_color_table.create(Class, Spec, Pool)) != NULL)
		{
			*TablePtr = &id_color_table;
		}
	}

	return data;
}
//+
///id_image_dispose
static void id_image_dispose(struct in_CodeTable *Table,APTR Data)
{
	#ifdef DB_DELETE
	IFEELIN F_Log(0,"id_image_delete (0x%08lx)",image);
	#endif

//    F_OPool(CUD->Pool);

	if (Table && Table->delete)
	{
		Table->delete(Data);
	}
   
//    F_RPool(CUD->Pool);
}
//+

///id_image_construct
struct FeelinAssociated * id_image_construct(STRPTR Spec, APTR Pool, FClass *Class, FRender *Render)
{
	if (Spec)
	{
		STRPTR rimg;

		struct FeelinAssociated *associated = IFEELIN F_NewP(Pool,sizeof (struct FeelinAssociated));

		if (associated == NULL)
		{
			return FALSE;
		}

		if ((rimg = IFEELIN F_StrNewP(Pool, NULL, "%s", Spec)) != NULL)
		{
			STRPTR simg = rimg;

			#ifdef DB_ASSOCIATED
			IFEELIN F_Log(0,"spec (%s) separator (%lc)",rimg,FV_ImageDisplay_Separator);
			#endif

			while (*simg && *simg != FV_ImageDisplay_Separator) simg++;

			if (simg && *simg  == FV_ImageDisplay_Separator)
			{
				*simg++ = 0;
			}
			else
			{
				simg = NULL;
			}

			if (rimg)
			{
				associated->data[0] = id_image_new(Class, rimg, &associated->table[0], Pool);

				if (associated->data[0])
				{
					associated->flags |= FF_IMAGE_RENDER;

					if (associated->table[0] &&
						associated->table[0]->setup)
					{
						if (!(associated->table[0]->setup(associated->data[0], Class, Render)))
						{
							id_image_dispose(associated->table[0], associated->data[0]);

							associated->flags &= ~FF_IMAGE_RENDER;
						}
					}
				}

				#ifdef DB_ASSOCIATED
				IFEELIN F_Log(0,"render (%s) >> api (0x%08lx) data (0x%08lx)",rimg,associated->table[0],associated->data[0]);
				#endif
			}

			if (simg)
			{
				associated->data[1] = id_image_new(Class, simg, &associated->table[1], Pool);

				if (associated->data[1])
				{
					associated->flags |= FF_IMAGE_SELECT;

					if (associated->table[1] &&
						associated->table[1]->setup)
					{
						if (!(associated->table[1]->setup(associated->data[1], Class, Render)))
						{
							id_image_dispose(associated->table[1], associated->data[1]);

							associated->flags &= ~FF_IMAGE_SELECT;
						}
					}
				}

				#ifdef DB_ASSOCIATED
				IFEELIN F_Log(0,"select (%s) >> api (0x%08lx) data (0x%08lx)",simg,associated->table[1],associated->data[1]);
				#endif
			}

			IFEELIN F_Dispose(rimg);

			return associated;
		}
	}

	return NULL;
}
//+
///id_image_destruct
void id_image_destruct(struct FeelinAssociated *Associated, FClass *Class, FRender *Render)
{
	#ifdef DB_ASSOCIATED
	IFEELIN F_Log(0,"destruct (0x%08lx) table (0x%08lx)",Msg->Data,associated->table[0]);
	#endif

	if (Associated->table[0])
	{
		if (Associated->table[0]->cleanup)
		{
			Associated->table[0]->cleanup(Associated->data[0], Class, Render);
		}

		if (Associated->table[0]->delete)
		{
			Associated->table[0]->delete(Associated->data[0]);
		}
	}

	if (Associated->table[1])
	{
		if (Associated->table[1]->cleanup)
		{
			Associated->table[1]->cleanup(Associated->data[1], Class, Render);
		}

		if (Associated->table[1]->delete)
		{
			Associated->table[1]->delete(Associated->data[1]);
		}
	}

	IFEELIN F_Dispose(Associated);
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///ID_New
F_METHOD(APTR,ID_New)
{
//    LOD->spec_fallback = "fill";
 
	return (APTR) F_SUPERDO();
}
//+
///ID_Dispose
F_METHOD(void,ID_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	if (LOD->render)
	{
		IFEELIN F_Do(Obj, FM_ImageDisplay_Cleanup);
	}
}
//+
///ID_Get
F_METHOD(void,ID_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	APTR data=NULL;
	in_Code_Get *func=NULL;

	if (LOD->associated != NULL)
	{
		data = LOD->associated->data[0];
		
		if (LOD->associated->table[0] != NULL)
		{
			func = LOD->associated->table[0]->get;
		}
	}

	while (IFEELIN F_DynamicNTI(&Tags,&item,NULL))
	switch (item.ti_Tag)
	{
		case FA_ImageDisplay_Width:
		case FA_ImageDisplay_Height:
		{
			if (func)
			{
				F_STORE(func(data,Class,item.ti_Tag));
			}
			else F_STORE(8);
		}
		break;
		 
		case FA_ImageDisplay_Mask:
		{
			if (func)
			{
				F_STORE(func(data,Class,item.ti_Tag));
			}
			else F_STORE(FALSE);
		}
		break;
	}

	F_SUPERDO();
}
//+
///ID_Set
F_METHOD(void,ID_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	bool32 update = FALSE;

	while (IFEELIN F_DynamicNTI(&Tags,&item,NULL))
	switch (item.ti_Tag)
	{
		case FA_ImageDisplay_Spec:
		{
			LOD->spec = (STRPTR) item.ti_Data;
			
			update = TRUE;
		}
		break;
	 
		case FA_ImageDisplay_Origin:
		{
			LOD->Origin = (FBox *)(item.ti_Data);
		}
		break;

		case FA_ImageDisplay_Default:
		{
			LOD->spec_default = (STRPTR) item.ti_Data;
			
			update = TRUE;
		}
		break;
	
		case FA_ImageDisplay_Fallback:
		{
			LOD->spec_fallback = (STRPTR) item.ti_Data;
			
			update = TRUE;
		}
		break;
	}
	
	if (update)
	{
		FRender *render = LOD->render;

		if (render)
		{
			IFEELIN F_Do(Obj,FM_ImageDisplay_Cleanup,render);
			LOD->render = render;

			IFEELIN F_Do(Obj,FM_ImageDisplay_Setup,render);
		}
	}
 
	F_SUPERDO();
}
//+
///ID_Setup
F_METHODM(uint32,ID_Setup,FS_ImageDisplay_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if ((LOD->render = Msg->Render))
	{
		if (LOD->spec)
		{
			if ((uint32)(LOD->spec) < 255)
			{
				IFEELIN F_Log(0,"FI_%ld is deprecated",LOD->spec);
			
				return FALSE;
			}

			LOD->associated = id_image_construct(LOD->spec, CUD->pool, Class, LOD->render);

			return (uint32)(LOD->associated != NULL);
		}
   }
   return FALSE;
}
//+
///ID_Cleanup
F_METHOD(void,ID_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->render)
	{
		if (LOD->associated)
		{
			id_image_destruct(LOD->associated, Class, LOD->render);
		}
		
		LOD->render = NULL;
	}
}
//+
///ID_Draw
F_METHODM(bool32,ID_Draw,FS_ImageDisplay_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	APTR data=NULL;
	in_Code_Draw *func=NULL;

	if (Msg->Render == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Render is NULL");

		return FALSE;
	}

	if (Msg->Render->RPort == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "RPort is NULL for Render (0x%08lx)", Msg->Render);

		return FALSE;
	}

	if (Msg->Rect == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Rectangle is NULL", Msg->Rect);

		return FALSE;
	}

	if (LOD->associated)
	{
		data = LOD->associated->data[0];

		if (LOD->associated->table[0])
		{
			func = LOD->associated->table[0]->draw;
		}
												
		if (FF_ImageDisplay_Draw_Select & Msg->Flags)
		{
			if (LOD->associated->table[1] && LOD->associated->data[1])
			{
				data = LOD->associated->data[1];
		 
				if (LOD->associated->table[1])
				{
					func = LOD->associated->table[1]->draw;
				}
			}
		}
 
//        IFEELIN F_Log(0,"associated (0x%08lx) data (0x%08lx) func (0x%08lx)",LOD->associated,data,func);
	}
	 
	if (func)
	{
		struct in_DrawMessage msg;

		msg.Render  = Msg->Render;
		msg.Origin  = LOD->Origin;
		msg.Flags   = LOD->flags;
			
		if ((FF_ImageDisplay_Draw_Region & Msg->Flags) != 0)
		{
			struct RegionRectangle *rr;
			FRect r;
			msg.Rect = &r;

			for (rr = ((struct Region *)(Msg->Rect))->RegionRectangle ; rr ; rr = rr->Next)
			{
				r.x1 = ((struct Region *)(Msg->Rect))->bounds.MinX + rr->bounds.MinX;
				r.y1 = ((struct Region *)(Msg->Rect))->bounds.MinY + rr->bounds.MinY;
				r.x2 = ((struct Region *)(Msg->Rect))->bounds.MinX + rr->bounds.MaxX;
				r.y2 = ((struct Region *)(Msg->Rect))->bounds.MinY + rr->bounds.MaxY;

				if (r.x1 <= r.x2 && r.y1 <= r.y2)
				{
					if (!func(data, Class, &msg))
					{
						IGRAPHICS SetAPen(Msg->Render->RPort,Msg->Render->Palette->Pens[FV_Pen_Fill]);
						IGRAPHICS RectFill(Msg->Render->RPort,r.x1,r.y1,r.x2,r.y2);
					}
				}
				else
				{
					IFEELIN F_Log(0, "invalid coordinates: x (%ld, %ld) y (%ld, %ld)", r.x1, r.x2, r.y1, r.y2);
				}
			}
		}
		else
		{
			if (Msg->Rect->x1 <= Msg->Rect->x2 && Msg->Rect->y1 <= Msg->Rect->y2)
			{
				msg.Rect = Msg->Rect;

				if (!func(data,Class,&msg))
				{
					IGRAPHICS SetAPen(Msg->Render->RPort,Msg->Render->Palette->Pens[FV_Pen_Fill]);
					IGRAPHICS RectFill(Msg->Render->RPort,Msg->Rect->x1,Msg->Rect->y1,Msg->Rect->x2,Msg->Rect->y2);
				}
			}
			else
			{
				IFEELIN F_Log(0, "invalid coordinates: x (%ld, %ld) y (%ld, %ld)", Msg->Rect->x1, Msg->Rect->x2, Msg->Rect->y1, Msg->Rect->y2);
			}
		}
	}
	else
	{
		uint32 p = Msg->Render->Palette->Pens[FV_Pen_Fill];

		if (FF_ImageDisplay_Draw_Region & Msg->Flags)
		{
			struct RegionRectangle *rr;
			FRect r;

			for (rr = ((struct Region *)(Msg->Rect))->RegionRectangle ; rr ; rr = rr->Next)
			{
				r.x1 = ((struct Region *)(Msg->Rect))->bounds.MinX + rr->bounds.MinX;
				r.y1 = ((struct Region *)(Msg->Rect))->bounds.MinY + rr->bounds.MinY;
				r.x2 = ((struct Region *)(Msg->Rect))->bounds.MinX + rr->bounds.MaxX;
				r.y2 = ((struct Region *)(Msg->Rect))->bounds.MinY + rr->bounds.MaxY;

				if (r.x1 <= r.x2 && r.y1 <= r.y2)
				{
					IGRAPHICS SetAPen(Msg->Render->RPort,p);
					IGRAPHICS RectFill(Msg->Render->RPort,r.x1,r.y1,r.x2,r.y2);
				}
			}
		}
		else if (Msg->Rect->x1 <= Msg->Rect->x2 && Msg->Rect->y1 <= Msg->Rect->y2)
		{
			IGRAPHICS SetAPen(Msg->Render->RPort,p);
			IGRAPHICS RectFill(Msg->Render->RPort,Msg->Rect->x1,Msg->Rect->y1,Msg->Rect->x2,Msg->Rect->y2);
		}
	}
}
//+

#endif
