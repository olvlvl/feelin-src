/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>

#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_LABEL,
		FV_ATTRIBUTE_IMAGE,
		FV_ATTRIBUTE_ALTIMAGE,
		FV_ATTRIBUTE_PREPARSE,
		FV_ATTRIBUTE_ALTPREPARSE

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	STRPTR                          label;
	STRPTR                          image;
	STRPTR							altimage;
	STRPTR                          preparse;
	STRPTR                          altpreparse;

	FObject                         child;
};
