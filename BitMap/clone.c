#include "Private.h"

///BitMap_Clone
F_METHOD(FObject,BitMap_Clone)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    uint32 pixel_array_size = LOD->cpp.PixelArrayMod * LOD->cpp.Height;
    uint32 color_array_size = LOD->cpp.ColorCount * sizeof (uint32);

    APTR pixel_array = NULL;
    APTR color_array = NULL;

    FObject clone;

    if ((LOD->cpp.PixelArray != NULL) && (pixel_array_size != 0))
    {
        pixel_array = IFEELIN F_New(pixel_array_size);

        if (pixel_array == NULL)
        {
            IFEELIN F_Log(FV_LOG_USER, "not enough memory to clone pixels array");

            return NULL;
        }
    }
    else
    {
        IFEELIN F_Log(FV_LOG_DEV, "bitmap is empty, nothing to clone");

        return NULL;
    }

    if ((LOD->cpp.ColorArray != NULL) && (color_array_size != 0))
    {
        color_array = IFEELIN F_New(color_array_size);

        if (color_array == NULL)
        {
            IFEELIN F_Log(FV_LOG_USER, "not enough memory to clone colors array");

            return NULL;
        }
    }

    IEXEC CopyMem(LOD->cpp.PixelArray, pixel_array, pixel_array_size);

    if (color_array)
    {
        IEXEC CopyMem(LOD->cpp.ColorArray, color_array, color_array_size);
    }

    clone = BitMapObject,

        F_ATTRIBUTE_ID(WIDTH),         LOD->cpp.Width,
        F_ATTRIBUTE_ID(HEIGHT),        LOD->cpp.Height,
        F_ATTRIBUTE_ID(DEPTH),         LOD->cpp.Depth,

        F_ATTRIBUTE_ID(PIXELSIZE),     LOD->cpp.PixelSize,
        F_ATTRIBUTE_ID(PIXELARRAY),    pixel_array,
        F_ATTRIBUTE_ID(PIXELARRAYMOD), LOD->cpp.PixelArrayMod,

        F_ATTRIBUTE_ID(COLORTYPE),     LOD->cpp.ColorType,
        F_ATTRIBUTE_ID(COLORCOUNT),    LOD->cpp.ColorCount,
        F_ATTRIBUTE_ID(COLORARRAY),    color_array,

        TAG_DONE);

    if (clone == NULL)
    {
        IFEELIN F_Dispose(color_array);
        IFEELIN F_Dispose(pixel_array);
    }

    return clone;
}
//+
///BitMap_Steal
F_METHODM(bool32,BitMap_Steal,FS_BitMap_Steal)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    bitmap_clear(Class,Obj);

    if (Msg->Source)
    {
        if (IFEELIN F_Get(Msg->Source, F_ATTRIBUTE_ID(PIXELARRAY)) != 0)
        {
            struct LocalObjectData *s_lod = F_LOD(Class,Msg->Source);

            IEXEC CopyMem(&s_lod->cpp, &LOD->cpp, sizeof (FCorePicturePublic));

            #ifdef DB_STEAL

            IFEELIN F_Log
            (
                0, "pixel array (0x%08lx) color array (0x%08lx)",

                _bitmap_pixel_array, _bitmap_color_array
            );

            #endif

            s_lod->cpp.PixelArray = NULL;
            s_lod->cpp.ColorArray = NULL;

            bitmap_clear(Class, Msg->Source);

            return TRUE;
        }
        else
        {
            IFEELIN F_Log(FV_LOG_DEV, "suspicious object to steal from %s{%08lx}, PixelArray is NULL", _object_classname(Msg->Source), Msg->Source);
        }
    }

    return FALSE;
}
//+
