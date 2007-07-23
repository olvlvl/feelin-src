#include "Private.h"

//#define DB_ADDPALETTE
//#define DB_ADDPALETTE_COLORS

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

#include <feelin/support/f_decode_color.c>

///display_decode_color
STATIC uint32 display_decode_color(FClass *Class, FObject Obj, STRPTR Spec, FPalette *Reference)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	uint32 value = 0;
	uint32 type;

	type = f_decode_color(Spec, ALL, &value);

	switch (type)
	{
		case FV_COLORTYPE_PALETTE:
		{
			if (Reference != NULL)
			{
				value = Reference->ARGB[value];
			}
			else
			{
				value = 0;

				IFEELIN F_Log(FV_LOG_DEV, "Unable to decode (%s) because Reference is NULL", Spec);
			}
		}
		break;

		case FV_COLORTYPE_PEN:
		{
			if (LOD->ColorMap != NULL)
			{
				uint32 rgb[3];
				int32  val;
				uint32 depth = MIN(8,LOD->Depth);

				f_stcd(Spec, &val);

				if ((val = val % (1 << depth)) < 0)
				{
					val = (1 << depth) + val;
				}

				IGRAPHICS GetRGB32(LOD->ColorMap,val,1,(uint32 *)(&rgb));
				value = (0x00FF0000 & rgb[0]) | (0x0000FF00 & rgb[1]) | (0x000000FF & rgb[2]);
			}
			else
			{
				IFEELIN F_Log(FV_LOG_DEV, "Unable to decode (%s) because ColorMap is NULL", Spec);
			}

		}
		break;
	}

	return value;
}
//+

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///display_find_pen
STATIC struct in_Color * display_find_pen(FList *Colors,uint32 Pen)
{
   struct in_Color *col;

   for (col = (struct in_Color *)(Colors->Head) ; col ; col = col->Next)
   {
	  if (col->Pen == Pen) break;
   }

   return col;
}
//+
///display_find_rgb
STATIC struct in_Color * display_find_rgb(FList *Colors,uint32 ARGB)
{
   struct in_Color *col;

   for (col = (struct in_Color *)(Colors->Head) ; col ; col = col->Next)
   {
	  if (col->ARGB == ARGB) break;
   }

   return col;
}
//+
///display_match_argb
STATIC struct in_Palette * display_match_argb(FClass *Class,FObject Obj,uint32 Count,uint32 *ARGB)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	struct in_Palette *palette;

   	uint32 i;

	for (palette = LOD->palettes ; palette ; palette = palette->next)
	{
  	    if (palette->public.Count != Count)
  	    {
			continue;
		}

	    for (i = 0 ; i < Count ; i++)
	    {
			if (palette->public.ARGB[i] != ARGB[i])
			{
				break;
			}
	    }

	    if (i == Count)
	    {
			break;
  	    }
	}

	return palette;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///D_AddColor
