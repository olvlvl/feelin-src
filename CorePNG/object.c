#include "Private.h"

#define F_ENABLE_PALETTE
#define F_ENABLE_PALETTE_TRANSPARENCY

//#define DB_LOAD

/*
 
Gofromiel @ Georg : I replaced  AllocVec()  and  FreeVec()  by  F_New()  and
F_Dispose(). Is it ok ? If not explain my why, I like to learn :-)

*/
 
#ifdef __AROS__

void *malloc(size_t size)
{
    return IFEELIN F_New(size);
}

void free(void *mem)
{
    IFEELIN F_Dispose(mem);
}

void *calloc(size_t nmemb, size_t size)
{
    return IFEELIN F_New(nmemb * size);
}

#endif

///my_error_fn
SAVEDS STATIC void my_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
    IFEELIN F_Log(0,"PNG error: %s", error_msg ? error_msg : "");
    /*
    if (png_ptr && png_jmpbuf(png_ptr))
    {
        longjmp(png_jmpbuf(png_ptr), 1);
    }
    */
}
//+
///my_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
SAVEDS STATIC void my_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
    IFEELIN F_Log(0,"PNG warning: %s", warning_msg ? warning_msg : "");
}
//+
///my_read_fn
SAVEDS STATIC void my_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    BPTR file = (BPTR) png_get_io_ptr(png_ptr);
    png_uint_32 count;

    count = IDOS_ Read(file, data, length);
    
    if (count != length)
    {
        png_error(png_ptr, "Read error!");
    }
}
//+

#define HEADER_CHECK_SIZE 8

