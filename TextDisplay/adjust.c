#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///td_line_adjust
void td_line_adjust(FTDLine *Line,FTDAdjust *Adjust)
{
    FTDChunk *chunk;

    uint16 width  = Adjust->limit_w;
    uint16 dots_w = td_text_extent(Adjust->font, "...", 3, &Adjust->te);

    Line->adjust_width = 0;
    Line->adjust_chars = 0;
    Line->adjust_image = 0;

    for (chunk = (FTDChunk *) Line->ChunkList.Head ; chunk ; chunk = chunk->Next)
    {
        if (_td_chunk_type == FV_TD_CHUNK_TEXT)
        {
            #ifdef F_ENABLE_FONT
            
            if (_td_chunk_text_font && _td_chunk_text_font->setup_font)
            {
                dots_w = td_text_extent(_td_chunk_text_font->setup_font,"...",3,&Adjust->te);
            }
            break;
        
            #else
                
            dots_w = td_text_extent(Adjust->font, "...", 3, &Adjust->te);
 
            #endif
        }
    }

    /* dots_w must be computed with the last font used on the line */

    if (Line->Width > width)
    {
        #ifdef DB_ADJUST
        IFEELIN F_Log(0,"[overflow] line (%ld) rect (%ld) dots (%ld)",Line->Width,width,dots_w);
        #endif
        
        if (dots_w > width) return;

        Line->adjust_width = dots_w;
        width -= dots_w;

        /* on tient � jour  'chars_remain'  (le  nombre  de  caract�res  qui
        tiennent,  qui  restent  donc � dessiner) et 'line_width' (la lageur
        finale de la ligne, qui sera utilis�e pour la justification).

        si la ligne est trop grande, on regarde si les trois  petits  points
        tiennent.*/

        #if 0
        if (dots_w < width)
        #endif
        {
            
            /* s'ils tiennent, on parcourt  les  chunks  en  calculant  leur
            largeur (en n'oubliant pas d'enlever la largeur des trois petits
            points '...' . si un chunk est trop grand, on calcule  alors  le
            nombre de caract�res qui peuvent �tres ajout�s et on termine. */

            for (chunk = (FTDChunk *) Line->ChunkList.Head ; chunk ; chunk = chunk->Next)
            {
                if (chunk->Width < width)
                {
                    width -= chunk->Width;
                    Line->adjust_width += chunk->Width;

                    #ifdef DB_ADJUST
                    IFEELIN F_Log(0,"TYPE %ld - %ld x %ld",chunk->Type,chunk->Width,chunk->Height);
                    #endif

                    switch (chunk->Type)
                    {
                        case FV_TD_CHUNK_TEXT:   Line->adjust_chars += ((FTDChunkText *)(chunk))->chars; break;
                        case FV_TD_CHUNK_IMAGE:  Line->adjust_image += chunk->Width; break;
                    }
                }
                else
                {
                    switch (chunk->Type)
                    {
                        case FV_TD_CHUNK_TEXT:
                        {
                            struct TextFont *font;
                            uint16 chars;
                            uint16 w;
                            uint16 cw;

                            #ifdef F_ENABLE_FONT
                            if (_td_chunk_text_font && _td_chunk_text_font->setup_font)
                            {
                                font = _td_chunk_text_font->setup_font;
                            }
                            else
                            #endif
                            {
                                font = Adjust->font;
                            }

                            chars = width / font->tf_XSize;

                            #ifdef DB_ADJUST
                            IDOS_ FPrintf(FeelinBase->Console,"[text] chu_w %ld - rem_w %ld : '",chunk->Width,width);
                            IDOS_ Flush(FeelinBase->Console);
                            IDOS_ Write(FeelinBase->Console,((FTDChunkText *)(chunk))->text,((FTDChunkText *)(chunk))->chars);
                            IDOS_ Write(FeelinBase->Console,"'\n",2);
                            #endif

                            w = td_text_extent(font,_td_chunk_text_text,chars,&Adjust->te);

                            #ifdef DB_ADJUST
                            IFEELIN F_Log(0,"chunkW %ld (%ld chars) - freeW %ld (%ld chars)",chunk->Width,((FTDChunkText *)(chunk))->chars,width,chars);
                            #endif

                            while (((cw = td_text_extent(font,_td_chunk_text_text + chars,1,&Adjust->te)) + w) < width)
                            {
                                chars++; w += cw;
                            }

                            #ifdef DB_ADJUST
                            IFEELIN F_Log(0,"finalW %ld - finalC %ld",w,chars);
                            #endif

                            Line->adjust_width += w;
                            Line->adjust_chars += chars;

                            #ifdef DB_ADJUST
                            IFEELIN F_Log(0,"line_width %ld - chars_remain %ld",Comp->line_width,Comp->chars_remain);
                            #endif

                            goto __check_justify;
                        }
                        break;

                        case FV_TD_CHUNK_IMAGE:
                        {
                            #ifdef DB_ADJUST
                            IDOS_ FPrintf(FeelinBase->Console,"[image] chu_w %ld - rem_w %ld\n",chunk->Width,width);
                            #endif

                            Line->adjust_width += width;
                            Line->adjust_image += width;

                            goto __check_justify;
                        }
                        break;
                    }
                }
            }
        }
    }
    else
    {
        for (chunk = (FTDChunk *)(Line->ChunkList.Head) ; chunk ; chunk = chunk->Next)
        {
            switch (chunk->Type)
            {
                case FV_TD_CHUNK_TEXT:   Line->adjust_chars += _td_chunk_text_chars; break;
                case FV_TD_CHUNK_IMAGE:  Line->adjust_image += chunk->Width; break;
            }
        }

        Line->adjust_width = Line->Width;
    }

__check_justify:

    /* To justify the text I only consider inner  spaces,  ignoring  leading
    and  trailing  spaces.  Leading and trailing spaces keep a normal width,
    only inner spaces are 'proportionnalized'. */

    if (Line->align == FV_TD_LINE_ALIGN_JUSTIFY && Adjust->limit_w != FV_TD_NOLIMIT)
    {
        struct TextFont *font;

        uint32 rem_chars = Line->adjust_chars;
        uint32 pixels_used_by_spaces = 0;
        uint32 space_total_number = 0;

        /*  leading  and  trailing  spaces  must  be  ignored,  the   boolean
        'leading_done'  is  used  to know if leading spaces have already been
        discard */

        int8 leading_done = FALSE;

        for (chunk = (FTDChunk *) Line->ChunkList.Head ; chunk ; chunk = chunk->Next)
        {
            if (chunk->Type == FV_TD_CHUNK_TEXT)
            {
                uint32 num = 0;
                uint32 len = MIN(rem_chars,_td_chunk_text_chars);
                STRPTR str = _td_chunk_text_text;
                STRPTR end = str + len - 1;

                #ifdef F_ENABLE_FONT
                if (_td_chunk_text_font && _td_chunk_text_font->setup_font)
                {
                    font = _td_chunk_text_font->setup_font;
                }
                else
                #endif
                {
                    font = Adjust->font;
                }

                rem_chars -= len;

                if (leading_done == FALSE)
                {
                    while (*str == ' ') str++; leading_done = TRUE;
                }

                while (str <= end)
                {
                    if (*str++ == ' ') num++;
                }

                str--;

                #ifdef DB_ADJUST
                IFEELIN F_Log(0,"1# %ld spaces, end char (%lc) '%4.4s'",num,str[-1],((FTDChunkText *)(chunk))->text);
                #endif

                /* discart trailing spaces */

                if (!chunk->Next)
                {
                    while (*str-- == ' ')
                    {
                        num--;
                    }

                    #ifdef DB_ADJUST
                    IFEELIN F_Log(0,"2# %ld spaces, end char (%lc) '%4.4s'",num,str[-1],((FTDChunkText *)(chunk))->text);
                    #endif
                }

                space_total_number    += num;
                pixels_used_by_spaces += num * td_text_extent(font," ",1,&Adjust->te);
            }

            if (!rem_chars) break;
        }

        if (space_total_number)
        {
            Line->space_width = (Adjust->limit_w - (Line->adjust_width - pixels_used_by_spaces)) / space_total_number;
        }
        else
        {
            Line->space_width = 0;//Adjust->limit_w - (Line->adjust_width - pixels_used_by_spaces);
        }
        Line->space_loss  = (Adjust->limit_w - Line->space_width * space_total_number - (Line->adjust_width - pixels_used_by_spaces));

        #ifdef DB_ADJUST
        IFEELIN F_Log(0,"%ld spaces, pixels_spaces %ld - pixels_text %ld - available %ld - space_w %ld - loss %ld",space_total_number,pixels_used_by_spaces,Line->adjust_width - pixels_used_by_spaces,Adjust->limit_w,Line->space_width,Line->space_loss);
        #endif
    }
}
//+

