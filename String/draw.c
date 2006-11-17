#include "Private.h"

/*

	FIXME: These functions are terrible, I'll try to rewrite them when I get
	some time.
 
*/
 
//#define DB_DRAW

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///String_ClipText
STATIC void String_ClipText(FClass *Class, FObject Obj, struct RastPort *rp,uint32 w, uint32 cur,uint32 len,uint32 cw,struct FeelinString_TextFit *Fit)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 pos = Fit->Position;
	STRPTR str = Fit->String;
	uint16 tw  = Fit->TextWidth;
	uint16 dc;
	uint32 a;

	if (_widget_is_selected && cur == len) tw += cw;

	if (tw <= w)
	{
		dc    = len;
		pos   = 0;
		Fit->Clip = FALSE;
	}
	else
	{
		Fit->Clip = TRUE;

		if (cur < pos)
		{
			pos = cur;
		}
		else
		{
			dc = 0 ; tw = 0;

			while ((tw += IGRAPHICS TextLength(rp,str+pos+dc,1)) <= w)
			{
				if (pos + dc == len) break; dc++;
			}

			if ((pos + dc) == len) // dc déjà au minimum
			{
				while (IGRAPHICS TextLength(rp,str+len-dc,dc) <= (w - ((cur == len) ? cw : 0))) dc++;
				pos = len - dc + 1;
			}
			else if (IGRAPHICS TextLength(rp,str+pos,cur-pos) > (w-cw))
			{
				dc = 1;
				while (IGRAPHICS TextLength(rp,str+cur-dc,dc) <= (w-cw)) dc++;
				pos = cur - dc + 1;
			}
		}

		str += pos;

//    Troncage

		dc = 0; tw = 0; /*a = 0;*/

		while ((a = IGRAPHICS TextLength(rp,str+dc,1)) <= (w-tw))
		{
			if ((pos + dc) == len) break; dc++; tw += a;
		}

		if (_widget_is_selected && cur == len) tw += cw;
	}

	Fit->Displayable   = dc;
	Fit->Position      = pos;
	Fit->String        = str;
	Fit->TextWidth     = tw;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///String_Draw