F_METHODM(struct in_Color *,D_AddColor,FS_Display_AddColor)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct in_Color *col;
	uint32 c,array[3];

	F_LOCK_COLORS;

	c = (0x00FF0000 & Msg->ARGB) >> 16; array[0] = c | (c << 8) | (c << 16) | (c << 24);
	c = (0x0000FF00 & Msg->ARGB) >>  8; array[1] = c | (c << 8) | (c << 16) | (c << 24);
	c = (0x000000FF & Msg->ARGB) >>  0; array[2] = c | (c << 8) | (c << 16) | (c << 24);

	#ifdef DB_ADDCOLOR
	IFEELIN F_Log(0,"ARGB %06lx - %08lx %08lx %08lx",Msg->ARGB,array[0],array[1],array[2]);
	#endif

	col = display_find_rgb(&LOD->ColorList, Msg->ARGB);

	if (col != NULL)
	{
		#ifdef DB_ADDCOLOR
		{
		  uint32 rgb[3];

			IFEELIN F_Log(0,"RGB 0x%06lx 0x%06lx 0x%06lx - PEN %ld",array[0],array[1],array[2],col->Pen);

			IGRAPHICS GetRGB32(LOD->ColorMap,col->Pen,1,(uint32 *)(&rgb));

			IFEELIN F_Log(0,">>> 0x%06lx 0x%06lx 0x%06lx - FOUND",rgb[0],rgb[1],rgb[2]);
		}
		#endif

		col->UserCount += 1;
	}
	else
	{
		bits32 flags = FF_Color_Shared;

		uint32 pen = IGRAPHICS ObtainBestPen
		(
			LOD->ColorMap,
		  
			array[0],array[1],array[2],
		  
			OBP_Precision,  PRECISION_EXACT,
			OBP_FailIfBad,  TRUE,
		  
			TAG_DONE
		);

		if (pen == -1)
		{
			pen = IGRAPHICS FindColor(LOD->ColorMap,array[0],array[1],array[2],1 << LOD->Depth);
			flags = 0;
		}

		#ifdef DB_ADDCOLOR
		{
			uint32 rgb[3];

			IFEELIN F_Log(0,"RGB 0x%06lx 0x%06lx 0x%06lx - PEN %ld",array[0],array[1],array[2],pen);

			IGRAPHICS GetRGB32(LOD->ColorMap,pen,1,(uint32 *)(&rgb));

			IFEELIN F_Log(0,">>> 0x%06lx 0x%06lx 0x%06lx",rgb[0],rgb[1],rgb[2]);
		}
		#endif

		col = display_find_pen(&LOD->ColorList,pen);

		if (col != NULL)
		{
			if (FF_Color_Shared & flags)
			{
				IGRAPHICS ReleasePen(LOD->ColorMap,pen);
			}
			col->UserCount++;
		}
		else 
		{
			col = IFEELIN F_NewP(CUD->colors_pool,sizeof (struct in_Color));

			if (col != NULL)
			{
				col->ARGB       = Msg->ARGB;
				col->Pen        = pen;
				col->Flags      = flags;
				col->UserCount  = 1;

				IFEELIN F_LinkTail(&LOD->ColorList, (FNode *) col);
			}
		}
	}
				
	#ifdef DB_ADDCOLOR
	IFEELIN F_Log(0,"Color 0x%08lx - ARGB %06lx - UserCount %4ld - Pen %ld",col,col->ARGB,col->UserCount,col->Pen);
	#endif

	F_UNLOCK_COLORS;

	return col;
}
//+
///D_RemColor
F_METHODM(void,D_RemColor,FS_Display_RemColor)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct in_Color * col;

	F_LOCK_COLORS;

	for (col = (struct in_Color *) LOD->ColorList.Head ; col ; col = col->Next)
	{
		if ((struct in_Color *)(Msg->Color) == col)
		{
			if (--col->UserCount == 0)
			{
				#ifdef DB_REMCOLOR
				IFEELIN F_Log(0,"Pen (0x%lx) RGB (0x%06lx)",Pen,Pen->xrgb);
				#endif

				IFEELIN F_LinkRemove(&LOD->ColorList,(FNode *) col);

				if (FF_Color_Shared & col->Flags && col->Pen != -1)
				{
				   IGRAPHICS ReleasePen(LOD->ColorMap,col->Pen);
				}

				IFEELIN F_Dispose(col);
			}
			break;
		}
	}

	if (!col)
	{
		IFEELIN F_Log(FV_LOG_DEV,"Unknown Color 0x%08lx",Msg->Color);
	}

	F_UNLOCK_COLORS;
}
//+
///D_AddPalette
F_METHODM(FPalette *, D_AddPalette, FS_Display_AddPalette)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	uint32 count = Msg->Count;
	uint32 *argb = Msg->ARGBs;

	struct in_Palette *palette;

	if (count > 256)
	{
		IFEELIN F_Log(FV_LOG_DEV, "suspicious count (%08lx) ARGB (%08lx)", count, argb);

		return NULL;
	}

	F_LOCK_COLORS;

	palette = display_match_argb(Class, Obj, count, argb);

	if (palette != NULL)
	{
		palette->references++;

		#ifdef DB_ADDPALETTE

		IFEELIN F_Log(0, "palette (0x%08lx) references (%ld)",

			F_PALETTE_PUBLICIZE(palette),
			palette->references

			);

		#endif
	}
	else
	{
		palette = IFEELIN F_NewP(CUD->palettes_pool, sizeof (struct in_Palette) + sizeof (FPalette) * count);

		if (palette != NULL)
		{
			uint32 i;

			uint32 *palette_pens = (uint32 *) (palette + 1);
			uint32 *palette_argb = palette_pens + count;
			FColor **palette_colors = (FColor **) (palette_argb + count);

			for (i = 0 ; i < count ; i++)
			{
				FColor *color = (FColor *) IFEELIN F_Do(Obj, FM_Display_AddColor, argb[i]);

				if (color != NULL)
				{
					palette_pens[i] = color->Pen;
					palette_argb[i] = argb[i];
					palette_colors[i] = color;
				}
				else
				{
					IFEELIN F_Log(0, "Unable to allocate color (0x%08lx)", argb[i]);
				}
			}

			/* initialise the palette */

			palette->next = LOD->palettes;
			LOD->palettes = palette;
			
			palette->references = 1;
			palette->public.Count = count;
			palette->public.Pens = palette_pens;
			palette->public.ARGB = palette_argb;
			palette->public.Colors = palette_colors;

			#ifdef DB_ADDPALETTE
			
			IFEELIN F_Log
			(
				FV_LOG_DEV, "palette (0x%08lx) pens (%ld)(0x%08lx) argb (0x%08lx) colors (0x%08lx)",

				F_PALETTE_PUBLICIZE(palette),
				palette->public.Count,
				palette->public.Pens,
				palette->public.ARGB,
				palette->public.Colors
			);

			#endif

			#ifdef DB_ADDPALETTE_COLORS

			for (i = 0 ; i < palette->public.Count ; i++)
			{
				IFEELIN F_Log(0, "[%ld] pen (%3ld) rgb (0x%06lx) color (0x%08lx)(%3ld)(0x%06lx)",

					i,
					palette->public.Pens[i],
					palette->public.ARGB[i],
					palette->public.Colors[i],
					palette->public.Colors[i]->Pen,
					palette->public.Colors[i]->ARGB

				);
			}

			#endif
		}
	}

	F_UNLOCK_COLORS;

	return palette ? F_PALETTE_PUBLICIZE(palette) : NULL;
}
//+
///D_RemPalette
F_METHODM(uint32, D_RemPalette, FS_Display_RemPalette)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
   
	struct in_Palette *prev = NULL;
	struct in_Palette *node;

	if (Msg->Palette == NULL)
	{
		return 0;
	}

	F_LOCK_COLORS;

	for (node = LOD->palettes ; node ; prev = node, node = node->next)
	{
		if (Msg->Palette != F_PALETTE_PUBLICIZE(node))
		{
			continue;
		}
		
		node->references--;

		if (node->references == 0)
		{
			uint32 i;

			if (prev)
			{
				prev->next = node->next;
			}
			else
			{
				LOD->palettes = node->next;
			}

			for (i = 0 ; i < node->public.Count ; i++)
			{
				IFEELIN F_Do(Obj, FM_Display_RemColor, node->public.Colors[i]);
			}

			IFEELIN F_Dispose(node);
		}
			
		break;
	}

	if (node == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "unknown Palette (0x%08lx) !", Msg->Palette);
	}

	F_UNLOCK_COLORS;

	return 0;
}
//+

///D_CreateColor
F_METHODM(FColor *, D_CreateColor, FS_Display_CreateColor)
{
	#if 1

	return (FColor *) IFEELIN F_Do(Obj, FM_Display_AddColor, display_decode_color(Class, Obj, Msg->Spec, Msg->Reference));

	#else

	return (FColor *) IFEELIN F_Do(Obj, FM_Display_AddColor, IFEELIN F_Do(Obj, F_METHOD_ID(DECODE_COLOR), Msg->Spec, Msg->Reference));

	#endif
}
//+
