#include "Private.h"
#include "processing.h"

///bitmap_processing_convert
bool32 bitmap_processing_convert(FClass *Class, FObject Obj, struct in_Processing_Convert *data)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

//    IFEELIN F_Log(0,"convert (%ld) to (%ld)",LOD->cpp.PixelSize, data->pixel_size);

    if (LOD->cpp.PixelSize == data->pixel_size)
    {
        return TRUE;
    }

    switch (data->pixel_size)
    {
        case 1:
        {
            IFEELIN F_Log(0,"convert to pixel size (%ld) is not yet implemented", 1);
        }
        break;

        case 3:
        {
            IFEELIN F_Log(0,"convert to pixel size (%ld) is not yet implemented", 3);
        }
        break;

        case 4:
        {
            uint32 *buffer = IFEELIN F_New(LOD->cpp.Width * LOD->cpp.Height * 4);

            if (buffer)
            {
                uint32 *target = buffer;

                switch (LOD->cpp.PixelSize)
                {
                    case 1:
                    {
                        uint8 *source = LOD->cpp.PixelArray;
                        uint32 h = LOD->cpp.Height;

                        if (LOD->cpp.ColorArray)
                        {
                            //IFEELIN F_Log(0,"using color array");

                            while (h--)
                            {
                                uint32 stop = (uint32)(source) + (LOD->cpp.Width * sizeof (uint8));

                                while ((uint32)(source) < stop)
                                {
                                    *target++ = LOD->cpp.ColorArray[*source++];
                                }
                            }
                        }
                        else
                        {
                            //IFEELIN F_Log(0,"using grey");

                            while (h--)
                            {
                                uint32 stop = (uint32)(source) + (LOD->cpp.Width * sizeof (uint8));

                                while ((uint32)(source) < stop)
                                {
                                    uint8 grey = *source++;

                                    *target++ = (grey << 24) | (grey << 16) | (grey << 8) | 0xFF;
                                }
                            }
                        }
                    }
                    break;

                    case 3:
                    {
                        uint8 *source = LOD->cpp.PixelArray;
                        uint32 h = LOD->cpp.Height;

                        while (h--)
                        {
                            uint32 stop = (uint32)(source) + ((LOD->cpp.Width - 1) * sizeof (uint8) * 3);

                            while ((uint32)(source) < stop)
                            {
                                uint32 rgba;

                                rgba = *source++;
                                rgba <<= 8;
                                rgba |= *source++;
                                rgba <<= 8;
                                rgba |= *source++;
                                rgba <<= 8;
                                rgba |= 0xFF;

                                *target++ = rgba;
                            }
                        }
                    }
                    break;
                }

                IFEELIN F_Dispose(LOD->cpp.PixelArray);

                LOD->cpp.PixelSize      = 4;
                LOD->cpp.PixelArray     = buffer;
                LOD->cpp.PixelArrayMod  = LOD->cpp.Width * LOD->cpp.PixelSize;

                /* We keep color information. If we need them  later,  we  won't
                have to compute them again, unless... */

                #if 0 

                IFEELIN F_Dispose(LOD->cpp.ColorArray);

                LOD->cpp.ColorType      = FV_COLOR_TYPE_RGB;
                LOD->cpp.ColorCount     = 0;
                LOD->cpp.ColorArray     = NULL;

                #endif

                return TRUE;
            }
        }
    }

    return FALSE;
}
//+
