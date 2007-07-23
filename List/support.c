#include "Private.h"

//#define DB_LAYOUT
 
/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///line_display

/* Call the FM_List_Display method for the given Line. It fills  out  'Line
-> Strings' and 'Line->PreParses' arrays with strings to be displayed. */

void line_display(FClass *Class,FObject Obj,FLine *Line)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Line)
	{
		uint32 i;

		for (i = 0 ; i < LOD->ColumnCount ; i++)
		{
		   Line->PreParses[i] = NULL;
		}

		IFEELIN F_Do(Obj, F_METHOD_ID(DISPLAY), Line->Entry, Line->Strings, Line->PreParses);
	}
}
//+
///line_compute_dimensions
/*

   Compute the dimensions of a Line.

*/
void line_compute_dimensions(FClass *Class,FObject Obj,FLine *Line)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	Line->Height = 0;

	if (_area_render && LOD->Columns && Line)
	{
		uint32 j;

/*** only compute widths when needed ***********************************************************/

		if (!(FF_LINE_COMPUTED & Line->Flags))
		{
			for (j = 0 ; j < LOD->ColumnCount ; j++)
			{
				uint32 tw=0,th=0;
				line_display(Class,Obj,Line);

				if (Line->Strings[j])
				{
					IFEELIN F_Do
					(
						LOD->TDObj, FM_Set,
						
						FA_TextDisplay_Font,       _area_font,
						FA_TextDisplay_Width,      FV_Area_Max,
						FA_TextDisplay_Height,     FV_Area_Max,
						FA_TextDisplay_Contents,   Line->Strings[j],
						FA_TextDisplay_PreParse,   Line->PreParses[j],
						
						TAG_DONE
					);

					IFEELIN F_Do
					(
						LOD->TDObj, FM_Get,
						
						FA_TextDisplay_Width,  &tw,
						FA_TextDisplay_Height, &th,
					
						TAG_DONE
					);
									  
					#ifdef DB_LAYOUT
					IFEELIN F_Log(0,"(%ld:%ld)(%s&%s) >> %ld x %ld",Line->Position,j,Line->PreParses[j],Line->Strings[j],tw,th);
					#endif
				}
				   
				if (FF_COLUMN_PADLEFT & LOD->Columns[j].Flags)
				{
					tw += LOD->Columns[j].Padding;
				}
				if (FF_COLUMN_PADRIGHT & LOD->Columns[j].Flags)
				{
					tw += LOD->Columns[j].Padding;
				}
				if (FF_COLUMN_PREVBAR & LOD->Columns[j].Flags)
				{
					tw++;
				}
				if (FF_COLUMN_BAR & LOD->Columns[j].Flags)
				{
					tw++;
				}

				Line->Height    = MAX(Line->Height,th + LOD->spacing);
				Line->Widths[j] = tw;
			}
			Line->Flags |= FF_LINE_COMPUTED;
		}

/*** adjust columns maximum width **************************************************************/

		for (j = 0 ; j < LOD->ColumnCount ; j++)
		{
			LOD->Columns[j].Width = MAX(LOD->Columns[j].Width,Line->Widths[j]);
				 
			#ifdef DB_LAYOUT
			IFEELIN F_Log(0,"Col(%ld) WIDTH (%ld)",j,LOD->Columns[j].Width);
			#endif
		}

/*** adjust maximum line height ****************************************************************/

		LOD->MaximumH = MAX(LOD->MaximumH,Line->Height);
	}
}
//+

///list_adjust_first
void list_adjust_first(FClass *Class,FObject Obj,uint32 first_pos)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	#ifdef DB_LAYOUT
	IFEELIN F_Log(0,"first_pos (%ld) Visible (%ld)",first_pos,LOD->Visible);
	#endif

	if (LOD->Visible)
	{
		FLine *line;
		uint32 last_pos = first_pos + LOD->Visible - 1;
		
/* HOMBRE @ GOFROMIEL : J'ai ajouté le test de LOD->LineList.Tail, car s'il
est null, c'est le crash assuré (je m'en suis apperçu en débugant DOSList)
*/
   
		if ((LOD->LineList.Tail != NULL) && (last_pos > ((FLine *)(LOD->LineList.Tail))->Position))
		{
			#ifdef DB_LAYOUT
			IFEELIN F_Log(0,"last_pos too far (%ld)(%ld)",last_pos,((FLine *)(LOD->LineList.Tail))->Position);
			#endif
   
			last_pos = ((FLine *)(LOD->LineList.Tail))->Position;
			first_pos = last_pos - LOD->Visible + 1;
			
			if ((int32)(first_pos) < 0)
			{
				#ifdef DB_LAYOUT
				IFEELIN F_Log(0,"first_pos invalid (%ld)",first_pos);
				#endif
	
				first_pos = 0;
			}
		}

		#ifdef DB_LAYOUT
		if (LOD->First)
		{
			IFEELIN F_Log(0,"list_adjust_first : first 0x%08lx (%ld)",LOD->First,LOD->First->Position);
		}
		else
		{
			IFEELIN F_Log(0,"list_adjust_first : no first element");
		}
		#endif
   
		for (line = (FLine *)(LOD->LineList.Head) ; line ; line = line->Next)
		{
			if (line->Position == first_pos)
			{
				LOD->First = line; break;
			}
		}

		for ( ; line ; line = line->Next)
		{
			if (line->Position == last_pos)
			{
				LOD->Last = line; break;
			}
		}
	}
	else
	{
		LOD->First = NULL;
		LOD->Last  = NULL;
	}
}
//+
///list_update
void list_update(FClass *Class,FObject Obj,bool32 Notify)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render && LOD->Columns)
	{
		uint32 prev_visible = LOD->Visible;
		FLine *prev_first = LOD->First;

		if (LOD->LineCount && LOD->MaximumH)
		{
			LOD->Visible = MIN(LOD->LineCount,(_area_ch - LOD->TitleBarH + LOD->spacing) / LOD->MaximumH);
		}

		#ifdef DB_LAYOUT
		IFEELIN F_Log(0,"update: first 0x%08lx (%ld)",LOD->First,LOD->First->Position);
		#endif
		
		list_adjust_first(Class,Obj,(LOD->First) ? LOD->First->Position : 0);
		
		titlebar_adjust_dimensions(Class,Obj);

/*** update attributes *************************************************************************/

		if (((LOD->Quiet >= 0) && (Notify)) && ((prev_visible != LOD->Visible) || (prev_first != LOD->First)))
		{
			
			/* only modified attributes trigger notification, avoiding a lot
			of notification loops */

			IFEELIN F_SuperDo
			(
				Class,Obj,FM_Set,
			
				(prev_visible != LOD->Visible) ? F_ATTRIBUTE_ID(VISIBLE) : TAG_IGNORE,LOD->Visible,
				(prev_first != LOD->First) ? F_ATTRIBUTE_ID(FIRST) : TAG_IGNORE,(LOD->First) ? LOD->First->Position : 0,
			   
				TAG_DONE
			);
		}
	}
}
//+

