/*

$VER: 01.00 (2005/08/17)

    This class is used to decode  PNG  images,  taking  care  of  preserving
    alpha-channel informations which are discarted by datatypes.
 
*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void, CorePNG_New);
F_METHOD_PROTO(void, CorePNG_Dispose);
//+

#ifdef __MORPHOS__

struct Library *PNGBase = NULL;

///Class_New
F_METHOD(uint32,Class_New)
{
    PNGBase = IEXEC OpenLibrary("png.library",51);
 
    if (PNGBase)
    {
        return F_SUPERDO();
    }

    return 0;
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
    if (PNGBase)
    {
        IEXEC CloseLibrary(PNGBase); PNGBase = NULL;
    }

    return F_SUPERDO();
}
//+

#endif

F_QUERY()
{
    switch (Which)
    {
#ifdef __MORPHOS__
///Meta
        case FV_Query_MetaClassTags:
        {
            STATIC F_METHODS_ARRAY =
            {
                F_METHODS_ADD_STATIC(Class_New,      FM_New),
                F_METHODS_ADD_STATIC(Class_Dispose,  FM_Dispose),

                F_ARRAY_END
            };

            STATIC F_TAGS_ARRAY =
            {
                F_TAGS_ADD_SUPER(Class),
                F_TAGS_ADD_METHODS,

                F_ARRAY_END
            };

            return F_TAGS_PTR;
        }
//+
#endif
///Class
        case FV_Query_ClassTags:
        {
            STATIC F_VALUES_ARRAY(SourceType) =
            {
                F_VALUES_ADD("none",FV_CoreMedia_SourceType_None),
                F_VALUES_ADD("file",FV_CoreMedia_SourceType_File),
                F_VALUES_ADD("handle",FV_CoreMedia_SourceType_Handle),
           
                F_ARRAY_END
            };
   
            STATIC F_ATTRIBUTES_ARRAY =
            {
                F_ATTRIBUTES_ADD("Source", FV_TYPE_POINTER),
                F_ATTRIBUTES_ADD_VALUES("SourceType", FV_TYPE_INTEGER, SourceType),

                F_ARRAY_END
            };

            STATIC F_METHODS_ARRAY =
            {
                F_METHODS_ADD_STATIC(CorePNG_New, FM_New),
                F_METHODS_ADD_STATIC(CorePNG_Dispose, FM_Dispose),
                
                F_ARRAY_END
            };

            STATIC F_RESOLVEDS_ARRAY =
            {
                F_RESOLVEDS_ADD("FA_BitMap_Width"),
                F_RESOLVEDS_ADD("FA_BitMap_Height"),
                F_RESOLVEDS_ADD("FA_BitMap_Depth"),
                F_RESOLVEDS_ADD("FA_BitMap_PixelSize"),
                F_RESOLVEDS_ADD("FA_BitMap_PixelArray"),
                F_RESOLVEDS_ADD("FA_BitMap_PixelArrayMod"),
                F_RESOLVEDS_ADD("FA_BitMap_ColorCount"),
                F_RESOLVEDS_ADD("FA_BitMap_ColorArray"),
                F_RESOLVEDS_ADD("FA_BitMap_ColorType"),

                F_ARRAY_END
            };

            STATIC F_TAGS_ARRAY =
            {
                F_TAGS_ADD_SUPER(BitMap),
                F_TAGS_ADD_LOD,
                F_TAGS_ADD_METHODS,
                F_TAGS_ADD_ATTRIBUTES,
                F_TAGS_ADD_RESOLVEDS,

                F_ARRAY_END
            };

            return F_TAGS_PTR;
        }
//+
    }
    return NULL;
}
