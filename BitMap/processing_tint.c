#include "Private.h"
#include "processing.h"

///bitmap_processing_tint
bool32 bitmap_processing_tint(FClass *Class, FObject Obj, struct in_Processing_Tint *data)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    uint16 ratio = data->ratio;

    int32 r = (data->rgb & 0xFF0000) >> 16;
    int32 g = (data->rgb & 0xFF00) >> 8;
    int32 b = (data->rgb & 0xFF);

    int32 s;

    uint32 col;
    uint32 rgba;

    switch (LOD->cpp.PixelSize)
    {
        case 1:
        {
            uint32 *array = LOD->cpp.ColorArray;
            uint32 n = LOD->cpp.ColorCount;

            if (array && n)
            {
                while (n--)
                {
                    col = *array;

                    s = (col & 0xFF000000) >> 24;
                    s += (((r - s) * ratio) >> 8);
                    rgba = s << 8;

                    s = (col & 0xFF0000) >> 16;
                    rgba |= s + (((g - s) * ratio) >> 8);
                    rgba <<= 8;

                    s = (col & 0xFF00) >> 8;
                    rgba |= s + (((b - s) * ratio) >> 8);
                    rgba <<= 8;

                    rgba |= (col & 0xFF);  // keep alpha channel in dest

                    *array++ = rgba;
                }
            }
        }
        break;

        case 3:
        {
            uint8 *src = LOD->cpp.PixelArray;
            uint32 n = LOD->cpp.PixelArrayMod / LOD->cpp.PixelSize * LOD->cpp.Height;

            while (n--)
            {
                s = *src; *src++ = s + (((r - s) * ratio) >> 8);
                s = *src; *src++ = s + (((g - s) * ratio) >> 8);
                s = *src; *src++ = s + (((b - s) * ratio) >> 8);
            }
        }
        break;

        case 4:
        {
            uint32 *src = LOD->cpp.PixelArray;
            uint32 n = LOD->cpp.PixelArrayMod / LOD->cpp.PixelSize * LOD->cpp.Height;

            while (n--)
            {
                col = *src;

                s = (col & 0xFF000000) >> 24;
                s += (((r - s) * ratio) >> 8);
                rgba = s << 8;

                s = (col & 0xFF0000) >> 16;
                rgba |= s + (((g - s) * ratio) >> 8);
                rgba <<= 8;

                s = (col & 0xFF00) >> 8;
                rgba |= s + (((b - s) * ratio) >> 8);
                rgba <<= 8;

                rgba |= (col & 0xFF);  // keep alpha channel in dest

                *src++ = rgba;
            }
        }
        break;

        default: return FALSE;
    }

    return TRUE;
}
//+
