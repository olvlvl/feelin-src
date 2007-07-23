/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/feelin.h>

#include <feelin/string.h>
#include <feelin/preference.h>

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/feelin.h>

#include "_locale/enums.h"

extern FObject WinServer;

/************************************************************************************************
*** Attribute & Methods *************************************************************************
************************************************************************************************/

enum    {

		FV_ATOM_LEFT,
		FV_ATOM_TOP,
		FV_ATOM_WIDTH,
		FV_ATOM_HEIGHT,
		FV_ATOM_BOX

		};

enum    {

		FV_PROPERTY_RENDER_COMPLEX,
		FV_PROPERTY_REFRESH_SIMPLE,
		FV_PROPERTY_DECORATOR

		};

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

struct in_CycleNode
{
	struct in_CycleNode            *Next;
	struct in_CycleNode            *Prev;

	FObject                         Widget;
	FAreaPublic                    *AreaPublic;
	FWidgetPublic                  *WidgetPublic;
};

#define _chained_is_drawable                ((FF_Area_Drawable & node->AreaPublic->Flags) != 0)
#define _chained_is_disabled                ((FF_Widget_Disabled & node->WidgetPublic->Flags) != 0)
#define _chained_is_not_disabled            ((FF_Widget_Disabled & node->WidgetPublic->Flags) == 0)

struct in_EventHandler
{
	FNode                           node;

	int32                           priority;
	bits32                          events;

	FObject                         target;
	FClass                         *target_class;
};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

/*

	To keep box compatibility with the Area class, Area's margins  are  used
	to  display  Window's  borders.  Thus, all coordinates are valid, except
	_area_x and _area_y wich are always set to zero.

*/

struct LocalObjectData
{
	#ifdef F_NEW_GLOBALCONNECT
	F_MEMBER_ELEMENT_PUBLIC;
	#endif
	F_MEMBER_AREA_PUBLIC;

	struct Window                  *window;
	FBox							window_box;
	FBox                            window_zoom;

	FList                           event_handlers_list;
	FList                           CycleChain;

	FBox                            user_box;
	bits16                          user_box_flags;

	bits8                           sys_flags;
	bits8                           GADFlags;
	bits8                           win_flags;
	
	uint16                          RethinkNest;
	uint16                          _pad0;

	STRPTR                          WinTitle;
	STRPTR                          ScrTitle;

	FObject                         root;
	FAreaPublic                    *root_public;
	
	FObject                         decorator;
	FAreaPublic					   *decorator_area_public;

	FObject                         ActiveObj;
	FObject                         PopHelp;
	STRPTR                          ContextHelp;

	bits32                          events;
	
	struct Screen                  *screen;
	APTR                            nodes_pool;
};

#define FF_WINDOW_BOX_XDEFINED                  (1 << 0)
#define FF_WINDOW_BOX_YDEFINED                  (1 << 1)
#define FF_WINDOW_BOX_WDEFINED                  (1 << 2)
#define FF_WINDOW_BOX_HDEFINED                  (1 << 3)
#define FF_WINDOW_BOX_XPERCENT                  (1 << 4)
#define FF_WINDOW_BOX_YPERCENT                  (1 << 5)
#define FF_WINDOW_BOX_WPERCENT                  (1 << 6)
#define FF_WINDOW_BOX_HPERCENT                  (1 << 7)
#define FF_WINDOW_BOX_RHANDLE                   (1 << 8)
#define FF_WINDOW_BOX_BHANDLE                   (1 << 9)

#define FF_WINDOW_SYS_OPENREQUEST               (1 << 0)
#define FF_WINDOW_SYS_KNOWNMINMAX               (1 << 1)
#define FF_WINDOW_SYS_COMPLEX                   (1 << 2) // Use complex refreshmode when some part of the window need to be redrawn
#define FF_WINDOW_SYS_REFRESH_SIMPLE            (1 << 3)
#define FF_WINDOW_SYS_REFRESH_DONE              (1 << 4)
#define FF_WINDOW_SYS_REFRESH_NEED              (1 << 5)
#define FF_WINDOW_SYS_LAYOUT_EVENT              (1 << 6)
#define FF_WINDOW_SYS_NOTIFY_EVENTS				(1 << 7)

