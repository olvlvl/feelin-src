/*

    Support to transform pixels and colors to ARGB.  Only  RGB  and  Palette
    sources are supported, I should add more in the future such as Grey...

*/

#include "Private.h"

#if 0

///bitmap_transform_rgb
STATIC void bitmap_transform_rgb(struct FeelinBitMapTransform *fbt, uint8 *source, uint32 *target, uint32 pixels)
{
    uint32 stop = (uint32)(source) + ((pixels - 1) * sizeof (uint8) * 3);

//    uint32 n=1;

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

//        n++;
    }

//    F_Log(0,"%ld pixels transformed (number %ld)",n, number);
}
//+
///bitmap_transform_palette
STATIC void bitmap_transform_palette(struct FeelinBitMapTransform *fbt, uint8 *source, uint32 *target, uint32 pixels)
{
    uint32 stop = (uint32)(source) + (pixels * sizeof (uint8));

    while ((uint32)(source) < stop)
    {
        *target++ = fbt->colors[*source++];
    }
}
//+

///bitmap_transform_init
uint32 bitmap_transform_init(struct FeelinBitMapTransform *fbt, uint8 PixelSize)
{
    /* FIXME: future extention may require color type checking too */
    
    switch (PixelSize)
    {
        case 1:
        {
            fbt->func = bitmap_transform_palette;
        }
        break;

        case 3:
        {
            fbt->func = bitmap_transform_rgb;
        }
        break;
    
        case 4:
        {
            fbt->func = NULL; // no transformation needed;
        }
        break;

        default:
        {
            IFEELIN F_Log(FV_LOG_DEV,"Unable to transform. Pixel size of %ld is not supported",PixelSize);

            fbt->func = NULL;

            return FALSE;
        }
        break;
    }

    return TRUE;
}
//+

#endif
