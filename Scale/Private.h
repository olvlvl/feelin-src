/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>

#include <proto/graphics.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	#ifdef F_NEW_GLOBALCONNECT
	F_MEMBER_ELEMENT_PUBLIC;
	#endif
	F_MEMBER_AREA_PUBLIC;
	int16                           Height;
	int16                           MinWidth;
};
