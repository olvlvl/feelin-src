#include "Private.h"
				
//#define DB_LAYOUT
//#define DB_ASKMINMAX

#define _gad_init(obj)                          FAreaPublic * gad_area_public = F_GET_AREA_PUBLIC(obj)
#define _gad_box                                (gad_area_public->Box)
#define _gad_minmax                             (gad_area_public->MinMax)
#define _gad_x                                  (_gad_box.x)
#define _gad_y                                  (_gad_box.y)
#define _gad_w                                  (_gad_box.w)
#define _gad_h                                  (_gad_box.h)
#define _gad_minw                               (_gad_minmax.MinW)
#define _gad_minh                               (_gad_minmax.MinH)
#define _gad_maxw                               (_gad_minmax.MaxW)
#define _gad_maxh                               (_gad_minmax.MaxH)
#define _gad_set_damaged                        gad_area_public->Flags |= FF_Area_Damaged

///code_layout
F_HOOK(uint32, code_layout)
{
	struct LocalObjectData *LOD = F_LOD(((FClass *) Hook->h_Data), Obj);

/*** size gadgets *******************************************************************************

	Size gadgets are dynamically added and  removed  when  the  window  gets
	resizable and fixed, which may happens at any time.

*/

	if (LOD->intuition_gadget_size != NULL)
	{
		struct Window *win = (struct Window *) IFEELIN F_Get(_area_win, FA_Window_System);

		if (FF_DECO_SIZEBAR & LOD->flags)
		{
			IINTUITION SetAttrs(LOD->intuition_gadget_size,

				GA_Left, _area_cx + 1,
				GA_Top, _area_cy2 + 1,
				GA_Width, _area_cw - 2,
				GA_Height, _area_y2 - _area_cy2 + 1,

				TAG_DONE);

			if (win && ((FF_DECO_SIZEBAR_ADDED & LOD->flags) == 0))
			{
				IINTUITION AddGadget(win, LOD->intuition_gadget_size, -1);

				LOD->flags |= FF_DECO_SIZEBAR_ADDED;
			}
		}
		else
		{
			if (win && ((FF_DECO_SIZEBAR_ADDED & LOD->flags) != 0))
			{
				IINTUITION RemoveGadget(win, LOD->intuition_gadget_size);

				LOD->flags &= ~FF_DECO_SIZEBAR_ADDED;
			}
		}
	}

/*** adjust *************************************************************************************

	The number of gadgets is adjusted to the available space.  If  there  is
	not  enough  space to display the title of the window, the other gadgets
	(close, zoom, depth...) are hidden to make room

*/

	if (LOD->need_w > _area_w)
	{
		if ((FF_DECO_TITLEONLY & LOD->flags) == 0)
		{
			LOD->flags |= FF_DECO_TITLEONLY;

			IFEELIN F_Do(Obj, FM_MultiSet, FA_Widget_Hidden, TRUE, LOD->gadget_close, LOD->gadget_zoom, LOD->gadget_depth, NULL);
		}
	}
	else if (FF_DECO_TITLEONLY & LOD->flags)
	{
		LOD->flags &= ~FF_DECO_TITLEONLY;

		IFEELIN F_Do(Obj, FM_MultiSet, FA_Widget_Hidden, FALSE, LOD->gadget_close, LOD->gadget_zoom, LOD->gadget_depth, NULL);
	}

/*** bar ***************************************************************************************/

	if (LOD->bar_public != NULL)
	{
		int16 x = _area_x + 1;
		int16 y = _area_y + 1;
		uint16 w = _area_w - 2;
		uint16 h = LOD->borders.t - 2;

		if ((_pub_x(bar) != x) || (_pub_y(bar) != y) ||
			(_pub_w(bar) != w) || (_pub_h(bar) != h))
		{
			_pub_x(bar) = x;
			_pub_y(bar) = y;
			_pub_w(bar) = w;
			_pub_h(bar) = h;

			_pub_set_damaged(bar);
		}

		IFEELIN F_Do(LOD->bar, FM_Area_Layout);

		if (LOD->title && LOD->intuition_gadget_drag)
		{
			_gad_init(LOD->title);

			IINTUITION SetAttrs(LOD->intuition_gadget_drag,

				GA_Left,     _gad_x,
				GA_Top,      _gad_y,
				GA_Width,    _gad_w,
				GA_Height,   _gad_h,

				TAG_DONE);
		}
	}

	return TRUE;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Deco_New
F_METHOD(uint32,Deco_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_SAVE_AREA_PUBLIC;

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Area_Fillable, FALSE,
		FA_Widget_Chainable, FALSE,
		FA_Group_LayoutHook, &CUD.layout_hook,

	TAG_MORE, Msg);
}
//+

