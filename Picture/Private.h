/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#endif

#include <exec/memory.h>
#include <graphics/gfx.h>
#include <dos/dosextens.h>
#include <datatypes/datatypes.h>
#include <datatypes/pictureclass.h>
#include <intuition/screens.h>
#include <libraries/feelin.h>

#include <feelin/coremedia.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/datatypes.h>
#include <proto/intuition.h>
#include <proto/feelin.h>

#ifdef __amigaos4__
#define IDATATYPES                              IDataTypes->
#else
#define IDATATYPES
#endif

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

        FV_ATTRIBUTE_NAME

        };

enum    {

        FV_RESOLVED_WIDTH,
        FV_RESOLVED_HEIGHT,
        FV_RESOLVED_DEPTH,
        FV_RESOLVED_PIXELSIZE,
        FV_RESOLVED_PIXELARRAY,
        FV_RESOLVED_PIXELARRAYMOD,
        FV_RESOLVED_COLORTYPE,
        FV_RESOLVED_COLORARRAY,
        FV_RESOLVED_COLORCOUNT,
        FV_RESOLVED_STEAL

        };
