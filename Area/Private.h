/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <proto/exec.h>
#include <proto/feelin.h>
#include <proto/graphics.h>
#include <proto/utility.h>

#include <intuition/screens.h>
#include <libraries/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATOM_TOUCH,
		FV_ATOM_FOCUS,
		FV_ATOM_GHOST

		};

enum    {

		FV_PROPERTY_WIDTH,
		FV_PROPERTY_HEIGHT,

		FV_PROPERTY_MIN_WIDTH,
		FV_PROPERTY_MIN_HEIGHT,
		FV_PROPERTY_MAX_WIDTH,
		FV_PROPERTY_MAX_HEIGHT,

		FV_PROPERTY_MARGIN,
		FV_PROPERTY_MARGIN_LEFT,
		FV_PROPERTY_MARGIN_RIGHT,
		FV_PROPERTY_MARGIN_TOP,
		FV_PROPERTY_MARGIN_BOTTOM,

		FV_PROPERTY_PADDING,
		FV_PROPERTY_PADDING_LEFT,
		FV_PROPERTY_PADDING_RIGHT,
		FV_PROPERTY_PADDING_TOP,
		FV_PROPERTY_PADDING_BOTTOM,

		FV_PROPERTY_FONT,
		FV_PROPERTY_BACKGROUND,
		FV_PROPERTY_COLOR_SCHEME,

		FV_PROPERTY_COLOR,

		FV_PROPERTY_PALETTE,
		FV_PROPERTY_PALETTE_FILL,
		FV_PROPERTY_PALETTE_SHINE,
		FV_PROPERTY_PALETTE_DARK,
		FV_PROPERTY_PALETTE_HIGHLIGHT,
		FV_PROPERTY_PALETTE_CONTRAST,
		FV_PROPERTY_PALETTE_LUMINANCE,
		FV_PROPERTY_PALETTE_SATURATION

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	#ifdef F_NEW_GLOBALCONNECT
	F_MEMBER_ELEMENT_PUBLIC;
	#endif

	FAreaPublic                     public;

	bits32                          flags;

	FPalette                       *palette_neutral;
	FPalette                       *palette_focus;
	FPalette                       *palette_touch;
	FPalette                       *palette_ghost;

	FObject							background;
	FObject							background_inherited;

	struct LocalPropertiesData	   *state;
	struct LocalPropertiesData	   *properties;

	struct LocalObjectData		   *parent_lod;
};

#define FF_AREA_INHERITED_PALETTE               (1 <<  4)
#define FF_AREA_INHERITED_PALETTE_FOCUS         (1 <<  5)
#define FF_AREA_INHERITED_PALETTE_TOUCH         (1 <<  6)
#define FF_AREA_INHERITED_PALETTE_GHOST         (1 <<  7)

/* because the FAreaPublic is embeded in our local object data, we  need  to
override  the  _area_public  macro  in order to use the beautiful and useful
macros define by the Area class */

#undef _area_public
#define _area_public                LOD->public.

/************************************************************************************************
*** Properties **********************************************************************************
************************************************************************************************/

struct in_ColorProperty
{
	uint32							value;
	bits32							type;
};

enum	{

		FV_PALETTE_FILL,
		FV_PALETTE_SHINE,
		FV_PALETTE_DARK,
		FV_PALETTE_TEXT,
		FV_PALETTE_HIGHLIGHT,

		FV_PALETTE_COUNT

		};

struct in_PaletteProperty
{
	bits32							flags;

	struct in_ColorProperty			entries[FV_PALETTE_COUNT];

	int32							contrast;
	int32							luminance;
	uint32							saturation;
};

#define FF_PALETTE_HAS_FILL			(1 << FV_PALETTE_FILL)
#define FF_PALETTE_HAS_SHINE		(1 << FV_PALETTE_SHINE)
#define FF_PALETTE_HAS_DARK			(1 << FV_PALETTE_DARK)
#define FF_PALETTE_HAS_TEXT			(1 << FV_PALETTE_TEXT)
#define FF_PALETTE_HAS_HIGHLIGHT	(1 << FV_PALETTE_HIGHLIGHT)

#define FF_PALETTE_HAS_CONTRAST		(1 << (FV_PALETTE_COUNT + 1))
#define FF_PALETTE_HAS_LUMINANCE    (1 << (FV_PALETTE_COUNT + 2))
#define FF_PALETTE_HAS_SATURATION 	(1 << (FV_PALETTE_COUNT + 3))

struct LocalPropertiesData
{
	bits32							flags;

	uint16							width;
	uint16							height;

	FMinMax							minmax;

	FPadding						margin;
	FPadding						padding;

	STRPTR							font;

	struct in_PaletteProperty		palette;

	STRPTR							background;
};

#define FF_AREA_DEFINED_W			(1 << 0)
#define FF_AREA_DEFINED_H			(1 << 1)
#define FF_AREA_DEFINED_MIN_W		(1 << 2)
#define FF_AREA_DEFINED_MIN_H		(1 << 3)
#define FF_AREA_DEFINED_MAX_W		(1 << 4)
#define FF_AREA_DEFINED_MAX_H		(1 << 5)
#define FF_AREA_AUTO_W				(1 << 6)
#define FF_AREA_AUTO_H				(1 << 7)

#define DEF_AREA_BACKGROUND_GHOST				"fill"

bool32 area_set_state(FClass *Class, FObject Obj);

