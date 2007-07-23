#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///titlebar_dispose
void titlebar_dispose(FClass *Class,FObject Obj)
{
	 struct LocalObjectData *LOD = F_LOD(Class,Obj);

	 LOD->ColumnCount = 0;

	 if (LOD->TitleBar)
	 {
		  IFEELIN F_Dispose(LOD->TitleBar);

		  LOD->TitleBar = NULL;
		  LOD->Columns = NULL;
	 }
}
//+
///titlebar_new

/* The columns titles are handle as any other Line. This function parses the
given  format  string  and  create a FLine structure extanded to have enough
space to hold  FColumn  strucs.  This  extanded  space  is  pointed  by  the
'Columns' field of the LocalObjectData structure.

The function will also free previously allocated ressources before  creating
new ones.

Return FALSE on failure. */

int32 titlebar_new(FClass *Class,FObject Obj,STRPTR Fmt)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FObject xml;

	titlebar_dispose(Class,Obj);

	if (Fmt == NULL)
	{
		IFEELIN F_Log(0,"Columns format is NULL"); return FALSE;
	}
	else if (*Fmt == '\0')
	{
		IFEELIN F_Log(0,"Columns format is EMPTY (0x%08lx)",Fmt); return FALSE;
	}

	xml = XMLDocumentObject,

		"Source",      Fmt,
		"SourceType",  FV_Document_SourceType_Memory,

	End;
	
	if (xml != NULL)
	{
		FXMLMarkup *markup;
		uint32 n=0;

		for (markup = (FXMLMarkup *) IFEELIN F_Get(xml,(uint32) "Markups") ; markup ; markup = markup->Next)
		{
			if (markup->Atom == F_ATOM(COL))
			{
				FXMLAttribute *attribute;
				uint32 span=1;
				
				for (attribute = (FXMLAttribute *)(markup->AttributesList.Head) ; attribute ; attribute = attribute->Next)
				{
					if (attribute->Atom == F_ATOM(SPAN))
					{
						span = attribute->NumericValue;
					}
				}
	 
				n += span;
			}
		}

		if (n)
		{
			LOD->TitleBar = IFEELIN F_NewP(LOD->LinePool,
			   sizeof (FLine) +                            // Struct header
			   ((n + 1) * sizeof (STRPTR)) +               // Array of Strings
			   ((n + 1) * sizeof (STRPTR)) +               // Array of PreParses
			   ((n + 1) * sizeof (uint16))  +               // Array of Widths
			   (n * sizeof (struct FeelinList_Column)));   // Array of FeelinList_Columns

			if (LOD->TitleBar)
			{
				LOD->TitleBar->Strings   = (STRPTR *)(((uint32)(LOD->TitleBar) + sizeof (FLine)));
				LOD->TitleBar->PreParses = (STRPTR *)((uint32)(LOD->TitleBar->Strings)   + sizeof (STRPTR) * (n + 1));
				LOD->TitleBar->Widths    = (uint16  *)((uint32)(LOD->TitleBar->PreParses) + sizeof (STRPTR) * (n + 1));
				LOD->Columns             = (APTR)((uint32)(LOD->TitleBar->Widths) + sizeof (uint16) * (n + 1));

				LOD->TitleBar->Position  = -1;

				LOD->ColumnCount = n;
				
				for (n = 0, markup = (FXMLMarkup *) IFEELIN F_Get(xml,(uint32) "Markups") ; markup ; markup = markup->Next, n++)
				{
					if (markup->Atom == F_ATOM(COL))
					{
						FXMLAttribute *attribute;
						uint32 span=0;
						
						/*** default values ***/

						LOD->Columns[n].Padding = 4;
						
						/*** reading ***/

						for (attribute = (FXMLAttribute *)(markup->AttributesList.Head) ; attribute ; attribute = attribute->Next)
						{
							if (attribute->Atom == F_ATOM(BAR))
							{
								if (attribute->NumericValue)
								{
								   LOD->Columns[n].Flags |= FF_COLUMN_BAR;
								}
							}
							else if (attribute->Atom == F_ATOM(FIXED))
							{
								if (attribute->NumericValue)
								{
								   LOD->Columns[n].Flags |= FF_COLUMN_FIXED;
								}
							}
							else if (attribute->Atom == F_ATOM(PADDING))
							{
								LOD->Columns[n].Padding = attribute->NumericValue;
							}
							else if (attribute->Atom == F_ATOM(SPAN))
							{
								if ((span = attribute->NumericValue) != 0)
								{
								   span--;
								}
							}
							else if (attribute->Atom == F_ATOM(WEIGHT))
							{
								LOD->Columns[n].Weight = attribute->NumericValue;
							}
						}
					 
						if (span)
						{
							FColumn *ref = &LOD->Columns[n];

							while (span--)
							{
								n++;

								LOD->Columns[n].Flags    = ref->Flags;
								LOD->Columns[n].Padding  = ref->Flags;
								LOD->Columns[n].Weight   = ref->Weight;
							}
						}
					}
				}
			}
		}

		IFEELIN F_DisposeObj(xml);
	}
	return (LOD->TitleBar) ? TRUE : FALSE;
}
//+

