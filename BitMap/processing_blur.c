#include "Private.h"
#include "processing.h"

///bitmap_processing_blur
bool32 bitmap_processing_blur(FClass *Class, FObject Obj, struct in_Processing_Blur *data)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    uint32 *dst;
    int32 w = LOD->cpp.Width * LOD->cpp.Height;

    if (LOD->cpp.PixelSize != 4)
    {
        struct in_Processing_Convert msg = { 4 };

        if (bitmap_processing_convert(Class,Obj,&msg) == FALSE)
        {
            return FALSE;
        }
    }

    dst = IFEELIN F_New(LOD->cpp.PixelArrayMod * LOD->cpp.Height);

    if (dst)
    {
        uint32 *src = LOD->cpp.PixelArray;

        dst[0] = ((((src[0] & 0xFF00FF00) * 3 + (src[1] & 0xFF00FF00) + 0x02000200) >> 2) & 0xFF00FF)
            + ((((src[0] & 0x00FF0000) * 3 + (src[1] & 0x00FF0000) + 0x00020000) >> 2) & 0x00FF0000);

        ++dst;

        w -= 2;

        if (w)
        {
            w = -w;

            src = src - w;
            dst = dst - w;

            do
            {
                uint32 s1 = src[w+0];
                uint32 s2 = src[w+1];
                uint32 s3 = src[w+2];

                dst[w+0] = ((((s1 & 0xFF00FF00) + 2 * (s2 & 0xFF00FF00) + (s3 & 0xFF00FF00) + 0x02000200) >> 2) & 0xFF00FF00)
                    + ((((s1 & 0x00FF0000) + 2 * (s2 & 0x00FF0000) + (s3 & 0x00FF0000) + 0x00020000) >> 2) & 0x00FF0000);
            }
            while (++w);
        }

        dst[0] = ((((src[0] & 0x00FF00FF) + (src[1] & 0x00FF00FF) * 3 + 0x00020002) >> 2) & 0xFF00FF)
            + ((((src[0] & 0x0000FF00) + (src[1] & 0x0000FF00) * 3 + 0x00000200) >> 2) & 0x00FF00);

        IFEELIN F_Dispose(LOD->cpp.PixelArray);
        LOD->cpp.PixelArray = dst;

        return TRUE;
    }

    return FALSE;
}
//+
