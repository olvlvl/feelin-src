/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <intuition/screens.h>
#include <libraries/feelin.h>

#include <feelin/coremedia.h>
#include <feelin/rtg.h>

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/feelin.h>

#ifdef __AROS__
#include <aros/macros.h>
#include <aros/debug.h>
#endif

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_WIDTH,
		FV_ATTRIBUTE_HEIGHT,
		FV_ATTRIBUTE_DEPTH,                    //FIXME: NOT IMPLEMENTED

		FV_ATTRIBUTE_PIXELSIZE,
		FV_ATTRIBUTE_PIXELARRAY,
		FV_ATTRIBUTE_PIXELARRAYMOD,

		FV_ATTRIBUTE_COLORTYPE,
		FV_ATTRIBUTE_COLORCOUNT,
		FV_ATTRIBUTE_COLORARRAY,

		FV_ATTRIBUTE_BLITMODE,
		FV_ATTRIBUTE_BACKGROUND

		#if 0

		,FA_BitMap_Pens,
		FA_BitMap_NumPens

		FA_BitMap_ScaleFilter,

		FA_BitMap_Margins

		#endif

		};

enum    {

		FV_METHOD_BLIT,
		FV_METHOD_CLONE,
		FV_METHOD_STEAL,
		FV_METHOD_RENDER,
		FV_METHOD_ADDPROCESSING,
		FV_METHOD_REMPROCESSING

		};

#define FV_PIXEL_TYPE_BITMAP                    0xFFFFFFFF

/************************************************************************************************
*** Structures **********************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	bits32                          flags;
	FCorePicturePublic              cpp;
	FList                           processing;
};

#define _bitmap_width                           LOD->cpp.Width
#define _bitmap_height                          LOD->cpp.Height
#define _bitmap_depth                           LOD->cpp.Depth

#define _bitmap_pixel_size                      LOD->cpp.PixelSize
#define _bitmap_pixel_array                     LOD->cpp.PixelArray
#define _bitmap_pixel_array_mod                 LOD->cpp.PixelArrayMod

#define _bitmap_color_type                      LOD->cpp.ColorType
#define _bitmap_color_count                     LOD->cpp.ColorCount
#define _bitmap_color_array                     LOD->cpp.ColorArray

/*** Blit **************************************************************************************/

struct FeelinBitMapBlit;

typedef void (*feelin_func_blit)                (struct FeelinBitMapBlit *fbb, APTR source, uint16 source_x, uint16 source_y, uint16 target_x, uint16 target_y, uint16 target_w, uint16 target_h);

struct FeelinBitMapBlit
{
	feelin_func_blit                blit_func;

	uint16                          PixelSize;
	uint16                          ColorType;
	uint32                         *ColorArray;

	uint32							background;

	/* if background is opaque (alpha == 0xFF) the value is used to fill the
	line buffer before copying alpha bitmap information */

	uint32                          Modulo;
	struct RastPort                *RPort;
};

#define FF_BITMAP_RENDERED_BITMAP               (1 << 0)

#if 0
///


#define GSIMUL 4096


/*** Transform *********************************************************************************/

struct FeelinBitMapTransform;

typedef void (*feelin_func_transform)           (struct FeelinBitMapTransform *fbt, uint8 *source, uint32 *target, uint32 pixels);

struct FeelinBitMapTransform
{
	feelin_func_transform           func;
	uint32                         *buffer;
	uint32                         *colors;
};

uint32 bitmap_transform_init(struct FeelinBitMapTransform *fbt, uint8 PixelSize);

/*** Scale *************************************************************************************/

struct FeelinBitMapScale;

typedef void (*feelin_func_scale)               (struct FeelinBitMapScale *fbs, APTR source);
typedef void (*feelin_func_scale_y)             (struct FeelinBitMapScale *fbs, uint32 *source, uint32 *target, uint32 y);
typedef void (*feelin_func_scale_x)             (struct FeelinBitMapScale *fbs, uint32 *source, uint32 *target, uint32 tpy);
typedef uint32 (*feelin_func_get_pixel)         (struct FeelinBitMapScale *fbs, uint32 *s, uint32 x, uint32 y);

struct FeelinBitMapScale
{
	uint16                          source_w;
	uint16                          source_h;
	uint8                           source_pixel_size;
	uint8                           source_color_type;

	uint16                          _pad0;

	uint32                          source_modulo;
	uint32                          source_modulo_real;

	uint16                          target_x;
	uint16                          target_y;
	uint16                          target_w;
	uint16                          target_h;
	uint32                          target_modulo;

	uint32                          xfrac;
	uint32                          yfrac;
	uint32                          spy;

	struct FeelinBitMapTransform    transform;

	feelin_func_scale               scale_func;
	feelin_func_scale_y             scale_y_func;
	feelin_func_scale_x             scale_x_func;
	feelin_func_get_pixel           get_pixel_func;

	APTR                            user_data;
};

void bitmap_scale(struct FeelinBitMapScale *fbs, APTR source, uint16 source_x, uint16 source_y, uint32 filter);

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

void bitmap_render_free(FClass *Class, FObject Obj);

//+
#endif

/*** prototypes ********************************************************************************/

void bitmap_clear(FClass *Class, FObject Obj);

