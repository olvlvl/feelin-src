/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <proto/exec.h>
#include <proto/layers.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/feelin.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/feelin.h>
#include <feelin/preference.h>

extern struct ClassUserData			CUD;

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

struct ClassUserData
{
	struct Hook						layout_hook;
};

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_PROPERTY_PREPARSE_ACTIVE,
		FV_PROPERTY_PREPARSE_INACTIVE,
		FV_PROPERTY_SIZEBAR_BACKGROUND,
		FV_PROPERTY_SIZEBAR_HEIGHT

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

#define DEF_SPACING_HORIZONTAL                  6
#define DEF_SPACING_VERTICAL                    3
#define DEF_DRAGBAR_BACK                        "<image type='gradient' start='shine' end='halfdark' />"
#define DEF_SIZEBAR_HEIGHT                      6
#define DEF_SIZEBAR_BACK                        "<image type='gradient' start='shine' end='halfdark' />"
#define DEF_PREPARSE_ACTIVE                     NULL
#define DEF_PREPARSE_INACTIVE                   NULL

#define DEF_IMAGE_CLOSE                         "<image type='picture' src='feelin:resources/decorators/flatty/close-render.png' />" F_IMAGEDISPLAY_SEPARATOR "<image type='picture' src='feelin:resources/decorators/flatty/close-select.png' />"
#define DEF_IMAGE_ZOOM                          "<image type='picture' src='feelin:resources/decorators/flatty/zoom-render.png' />" F_IMAGEDISPLAY_SEPARATOR "<image type='picture' src='feelin:resources/decorators/flatty/zoom-select.png' />"
#define DEF_IMAGE_DEPTH                         "<image type='picture' src='feelin:resources/decorators/flatty/depth-render.png' />" F_IMAGEDISPLAY_SEPARATOR "<image type='picture' src='feelin:resources/decorators/flatty/depth-select.png' />"

#define FF_DECO_DRAGBAR             (1 << 0)
#define FF_DECO_SIZEBAR             (1 << 1)
#define FF_DECO_SIZEBAR_ADDED       (1 << 2)
#define FF_DECO_TITLEONLY           (1 << 3)

#define FV_DRAGBAR_MIN_WIDTH        10

struct LocalObjectData
{
	F_MEMBER_AREA_PUBLIC;

	bits8                           flags;
	uint8                           sizebar_h;
	uint16                          need_w;

	FPadding						borders;

	#if 0
	STRPTR                          preparse_active;
	STRPTR                          preparse_inactive;
	#endif

	FObject                         bar;
	FAreaPublic					   *bar_public;
	FObject                         title;
	FAreaPublic                    *title_public;
	FObject                         sizebar_back;

	APTR                            notify_handler_window_active;
	APTR                            notify_handler_window_title;

	FObject                         gadget_close;
	FObject                         gadget_zoom;
	FObject                         gadget_depth;

	APTR                            intuition_gadget_size;
	APTR                            intuition_gadget_drag;
};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

#define _pub_x(pub_)                            LOD->pub_##_public->Box.x
#define _pub_y(pub_)                            LOD->pub_##_public->Box.y
#define _pub_w(pub_)                            LOD->pub_##_public->Box.w
#define _pub_h(pub_)                            LOD->pub_##_public->Box.h
#define _pub_minw(pub_)                         LOD->pub_##_public->MinMax.MinW
#define _pub_minh(pub_)                         LOD->pub_##_public->MinMax.MinH
#define _pub_maxw(pub_)                         LOD->pub_##_public->MinMax.MaxW
#define _pub_maxh(pub_)                         LOD->pub_##_public->MinMax.MaxH
#define _pub_is_damaged(pub_)					((LOD->pub_##_public->Flags & FF_Area_Damaged) != 0)
#define _pub_set_damaged(pub_)					LOD->pub_##_public->Flags |= FF_Area_Damaged

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

F_HOOK(uint32, code_layout);