///titlebar_compute_dimensions
void titlebar_compute_dimensions(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FLine *line = LOD->TitleBar;

	LOD->TitleBarH = 0;

	if (line)
	{
		if (_area_render)
		{
			FAreaPublic *fd = LOD->TitleFramePublic;
			uint32 j;
			int8 prev_bar=FALSE;

			line_display(Class,Obj,line);
			
			line->Height = 0;

			for (j = 0 ; j < LOD->ColumnCount ; j++)
			{
				uint32 tw=0,th=0;

				if (line->Strings[j])
				{
					IFEELIN F_Do(LOD->TDObj,FM_Set,
									  
						FA_TextDisplay_Font,       _area_font,
						FA_TextDisplay_Width,      -1,
						FA_TextDisplay_Height,     -1,
						FA_TextDisplay_Contents,   line->Strings[j],
						FA_TextDisplay_PreParse,   line->PreParses[j],
									  
						TAG_DONE);

					IFEELIN F_Do(LOD->TDObj,FM_Get,

						FA_TextDisplay_Width,      &tw,
						FA_TextDisplay_Height,     &th,
									  
						TAG_DONE);
									  
					/* frame borders ********************************************/
									  
					if (fd)
					{
						#warning frame borders

						tw += fd->Border.l + fd->Border.r + fd->Padding.l + fd->Padding.r;
						th += fd->Border.t + fd->Border.b + fd->Padding.t + fd->Padding.b;
					}
									  
					/* padding flags ********************************************/

					if (LOD->Columns[j].Padding)
					{
						if (j == 0)
						{
							if (j + 1 < LOD->ColumnCount)
							{
								LOD->Columns[j].Flags |= FF_COLUMN_PADRIGHT;
							}
						}
						else if (j + 1 < LOD->ColumnCount)
						{
							LOD->Columns[j].Flags |= FF_COLUMN_PADLEFT | FF_COLUMN_PADRIGHT;
						}
						else
						{
							LOD->Columns[j].Flags |= FF_COLUMN_PADLEFT;
						}
					}
				 
					if (FF_COLUMN_PADLEFT & LOD->Columns[j].Flags)
					{
						tw += LOD->Columns[j].Padding;
					}
					if (FF_COLUMN_PADRIGHT & LOD->Columns[j].Flags)
					{
						tw += LOD->Columns[j].Padding;
					}

					/* bar flags ************************************************/
									  
					if (prev_bar)
					{
						LOD->Columns[j].Flags |= FF_COLUMN_PREVBAR;
						tw++;
					}
					else
					{
						LOD->Columns[j].Flags &= ~FF_COLUMN_PREVBAR;
					}
									  
					if ((FF_COLUMN_BAR & LOD->Columns[j].Flags) && (j + 1 < LOD->ColumnCount))
					{
						tw ++; prev_bar = TRUE;
					}
					else
					{
						prev_bar = FALSE;
					}
				}
			 
				line->Height = MAX(line->Height,th);
				line->Widths[j] = tw;
				
				LOD->Columns[j].Width = line->Widths[j];
			}
		 
			LOD->TitleBarH = line->Height;
		}
	}
}
//+
///titlebar_adjust_dimensions
void titlebar_adjust_dimensions(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FLine *line = LOD->TitleBar;

	if (line)
	{
		uint32 total_width=0;
		uint32 i;

		for (i = 0 ; i < LOD->ColumnCount ; i++)
		{
			#ifdef DB_LAYOUT
			IFEELIN F_Log(0,"adjust >> col (%ld) weight (%ld) width (%ld)",i,LOD->Columns[i].Weight,LOD->Columns[i].Width);
			#endif
		   
			total_width += (LOD->Columns[i].Weight) ? LOD->Columns[i].Weight : LOD->Columns[i].Width;
		}

		if (total_width)
		{
			uint32 space = _area_cw;

			for (i = 0 ; i < LOD->ColumnCount ; i++)
			{
				if (LOD->Columns[i].Flags & FF_COLUMN_FIXED)
				{
					LOD->Columns[i].AdjustedWidth = LOD->Columns[i].Width;
					space -= LOD->Columns[i].Width;
					total_width -= (LOD->Columns[i].Weight) ? LOD->Columns[i].Weight : LOD->Columns[i].Width;
				}
			}

			for (i = 0 ; i < LOD->ColumnCount ; i++)
			{
				if (!(LOD->Columns[i].Flags & FF_COLUMN_FIXED))
				{
					if (LOD->Columns[i].Weight)
					{
						LOD->Columns[i].AdjustedWidth = space * LOD->Columns[i].Weight / total_width;
					}
					else
					{
						LOD->Columns[i].AdjustedWidth = space * LOD->Columns[i].Width / total_width;
					}
				}
			}
		}
	}
}
//+
///titlebar_setup
bool32 titlebar_setup(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	LOD->TitleBarH = 0;
 
	if (LOD->TitleBar != NULL)
	{
		uint32 j;
		uint32 value;
   
		for (j = 0 ; j < LOD->ColumnCount ; j++)
		{
			LOD->TitleBar->Widths[j] = 0;
		}
   
/*** scheme ************************************************************************************/

		#if 0 // FIXME

		value = NULL;

		IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(TITLE_COLOR_SCHEME), &value, NULL);

		if (value)
		{
			LOD->TitleScheme = (FPalette *) IFEELIN F_Do(_area_display, FM_Display_CreateColorScheme, value, LOD->Scheme);
		}

		_area_render->Palette = LOD->TitleScheme ? LOD->TitleScheme : LOD->Scheme;

		#else

		_area_render->Palette = LOD->Scheme;

		#endif
   
/*** frame *************************************************************************************/

		#if 1 // FIXME

		LOD->TitleFrame = BorderObject,
			
			FA_Element_Class, _list_is_readonly ? "list-read-title" : "list-title",
			
			End;
		 
		if (LOD->TitleFrame)
		{
			IFEELIN F_Do(LOD->TitleFrame, FM_Connect, Obj);

			#ifdef F_NEW_GLOBALCONNECT
			IFEELIN F_Do(LOD->TitleFrame, FM_Element_GlobalConnect, _area_app, _area_win);
			#endif

			if (IFEELIN F_Do(LOD->TitleFrame, FM_Element_Setup, _area_render) != FALSE)
			{
				LOD->TitleFramePublic = F_GET_AREA_PUBLIC(LOD->TitleFrame);
			}
			else
			{
				IFEELIN F_Dispose(LOD->TitleFrame);
				
				LOD->TitleFrame = NULL;
			}
		}

		if (LOD->TitleFrame == NULL)
		{
			return FALSE;
		}

		#endif
 
		titlebar_compute_dimensions(Class,Obj);
	}
	return TRUE;
}
//+
///titlebar_cleanup
void titlebar_cleanup(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render)
	{
		if (LOD->TitleFrame)
		{
			#warning frame back

			#if 0
			if (FF_LIST_TITLE_INHERITED_BACK & LOD->Flags)
			{
				LOD->TitleFramePublic->Back = NULL;
				LOD->Flags &= FF_LIST_TITLE_INHERITED_BACK;
			}
			#endif
 
			IFEELIN F_Do(LOD->TitleFrame, FM_Element_Cleanup);

			IFEELIN F_Do(LOD->TitleFrame, FM_Disconnect);

			#ifdef F_NEW_GLOBALCONNECT
			IFEELIN F_Do(LOD->TitleFrame, FM_Element_GlobalDisconnect);
			#endif

			IFEELIN F_DisposeObj(LOD->TitleFrame);

			LOD->TitleFrame = NULL;
			LOD->TitleFramePublic = NULL;
		}
   
		if (LOD->TitleScheme)
		{
			IFEELIN F_Do(_area_display,FM_Display_RemPalette,LOD->TitleScheme);
			
			LOD->TitleScheme = NULL;
		}
	}
}
//+

