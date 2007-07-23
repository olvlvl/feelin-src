/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <proto/graphics.h>
#include <proto/feelin.h>

#include <devices/inputevent.h>
#include <libraries/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_QUICKDRAW

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	F_MEMBER_ELEMENT_PUBLIC;
	F_MEMBER_AREA_PUBLIC;
	F_MEMBER_WIDGET_PUBLIC;

	bits32                          flags;
	uint16                          Offset;
	int16                           Mouse;

	uint16                          BNum,ANum;
	uint16                          BMin,AMin;
	uint16                          BMax,AMax;
	uint16                          BSize,ASize;

	FObject                         ActiveObj;
};

#define FF_BALANCE_MOVING                       (1 << 0)
#define FF_BALANCE_VERTICAL                     (1 << 1)
#define FF_BALANCE_QUICKDRAW                    (1 << 2)
#define FF_BALANCE_COUPLE                       (1 << 3)
#define FF_BALANCE_COMPLEXSET                   (1 << 4)

#define _balance_is_moving                      ((LOD->flags & FF_BALANCE_MOVING) != 0)
#define _balance_set_moving                     LOD->flags |= FF_BALANCE_MOVING
#define _balance_clear_moving                   LOD->flags &= ~FF_BALANCE_MOVING

#define _balance_is_vertical                    ((LOD->flags & FF_BALANCE_VERTICAL) != 0)
#define _balance_set_vertical                   LOD->flags |= FF_BALANCE_VERTICAL
#define _balance_clear_vertical                 LOD->flags &= ~FF_BALANCE_VERTICAL

#define _balance_is_quickdraw                   ((LOD->flags & FF_BALANCE_QUICKDRAW) != 0)
#define _balance_set_quickdraw                  LOD->flags |= FF_BALANCE_QUICKDRAW
#define _balance_clear_quickdraw                LOD->flags &= ~FF_BALANCE_QUICKDRAW

#define _balance_is_complexset                  ((LOD->flags & FF_BALANCE_COMPLEXSET) != 0)
#define _balance_set_complexset                 LOD->flags |= FF_BALANCE_COMPLEXSET
#define _balance_clear_complexset               LOD->flags &= ~FF_BALANCE_COMPLEXSET

/* FF_Balance_ComplexSet is set when the  rendering  mode  was  not  set  to
complex, and so have been modified for a better rendering */

#define _sub_next                               (node->Next)
#define _sub_prev                               (node->Prev)
#define _sub_box                                (node->AreaPublic->Box)
#define _sub_x                                  (_sub_box.x)
#define _sub_y                                  (_sub_box.y)
#define _sub_w                                  (_sub_box.w)
#define _sub_h                                  (_sub_box.h)
#define _sub_x2                                 (_sub_box.x + _sub_box.w - 1)
#define _sub_y2                                 (_sub_box.y + _sub_box.h - 1)
#define _sub_minmax                             (node->AreaPublic->MinMax)
#define _sub_minw                               (_sub_minmax.MinW)
#define _sub_minh                               (_sub_minmax.MinH)
#define _sub_maxw                               (_sub_minmax.MaxW)
#define _sub_maxh                               (_sub_minmax.MaxH)
#define _sub_fixw                               (_sub_minmax.MinW == _sub_minmax.MaxW)
#define _sub_fixh                               (_sub_minmax.MinH == _sub_minmax.MaxH)
#define _sub_weight                             (node->WidgetPublic->Weight)

#define _sub_is_compute_w                       ((FF_WIDGETNODE_COMPUTE_W & node->Flags) != 0)
#define _sub_set_compute_w                      node->Flags |= FF_WIDGETNODE_COMPUTE_W
#define _sub_clear_compute_w                    node->Flags &= ~FF_WIDGETNODE_COMPUTE_W

#define _sub_is_compute_h                       ((FF_WIDGETNODE_COMPUTE_H & node->Flags) != 0)
#define _sub_set_compute_h                      node->Flags |= FF_WIDGETNODE_COMPUTE_H
#define _sub_clear_compute_h                    node->Flags &= ~FF_WIDGETNODE_COMPUTE_H

#define _sub_is_damaged                         ((FF_Area_Damaged & node->AreaPublic->Flags) != 0)
#define _sub_set_damaged                        node->AreaPublic->Flags |= FF_Area_Damaged

#define _prev_x2                                (prev->AreaPublic->Box.x + prev->AreaPublic->Box.w - 1)
#define _prev_y2                                (prev->AreaPublic->Box.y + prev->AreaPublic->Box.h - 1)
#define _next_x                                 (next->AreaPublic->Box.x)
#define _next_y                                 (next->AreaPublic->Box.y)

#define _head_x                                 (head->AreaPublic->Box.x)
#define _head_y                                 (head->AreaPublic->Box.y)
#define _tail_x                                 (tail->AreaPublic->Box.x)
#define _tail_y                                 (tail->AreaPublic->Box.y)
#define _tail_x2                                (tail->AreaPublic->Box.x + tail->AreaPublic->Box.w - 1)
#define _tail_y2                                (tail->AreaPublic->Box.y + tail->AreaPublic->Box.h - 1)
#define _tail_set_damaged                       tail->AreaPublic->Flags |= FF_Area_Damaged

#define _each                                   node = head ; node ; node = node->Next
