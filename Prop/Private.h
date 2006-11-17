/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <intuition/intuition.h>
#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/graphics.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_ENTRIES,
		FV_ATTRIBUTE_VISIBLE,
		FV_ATTRIBUTE_FIRST,
		FV_ATTRIBUTE_STEP,
		FV_ATTRIBUTE_USELESS

		};

enum    {

		FV_METHOD_DECREASE,
		FV_METHOD_INCREASE

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

	uint32                          entries;
	uint32                          visible;
	uint32                          first;
	uint32                          step;

	bits16                          flags;
	uint16                          previous_pos;
	uint32                          mouse_off;
	uint32                          saved;

	FObject                         knob;
	FAreaPublic                    *knob_area_public;
};

#define FF_Prop_Scroll                          (1 << 0)

#define _knob_area_public                       (LOD->knob_area_public)
#define _knob_box                               (_knob_area_public -> Box)
#define _knob_x                                 (_knob_area_public -> Box.x)
#define _knob_y                                 (_knob_area_public -> Box.y)
#define _knob_w                                 (_knob_area_public -> Box.w)
#define _knob_h                                 (_knob_area_public -> Box.h)
#define _knob_minmax                            (_knob_area_public -> MinMax)
#define _knob_minw                              (_knob_minmax.MinW)
#define _knob_minh                              (_knob_minmax.MinH)
#define _knob_maxw                              (_knob_minmax.MaxW)
#define _knob_maxh                              (_knob_minmax.MaxH)
#define _knob_font                              (_knob_area_public -> Font)
#define _knob_set_damaged                       (_knob_area_public->Flags |= FF_Area_Damaged)

enum    {

		FK_PROP_NONE,
		FK_PROP_MORE,
		FK_PROP_LESS,
		FK_PROP_STEP_MORE,
		FK_PROP_STEP_LESS,
		FK_PROP_MIN,
		FK_PROP_MAX

		};

void prop_layout_knob(FClass *Class, FObject Obj, int16 x, int16 y, uint16 w, uint16 h);