///Deco_Setup
F_METHODM(bool32,Deco_Setup,FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FObject win = Msg->Render->Window;

	STRPTR value;

	// intuition gadget

	LOD->intuition_gadget_size = IINTUITION NewObject(NULL,"buttongclass",GA_SysGType,GTYP_SIZING,TAG_DONE);

	if (LOD->intuition_gadget_size == NULL)
	{
		return FALSE;
	}

	// does our window have a drag bar ?

	if (IFEELIN F_Get(win, FA_Window_GadDragbar) != FALSE)
	{
		LOD->bar = GroupObject,

			FA_Widget_Chainable, FALSE,
			FA_Element_Class, "decorator-flatty-bar",

			End;

		if (!IFEELIN F_Do(Obj, FM_AddMember, LOD->bar, FV_AddMember_Tail))
		{
			return FALSE;
		}

		LOD->bar_public = F_GET_AREA_PUBLIC(LOD->bar);

		// intuition gadget

		LOD->intuition_gadget_drag = IINTUITION NewObject(NULL, "buttongclass", GA_SysGType, GTYP_WDRAGGING, TAG_DONE);

		if (LOD->intuition_gadget_drag == NULL)
		{
			return FALSE;
		}

		// close gadget

		if (IFEELIN F_Get(win, FA_Window_GadClose) != FALSE)
		{
			LOD->gadget_close = ImageObject,

				FA_Widget_SetMax, FV_Widget_SetBoth,

				FA_Widget_Chainable, FALSE,
				
				#ifdef F_NEW_WIDGET_MODE
				FA_Widget_Mode, FV_Widget_Mode_Button,
				#else
				FA_Widget_Mode, FV_Widget_Mode_Release,
				#endif
				// FA_ContextHelp, "Close Window",

				FA_Element_Class, "decorator-flatty-close",

				End;

			if (LOD->gadget_close)
			{
				if (IFEELIN F_Do(LOD->bar, FM_AddMember, LOD->gadget_close, FV_AddMember_Tail) == FALSE)
				{
					return FALSE;
				}

				IFEELIN F_Do(LOD->gadget_close, FM_Notify,
					
					FA_Widget_Pressed, FALSE,
					win, FM_Set, 2, FA_Window_CloseRequest, TRUE);
			}
			else return FALSE;
		}

		// title

		LOD->title = TextObject,

			FA_Element_Class, "decorator-title",

			FA_Widget_SetMin, FV_Widget_SetHeight,
			
			FA_Widget_Chainable, FALSE,
			
			FA_Text_Contents, IFEELIN F_Get(Msg->Render->Window, FA_Window_Title),
			FA_Text_Shortcut, FALSE,
			FA_Text_VCenter, TRUE,

			End;

		if (IFEELIN F_Do(LOD->bar, FM_AddMember, LOD->title, FV_AddMember_Tail) == FALSE)
		{
			return FALSE;
		}

		LOD->title_public = F_GET_AREA_PUBLIC(LOD->title);

		// zoom gadget

		LOD->gadget_zoom = ImageObject,

			FA_Element_Class, "decorator-flatty-zoom",

			FA_Widget_SetMax, FV_Widget_SetBoth,

			FA_Widget_Chainable, FALSE,
			#ifdef F_NEW_WIDGET_MODE
			FA_Widget_Mode, FV_Widget_Mode_Button,
			#else
			FA_Widget_Mode, FV_Widget_Mode_Release,
			#endif

			//FA_ContextHelp, "Alternate Dimensions",

			End;

		if (LOD->gadget_zoom)
		{
			if (IFEELIN F_Do(LOD->bar,FM_AddMember,LOD->gadget_zoom,FV_AddMember_Tail) == FALSE)
			{
				return FALSE;
			}

			IFEELIN F_Do(LOD->gadget_zoom, FM_Notify,
				
				FA_Widget_Pressed, FALSE,
				win, FM_Window_Zoom, 1, FV_Window_Zoom_Toggle);
		}
		else return FALSE;


		// depth gadget

		if (IFEELIN F_Get(win,FA_Window_GadDepth))
		{
			LOD->gadget_depth = ImageObject,

				FA_Element_Class, "decorator-flatty-depth",

				FA_Widget_SetMax, FV_Widget_SetBoth,

				FA_Widget_Chainable, FALSE,
				#ifdef F_NEW_WIDGET_MODE
				FA_Widget_Mode, FV_Widget_Mode_Button,
				#else
				FA_Widget_Mode, FV_Widget_Mode_Release,
				#endif

				//FA_ContextHelp, "Depth arrange Window",

				End;

			if (LOD->gadget_depth)
			{
				if (IFEELIN F_Do(LOD->bar,FM_AddMember,LOD->gadget_depth,FV_AddMember_Tail) == FALSE)
				{
					return FALSE;
				}

				IFEELIN F_Do(LOD->gadget_depth, FM_Notify,
					
					FA_Widget_Pressed, FALSE,
					win, FM_Window_Depth, 1, FV_Window_Depth_Toggle);
			}
			else return FALSE;
		}
	}

	/* Gadgets must be linked to the decorator BEFORE passing the method to
	the superclass, otherwise children won't be setuped. */

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	#if 0
	if (LOD->title != NULL)
	{
		STRPTR preparse_active = NULL;
		STRPTR preparse_inactive = NULL;

		IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(PREPARSE_ACTIVE), &preparse_active, NULL);
		IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(PREPARSE_INACTIVE), &preparse_inactive, NULL);

		IFEELIN F_Do
		(
			LOD->title, FM_Set,

			FA_Text_PreParse, preparse_inactive,
			FA_Text_AltPreParse, preparse_active,

			TAG_DONE
		);
	}
	#endif

	value = "fill";

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(SIZEBAR_BACKGROUND), &value, NULL);

	#warning FIXME: utiliser un enfant

	LOD->sizebar_back = ImageDisplayObject,

		FA_ImageDisplay_Spec, value,

		End;

	if (IFEELIN F_Do(LOD->sizebar_back, FM_ImageDisplay_Setup, _area_render) == FALSE)
	{
		return FALSE;
	}
		
	value = (STRPTR) DEF_SIZEBAR_HEIGHT;

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(SIZEBAR_HEIGHT), NULL, &value);

	LOD->sizebar_h = (uint32) value;

	LOD->notify_handler_window_active = (FNotifyHandler *) IFEELIN F_Do(_area_win, FM_Notify, FA_Window_Active, FV_Notify_Always, Obj, FM_Set, 2, FA_Widget_Active, FV_Notify_Value);
	LOD->notify_handler_window_title  = (FNotifyHandler *) IFEELIN F_Do(_area_win, FM_Notify, FA_Window_Title, FV_Notify_Always, LOD->title, FM_Set,2, FA_Text_Contents, FV_Notify_Value);

	return TRUE;
}
//+
///Deco_Cleanup
F_METHOD(uint32,Deco_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FFamilyNode *last = NULL;
	FFamilyNode *node;

	if (LOD->intuition_gadget_size != NULL)
	{
		IINTUITION DisposeObject(LOD->intuition_gadget_size);

		LOD->intuition_gadget_size = NULL;
	}

	if (LOD->intuition_gadget_drag != NULL)
	{
		IINTUITION DisposeObject(LOD->intuition_gadget_drag);

		LOD->intuition_gadget_drag = NULL;
	}

	if (_area_render != NULL)
	{
		if (LOD->sizebar_back != NULL)
		{
			IFEELIN F_Do(LOD->sizebar_back, FM_ImageDisplay_Cleanup, _area_render);
		
			IFEELIN F_DisposeObj(LOD->sizebar_back);

			LOD->sizebar_back = NULL;
		}

		IFEELIN F_Do(_area_win,FM_UnNotify,LOD->notify_handler_window_active); LOD->notify_handler_window_active = NULL;
		IFEELIN F_Do(_area_win,FM_UnNotify,LOD->notify_handler_window_title);  LOD->notify_handler_window_title  = NULL;
	}

	F_SUPERDO();

	LOD->bar_public = NULL;
	LOD->title_public = NULL;

	#if 0
	LOD->preparse_active = NULL;
	LOD->preparse_inactive = NULL;
	#endif

	while ((node = (FFamilyNode *) IFEELIN F_Get(Obj,FA_Family_Tail)) != NULL)
	{
		if (node == last)
		{
			IFEELIN F_Log(FV_LOG_DEV, "UNABLE TO REMOVE OBJECT (0x%08lx) - LIST IS ABANDONNED", node->Object);

			break;
		}

		last = node;

		IFEELIN F_DisposeObj(node->Object);
	}

	return 0;
}
//+