///titlebar_draw
void titlebar_draw(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->TitleBar)
	{
		FBox b;
		uint32 j;
		FPalette *previous_palette = _area_render->Palette;

		line_display(Class,Obj,LOD->TitleBar);

		b.x = _area_cx;
		b.y = _area_cy;
		b.h = LOD->TitleBarH;

		_area_render->Palette = (LOD->TitleScheme) ? LOD->TitleScheme : LOD->Scheme;

		IFEELIN F_Do(LOD->TitleFrame, FM_Area_Show);

		for (j = 0 ; j < LOD->ColumnCount; j++)
		{
			FRect r;
			b.w = LOD->Columns[j].AdjustedWidth;
			
			if (LOD->TitleFrame)
			{
				FBox *fb = &LOD->TitleFramePublic->Box;

				fb->x = b.x;
				fb->y = b.y;
				fb->w = b.w;
				fb->h = b.h;

				IFEELIN F_Do(LOD->TitleFrame, FM_Area_Layout);
				IFEELIN F_Draw(LOD->TitleFrame, FF_Draw_Object | FF_Draw_Damaged);

				r.x1 = b.x + LOD->TitleFramePublic->Border.l + LOD->TitleFramePublic->Padding.l;
				r.y1 = b.y + LOD->TitleFramePublic->Border.t + LOD->TitleFramePublic->Padding.t;
				r.x2 = b.x + b.w - 1 - LOD->TitleFramePublic->Border.r - LOD->TitleFramePublic->Padding.r;
				r.y2 = b.y + b.h - 1 - LOD->TitleFramePublic->Border.b - LOD->TitleFramePublic->Padding.b;
			}
			else
			{
				r.x1 = b.x;
				r.y1 = b.y;
				r.x2 = b.x + b.w - 1;
				r.y2 = b.y + b.h - 1;
			}

			if (FF_COLUMN_PADLEFT & LOD->Columns[j].Flags)
			{
				r.x1 += LOD->Columns[j].Padding;
			}
			if (FF_COLUMN_PADRIGHT & LOD->Columns[j].Flags)
			{
				r.x2 -= LOD->Columns[j].Padding;
			}
			
			if (LOD->TitleBar->Strings[j])
			{
				IFEELIN F_Do(LOD->TDObj,FM_Set,

					FA_TextDisplay_Width,      r.x2 - r.x1 + 1,
					FA_TextDisplay_Height,     b.h,
					FA_TextDisplay_Contents,   LOD->TitleBar->Strings[j],
					FA_TextDisplay_PreParse,   LOD->TitleBar->PreParses[j],

					TAG_DONE);

				IFEELIN F_Do(LOD->TDObj,FM_TextDisplay_Draw,&r);
			}
			
			b.x += LOD->Columns[j].AdjustedWidth;
		}

		IFEELIN F_Do(LOD->TitleFrame, FM_Area_Hide);

		_area_render->Palette = previous_palette;
	}
}
//+