F_METHODM(uint32,String_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct RastPort *rp = _area_rp;
	struct FeelinString_TextFit   fit;
	int32 x1  = _area_cx, x2 = x1 + _area_cw - 1;
	int32 y1  = _area_cy, y2 = y1 + _area_ch - 1;
	uint32 pos = LOD->Pos;
	uint32 len = LOD->Len;
	uint32 cur = LOD->Cur;
	uint32 old = LOD->Old;
	STRPTR str = LOD->String;
	int32 tx,cx;
	uint32 tw,cw;
	uint32 repair=REPAIR_NOTHING;

	F_SUPERDO();

	y1 = (y2 - y1 + 1 - _area_font->tf_YSize) / 2 + y1; /* Center string vertically */
	y2 = y1 + _area_font->tf_YSize - 1;
	
	if (!((Msg->Flags & (FF_Draw_Move | FF_Draw_Blink)))) LOD->TextW = IGRAPHICS TextLength(rp,str,len);

	cw = LOD->CursorW;
	tw = LOD->TextW;

///FF_Draw_Blink
	if (FF_Draw_Blink & Msg->Flags)
	{
		APTR imobj = (FF_String_Blink & LOD->Flags) ? LOD->Blink : LOD->Cursor;
		cx = LOD->CursorX;

		if (imobj)
		{
			FRect rect;

			rect.x1 = cx ; rect.x2 = cx + cw - 1;
			rect.y1 = y1 ; rect.y2 = y2;

			IFEELIN F_Do(imobj,FM_ImageDisplay_Draw,_area_render,&rect,0);
		}
		else
		{
			if (FF_String_Blink & LOD->Flags)
			{
				IFEELIN F_Erase(Obj,cx,y1,cx + cw - 1,y2,0);
			}
			else
			{
				_FPen(FV_Pen_Highlight);
				_Boxf(cx,y1,cx+cw-1,y2);
			}
		}

		if (cur != len)
		{
			_APen((FF_String_Blink & LOD->Flags) ? LOD->APen->Pen : LOD->BPen->Pen);
			_Move(cx,y1 + _area_font->tf_Baseline);
			_DrMd(JAM1);
			_Text(str+cur,MIN(LOD->Sel,LOD->Dsp));
			_DrMd(JAM2);
		}

		return TRUE;
	}
//+

	fit.Position  = pos;
	fit.String    = str;
	fit.TextWidth = tw;

	if (cur == len)
	{
		cw = IGRAPHICS TextLength(rp," ",1);
	}
	else
	{
		cw = IGRAPHICS TextLength(rp,LOD->String + cur,LOD->Sel);
	}

	String_ClipText(Class,Obj,rp,x2 - x1 + 1, cur,len,cw, &fit);

	pos = fit.Position;
	str = fit.String;
	tw  = fit.TextWidth;

	switch (LOD->Justify)
	{
		case FV_String_Center:  tx = (x2 - x1 + 1 - tw) / 2 + x1; break;
		case FV_String_Right:   tx = x2 - tw + 1; break;
		default:                tx = x1; break;
	}

	if (tx < x1)
	{
		#ifdef DB_DRAW
		IFEELIN F_Log(0,"ERROR: text_x (%ld < %ld)",tx,x1);
		#endif
		
		tx = x1;
	}

	cx = IGRAPHICS TextLength(rp,str,cur-pos) + tx;
	
	if (cx + cw - 1 > x2)
	{
		#ifdef DB_DRAW
		IFEELIN F_Log(0,"ERROR: cursor (%ld > %ld)",cx + cw - 1,x2);
		#endif
		
		cx = x2 - cw;
	}
			
	LOD->CursorX = cx;
	LOD->CursorW = cw; /* FIXME: Is this necessary ?? */
	LOD->TextX   = tx;
	LOD->Dsp     = fit.Displayable;
	
	if (FF_Draw_Update & Msg->Flags)
	{
		if (pos != LOD->Pos)
		{
			repair = REPAIR_ALL;
		}
		else if (!LOD->Len)
		{
			repair = REPAIR_ALL;
		}
		else if (cur == len || old == len)
		{
			if (LOD->Justify == FV_String_Left)
			{
				if (FF_Draw_Move & Msg->Flags)          repair = REPAIR_OLDC;
				else if (FF_Draw_Insert & Msg->Flags)   repair = REPAIR_FROM_OLDC;
				else                                      repair = REPAIR_FROMCUR;
			}
			else
			{
				repair = REPAIR_ALL;
			}
		}
		else if (FF_Draw_Move & Msg->Flags)
		{
			repair = REPAIR_OLDC;
		}
		else if (FF_Draw_Insert & Msg->Flags)
		{
			switch (LOD->Justify)
			{
				case FV_String_Left:   repair = REPAIR_FROM_OLDC; break;
				case FV_String_Center: repair = REPAIR_ALL; break;
				case FV_String_Right:  repair = (fit.Clip) ? REPAIR_ALL : REPAIR_TOCUR; break;
			}
		}
		else if ((FF_Draw_Delete | FF_Draw_Backspace) & Msg->Flags)
		{
			switch (LOD->Justify)
			{
				case FV_String_Left:    repair = REPAIR_FROMCUR; break;
				case FV_String_Center:  repair = REPAIR_ALL; break;
				case FV_String_Right:   repair = (fit.Clip) ? REPAIR_ALL : ((FF_Draw_Delete & Msg->Flags) ? REPAIR_TOCUR : REPAIR_WITHCUR); break;
			}
		}

		if (repair)
		{
			if (repair == REPAIR_OLDC || repair == REPAIR_FROM_OLDC)
			{
				x1 = tx + IGRAPHICS TextLength(rp,LOD->String + LOD->Pos, old - LOD->Pos);
			}

			switch (repair)
			{
				case REPAIR_OLDC:    x2 = x1 + IGRAPHICS TextLength(rp,LOD->String + old, 1) - 1; break;
				case REPAIR_WITHCUR: x2 = cx; break;
				case REPAIR_FROMCUR: x1 = cx; break;
				case REPAIR_TOCUR:   x2 = cx-1; break;
			}

			IFEELIN F_Erase(Obj,x1,y1,x2,y2,0);
		}
	}

	if (_widget_is_selected)
	{
		if (LOD->Cursor)
		{
			FRect r;

			r.x1 = cx; r.x2 = cx + cw - 1;
			r.y1 = y1; r.y2 = y2;

			IFEELIN F_Do(LOD->Cursor,FM_ImageDisplay_Draw,_area_render,&r,0);
		}
		else
		{
			_FPen(FV_Pen_Highlight);
			_Boxf(cx,y1,cx+cw-1,y2);
		}
	}

	y1 += _area_font->tf_Baseline;

	_APen(_widget_is_selected ? LOD->APen->Pen : LOD->IPen->Pen);
	_Move(tx,y1);
	_DrMd(JAM1);
	_Text(str,fit.Displayable);

	if (_widget_is_selected && cur != len)
	{
		_APen(LOD->BPen->Pen);
		_Move(cx,y1);
		_Text(LOD->String+cur,1);
	}

	_DrMd(JAM2);

	LOD->Old = cur;
	LOD->Pos = pos;

	return TRUE;
}
//+
