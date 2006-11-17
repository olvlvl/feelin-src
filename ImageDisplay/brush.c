#if 0

#include "Private.h"
#include <hardware/blit.h>

//#define DB_SHARED_OPEN
//#define DB_SHARED_CLOSE

typedef struct in_Image_Brush
{
    FObject                         picture;
    uint32                          width;      // must be uint32
    uint32                          height;     // must be uint32
    uint32                          color_type;
    uint32                          color_count;
    uint32                         *color_array;
    FPalette                       *prev_palette;
}
FImage;

/************************************************************************************************
*** Brush ***************************************************************************************
************************************************************************************************/

///id_brush_create
F_CODE_CREATE_XML(id_brush_create)
{
    FImage *image = NULL;

    STRPTR src=NULL;
      
    for ( ; Attribute ; Attribute = Attribute->Next)
    {
        if (Attribute->Atom == F_ATOM(SRC))
        {
            src = Attribute->Value;
        }
    }

    if (src)
    {
        STRPTR buf;
        FObject picture = NULL;;
        
        if ((buf = IFEELIN F_New(1024)) != NULL)
        {
            
            IFEELIN F_StrFmt(buf,"feelin:resources/brushes/");
            IDOS_ AddPart(buf,src,1024);

            picture = PictureObject,

                "FA_Picture_Name", buf,

                End;

            IFEELIN F_Dispose(buf);
        }
        
        if (picture)
        {
            if ((image = IFEELIN F_NewP(Pool, sizeof (FImage))) != NULL)
            {
                image->picture = picture;
            }
            else
            {
                IFEELIN F_DisposeObj(picture);
            }
        }
    }
 
    return image;
}
//+
///id_brush_delete
F_CODE_DELETE(id_brush_delete)
{
    IFEELIN F_DisposeObj(image->picture);
    IFEELIN F_Dispose(image);
}
//+
///id_brush_setup
F_CODE_SETUP(id_brush_setup)
{
    uint32 *pal;
 
    IFEELIN F_Do(image->picture, FM_Get,

        F_AUTO_ID(WIDTH),  &image->width,
        F_AUTO_ID(HEIGHT), &image->height,
        F_AUTO_ID(COLORTYPE), &image->color_type,
        F_AUTO_ID(COLORCOUNT), &image->color_count,
        F_AUTO_ID(COLORARRAY), &pal,

        TAG_DONE);
 
    if ((image->color_type & ~FF_COLOR_TYPE_ALPHA) == FV_COLOR_TYPE_PALETTE)
    {
        image->color_array = IFEELIN F_New(image->color_count * sizeof (uint32));
        
        if (image->color_array == NULL)
        {
            IEXEC CopyMem(pal, image->color_array, image->color_count * sizeof (uint32));
 
            return FALSE;
        }
    }
 
    return TRUE;
}
//+
///id_brush_cleanup
F_CODE_CLEANUP(id_brush_cleanup)
{
    IFEELIN F_Dispose(image->color_array); image->color_array = NULL;
}
//+
///id_brush_get
F_CODE_GET(id_brush_get)
{
    int32 rc=0;

    switch (Attribute)
    {
        case FA_ImageDisplay_Width:
        {
            rc = image->width;
        }
        break;

        case FA_ImageDisplay_Height:
        {
            rc = image->height;
        }
        break;
     
        case FA_ImageDisplay_Mask:
        {
            rc = (0 != (FF_COLOR_TYPE_ALPHA & image->color_type));
        }
        break;
    }

    return rc;
}
//+

///id_brush_draw
F_CODE_DRAW(id_brush_draw)
{
    FBox b;
    
    b.x = Msg->Rect->x1;
    b.y = Msg->Rect->y1;
    b.w = Msg->Rect->x2 - Msg->Rect->x1 + 1;
    b.h = Msg->Rect->y2 - Msg->Rect->y1 + 1;

    if (image->color_array && image->prev_palette != Msg->Render->Palette)
    {
        uint32 argb;

        argb = Msg->Render->Palette->ARGB[FV_Pen_Shine];
        image->color_array[1] = F_ARGB_TO_RGBA(argb);
        argb = Msg->Render->Palette->ARGB[FV_Pen_HalfShine];
        image->color_array[2] = F_ARGB_TO_RGBA(argb);
        argb = Msg->Render->Palette->ARGB[FV_Pen_Fill];
        image->color_array[3] = F_ARGB_TO_RGBA(argb);
        argb = Msg->Render->Palette->ARGB[FV_Pen_HalfShadow];
        image->color_array[4] = F_ARGB_TO_RGBA(argb);
        argb = Msg->Render->Palette->ARGB[FV_Pen_Shadow];
        image->color_array[5] = F_ARGB_TO_RGBA(argb);
        argb = Msg->Render->Palette->ARGB[FV_Pen_HalfDark];
        image->color_array[6] = F_ARGB_TO_RGBA(argb);
        argb = Msg->Render->Palette->ARGB[FV_Pen_Dark];
        image->color_array[7] = F_ARGB_TO_RGBA(argb);
        
        if (image->color_count > 8)
        {
            argb = Msg->Render->Palette->ARGB[FV_Pen_Text];
            image->color_array[8] = F_ARGB_TO_RGBA(argb);
            argb = Msg->Render->Palette->ARGB[FV_Pen_Highlight];
            image->color_array[9] = F_ARGB_TO_RGBA(argb);
        }
         
        image->prev_palette = Msg->Render->Palette;
    }
 
    if (image->width < b.w)
    {
        b.x = (b.w - image->width) / 2 + b.x;
        b.w = image->width;
    }

    if (image->height < b.h)
    {
        b.y = (b.h - image->height) / 2 + b.y;
        b.h = image->height;
    }
    
    IFEELIN F_Do(image->picture, F_AUTO_ID(BLIT),

        Msg->Render->RPort,

        0, 0, &b,

        "FA_BitMap_BlitMode", "Copy",
        (image->color_array) ? F_AUTO_ID(COLORARRAY) : TAG_IGNORE, image->color_array,

        TAG_DONE);
 
    return TRUE;
}
//+

struct in_CodeTable id_brush_table =
{
    (in_Code_Create *)   &id_brush_create,
    (in_Code_Delete *)   &id_brush_delete,
    (in_Code_Setup *)    &id_brush_setup,
    (in_Code_Cleanup *)  &id_brush_cleanup,
    (in_Code_Get *)      &id_brush_get,
    (in_Code_Draw *)     &id_brush_draw
};

#endif
