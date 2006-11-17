#include "Private.h"
#include "processing.h"

///bitmap_processing_radial
bool32 bitmap_processing_radial(FClass *Class, FObject Obj, struct in_Processing_Radial *data)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    uint32 i;
    uint32 j;
    uint32 mod;
    uint8 *src;

    /* parameters */

    uint32 x = (LOD->cpp.Width - 1) / 2;
    uint32 y = (LOD->cpp.Height - 1) / 2;

    uint32 keep = 10;

    // pre carré

    uint32 min = MIN(LOD->cpp.Width, LOD->cpp.Height) / 2;
    keep = MIN(keep, min - 1);

    min *= min;
    keep *= keep;

    // we need a rgba source

    if (LOD->cpp.PixelSize != 4)
    {
        struct in_Processing_Convert msg = { 4 };

        if (bitmap_processing_convert(Class,Obj,&msg) == FALSE)
        {
            return FALSE;
        }
    }

    LOD->cpp.ColorType |= FF_COLOR_TYPE_ALPHA;

    src = LOD->cpp.PixelArray;
    mod = LOD->cpp.PixelArrayMod;

    IFEELIN F_Log(0,"src (0x%08lx) mod (%ld) x (%ld) y (%ld) w (%ld) h (%ld)",src,mod,x,y,LOD->cpp.Width,LOD->cpp.Height);

    for (i = 0 ; i < LOD->cpp.Height; i++)
    {
        for (j = 0 ; j < LOD->cpp.Width; j++)
        {
            uint32 dy = y - i;
            uint32 dx = x - j;
            uint32 distance = dy * dy + dx * dx;

            if (distance < keep)
            {
                *(src + i * mod + j * 4 + 3) = 0xFF;
            }
            else if (distance > min)
            {
                *(src + i * mod + j * 4 + 3) = 0;
            }
            else
            {
                *(src + i * mod + j * 4 + 3) = 255 - ((distance - keep) * 255 / (min - keep));
            }
        }
    }

    return TRUE;
}
//+