///core_png_load
STATIC bool32 core_png_load(FClass *Class, FObject Obj,STRPTR Name)
{
    struct LocalObjectData *LOD = F_LOD(Class, Obj);
    
    uint8 fileheader[HEADER_CHECK_SIZE];
    
    png_structp png_ptr=NULL;
    png_infop png_info_ptr=NULL;
    png_infop png_end_info_ptr=NULL;
    png_uint_32 png_width=0;
    png_uint_32 png_height=0;
    int png_bits=0;
    int png_type=0;
    int png_lace=0;
    int png_num_lace_passes;

    LOD->handle = IDOS_ Open(Name, MODE_OLDFILE);

    if (LOD->handle == NULL)
    {
        return FALSE;
    }

    if (IDOS_ Read(LOD->handle, fileheader, sizeof(fileheader)) != sizeof(fileheader))
    {
        return FALSE;
    }

    if (png_sig_cmp(fileheader, 0, sizeof(fileheader)) != 0)
    {
        IFEELIN F_Log(0,"\"%s\" is not a PiNG file",Name);

        return FALSE;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,

        0,               /* error ptr */
        my_error_fn,     /* error function */
        my_warning_fn    /* warning function */

        );

    if (!png_ptr)
    {
        IFEELIN F_Log(0,"Can't create png read struct!");

        return NULL;
    }

    png_info_ptr = png_create_info_struct(png_ptr);

    if (!png_info_ptr)
    {
        IFEELIN F_Log(0,"Can't create png info struct!");
        
        png_destroy_read_struct(&png_ptr, NULL, NULL);

        return NULL;
    }

    png_end_info_ptr = png_create_info_struct(png_ptr);

    if (!png_end_info_ptr)
    {
        IFEELIN F_Log(0,"Can't create png end info struct!");

        png_destroy_read_struct(&png_ptr, &png_info_ptr, NULL);

        return NULL;
    }

    png_set_read_fn(png_ptr, (png_voidp) LOD->handle, my_read_fn);

    png_set_sig_bytes(png_ptr, HEADER_CHECK_SIZE);
/*
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        F_Log(0,"Error!"));
        png_destroy_read_struct(&png_ptr, &png_info_ptr, &png_end_info_ptr);
        if (buffer) FreeVec(buffer);
        PNG_Exit(&png, ERROR_UNKNOWN);
        return FALSE;
    }
*/
    png_read_info(png_ptr, png_info_ptr);

    png_get_IHDR
    (
        png_ptr, png_info_ptr,

        &png_width, &png_height, &png_bits, &png_type, &png_lace, NULL, NULL
    );
    
    #ifdef DB_LOAD

    IFEELIN F_Log(0,"PNG IHDR: Size %ld x %ld  Bits %d  Type %d Lace %d",
        png_width,
        png_height,
        png_bits,
        png_type,
        png_lace);

    #endif

    if (png_bits == 16)
    {
        #ifdef DB_LOAD
        F_Log(0,"png_set_strip_16");
        #endif

        png_set_strip_16(png_ptr);
    }
    else if (png_bits < 8)
    {
        #ifdef DB_LOAD
        F_Log(0,"png_set_packing");
        #endif

        png_set_packing(png_ptr);

        if (png_type == PNG_COLOR_TYPE_GRAY)
        {
            #ifdef DB_LOAD
            F_Log(0,"png_set_gray_1_2_4_to_8");
            #endif

            png_set_gray_1_2_4_to_8(png_ptr);
        }
    }

    LOD->width = png_width;
    LOD->height = png_height;
    LOD->depth = png_bits;

    #ifdef DB_LOAD
    IFEELIN F_Log(0,"width %ld, height %ld, bits %ld, rowbytes %ld, num_palette %ld, num_trans %ld, color_type %ld, channels %ld",
        png_info_ptr->width, png_info_ptr->height, png_bits, png_info_ptr->rowbytes,
        png_info_ptr->num_palette, png_info_ptr->num_trans, png_info_ptr->color_type,
        png_info_ptr->channels);
    #endif
 
    #if 0
    if (png_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        IFEELIN F_Log(0,"png_set_strip_alpha");

        png_set_strip_alpha(png_ptr);
    }
    #endif

    #if 0
    {
        double png_file_gamma;
        double png_screen_gamma = 2.2;

        if (!(png_get_gAMA(png_ptr, png_info_ptr, &png_file_gamma)))
        {
            png_file_gamma = 0.45455;
        }

        png_set_gamma(png_ptr, png_file_gamma, png_screen_gamma);
    }
    #endif

    png_num_lace_passes = png_set_interlace_handling(png_ptr);

    switch (png_type)
    {
        case PNG_COLOR_TYPE_GRAY:
        {
            LOD->pixel_size = sizeof (uint8);
            LOD->color_type = FV_COLOR_TYPE_GREY;
            
            #ifdef DB_LOAD
            IFEELIN F_Log(0,"FV_COLOR_TYPE_GREY");
            #endif
        }
        break;
         
        case PNG_COLOR_TYPE_GRAY_ALPHA:
        {
            LOD->pixel_size = sizeof (uint8) * 2;
            LOD->color_type = FV_COLOR_TYPE_GREY_ALPHA;

            #ifdef DB_LOAD
            IFEELIN F_Log(0,"FV_COLOR_TYPE_GREY_ALPHA");
            #endif
        }
        break;

        case PNG_COLOR_TYPE_PALETTE:
        {
            LOD->pixel_size = sizeof (uint8);
            LOD->color_type = FV_COLOR_TYPE_PALETTE;

            #ifdef DB_LOAD
            IFEELIN F_Log(0,"FV_COLOR_TYPE_PALETTE");
            #endif
        }
        break;
    
        case PNG_COLOR_TYPE_RGB:
        {
            LOD->pixel_size = sizeof (uint8) * 3;
            LOD->color_type = FV_COLOR_TYPE_RGB;

            #ifdef DB_LOAD
            IFEELIN F_Log(0,"FV_COLOR_TYPE_RGB");
            #endif
        }
        break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
        {
            LOD->pixel_size = sizeof (uint8) * 4;
            LOD->color_type = FV_COLOR_TYPE_RGB_ALPHA;

            #ifdef DB_LOAD
            IFEELIN F_Log(0,"FV_COLOR_TYPE_RGB_ALPHA");
            #endif
        }
        break;

        default:
        {
            IFEELIN F_Log(0, "Unknown PNG Color Type! (%ld)", png_type);

            goto __end;
        }
        break;
    }

    png_read_update_info(png_ptr, png_info_ptr);

    #ifdef F_ENABLE_PALETTE

    if ((png_type == PNG_COLOR_TYPE_PALETTE) ||
        (png_type == PNG_COLOR_TYPE_GRAY) ||
        (png_type == PNG_COLOR_TYPE_GRAY_ALPHA))
    {
        png_colorp col = 0;
        int numcolors = 1L << LOD->depth;

        if (png_type == PNG_COLOR_TYPE_PALETTE)
        {
            if (!png_get_PLTE(png_ptr, png_info_ptr, &col, &numcolors))
            {
                IFEELIN F_Log(0, "PLTE chunk missing!"); numcolors = 0;
            }
        }

        if (numcolors)
        {
            LOD->color_count = numcolors;
            LOD->color_array = IFEELIN F_New(sizeof (uint32) * LOD->color_count);
        }

        //IFEELIN F_Log(0,"ColorCount (%ld) ColorArray (0x%08lx)", LOD->color_count, LOD->color_array);

        /* Gray images should in theory not require the following code,  as
        picture.datatype   should  handle  it  automatically  when  we  use
        PBPAFMT_GREY8. But just to be sure ... */

        if (LOD->color_array)
        {
            uint32 i;
            uint32 rgba;
                    
            if (png_type == PNG_COLOR_TYPE_PALETTE)
            {
                for (i = 0 ; i < LOD->color_count ; i++)
                {
                    rgba = col->red;
                    rgba <<= 8;

                    rgba |= col->green;
                    rgba <<= 8;

                    rgba |= col->blue;
                    rgba <<= 8;

                    rgba |= 0xFF;

                    LOD->color_array[i] = rgba;

                    //F_Log(0,"COL (0x%08lx) : RGBA (0x%08lx)",col,rgba);

                    col++;
                }
            }
            else
            {
                for (i = 0 ; i < LOD->color_count ; i++)
                {
                    rgba = i;
                    rgba <<= 8;

                    rgba |= i;
                    rgba <<= 8;

                    rgba |= i;
                    rgba <<= 8;

                    rgba |= 0xFF;

                    LOD->color_array[i] = rgba;
                }
            }
        }
    }
    
    #ifdef F_ENABLE_PALETTE_TRANSPARENCY

    if ((png_type == PNG_COLOR_TYPE_PALETTE) && (LOD->color_array != NULL))
    {
        png_bytep trans;
        int num_trans;
    
        LOD->color_type |= FF_COLOR_TYPE_ALPHA;
 
        if (png_get_tRNS(png_ptr, png_info_ptr, &trans, &num_trans, NULL))
        {
            uint32 i;
            
            num_trans = MIN(num_trans, LOD->color_count);
            
            for (i = 0 ; i < num_trans ; i++, trans++)
            {
                uint32 rgba = LOD->color_array[i];

                rgba >>= 8;
                rgba <<= 8;
                rgba |= *trans;
                
                LOD->color_array[i] = rgba;
            }
        }
    }

    #endif

    #endif

    LOD->pixel_array_mod = png_get_rowbytes(png_ptr, png_info_ptr);
    LOD->pixel_array = IFEELIN F_New(LOD->height * LOD->pixel_array_mod);
 
    if (LOD->pixel_array)
    {
        APTR buf;
        uint32 y;
        
        while (png_num_lace_passes--)
        {
            #ifdef DB_LOAD
            IFEELIN F_Log(0,"PASS %ld",png_num_lace_passes);
            #endif

            for (y = 0, buf = LOD->pixel_array; y < LOD->height; y++, buf = (APTR)((uint32)(buf) + LOD->pixel_array_mod))
            {
                #ifdef DB_LAOD
                IFEELIN F_Log(0,"read row %ld - modulo (%ld)(0x%08lx)",y,modulo, buf);
                #endif

                png_read_row(png_ptr, buf, NULL);
            }
        }
            
        #ifdef DB_LOAD
        IFEELIN F_Log(0,"PixelsArray (0x%08lx) PixelArrayMod (0x%08lx) PixelSize (%ld)",
            LOD->pixel_array, LOD->pixel_array_mod, LOD->pixel_size);
        #endif
    }

__end:
 
    #ifdef DB_LOAD
    {
        STRPTR color_type;

        switch (LOD->color_type)
        {
            case FV_COLOR_TYPE_GREY:            color_type = "GREY"; break;
            case FV_COLOR_TYPE_PALETTE:         color_type = "PALETTE"; break;
            case FV_COLOR_TYPE_RGB:             color_type = "RGB"; break;

            case FV_COLOR_TYPE_GREY_ALPHA:      color_type = "GREY_ALPHA"; break;
            case FV_COLOR_TYPE_PALETTE_ALPHA:   color_type = "PALETTE_ALPHA"; break;
            case FV_COLOR_TYPE_RGB_ALPHA:       color_type = "RGB_ALPHA"; break;

            default:
            {
                color_type = "UNKNOWN (please report)";
            }
        }

        IFEELIN F_Log
        (
            0, "PixelSize (%ld) PixelArray (0x%08lx)(%ld) ColorType (%s) ColorArray (0x%08lx)(%ld)",

            LOD->pixel_size,
            LOD->pixel_array,
            LOD->pixel_array_mod,
            color_type,
            LOD->color_array,
            LOD->color_count
        );
    }
    #endif

    png_read_end(png_ptr, png_end_info_ptr);
    png_destroy_read_struct(&png_ptr, &png_info_ptr, &png_end_info_ptr);
    
    IDOS_ Close(LOD->handle); LOD->handle = NULL;
        
    return (bool32) (LOD->pixel_array != NULL);
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///CorePNG_New
F_METHOD(uint32, CorePNG_New)
{
    struct LocalObjectData *LOD = F_LOD(Class, Obj);
    struct TagItem *Tags=Msg, item;
    
    APTR source = NULL;
    uint32 source_type = 0;
//    int32 source_owner = FALSE;
    
    while (IFEELIN F_DynamicNTI(&Tags, &item, Class))
    switch (item.ti_Tag)
    {
        case FV_ATTRIBUTE_SOURCE: source = (APTR) item.ti_Data; break;
        case FV_ATTRIBUTE_SOURCE_TYPE: source_type = item.ti_Data; break;
    }
    #if 0
    if (source != NULL && source_type == FV_CoreMedia_SourceType_File)
    {
        source = (APTR) IDOS_ Open((STRPTR) source, MODE_OLDFILE);
        source_type = FV_CoreMedia_SourceType_Handle;
        source_owner = TRUE;
    }

    if (source != NULL && source_type == FV_CoreMedia_SourceType_Handle)
    {
        core_png_load(Class, Obj, (BPTR) source);
    }
    
    if (source_owner)
    {
        IDOS_ Close((BPTR) source);
    }
    #endif
    
    if ((source != NULL) && (source_type == FV_CoreMedia_SourceType_File))
    {
        core_png_load(Class, Obj, (STRPTR) source);
    }
    else
    {
        IFEELIN F_Log(0,"Invalid source (0x%08lx) or Type (%ld)",source, source_type);
    }

    if (LOD->pixel_array)
    {
		return IFEELIN F_SuperDo(Class,Obj,Method,

            F_RESOLVED_ID(WIDTH),         LOD->width,
            F_RESOLVED_ID(HEIGHT),        LOD->height,
            F_RESOLVED_ID(DEPTH),         LOD->depth,
            F_RESOLVED_ID(PIXELSIZE),     LOD->pixel_size,
            F_RESOLVED_ID(PIXELARRAY),    LOD->pixel_array,
            F_RESOLVED_ID(PIXELARRAYMOD), LOD->pixel_array_mod,
            F_RESOLVED_ID(COLORCOUNT),    LOD->color_count,
            F_RESOLVED_ID(COLORARRAY),    LOD->color_array,
            F_RESOLVED_ID(COLORTYPE),     LOD->color_type,

            TAG_DONE);
    }

    return NULL;
}
//+
///CorePNG_Dispose
F_METHOD(void, CorePNG_Dispose)
{
    struct LocalObjectData *LOD = F_LOD(Class, Obj);
    
    if (LOD->handle)
    {
        IDOS_ Close(LOD->handle); LOD->handle = NULL;
    }

    F_SUPERDO();
}
//+
