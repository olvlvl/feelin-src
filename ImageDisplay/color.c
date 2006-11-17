#include "Private.h"

typedef struct FeelinImageData
{
   uint16                           type;
   uint32                           spec;
   FColor                          *color;
   bool16                           hicolors;
}
FImage;

///id_color_create
STATIC F_CODE_CREATE(id_color_create)
{
	FImage *image = IFEELIN F_NewP(Pool, sizeof (FImage));

	if (image != NULL)
	{
		image->type = Parse->colors_type[0];
		image->spec = Parse->colors_value[0];
	}

	return image;
}
//+
///id_color_delete
STATIC F_CODE_DELETE(id_color_delete)
{
	IFEELIN F_Dispose(image);
}
//+
///id_color_setup
STATIC F_CODE_SETUP(id_color_setup)
{
	if (image->type == FV_COLORTYPE_RGB)
	{
		if ((FF_Render_TrueColors & Render->Flags) && RTGFXBASE)
		{
			image->hicolors = TRUE;

			return TRUE;
		}
		else if ((image->color = (FColor *) IFEELIN F_Do(Render->Display, FM_Display_AddColor, image->spec)))
		{
			return TRUE;
		}
	 
		return FALSE;
	}

	return TRUE;
}
//+
///id_color_cleanup
STATIC F_CODE_CLEANUP(id_color_cleanup)
{
	if (image->color != NULL)
	{
		IFEELIN F_Do(Render->Display, FM_Display_RemColor, image->color);

		image->color = NULL;
	}
}
//+
///id_color_draw
STATIC F_CODE_DRAW(id_color_draw)
{
	switch (image->type)
	{
		case FV_COLORTYPE_ENTRY:
		{
			IGRAPHICS SetAPen(Msg->Render->RPort, Msg->Palette->Pens[image->spec]);
			IGRAPHICS RectFill(Msg->Render->RPort, Msg->Rect->x1, Msg->Rect->y1, Msg->Rect->x2, Msg->Rect->y2);
		}
		break;
		 
		case FV_COLORTYPE_PEN:
		{
			IGRAPHICS SetAPen(Msg->Render->RPort,image->spec);
			IGRAPHICS RectFill(Msg->Render->RPort,Msg->Rect->x1,Msg->Rect->y1,Msg->Rect->x2,Msg->Rect->y2);
		}
		break;

		case FV_COLORTYPE_RGB:
		{
			if (image->color != NULL)
			{
				IGRAPHICS SetAPen(Msg->Render->RPort,image->color->Pen);
				IGRAPHICS RectFill(Msg->Render->RPort,Msg->Rect->x1,Msg->Rect->y1,Msg->Rect->x2,Msg->Rect->y2);
			}
			#ifdef F_USE_PICASSO96
			else if (P96Base)
			#else
			else if (CyberGfxBase)
			#endif
			{
				#ifdef F_USE_PICASSO96
				IP96_ p96RectFill
				(
					Msg->Render->RPort,

					Msg->Rect->x1,
					Msg->Rect->y1,
					Msg->Rect->x2,
					Msg->Rect->y2,

					image->spec
				);
				#else
				ICYBERGFX FillPixelArray
				(
					Msg->Render->RPort,
					
					Msg->Rect->x1,
					Msg->Rect->y1,
					Msg->Rect->x2 - Msg->Rect->x1 + 1,
					Msg->Rect->y2 - Msg->Rect->y1 + 1,
					
					image->spec
				);
				#endif
			}
		}
		break;
	}
	return TRUE;
}
//+

///id_color_get_rgb
uint32 id_color_get_rgb(FImage *image, FRender *Render)
{
	uint32 rc = 0;

	switch (image->type)
	{
		case FV_COLORTYPE_ENTRY:
		{
			rc = Render->Palette->ARGB[image->spec];
		}
		break;

		#if F_CODE_DEPRECATED

		case FV_COLORTYPE_PEN:
		{
			#warning should be disabled !
		}
		break;

		#endif

		case FV_COLORTYPE_RGB:
		{
			rc = image->spec;
		}
		break;
	}

	return rc;
}
//+

struct in_CodeTable id_color_table =
{
	(in_Code_Create *) &id_color_create,
	(in_Code_Delete *) &id_color_delete,
	(in_Code_Setup *) &id_color_setup,
	(in_Code_Cleanup *) &id_color_cleanup,
	NULL,
	(in_Code_Draw *) &id_color_draw,
};
