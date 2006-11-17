#include "Private.h"

typedef struct FeelinImage
{
	uint16                          type[2];
	uint32                          data[2];
	FColor                         *color[2];
	uint32                          pattern;
}
FImage;

#define F_PATTERN_MAKE(id, size)		((size << 8) | (id))

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///id_pattern_create
STATIC F_CODE_CREATE(id_pattern_create)
{
	FImage *image = IFEELIN F_NewP(Pool,sizeof (FImage));

	if (image)
	{
		image->type[0] = Parse->colors_type[0];
		image->data[0] = Parse->colors_value[0];

		image->type[1] = Parse->colors_type[1];
		image->data[1] = Parse->colors_value[1];

		image->pattern = F_PATTERN_MAKE(Parse->pattern, Parse->pattern_size);
	}

	return image;
}
//+
///id_pattern_delete
STATIC F_CODE_DELETE(id_pattern_delete)
{
	IFEELIN F_Dispose(image);
}
//+
///id_pattern_setup
STATIC F_CODE_SETUP(id_pattern_setup)
{
	if (image->type[0] == FV_COLORTYPE_RGB)
	{
		image->color[0] = (FColor *) IFEELIN F_Do(Render->Display, FM_Display_AddColor, image->data[0]);
			
		if (!image->color[0])
		{
			return FALSE;
		}
	}
	
	if (image->type[1] == FV_COLORTYPE_RGB)
	{
		image->color[1] = (FColor *) IFEELIN F_Do(Render->Display, FM_Display_AddColor, image->data[1]);

		if (!image->color[1])
		{
			return FALSE;
		}
	}
	
	return TRUE;
}
//+
///id_pattern_cleanup
STATIC F_CODE_CLEANUP(id_pattern_cleanup)
{
	if (image->color[0])
	{
		IFEELIN F_Do(Render->Display,FM_Display_RemColor,image->color[0]); image->color[0] = NULL;

		image->color[0] = NULL;
	}

	if (image->color[1])
	{
		IFEELIN F_Do(Render->Display,FM_Display_RemColor,image->color[1]); image->color[1] = NULL;

		image->color[1] = NULL;
	}
}
//+