/*

FF_WINDOW_SYS_KNOWNMINMAX

	is used to reduce to number of FM_Area_AskMinMax  invokation.  Only  the
	function  Window_Resizable()  invokes  this  method,  but  the  function
	Window_Resizable() is called 3 times when the window is opened, thus the
	function  Window_Resizable().  This  flag is set the first the method is
	invoked, then if the flag is set  the  method  FM_Area_AskMinMax  is  no
	longer invoked. The flag *MUST* be cleared if another MinMax calculation
	is needed (rethink layout).

FF_WINDOW_SYS_LAYOUT_EVENT

	is used to avoid superfulous drawings  during  layout  operations.  This
	flag is set during FM_Window_DispatchEvent when a FF_EVENT_WINDOW_RESIZE
	event occurs. Because rendering is forbid nest count is  increased  when
	the  event  occurs,  the  flag must be used to decrease FA_Render_Forbid
	when   the   FF_EVENT_WINDOW_REFRESH    occurs.    This    is    because
	FF_EVENT_WINDOW_REFRESH  may  happens at any time if window is in simple
	refresh mode.

FF_WINDOW_SYS_NOTIFY_EVENTS

	if this flag is set,  modifying  events  triggers  the  FA_Window_Events
	attribute.

*/

#define FF_WINDOW_GAD_DRAG                      (1 << 0)
#define FF_WINDOW_GAD_CLOSE                     (1 << 1)
#define FF_WINDOW_GAD_DEPTH                     (1 << 2)
#define FF_WINDOW_GAD_ICONIFY                   (1 << 3)

#define FF_WINDOW_WIN_ACTIVABLE                 (1 << 0)
#define FF_WINDOW_WIN_ACTIVE                    (1 << 1)
#define FF_WINDOW_WIN_RESIZABLE                 (1 << 2)
#define FF_WINDOW_WIN_BORDERLESS                (1 << 3)
#define FF_WINDOW_WIN_BACKDROP                  (1 << 4)

/* FF_WINDOW_WIN_RESIZABLE is set by Window_MinMax() if the root object  is
not  fixed, and if the window is not borderless. The flag is made public by
the FA_Window_Resizable attribute. This attribute is used by decorators  to
know if they have to create size and zoom gadgets */

#define _root_public                            (LOD->root_public)
#define _root_box                               (_root_public->Box)
#define _root_x                                 (_root_box.x)
#define _root_y                                 (_root_box.y)
#define _root_w                                 (_root_box.w)
#define _root_h                                 (_root_box.h)
#define _root_minmax                            (_root_public->MinMax)
#define _root_minw                              (_root_minmax.MinW)
#define _root_minh                              (_root_minmax.MinH)
#define _root_maxw                              (_root_minmax.MaxW)
#define _root_maxh                              (_root_minmax.MaxH)

#define _deco_frame_public						(LOD->decorator_frame_public)
#define _deco_border                           	(_deco_area_public->Border)
#define _deco_bl                                (_deco_border.l)
#define _deco_bt                                (_deco_border.t)
#define _deco_br                                (_deco_border.r)
#define _deco_bb                                (_deco_border.b)
#define _deco_area_public						(LOD->decorator_area_public)
#define _deco_box								(_deco_area_public->Box)
#define _deco_x									(_deco_box.x)
#define _deco_y									(_deco_box.y)
#define _deco_w									(_deco_box.w)
#define _deco_h									(_deco_box.h)

#define _win_x                                  LOD->window_box.x
#define _win_y                                  LOD->window_box.y

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

bits32  window_collect_events   (FClass *Class, FObject Obj);
bool32  window_resizable        (FClass *Class, FObject Obj);
void    window_minmax           (FClass *Class, FObject Obj);
