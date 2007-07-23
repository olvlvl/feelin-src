#include "Private.h"
#include "processing.h"

///bitmap_processing_crop
bool32 bitmap_processing_crop(FClass *Class, FObject Obj, struct in_Processing_Crop *data)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    APTR buffer = IFEELIN F_New(LOD->cpp.PixelSize * LOD->cpp.Width * LOD->cpp.Height);

    IFEELIN F_Log(0,"crop bitmap (%ld x %ld) >> (%ld : %ld, %ld x %ld) pixel size (%ld)",

        LOD->cpp.Width,
        LOD->cpp.Height,

        data->x,
        data->y,
        data->w,
        data->h,

        LOD->cpp.PixelSize);

    if (buffer)
    {
        APTR source = (APTR) ((uint32)(LOD->cpp.PixelArray) + data->x * LOD->cpp.PixelSize + data->y * LOD->cpp.PixelArrayMod);
        APTR destination = buffer;
        uint32 h = data->h;

        while (h--)
        {
            IEXEC CopyMem(source, destination, data->w * LOD->cpp.PixelSize);

            source = (APTR) ((uint32)(source) + LOD->cpp.PixelArrayMod);
            destination = (APTR) ((uint32)(destination) + data->w * LOD->cpp.PixelSize);
        }

        IFEELIN F_Dispose(LOD->cpp.PixelArray);

        LOD->cpp.Width          = data->w;
        LOD->cpp.Height         = data->h;
        LOD->cpp.PixelArray     = buffer;
        LOD->cpp.PixelArrayMod  = data->w * LOD->cpp.PixelSize;

        IFEELIN F_Log(0,"new pixel array (0x%08lx)", LOD->cpp.PixelArray);

        return TRUE;
    }

    return FALSE;
}
//+
