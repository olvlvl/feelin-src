/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <graphics/text.h>
#include <libraries/feelin.h>

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Properties **********************************************************************************
************************************************************************************************/

enum	{

		FV_ATOM_TOUCH,
		FV_ATOM_FOCUS,
		FV_ATOM_GHOST

		};

enum	{

		FV_PROPERTY_PREPARSE,
		FV_PROPERTY_TEXT_POSITION

		};

struct LocalPropertiesData
{
	STRPTR							preparse;

	uint16							position_x;
	bits32							position_x_type;
	uint16							position_y;
	bits32							position_y_type;
};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	F_MEMBER_AREA_PUBLIC;
	F_MEMBER_WIDGET_PUBLIC;

	STRPTR                          text;
	STRPTR							preparse;
	STRPTR							preparse_user;
	
	FObject                         TextDisplay;

	bits32                          Flags;

	struct LocalPropertiesData	   *properties;
};

#define FF_Text_HCenter                         (1 << 0)
#define FF_Text_VCenter                         (1 << 1)
#define FF_Text_Static                          (1 << 2)
#define FF_Text_Shortcut                        (1 << 3)