///id_pattern_draw
STATIC F_CODE_DRAW(id_pattern_draw)
{
	struct RastPort *rp = Msg->Render->RPort;
	
	uint16 *pattern;
	uint16 pattern_size = 1;
	uint16 *prev_ptrn = rp->AreaPtrn;
	uint32 prev_ptsz = rp->AreaPtSz;
	
	uint32 apen = 1;
	uint32 bpen = 2;

	switch (image->type[0])
	{
		case FV_COLORTYPE_ENTRY:    apen = Msg->Palette->Pens[image->data[0]]; break;
		case FV_COLORTYPE_PEN:      apen = image->data[0]; break;
		case FV_COLORTYPE_RGB:      apen = (image->color[0]) ? image->color[0]->Pen : image->data[0]; break;
	}

	switch (image->type[1])
	{
		case FV_COLORTYPE_ENTRY:    bpen = Msg->Palette->Pens[image->data[1]]; break;
		case FV_COLORTYPE_PEN:      bpen = image->data[1]; break;
		case FV_COLORTYPE_RGB:      bpen = (image->color[1]) ? image->color[1]->Pen : image->data[1]; break;
	}

	switch (image->pattern)
	{
///DOTS
		case F_PATTERN_MAKE(FV_PATTERN_DOTS, FF_PATTERN_SMALL):
		{
			STATIC uint16 p[] =
			{
				0xCCCC, 0xCCCC,
				(uint16) ~0xCCCC, (uint16) ~0xCCCC
			};
			
			pattern = p; pattern_size = 2;
		}
		break;

		case F_PATTERN_MAKE(FV_PATTERN_DOTS, FF_PATTERN_MEDIUM):
		{
			STATIC uint16 p[] =
			{
				0xF0F0, 0xF0F0,
				0xF0F0, 0xF0F0,
				(uint16) ~0xF0F0, (uint16) ~0xF0F0,
				(uint16) ~0xF0F0, (uint16) ~0xF0F0
			};
			
			pattern = p; pattern_size = 3;
		}
		break;
	
		case F_PATTERN_MAKE(FV_PATTERN_DOTS, FF_PATTERN_LARGE):
		{
			STATIC uint16 p[] =
			{
				0xFF00, 0xFF00, 0xFF00, 0xFF00,
				0xFF00, 0xFF00, 0xFF00, 0xFF00,
				(uint16) ~0xFF00, (uint16) ~0xFF00, (uint16) ~0xFF00, (uint16) ~0xFF00,
				(uint16) ~0xFF00, (uint16) ~0xFF00, (uint16) ~0xFF00, (uint16) ~0xFF00
			};
			
			pattern = p; pattern_size = 4;
		}
		break;
//+
///GROOVE
		case F_PATTERN_MAKE(FV_PATTERN_GROOVE, FF_PATTERN_XSMALL):
		{
			STATIC uint16 p[] = { 0xFFFF, 0x0000 }; pattern = p;
		}
		break;
		
		case F_PATTERN_MAKE(FV_PATTERN_GROOVE, FF_PATTERN_SMALL):
		{
			STATIC uint16 p[] = { 0xFFFF, 0xFFFF, 0x0000, 0x0000 }; pattern = p; pattern_size = 2;
		}
		break;
		
		case F_PATTERN_MAKE(FV_PATTERN_GROOVE, FF_PATTERN_MEDIUM):
		{
			STATIC uint16 p[] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000 }; pattern = p; pattern_size = 3;
		}
		break;
	
		case F_PATTERN_MAKE(FV_PATTERN_GROOVE, FF_PATTERN_LARGE):
		{
			STATIC uint16 p[] =
			{
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0x0000, 0x0000, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000
			};
			
			pattern = p; pattern_size = 4;
		}
		break;
//+
///VERTICAL
		case F_PATTERN_MAKE(FV_PATTERN_STRIPES, FF_PATTERN_XSMALL):
		{
			STATIC uint16 p[] = { 0xAAAA, 0xAAAA, };

			pattern = p;
		}
		break;
		
		case F_PATTERN_MAKE(FV_PATTERN_STRIPES, FF_PATTERN_SMALL):
		{
			STATIC uint16 p[] = { 0xCCCC, 0xCCCC, };
			
			pattern = p;
		}
		break;
	
		case F_PATTERN_MAKE(FV_PATTERN_STRIPES, FF_PATTERN_MEDIUM):
		{
			STATIC uint16 p[] = { 0xF0F0, 0xF0F0 }; pattern = p;
		}
		break;
		
		case F_PATTERN_MAKE(FV_PATTERN_STRIPES, FF_PATTERN_LARGE):
		{
			STATIC uint16 p[] = { 0xFF00, 0xFF00 }; pattern = p;
		}
		break;
//+
///OBLIQUE
		case F_PATTERN_MAKE(FV_PATTERN_OBLIQUE, FF_PATTERN_SMALL):
		{
			/*
			
			// 1100110011001100 0110011001100110 0011001100110011 1001100110011001

			*/

			STATIC uint16 p[] =
			{
				0xCCCC, 0x6666, 0x3333, 0x9999
			};
			
			pattern = p; pattern_size = 2;
		}
		break;

		case F_PATTERN_MAKE(FV_PATTERN_OBLIQUE, FF_PATTERN_MEDIUM):
		{

			// 1111000011110000 0111100001111000 0011110000111100 0001111000011110
			// 0000111100001111 1000011110000111 1100001111000011 1110000111100001
 
			STATIC uint16 p[] =
			{

				0xF0F0, 0x7878, 0x3C3C, 0x1E1E,
				0x0F0F, 0x8787, 0xC3C3, 0xE1E1,
				0xF0F0, 0x7878, 0x3C3C, 0x1E1E,
				0x0F0F, 0x8787, 0xC3C3, 0xE1E1
				
			};
			
			pattern = p; pattern_size = 4;
		}
		break;
	
		case F_PATTERN_MAKE(FV_PATTERN_OBLIQUE, FF_PATTERN_LARGE):
		{
			
			// 1111111100000000 0111111110000000 0011111111000000 0001111111100000
			
			STATIC uint16 p[] =
			{

			0xFF00, 0x7F80, 0x3FC0, 0x1FE0,
			0x0FF0, 0x07F8, 0x03FC, 0x01FE,
			0x00FF, 0x807F, 0xC03F, 0xE01F,
			0xF00F, 0xF807, 0xFC03, 0xFE01

			};
			
			pattern = p; pattern_size = 4;
		}
		break;
//+

		default:
		{
			STATIC uint16 p[] =
			{
				0xAAAA,
				(uint16) ~0xAAAA
			};

			pattern = p;
		}
		break;
	}

	rp->AreaPtrn = pattern;
	rp->AreaPtSz = pattern_size;

	IGRAPHICS SetABPenDrMd(rp,apen,bpen,JAM2);

	_Boxf(Msg->Rect->x1,Msg->Rect->y1,Msg->Rect->x2,Msg->Rect->y2);

	rp->AreaPtSz = prev_ptsz;
	rp->AreaPtrn = prev_ptrn;

	return TRUE;
}
//+

struct in_CodeTable id_pattern_table =
{
	(in_Code_Create *)   &id_pattern_create,
	(in_Code_Delete *)   &id_pattern_delete,
	(in_Code_Setup *)    &id_pattern_setup,
	(in_Code_Cleanup *)  &id_pattern_cleanup,
	(in_Code_Get *)      NULL,
	(in_Code_Draw *)     &id_pattern_draw
};
