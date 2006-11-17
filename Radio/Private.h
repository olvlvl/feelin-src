/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>

#include <proto/feelin.h>

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_ACTIVE,
		FV_ATTRIBUTE_PREPARSE,
		FV_ATTRIBUTE_ALTPREPARSE,
		FV_ATTRIBUTE_LAYOUT

		};

enum	{

		FV_METHOD_ACTIVATE

		};

enum    {

		FV_Radio_Layout_None,
		FV_Radio_Layout_Above,                  // the image is placed above the label (or object)
		FV_Radio_Layout_Below                   // the image is placed below the label (or object)

		};

enum    {

		FV_AUTO_LABEL,
		FV_AUTO_IMAGE,
		FV_AUTO_ALTIMAGE,
		FV_AUTO_PREPARSE,
		FV_AUTO_ALTPREPARSE

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct in_Item
{
	struct in_Item                 *next;
	struct in_Item                 *prev;

	FObject                         radioitem;
	FObject                         radioitem_child;

	FObject                         image;
	FObject                         label;

	uint32                          position;
};

struct LocalObjectData
{
	#ifdef F_NEW_GLOBALCONNECT
	F_MEMBER_ELEMENT_PUBLIC;
	#endif
	FList                           items_list;

	struct in_Item                 *active;

	uint8                           rows;
	uint16                          count;
	uint8                           layout;

	STRPTR                          preparse_render;
	STRPTR                          preparse_select;
};

/************************************************************************************************
*** Preferences *********************************************************************************
************************************************************************************************/

#define DEF_RADIO_HSPACING                      5
#define DEF_RADIO_HSPACING                      5
