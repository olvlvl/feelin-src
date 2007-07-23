/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <exec/memory.h>
#include <graphics/text.h>
#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/feelin.h>

#include "_locale/enums.h"

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_PROPERTY_SPACING_HORIZONTAL,
		FV_PROPERTY_SPACING_VERTICAL

		};

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

struct ClassUserData
{
	APTR                            Pool;
	struct Hook                     Hook_CreateFamilyNode;
	struct Hook                     Hook_HLayout;
	struct Hook                     Hook_VLayout;
	struct Hook                     Hook_ALayout;
};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct FeelinGroupVirtualData
{
	uint16                          real_minw;
	uint16                          real_minh;
};

struct LocalObjectData
{
	#ifdef F_NEW_GLOBALCONNECT
	F_MEMBER_ELEMENT_PUBLIC;
	#endif

	F_MEMBER_AREA_PUBLIC;
	F_MEMBER_WIDGET_PUBLIC;

	FObject                         family;

	bits16                          flags;
	uint8                           hspacing;
	uint8                           vspacing;
	int32                           quiet;

	STRPTR                          name;

	struct Hook                    *LayoutHook;
	struct Hook                    *MinMaxHook;
	struct FeelinGroupVirtualData  *virtual_data;       // Only in Virtual Mode

	uint32                          members;            // computed during askminmax;

//  ArrayMode

	uint16                          rows;
	uint16                          columns;
};

#define FF_GROUP_SAMEWIDTH      (1 << 0)
#define FF_GROUP_SAMEHEIGHT     (1 << 1)
#define FF_GROUP_RELSIZING      (1 << 2)
#define FF_GROUP_SAMESIZE       (FF_GROUP_SAMEWIDTH | FF_GROUP_SAMEHEIGHT)
#define FF_GROUP_COLUMNS        (1 << 3)
#define FF_GROUP_USER_HSPACING  (1 << 4)
#define FF_GROUP_USER_VSPACING  (1 << 5)

/* FF_GROUP_COLUMNS is used when the group is in array mode */

#define _group_is_samewidth                 ((FF_GROUP_SAMEWIDTH & LOD->flags) != 0)
#define _group_is_sameheight                ((FF_GROUP_SAMEHEIGHT & LOD->flags) != 0)
#define _group_is_relsizing                 ((FF_GROUP_RELSIZING & LOD->flags) != 0)
#define _group_is_samesize                  ((FF_GROUP_SAMESIZE & LOD->flags) != 0)
#define _group_is_columns					((FF_GROUP_COLUMNS & LOD->flags) != 0)
#define _group_set_columns					LOD->flags |= FF_GROUP_COLUMNS
#define _group_clear_columns				LOD->flags &= ~FF_GROUP_COLUMNS

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

void Group_DoA(FClass *Class,FObject Obj,uint32 Method,APTR Msg);

/*** layout.c ***/

extern F_HOOK_PROTO(void, code_layout_horizontal);
extern F_HOOK_PROTO(void, code_layout_vertical);
extern F_HOOK_PROTO(void, code_layout_array);

/*** area.c ***/

int32 group_is_object_visible(FClass *Class, FObject Obj, FWidgetNode *node);

/************************************************************************************************
*** Macros **************************************************************************************
************************************************************************************************/

#define _sub_declare_all                        FWidgetNode *node

#define _sub_area                               node->AreaPublic
#define _sub_area_flags                         _sub_area->Flags
#define _sub_is_damaged                         ((FF_Area_Damaged & _sub_area_flags) != 0)
#define _sub_isnt_damaged                       ((FF_Area_Damaged & _sub_area_flags) == 0)
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

#define _sub_margin								(_sub_area->Margin)

#define _sub_widget                             (node->WidgetPublic)
#define _sub_flags                              _sub_widget->Flags
#define _sub_is_showable                        ((FF_Widget_Showable & _sub_flags) != 0)
#define _sub_isnt_showable                      ((FF_Widget_Showable & _sub_flags) == 0)
#define _sub_is_group                           ((FF_Widget_Group & _sub_flags) != 0)
#define _sub_isnt_group                         ((FF_Widget_Group & _sub_flags) == 0)
#define _sub_is_drawable                        ((FF_Area_Drawable & node->AreaPublic->Flags) != 0)
#define _sub_isnt_drawable                      ((FF_Area_Drawable & node->AreaPublic->Flags) == 0)

#define _sub_weight                             _sub_widget->Weight
#define _sub_align                              _sub_widget->Align

#define _sub_previous_box                       (node->PreviousBox)
#define _sub_previous_x                         (_sub_previous_box.x)
#define _sub_previous_y                         (_sub_previous_box.y)
#define _sub_previous_w                         (_sub_previous_box.w)
#define _sub_previous_h                         (_sub_previous_box.h)

#define _sub_set_compute_w                      node->Flags |= FF_WIDGETNODE_COMPUTE_W
#define _sub_set_compute_h                      node->Flags |= FF_WIDGETNODE_COMPUTE_H
#define _sub_clear_compute_w                    node->Flags &= ~FF_WIDGETNODE_COMPUTE_W
#define _sub_clear_compute_h                    node->Flags &= ~FF_WIDGETNODE_COMPUTE_H
#define _sub_is_compute_w                       ((FF_WIDGETNODE_COMPUTE_W & node->Flags) != 0)
#define _sub_isnt_compute_w                     ((FF_WIDGETNODE_COMPUTE_W & node->Flags) == 0)
#define _sub_is_compute_h                       ((FF_WIDGETNODE_COMPUTE_H & node->Flags) != 0)
#define _sub_isnt_compute_h                     ((FF_WIDGETNODE_COMPUTE_H & node->Flags) == 0)

/*
#define _sub_visible                            node->WidgetPublic->Visible
#define _sub_visible_x1                         node->WidgetPublic->Visible.x1
#define _sub_visible_y1                         node->WidgetPublic->Visible.y1
#define _sub_visible_x2                         node->WidgetPublic->Visible.x2
#define _sub_visible_y2                         node->WidgetPublic->Visible.y2
#define _sub_visible_w                          _sub_visible_x2 - _sub_visible_x1 + 1
#define _sub_visible_h                          _sub_visible_y2 - _sub_visible_y1 + 1
*/
#define _each                                   node = (FWidgetNode *) IFEELIN F_Get(LOD->family, FA_Family_Head) ; node ; node = node->Next

/*** Preference ********************************************************************************/

#define DEF_GROUP_BACK                          NULL
#define DEF_GROUP_POSITION                      FV_Frame_UpLeft
#define DEF_GROUP_PREPARSE                      "<pens up='shine' shadow='shadow'>"
#define DEF_GROUP_FONT                          NULL
#define DEF_GROUP_FRAME                         "<frame id='15' padding='5' />"

#define DEF_PAGE_PREPARSE                       "<align=center>"
#define DEF_PAGE_ALTPREPARSE                    "<align=center><pens shadow='shadow'><b>"
#define DEF_PAGE_FONT                           NULL
#define DEF_PAGE_BACK                           NULL

#define DEF_VIRTUAL_FRAME                       "<frame id='15' padding='5' />"
#define DEF_PREFERENCE_FRAME                    "<frame id='18' padding='5' />"
#define DEF_GROUP_HSPACING                      5
#define DEF_GROUP_VSPACING                      5

