/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/feelin.h>

#include <libraries/feelin.h>
#include <feelin/preference.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_TITLE

		};

enum    {

		FV_PROPERTY_PREPARSE,
		FV_PROPERTY_COLOR_SHINE,
		FV_PROPERTY_COLOR_SHADOW

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	#ifdef F_NEW_GLOBALCONNECT
	F_MEMBER_ELEMENT_PUBLIC;
	#endif
	F_MEMBER_AREA_PUBLIC;

	bits32                          flags;

	FObject                         TD;
	STRPTR                          Title;
	STRPTR                          PreParse;
	FColor                         *ColorUp;
	FColor                         *ColorDown;
};

#define FF_BAR_VERTICAL                         (1L << 0)
#define _bar_is_vertical                        ((FF_BAR_VERTICAL & LOD->flags) != 0)
#define _bar_set_vertical                       LOD->flags |= FF_BAR_VERTICAL
#define _bar_clear_vertical                     LOD->flags &= ~FF_BAR_VERTICAL

