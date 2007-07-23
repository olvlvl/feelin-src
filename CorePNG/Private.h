/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#if defined(__GNUC__) && !defined(__amigaos4__) && !defined(__MORPHOS__) && !defined(__AROS__)
#define BAD_LIBNIX
#endif

#include <stdio.h>

#include <libraries/feelin.h>
#include <feelin/coremedia.h>

#include <proto/exec.h>
#include <proto/dos.h>

#ifdef BAD_LIBNIX
#include <proto/utility.h>
#endif

#include <proto/feelin.h>

#include <png.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {   // FIXME: should be CoreMedia attribute, I need to define CoreMedia.

		FV_ATTRIBUTE_SOURCE,
		FV_ATTRIBUTE_SOURCE_TYPE

		};

enum    {

		FV_RESOLVED_WIDTH,
		FV_RESOLVED_HEIGHT,
		FV_RESOLVED_DEPTH,
		FV_RESOLVED_PIXELSIZE,
		FV_RESOLVED_PIXELARRAY,
		FV_RESOLVED_PIXELARRAYMOD,
		FV_RESOLVED_COLORCOUNT,
		FV_RESOLVED_COLORARRAY,
		FV_RESOLVED_COLORTYPE

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	BPTR                            handle;

	uint16                          width;
	uint16                          height;
	uint8                           depth;

	uint8                           pixel_size;
	APTR                            pixel_array;
	uint32                          pixel_array_mod;

	uint32                          color_count;
	uint32                         *color_array;
	bits32                          color_type;
};

