/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <intuition/intuition.h>
#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_RESOLVED_RESET,
		FV_RESOLVED_STRINGIFY,
		FV_RESOLVED_VALUE,
		FV_RESOLVED_MIN,
		FV_RESOLVED_MAX,
		FV_RESOLVED_BUFFER,
		FV_RESOLVED_STRINGARRAY

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

	FObject                        *knob;
	FAreaPublic                    *knob_area_public;

	bits32                          flags;

	int16                           kx;
	int16                           ky;
	uint16                          kw;
	uint16                          kh;

	int16                           previous_pos;
	uint16                          mouse_off;
	uint32                          saved;
	int32                           last_layout_value;
	int32                           last_drawn_value;

	FRender                        *own_render;
	struct RastPort                *own_rport;
	struct BitMap                  *own_bitmap;
};

#define FF_Slider_Scroll            (1 << 0)
#define FF_Slider_Buffer            (1 << 1)

#define FF_Draw_MoveKnob            FF_Draw_Custom_1

#define _knob_area_public                       (LOD->knob_area_public)
#define _knob_box                               (_knob_area_public -> Box)
#define _knob_x                                 (_knob_box.x)
#define _knob_y                                 (_knob_box.y)
#define _knob_w                                 (_knob_box.w)
#define _knob_h                                 (_knob_box.h)
#define _knob_minmax                            (_knob_area_public -> MinMax)
#define _knob_minw                              (_knob_minmax.MinW)
#define _knob_minh                              (_knob_minmax.MinH)
#define _knob_maxw                              (_knob_minmax.MaxW)
#define _knob_maxh                              (_knob_minmax.MaxH)
#define _knob_font                              (_knob_area_public -> Font)

#define _slider_is_scroll                       ((FF_Slider_Scroll & LOD->flags) != 0)
#define _slider_set_scroll                      LOD->flags |= FF_Slider_Scroll
#define _slider_clear_scroll                    LOD->flags &= ~FF_Slider_Scroll

void slider_cache_update(FClass *Class,FObject Obj);
//void Slider_KnobState(FClass *Class,FObject Obj,uint32 State);

