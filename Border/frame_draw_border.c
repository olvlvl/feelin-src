#include "Private.h"

///frame_draw_box
STATIC void frame_draw_box(int16 x1, int16 y1, int16 x2, int16 y2, uint32 Color, uint8 ColorType, FRender *Render)
{
	struct RastPort *rp = Render->RPort;

	switch (ColorType)
	{
		case FV_COLORTYPE_PALETTE:
	    {
			IGRAPHICS SetAPen(rp, Render->Palette->Pens[Color]);
			IGRAPHICS RectFill(rp, x1, y1, x2, y2);
		}
		break;

		case FV_COLORTYPE_RGB:
		{
			if (FF_Render_TrueColors & Render->Flags)
			{
				IFEELIN F_Do(Render, FM_Render_Fill, x1, y1, x2, y2, F_ARGB_TO_RGBA(Color));
			}
		}
		break;
	}
}
//+

void frame_draw_border(FClass *Class, FObject Obj, int16 x1, int16 y1, int16 x2, int16 y2, FRender *Render, struct Region *ClearRegion)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_Frame *frame = LOD->frame;
	struct in_Border *border;

	uint8 tw = frame->border_top.width;
	uint8 rw = frame->border_right.width;
	uint8 bw = frame->border_bottom.width;
	uint8 lw = frame->border_left.width;

	uint8 width;

	/* area vector data */

	struct AreaInfo	area_info;
	struct AreaInfo	*previous_area_info = Render->RPort->AreaInfo;
	uint16 area_buffer[20 * 5];

	IGRAPHICS InitArea(&area_info, (APTR) area_buffer, 20);

	Render->RPort->AreaInfo = &area_info;

	// top

	border = &frame->border_top;

	if (tw)
	{
		switch (frame->border_top.style)
		{
			case FV_BORDER_STYLE_SOLID:
			{
				struct RastPort *rp = Render->RPort;
				frame_draw_box(x1 + lw, y1, x2, y1 + tw - 1, border->color, border->color_type, Render);
				/*
				IGRAPHICS SetAPen(rp, 3);

				IGRAPHICS AreaMove(rp, x1, y1);
				IGRAPHICS AreaDraw(rp, x2, y1);
				IGRAPHICS AreaDraw(rp, x2 - rw + 1, y1 + tw - 1);
				IGRAPHICS AreaDraw(rp, x1 + lw - 1, y1 + tw - 1);

				IGRAPHICS AreaEnd(rp);
				*/
			}
			break;

			case FV_BORDER_STYLE_BROKE:
			{
				frame_draw_box(x1 + lw, y1, x2 - rw, y1 + tw - 1, border->color, border->color_type, Render);

				if (ClearRegion != NULL)
				{
					FRect r;

					r.x1 = x2 - rw + 1;
					r.y1 = y1;
					r.x2 = x2;
					r.y2 = y1 + tw - 1;

					IGRAPHICS OrRectRegion(ClearRegion, (struct Rectangle *) &r);
				}
			}
			break;
		}
	}

	// right

	border = &frame->border_right;

	if (rw)
	{
		switch (frame->border_top.style)
		{
			case FV_BORDER_STYLE_SOLID:
			{
				frame_draw_box(x2 - rw + 1, y1 + tw, x2, y2, border->color, border->color_type, Render);
			}
			break;

			case FV_BORDER_STYLE_BROKE:
			{
				frame_draw_box(x2 - rw + 1, y1 + tw, x2, y2 - bw, border->color, border->color_type, Render);

				if (ClearRegion != NULL)
				{
					FRect r;

					r.x1 = x2 - rw + 1;
					r.y1 = y2 - bw + 1;
					r.x2 = x2;
					r.y2 = y2;

					IGRAPHICS OrRectRegion(ClearRegion, (struct Rectangle *) &r);
				}
			}
			break;
		}
	}

	// bottom

	border = &frame->border_bottom;

	if (bw)
	{
		switch (frame->border_top.style)
		{
			case FV_BORDER_STYLE_SOLID:
			{
				frame_draw_box(x1, y2 - bw + 1, x2 - rw, y2, border->color, border->color_type, Render);
			}
			break;
			
			case FV_BORDER_STYLE_BROKE:
			{
				frame_draw_box(x1 + lw, y2 - bw + 1, x2 - rw, y2, border->color, border->color_type, Render);

				if (ClearRegion != NULL)
				{
					FRect r;

					r.x1 = x1;
					r.y1 = y2 - bw + 1;
					r.x2 = x1 + lw - 1;
					r.y2 = y2;

					IGRAPHICS OrRectRegion(ClearRegion, (struct Rectangle *) &r);
				}
			}
			break;
		}
	}

	// left

	border = &frame->border_left;
	width = border->width;

	if (width)
	{
		switch (frame->border_top.style)
		{
			case FV_BORDER_STYLE_SOLID:
			{
				frame_draw_box(x1, y1, x1 + width - 1, y2 - frame->border_bottom.width, border->color, border->color_type, Render);
			}
			break;

			case FV_BORDER_STYLE_BROKE:
			{
				frame_draw_box(x1, y1 + tw, x1 + width - 1, y2 - frame->border_bottom.width, border->color, border->color_type, Render);

				if (ClearRegion != NULL)
				{
					FRect r;

					r.x1 = x1;
					r.y1 = y1;
					r.x2 = x1 + lw - 1;
					r.y2 = y1 + tw - 1;

					IGRAPHICS OrRectRegion(ClearRegion, (struct Rectangle *) &r);
				}
			}
			break;
		}
	}

	/* restore area */

	Render->RPort->AreaInfo = previous_area_info;
}