///Deco_AskMinMax
F_METHOD(void, Deco_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (IFEELIN F_Get(_area_win, FA_Window_Resizable) != FALSE)
	{
		LOD->flags |= FF_DECO_SIZEBAR;

		#ifdef DB_ASKMINMAX
		IFEELIN F_Log(0,"window is resizable");
		#endif

		if ((LOD->flags & FF_DECO_TITLEONLY) == 0)
		{
			//IFEELIN F_Log(0, "show zoom gadget");
			IFEELIN F_Set(LOD->gadget_zoom, FA_Widget_Hidden, FF_Hidden_Check | FALSE);
		}
	}
	else
	{
		LOD->flags &= ~FF_DECO_SIZEBAR;

		#ifdef DB_ASKMINMAX
		IFEELIN F_Log(0,"window is NOT resizable");
		#endif

		if ((LOD->flags & FF_DECO_TITLEONLY) == 0)
		{
			//IFEELIN F_Log(0, "hide zoom gadget");
			IFEELIN F_Set(LOD->gadget_zoom, FA_Widget_Hidden, FF_Hidden_Check | TRUE);
		}
	}

	F_SUPERDO();
	
	_area_maxw = FV_Area_Max;
	_area_maxh = FV_Area_Max;

	// is the window resizable ?

	// height of our size bar

	LOD->sizebar_h = MAX(LOD->sizebar_h, DEF_SIZEBAR_HEIGHT);

	// needed width to display the whole title bar

	LOD->need_w = 0;

	if (LOD->bar != NULL)
	{
		uint32 space = IFEELIN F_Get(LOD->bar, FA_Group_HSpacing);

		if (LOD->title)
		{
			LOD->need_w += _pub_minw(title) + _area_font->tf_XSize * 3;
		}

		if (LOD->gadget_close)
		{
			_gad_init(LOD->gadget_close);

			LOD->need_w += space + _gad_minw;
		}
	 
		if (LOD->gadget_zoom)
		{
			_gad_init(LOD->gadget_zoom);

			LOD->need_w += space + _gad_minw;
		}

		if (LOD->gadget_depth)
		{
			_gad_init(LOD->gadget_depth);

			LOD->need_w += space + _gad_minw;
		}

		#ifdef DB_ASKMINMAX
		IFEELIN F_Log(0,"title bar width (%ld)", LOD->need_w);
		#endif
	}

	/* paddings are used as borders here */

	LOD->borders.l = 1; // left
	LOD->borders.r = 1; // and right borders size

	// window's title bar

	if (LOD->bar != NULL)
	{
		_gad_init(LOD->bar);

		#ifdef DB_ASKMINMAX
		IFEELIN F_Log(0,"bar: min (%ld x %ld) max (%ld x %ld)", _gad_minw, _gad_minh, _gad_maxw, _gad_maxh);
		#endif

		LOD->borders.t = 2 + _gad_minh;
	}
	else
	{
		LOD->borders.t = 1;
	}

	if ((FF_DECO_SIZEBAR & LOD->flags) != 0)
	{
		LOD->borders.b = 2 + LOD->sizebar_h;
	}
	else
	{
		LOD->borders.b = 1;
	}

	_area_border.t = LOD->borders.t;
	_area_border.r = LOD->borders.r;
	_area_border.b = LOD->borders.b;
	_area_border.l = LOD->borders.l;

	#ifdef DB_ASKMINMAX
	IFEELIN F_Log(0, "borders (0x%08lx): %ld %ld %ld %ld", &LOD->borders, LOD->borders.t, LOD->borders.r, LOD->borders.b, LOD->borders.l);
	#endif
}
//+
///Deco_Show
F_METHOD(uint32,Deco_Show)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render != NULL)
	{
		struct Window *win;
		uint32 active;
		uint32 resizable;

		IFEELIN F_Do
		(
			_area_win, FM_Get,

			FA_Window_System, &win,
			FA_Window_Active, &active,
			FA_Window_Resizable, &resizable,

			TAG_DONE
		);

		IFEELIN F_Set(Obj, FA_Widget_Active, active);

		if (win)
		{
			if (LOD->intuition_gadget_size)
			{
				if (resizable && ((FF_DECO_SIZEBAR_ADDED & LOD->flags) == 0))
				{
					IINTUITION AddGadget(win, LOD->intuition_gadget_size, -1);

					LOD->flags |= FF_DECO_SIZEBAR_ADDED;
				}
			}

			if (LOD->intuition_gadget_drag)
			{
				IINTUITION AddGadget(win, LOD->intuition_gadget_drag, -1);
			}

			if (LOD->gadget_close)
			{
				win->Flags |= WFLG_CLOSEGADGET;
			}
			if (LOD->gadget_zoom)
			{
				win->Flags |= WFLG_HASZOOM;
			}
		}
	}

	return F_SUPERDO();
}
//+
///Deco_Hide
F_METHOD(uint32,Deco_Hide)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render)
	{
		struct Window *win = (struct Window *) IFEELIN F_Get(_area_win, FA_Window_System);

		if (LOD->intuition_gadget_size)
		{
			if (FF_DECO_SIZEBAR_ADDED & LOD->flags)
			{
				IINTUITION RemoveGadget(win, LOD->intuition_gadget_size);

				LOD->flags &= ~FF_DECO_SIZEBAR_ADDED;
			}
		}

		if (LOD->intuition_gadget_drag)
		{
			IINTUITION RemoveGadget(win, LOD->intuition_gadget_drag);
		}
	}

	return F_SUPERDO();
}
//+
///Deco_SetState
F_METHODM(uint32, Deco_SetState, FS_Area_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	_area_border.l = LOD->borders.l;
	_area_border.r = LOD->borders.r;
	_area_border.t = LOD->borders.t;
	_area_border.b = LOD->borders.b;
/*
	IFEELIN F_Set(_area_render, FA_Render_Forbid, TRUE);

	if (Msg->State == FV_Area_State_Focus)
	{
		IFEELIN F_Set(LOD->title, FA_Text_PreParse, (uint32) "<b>");
	}
	else if (Msg->State == FV_Area_State_Neutral)
	{
		IFEELIN F_Set(LOD->title, FA_Text_PreParse, (uint32) "<color=red>");
	}

	IFEELIN F_Set(_area_render, FA_Render_Forbid, FALSE);
*/
	#if 1

	F_OBJDO(LOD->bar);
	F_OBJDO(LOD->title);

	if (Msg->State == FV_Area_State_Neutral)
	{
		IFEELIN F_Do(LOD->gadget_close, Method, FV_Area_State_Ghost);
		IFEELIN F_Do(LOD->gadget_zoom, Method, FV_Area_State_Ghost);
		IFEELIN F_Do(LOD->gadget_depth, Method, FV_Area_State_Ghost);
	}
	else if (Msg->State == FV_Area_State_Focus)
	{
		IFEELIN F_Do(LOD->gadget_close, Method, FV_Area_State_Neutral);
		IFEELIN F_Do(LOD->gadget_zoom, Method, FV_Area_State_Neutral);
		IFEELIN F_Do(LOD->gadget_depth, Method, FV_Area_State_Neutral);
	}

	#endif

	return TRUE;
}
//+
///Deco_Draw
F_METHODM(void,Deco_Draw,FS_Area_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct RastPort *rp = _area_rp;

	int16 x1 = _area_x;
	int16 y1 = _area_y;
	int16 x2 = _area_x2;
	int16 y2 = _area_y2;
	
	int16 ix1 = _area_cx - 1;
	int16 ix2 = _area_cx2 + 1;
	int16 iy1 = _area_cy - 1;
	int16 iy2 = _area_cy2 + 1;

	_APen(_area_pens[FV_Pen_Dark]);
	_Move(x1, y1); _Draw(x1, y2); _Draw(x2, y2); _Draw(x2, y1); _Draw(x1, y1);
	_Move(ix1, iy1); _Draw(ix2, iy1);
	_Move(ix1, iy2); _Draw(ix2, iy2);

	F_SUPERDO();

	if ((FF_DECO_SIZEBAR & LOD->flags) != 0)
	{
		FRect r;

		r.x1 = ix1 + 1;
		r.y1 = iy2 + 1;
		r.x2 = x2 - 1;
		r.y2 = y2 - 1;

		_area_render->Palette = _area_palette;

		IFEELIN F_Do(LOD->sizebar_back, FM_ImageDisplay_Draw, _area_render, &r, 0);
	}
}
//+

