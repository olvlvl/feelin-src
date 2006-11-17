/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <stdlib.h>
#include <stdarg.h>

#include <libraries/feelin.h>
#include <graphics/gfxmacros.h>
#include <feelin/preference.h>

#include <proto/exec.h>
#include <proto/feelin.h>
#include <proto/graphics.h>
#include <proto/utility.h>

#include <feelin/support/f_decode_color.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/
 
enum    {

		FV_ATOM_TOUCH,
		FV_ATOM_FOCUS,
		FV_ATOM_GHOST

		};

enum    {

		// border

		FV_PROPERTY_BORDER,
		
		FV_PROPERTY_BORDER_COLOR,
		FV_PROPERTY_BORDER_STYLE,
		FV_PROPERTY_BORDER_WIDTH,

		FV_PROPERTY_BORDER_TOP,
		FV_PROPERTY_BORDER_TOP_COLOR,
		FV_PROPERTY_BORDER_TOP_STYLE,
		FV_PROPERTY_BORDER_TOP_WIDTH,

		FV_PROPERTY_BORDER_RIGHT,
		FV_PROPERTY_BORDER_RIGHT_COLOR,
		FV_PROPERTY_BORDER_RIGHT_STYLE,
		FV_PROPERTY_BORDER_RIGHT_WIDTH,
		
		FV_PROPERTY_BORDER_BOTTOM,
		FV_PROPERTY_BORDER_BOTTOM_COLOR,
		FV_PROPERTY_BORDER_BOTTOM_STYLE,
		FV_PROPERTY_BORDER_BOTTOM_WIDTH,

		FV_PROPERTY_BORDER_LEFT,
		FV_PROPERTY_BORDER_LEFT_COLOR,
		FV_PROPERTY_BORDER_LEFT_STYLE,
		FV_PROPERTY_BORDER_LEFT_WIDTH,

		// frame

		FV_PROPERTY_BORDER_FRAME,

		// caption

		FV_PROPERTY_CAPTION_POSITION,
		FV_PROPERTY_CAPTION_PREPARSE,
		FV_PROPERTY_CAPTION_FONT,

		/* THE FOLLOWING ARE DEPRECATED */

		FV_PROPERTY_FRAME,
		FV_PROPERTY_FRAME_ID,
		FV_PROPERTY_FRAME_PREPARSE,
		FV_PROPERTY_FRAME_POSITION,
		FV_PROPERTY_FRAME_FONT

		};

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

struct in_Caption
{
	STRPTR                          label;
	STRPTR                          preparse;
	struct TextFont                *font;

	FObject                         display;

	bits8                           position;
	uint16                          border;

	uint16                          width;
	uint16                          height;
};

/* FFrameTitle is allocated only when needed (e.g. the frame has a title),
otherwise, a simple FFrameData is used. */

enum    {

		FV_CAPTION_POSITION_LEFT,
		FV_CAPTION_POSITION_CENTER,
		FV_CAPTION_POSITION_RIGHT

		};

#define FF_CAPTION_POSITION_BOTTOM				(1 << 7)

/************************************************************************************************
*** Properties **********************************************************************************
************************************************************************************************/

struct in_Border
{
	uint32							color;
	uint8							color_type;
	uint8							style;
	uint8                           width;
	uint8							_pad0;
};

enum	{

		FV_BORDER_STYLE_NONE,
		FV_BORDER_STYLE_SOLID,
		FV_BORDER_STYLE_BROKE,
		FV_BORDER_STYLE_ROUND

		};

/*** border-frame ******************************************************************************/

struct in_Frame
{
	uint8							type;

	uint8                          	id;
	uint16							_pad0;

	struct in_Border				border_top;
	struct in_Border				border_right;
	struct in_Border				border_bottom;
	struct in_Border				border_left;
};

/* the following enumeration define the frame type currently handled by  the
class.  The  enumeration  is  hierarchical e.g. if a 'FRAME' type is already
defined, it can only be overrode by a 'IMAGE' type */

enum	{

		FV_FRAME_TYPE_NONE,
		FV_FRAME_TYPE_BORDER,
		FV_FRAME_TYPE_FRAME,
		FV_FRAME_TYPE_IMAGE

		};

/***********************************************************************************************/

struct LocalPropertiesData
{
	struct in_Frame                 frames[FV_AREA_STATE_COUNT];

	STRPTR                          caption_preparse;
	STRPTR                          caption_font;
	int8                            caption_position;
	bool8							caption_position_bottom;
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

	struct LocalPropertiesData	   *properties;
	
	struct in_Frame                *frame;
	struct in_Caption         	   *caption;
};

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

void frame_draw_border(FClass *Class, FObject Obj, int16 x1, int16 y1, int16 x2, int16 y2, FRender *Render, struct Region *ClearRegion);

