/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <proto/exec.h>
#include <proto/feelin.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include <libraries/feelin.h>
#include <feelin/preference.h>
#include <graphics/gfxmacros.h>

enum    {

		FV_ATOM_SELECTED

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

/* all reference to _area_render check (_area_render) from being NULL before accessing
to any field */

struct LocalObjectData
{
	#ifdef F_NEW_GLOBALCONNECT
	F_MEMBER_ELEMENT_PUBLIC;
	#endif

	F_MEMBER_AREA_PUBLIC;

	FWidgetPublic                   public;

	bits16                          flags;

	uint8                           mode;
	uint8                           accel;
	APTR                            handler;
	bits32                          handler_events;

	int16                           hidden_count;
	int16                           disabled_count;

	STRPTR                          context_help;
	FObject                         context_menu;
};

/* PRIVATE FLAGS */

#define FF_WIDGET_CHAINABLE                     (1 << 0)
#define FF_WIDGET_DRAGGABLE                     (1 << 1)
#define FF_WIDGET_DROPABLE                      (1 << 2)
#define FF_WIDGET_DRAGGING                      (1 << 3)
#define FF_WIDGET_BUTTON						(1 << 4)

#undef _widget_public
#define _widget_public                          LOD->public.

#define _widget_set_selected                    _widget_flags |= FF_Widget_Selected
#define _widget_set_pressed                     _widget_flags |= FF_Widget_Pressed
#define _widget_set_active                      _widget_flags |= FF_Widget_Active
#define _widget_set_disabled                    _widget_flags |= FF_Widget_Disabled
#define _widget_set_showable                    _widget_flags |= FF_Widget_Showable
#define _widget_set_group                       _widget_flags |= FF_Widget_Group
#define _widget_clear_selected                  _widget_flags &= ~FF_Widget_Selected
#define _widget_clear_pressed                   _widget_flags &= ~FF_Widget_Pressed
#define _widget_clear_active                    _widget_flags &= ~FF_Widget_Active
#define _widget_clear_disabled                  _widget_flags &= ~FF_Widget_Disabled
#define _widget_clear_showable                  _widget_flags &= ~FF_Widget_Showable
#define _widget_clear_group                     _widget_flags &= ~FF_Widget_Group

#define _widget_is_chainable                    ((FF_WIDGET_CHAINABLE & LOD->flags) != 0)
#define _widget_set_chainable                   LOD->flags |= FF_WIDGET_CHAINABLE
#define _widget_clear_chainable                 LOD->flags &= ~FF_WIDGET_CHAINABLE

#define _widget_is_draggable                    ((FF_WIDGET_DRAGGABLE & LOD->flags) != 0)
#define _widget_set_draggable                   LOD->flags |= FF_WIDGET_DRAGGABLE
#define _widget_clear_draggable                 LOD->flags &= ~FF_WIDGET_DRAGGABLE

#define _widget_is_dropable                     ((FF_WIDGET_DROPABLE & LOD->flags) != 0)
#define _widget_set_dropable                    LOD->flags |= FF_WIDGET_DROPABLE
#define _widget_clear_dropable                  LOD->flags &= ~FF_WIDGET_DROPABLE

#define _widget_is_button						((FF_WIDGET_BUTTON & LOD->flags) != 0)
#define _widget_set_button						LOD->flags |= FF_WIDGET_BUTTON
#define _widget_clear_button					LOD->flags &= ~FF_WIDGET_BUTTON

/* FF_WIDGET_BUTTON is set when the input mode is 'Release' and  the  widget
is  pressed.  The  flag is cleared when the widget is released. The flags is
used to filter events */

void widget_try_add_handler(FClass *Class,FObject Obj);
void widget_try_rem_handler(FClass *Class,FObject Obj);
void widget_select_state(FClass *Class, FObject Obj);

