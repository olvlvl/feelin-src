/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/graphics.h>
#include <proto/feelin.h>

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_ACTIVE

		};

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

struct ClassUserData
{
	struct Hook                     minmax_hook;
	struct Hook                     layout_hook;
};

F_HOOK_PROTO(bool32,code_minmax);
F_HOOK_PROTO(bool32,code_layout);

/************************************************************************************************
*** Properties **********************************************************************************
************************************************************************************************/

struct LocalPropertiesData
{
	uint8                           padding_left;
	uint8                           padding_right;
	uint8                           padding_top;
	uint8                           padding_bottom;

	uint8                           active_padding_top;
	uint8                           active_padding_bottom;

	uint8                           inactive_padding_top;
	uint8                           inactive_padding_bottom;
	uint8                           inactive_margin_top;

	STRPTR                          tab_font;
	STRPTR                          active_tab_prep;
	STRPTR                          active_tab_back;
	STRPTR                          inactive_tab_prep;
	STRPTR                          inactive_tab_back;
};

enum    {

		FV_PROPERTY_PADDING_LEFT,
		FV_PROPERTY_PADDING_RIGHT,
		FV_PROPERTY_PADDING_TOP,
		FV_PROPERTY_PADDING_BOTTOM,

		FV_PROPERTY_ACTIVE_PADDING_TOP,
		FV_PROPERTY_ACTIVE_PADDING_BOTTOM,

		FV_PROPERTY_INACTIVE_PADDING_TOP,
		FV_PROPERTY_INACTIVE_PADDING_BOTTOM,
		FV_PROPERTY_INACTIVE_MARGIN_TOP,

		FV_PROPERTY_PREPARSE_ACTIVE,
		FV_PROPERTY_PREPARSE_INACTIVE,

		FV_PROPERTY_TAB_FONT,
		FV_PROPERTY_TAB_BACK_INACTIVE,
		FV_PROPERTY_TAB_BACK_ACTIVE,

		FV_PROPERTY_FONT

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
	F_MEMBER_WIDGET_PUBLIC;

	struct LocalPropertiesData     *style;

	FObject                         TD;
	struct TextFont                *font;

	FWidgetNode                    *node_active;
	FWidgetNode                    *node_update;            // Previous active page

	FObject                         back_active;
	FObject                         back_inactive;
	FBox                            back_origin;

	uint8                           tabs_height;
	uint16                          _pad1;
};


/*
 1111111111111111
 2222222222222222
4    FONT        6
4     YY         6
4    SIZE        6
433333333333333336
455555555555555556
4accccccccccccccb6
4a              b6
4a              b6
4a              b6
4addddddddddddddb6
477777777777777776

*/

#define FV_PAGE_BORDER_TOP                      2           // 5
#define FV_PAGE_BORDER_LEFT                     2           // 4
#define FV_PAGE_BORDER_RIGHT                    2           // 6
#define FV_PAGE_BORDER_BOTTOM                   2           // 7

#define FV_TAB_BORDER_TOP                       2           // 1
#define FV_TAB_PADDING_WIDTH                    10
#define FV_ITAB_BORDER_TOP                      1

#if 0
#define FV_FONT_SIZE                            10
#else
#define FV_FONT_SIZE                            LOD->font->tf_YSize
#endif

/************************************************************************************************
*** Macros **************************************************************************************
************************************************************************************************/

/* direct peeking and poking of FC_Area datas is no  longer  possible  e.g.
_area_w(sub)  is no longer valid. The following macros are very usefull and will
save a *LOT* of typing. */

#define _sub_declare_all                        FWidgetNode *node

#define _sub_area                               node->AreaPublic
#define _sub_area_flags                         _sub_area->Flags
#define _sub_is_damaged                         ((FF_Area_Damaged & _sub_area_flags) != 0)
#define _sub_set_damaged                        _sub_area_flags |= FF_Area_Damaged

#define _sub_box                                (_sub_area->Box)
#define _sub_x                                  _sub_box.x
#define _sub_y                                  _sub_box.y
#define _sub_w                                  _sub_box.w
#define _sub_h                                  _sub_box.h
#define _sub_x2                                 (_sub_x + _sub_w - 1)
#define _sub_y2                                 (_sub_y + _sub_h - 1)

#define _sub_minmax                             (_sub_area->MinMax)
#define _sub_minw                               (_sub_minmax.MinW)
#define _sub_minh                               (_sub_minmax.MinH)
#define _sub_maxw                               (_sub_minmax.MaxW)
#define _sub_maxh                               (_sub_minmax.MaxH)
#define _sub_fixw                               (_sub_minmax.MinW == _sub_minmax.MaxW)
#define _sub_fixh                               (_sub_minmax.MinH == _sub_minmax.MaxH)

#define _sub_widget                             (node->WidgetPublic)
#define _sub_flags                              _sub_widget->Flags
#define _sub_is_group                           ((FF_Widget_Group & _sub_flags) != 0)

#define _each                                   node = (FWidgetNode *) IFEELIN F_Get(Obj, FA_Family_Head) ; node ; node = node->Next

/************************************************************************************************
*** Preferences *********************************************************************************
************************************************************************************************/

#define DEF_PAGE_PADDING_LEFT                   5           // a
#define DEF_PAGE_PADDING_RIGHT                  5           // b
#define DEF_PAGE_PADDING_TOP                    5           // c
#define DEF_PAGE_PADDING_BOTTOM                 5           // d
#define DEF_TAB_PADDING_TOP                     1           // 2
#define DEF_TAB_PADDING_BOTTOM                  1           // 3

#define DEF_ITAB_MARGIN_TOP                     4
#define DEF_ITAB_PADDING_TOP                    1
#define DEF_ITAB_PADDING_BOTTOM                 1
