#include "Private.h"

//#define DB_SETUP
//#define DB_SETUP_FONT

///types

typedef struct in_FeelinTD_Context_Setup
{
   struct TextExtent                te;
   struct TextFont                 *font;
   char                             font_spec[64];
}
FTDContextSetup;

//+

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///td_setup
void td_setup(FClass *Class,FObject Obj)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    FTDLine *line;
    FTDChunk *chunk;
    FTDContextSetup context;

    context.font = LOD->Font;
    
/*** setup colors ******************************************************************************/
    
    #ifdef F_ENABLE_COLOR
    {
        struct in_Color *color;
     
        for (color = (struct in_Color *) LOD->colors ; color ; color = color->next)
        {
            color->color = (FColor *) IFEELIN F_Do(LOD->Render->Display, FM_Display_CreateColor, color->spec, LOD->Render->Palette);
        
            if (color->color)
            {
                color->pen = color->color->Pen;
            }
            else
            {
                color->pen = LOD->Render->Palette->Pens[FV_Pen_Text];
            }
        }
    }
    #endif
    
/*** setup fonts *******************************************************************************/

    #ifdef F_ENABLE_FONT
    {
        struct in_Font *font;
        
        for (font = (struct in_Font *) LOD->fonts ; font ; font = font->next)
        {
            if (font->setup_font == NULL)
            {
                IFEELIN F_StrFmt((STRPTR) &context.font_spec, "%s/%ld",font->face, font->size);
                
                #ifdef DB_SETUP
                F_Log(0,"font spec (%s)", &context.font_spec);
                #endif
                                    
                font->setup_font = (struct TextFont *) IFEELIN F_Do(LOD->Render->Application,FM_Application_OpenFont, NULL, &context.font_spec);
                
                if (font->setup_font == NULL)
                {
                    font->setup_font = LOD->Font;
                    font->flags |= FF_TD_FONT_REFERENCE_FONT;
                }
            }
        }
    }
    #endif
 
/*** setup lines *******************************************************************************/
  
    for (line = (FTDLine *) LOD->LineList.Head ; line ; line = line->Next)
    {
        uint16 font_above = 0;
        uint16 font_below = 0;
                                
        #ifdef DB_SETUP
        IFEELIN F_Log(0,"line font (0x%08lx) setup (0x%08lx)",line->font, line->font->setup_font);
        #endif
  
        context.font = line->font->setup_font;
        
        line->Width  = 0;
        line->Height = context.font->tf_YSize;

        for (chunk = (FTDChunk *) line->ChunkList.Head ; chunk ; chunk = chunk->Next)
        {
            switch (_td_chunk_type)
            {
                case FV_TD_CHUNK_TEXT:
                {
                    context.font = _td_chunk_text_font->setup_font;

                    td_text_extent(context.font, _td_chunk_text_text, _td_chunk_text_chars, &context.te);

                    #ifdef DB_SETUP
                    IFEELIN F_Log(0,"font (%s/%ld) (chunk: %ldx%ld)",context.font->tf_Message.mn_Node.ln_Name,context.font->tf_YSize,context.te.te_Width,context.te.te_Height);
                    #endif

                    _td_chunk_width  = context.te.te_Width;
                    _td_chunk_height = context.te.te_Height;

                    font_above = MAX(font_above,context.font->tf_Baseline);
                    font_below = MAX(font_below,context.font->tf_YSize - context.font->tf_Baseline);

                    line->baseline = MAX(line->baseline,context.font->tf_Baseline);
                }
                break;

                case FV_TD_CHUNK_IMAGE:
                {
                    FTDChunkImage *image = (FTDChunkImage *) chunk;

                    image->object = ImageDisplayObject,

                        FA_ImageDisplay_Spec,  image->spec,

                        End;
                    
                    if (image->object)
                    {
                        uint32 w,h;

                        IFEELIN F_Do(image->object,FM_ImageDisplay_Setup,LOD->Render);

                        IFEELIN F_Do
                        (
                            image->object, FM_Get,
                            
                            FA_ImageDisplay_Width, &w,
                            FA_ImageDisplay_Height, &h,
                            
                            TAG_DONE
                        );

                        _td_chunk_width  = (image->w) ? (image->w) : w;
                        _td_chunk_height = (image->h) ? (image->h) : h;
                    }
                }
                break;
            }

            line->Width += _td_chunk_width;
            line->Height = MAX(line->Height,_td_chunk_height);
        }

        line->Height = MAX(line->Height,font_above + font_below);
        line->baseline = font_above;

        #ifdef DB_SETUP
        IFEELIN F_Log(0,"line %ld x %ld - baseline %ld - above %ld, below %ld",line->Width,line->Height,line->baseline,font_above,font_below);
        #endif

        if (line->hr)
        {
            line->hr->spacing = context.font->tf_YSize / 2;

            if (!line->hr->compact)
            {
                line->Height += line->hr->size + line->hr->spacing * 2;
            }
        }

        LOD->Width = MAX(LOD->Width,line->Width);
        LOD->Height += line->Height;

        if (line->Next)
        {
            LOD->Height += line->spacing;
        }
    }

    #if 1

    if (((FF_TD_EXTRA_TOPLEFT | FF_TD_EXTRA_BOTTOMRIGHT) & LOD->Flags) != 0)
    {
       LOD->Width += 2;
       LOD->Height += 2;
    }

    #else

    if (FF_TD_EXTRA_TOPLEFT & LOD->Flags)
    {
       LOD->Width += 1;
       LOD->Height += 1;
    }

    if (FF_TD_EXTRA_BOTTOMRIGHT & LOD->Flags)
    {
       LOD->Width += 1;
       LOD->Height += 1;
    }

    #endif

    #ifdef DB_SETUP
    IFEELIN F_Log(0,"%ld x %ld",LOD->Width,LOD->Height);
    #endif
}
//+
