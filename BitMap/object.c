#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///bitmap_clear
void bitmap_clear(FClass *Class, FObject Obj)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    IFEELIN F_Dispose(_bitmap_pixel_array); 
    IFEELIN F_Dispose(_bitmap_color_array);

    _bitmap_width           = 0;
    _bitmap_height          = 0;
    _bitmap_depth           = 0;

    _bitmap_pixel_size      = 0;
    _bitmap_pixel_array     = NULL;
    _bitmap_pixel_array_mod = 0;

    _bitmap_color_type      = 0;
    _bitmap_color_count     = 0;
    _bitmap_color_array     = NULL;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///BitMap_New
F_METHOD(uint32,BitMap_New)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    struct TagItem *Tags = Msg,item;
    
    while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
    switch (item.ti_Tag)
    {
        case FV_ATTRIBUTE_WIDTH:
        {
            LOD->cpp.Width = item.ti_Data;
        }
        break;
         
        case FV_ATTRIBUTE_HEIGHT:
        {
            LOD->cpp.Height = item.ti_Data;
        }
        break;
        
        case FV_ATTRIBUTE_DEPTH:
        {
            LOD->cpp.Depth = item.ti_Data;
        }
        break;

        case FV_ATTRIBUTE_PIXELSIZE:
        {
            LOD->cpp.PixelSize = item.ti_Data;
        }
        break;

        case FV_ATTRIBUTE_PIXELARRAY:
        {
            LOD->cpp.PixelArray = (APTR) item.ti_Data;
        }
        break;
        
        case FV_ATTRIBUTE_PIXELARRAYMOD:
        {
            LOD->cpp.PixelArrayMod = item.ti_Data;
        }
        break;
        
        case FV_ATTRIBUTE_COLORTYPE:
        {
            LOD->cpp.ColorType = item.ti_Data;
        }
        break;
        
        case FV_ATTRIBUTE_COLORCOUNT:
        {
            LOD->cpp.ColorCount = item.ti_Data;
        }
        break;
        
        case FV_ATTRIBUTE_COLORARRAY:
        {
            LOD->cpp.ColorArray = (uint32 *) item.ti_Data;
        }
        break;
    }

    #ifdef DB_NEW

    IFEELIN F_Log(0,"(%ld x %ld) PixelSize (%ld) Array 0x%08lx Mod (%ld) - ColorType (0x%08lx) ColorArray (0x%08lx)",
        LOD->cpp.Width, LOD->cpp.Height, LOD->cpp.PixelSize, LOD->cpp.PixelArray,
        LOD->cpp.PixelArrayMod, LOD->cpp.ColorType, LOD->cpp.ColorArray);

    #endif

    if (LOD->cpp.Width && LOD->cpp.Height &&
        LOD->cpp.PixelArray && LOD->cpp.PixelArrayMod &&
        LOD->cpp.ColorType)
    {
        return F_SUPERDO();
    }
    return NULL;
}
//+
///BitMap_Dispose
F_METHOD(uint32,BitMap_Dispose)
{
    bitmap_clear(Class, Obj);
 
    return F_SUPERDO();
}
//+
///BitMap_Get
F_METHOD(uint32,BitMap_Get)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    struct TagItem *Tags = Msg,item;

    while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
    switch (item.ti_Tag)
    {
        case FV_ATTRIBUTE_WIDTH:           F_STORE(LOD->cpp.Width); break;
        case FV_ATTRIBUTE_HEIGHT:          F_STORE(LOD->cpp.Height); break;
        case FV_ATTRIBUTE_DEPTH:           F_STORE(LOD->cpp.Depth); break;
        
        case FV_ATTRIBUTE_PIXELSIZE:       F_STORE(LOD->cpp.PixelSize); break;
        case FV_ATTRIBUTE_PIXELARRAY:      F_STORE(LOD->cpp.PixelArray); break;
        case FV_ATTRIBUTE_PIXELARRAYMOD:   F_STORE(LOD->cpp.PixelArrayMod); break;
        
        case FV_ATTRIBUTE_COLORTYPE:       F_STORE(LOD->cpp.ColorType); break;
        case FV_ATTRIBUTE_COLORARRAY:      F_STORE(LOD->cpp.ColorArray); break;
        case FV_ATTRIBUTE_COLORCOUNT:      F_STORE(LOD->cpp.ColorCount); break;
 
        #if 0

        case FV_ATTRIBUTE_Rendered:        F_STORE(LOD->rendered); break;
        case FV_ATTRIBUTE_RenderedType:    F_STORE(0 != (FF_BITMAP_RENDERED_BITMAP & LOD->flags)); break;
        case FV_ATTRIBUTE_RenderedWidth:   F_STORE(LOD->rendered_w); break;
        case FV_ATTRIBUTE_RenderedHeight:  F_STORE(LOD->rendered_h); break;
        #ifdef F_ENABLE_REMAP
        case FV_ATTRIBUTE_Pens:            F_STORE(LOD->pens); break;
        case FV_ATTRIBUTE_NumPens:         F_STORE(LOD->numpens); break;
        #endif

        #endif
    }

    return F_SUPERDO();
}
//+
