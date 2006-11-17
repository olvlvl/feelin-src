/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <string.h>

#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/exec.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_AUTO_FINDHANDLE,
		FV_AUTO_CREATEHANDLE,

		FV_AUTO_OBTAINSTYLE,
		FV_AUTO_RELEASESTYLE,
		FV_AUTO_OBTAINSTYLEBYNAME

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	STRPTR                          id;
	STRPTR                          persist;
	STRPTR                          style;
	STRPTR                          styleclass;

	FElementPublic					public;

	FRender 					   *render;
	FObject                         preferences;

	FPreferenceStyle               *composed_style;

	#ifndef F_NEW_STYLES_EXTENDED

	FPreferenceProperty           **properties;
	FPreferenceStyle               *class_style;

	#endif
};

#undef _element_public
#define _element_public				LOD->public.
