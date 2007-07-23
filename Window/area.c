#include "Private.h"

//#define DB_LAYOUT
//#define DB_DRAW
//#define DB_ASKMINMAX
//#define F_ENABLE_BUFFERING

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///window_resizable

/* This function is used to know if the window is resizable. If  the  window
is  resizable the flag FF_WINDOW_WIN_RESIZABLE is set to 'win_flags'. If the
minmax   of   the    root    object    has    not    been    computed    yet
(FF_WINDOW_SYS_KNOWNMINMAX  cleared),  the FM_AskMinMax method is invoked on
the root object and the flags FF_WINDOW_SYS_KNOWNMINMAX is set to 'sys_flags'
*/

bool32 window_resizable(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (_root_public != NULL)
	{
		if ((FF_WINDOW_SYS_KNOWNMINMAX & LOD->sys_flags) == 0)
		{
			_root_minw = 0;
			_root_minh = 0;
			_root_maxw = FV_Area_Max;
			_root_maxh = FV_Area_Max;

			IFEELIN F_Do(LOD->root, FM_Area_AskMinMax);

			#ifdef DB_RESIZABLE
			IFEELIN F_Log(0,"MIN %ld x %ld - MAX %ld x %ld",_root_minw,_root_minh,_root_maxw,_root_maxh);
			#endif

			LOD->sys_flags |= FF_WINDOW_SYS_KNOWNMINMAX;
		}

		/* FF_WINDOW_WIN_RESIZABLE is made public by the FA_Window_Resizable
		attribute. This attribute is used by decorators to know if they need
		to create zoom and size gadgets. */

		/* FIXME-060719: Since Window class is now a subclass of  Area,  the
		FA_Window_Resizable attribute is obsolete. One can check _area_minw,
		_area_maxw, _area_minh and _area_minw  to  know  if  the  window  is
		resizable  or not, just like the Group class does with its children.
		*/

		if (_root_minw == _root_maxw && _root_minh == _root_maxh)
		{
			LOD->win_flags &= ~FF_WINDOW_WIN_RESIZABLE;
		}
		else
		{
			LOD->win_flags |= FF_WINDOW_WIN_RESIZABLE;
		}
	}
	else
	{
		LOD->win_flags |= FF_WINDOW_WIN_RESIZABLE;
	}

	if (FF_WINDOW_WIN_BORDERLESS & LOD->win_flags)
	{
		LOD->win_flags &= ~FF_WINDOW_WIN_RESIZABLE;
	}

	return (bool32)(0 != (LOD->win_flags & FF_WINDOW_WIN_RESIZABLE));
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Window_AskMinMax
F_METHOD(uint32, Window_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	struct Screen *scr = LOD->screen;

	window_resizable(Class,Obj);

/*** contents minmax ***************************************************************************/

	if (_root_public != NULL)
	{
		_area_minw = _root_minw;
		_area_minh = _root_minh;
		_area_maxw = _root_maxw;
		_area_maxh = _root_maxh;
	}
	else
	{
		_area_minw = 0;
		_area_minh = 0;
		_area_maxw = FV_Area_Max;
		_area_maxh = FV_Area_Max;
	}

/*** margins ************************************************************************************

	Area's margin space is used by the Window class for its borders, keeping
	box compatibility.

*/

	if ((FF_WINDOW_WIN_BORDERLESS & LOD->win_flags) != 0)
	{
		_area_margin.l =
		_area_margin.r =
		_area_margin.t =
		_area_margin.t = 0;
	}
	else
	{
		if (LOD->decorator != NULL)
		{
			F_OBJDO(LOD->decorator);

			_area_margin.l = _deco_bl;
			_area_margin.r = _deco_br;
			_area_margin.t = _deco_bt;
			_area_margin.b = _deco_bb;
		}
		else
		{
			_area_margin.l = scr->WBorLeft;
			_area_margin.r = scr->WBorRight;
			_area_margin.t = scr->WBorTop + scr->RastPort.Font->tf_YSize + 1;

			// FIXME: BarHeight should be good, but sometimes it isn't !?

			_area_margin.b = scr->BarHeight - 3;
		}
	}

/*** limits ************************************************************************************/

	F_SUPERDO();

	_area_maxw = MIN(_area_maxw, scr->Width);
	_area_maxh = MIN(_area_maxh, scr->Height);

/*** zoom **************************************************************************************/

	LOD->window_zoom.w = _area_maxw;
	LOD->window_zoom.h = _area_maxh;

	LOD->window_zoom.x = (scr->Width - LOD->window_zoom.w) / 2;
	LOD->window_zoom.y = (scr->Height - LOD->window_zoom.h) / 2;

	#ifdef DB_ASKMINMAX
	IFEELIN F_Log(0, "min (%ld x %ld) max (%ld x %ld)", _area_minw, _area_minh, _area_maxw, _area_maxh);
	#endif

	return TRUE;
}
//+

///Window_Layout
F_METHOD(bool32,Window_Layout)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}
	
	/* FIXME-060706: this flag should be set elsewhere */

	_area_flags |= FF_Area_Damaged;

//	  IFEELIN F_Set(_area_render, FA_Render_Forbid, TRUE);

	if (LOD->decorator != NULL)
	{
		_deco_x = 0;
		_deco_y = 0;
		_deco_w = _area_w;
		_deco_h = _area_h;

		F_OBJDO(LOD->decorator);
	}

	#ifdef DB_LAYOUT
	
	IFEELIN F_Log
	(
		0, "box (%ld : %ld) (%ld x %ld) content (%ld : %ld) (%ld x %ld)",
		
		_area_x, _area_y, _area_w, _area_h,
		_area_cx, _area_cy, _area_cw, _area_ch
	);
	
	#endif

	if (LOD->root != NULL)
	{
		_root_x = _area_cx;
		_root_y = _area_cy;
		_root_w = _area_cw;
		_root_h = _area_ch;

		F_OBJDO(LOD->root);
	}

//	  IFEELIN F_Set(_area_render, FA_Render_Forbid, FALSE);

	return TRUE;
}
//+
///Window_Show
F_METHOD(uint32, Window_Show)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (LOD->window != NULL)
	{
		return TRUE;
	}

	LOD->window = (struct Window *) IFEELIN F_Do(WinServer, FM_AddMember, Obj);

	if (LOD->window == NULL)
	{
		IFEELIN F_Log(FV_LOG_USER, "WinServer was unable to open window (%s)", IFEELIN F_Get(Obj, FA_Element_Id));

		return FALSE;
	}

	IFEELIN F_Set(_area_render, FA_Render_RPort, (uint32) LOD->window->RPort);

	IFEELIN F_Do(Obj, FM_Set,

		FA_NoNotify,            TRUE,
		FA_Window_Title,        LOD->WinTitle,
		FA_Window_ScreenTitle,  LOD->ScrTitle,

		TAG_DONE);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	if (LOD->decorator != NULL)
	{
		if (F_OBJDO(LOD->decorator) == FALSE)
		{
			return FALSE;
		}
	}

	if (LOD->root != NULL)
	{
		if (F_OBJDO(LOD->root) == FALSE)
		{
			return FALSE;
		}
	}

	LOD->sys_flags |= FF_WINDOW_SYS_NOTIFY_EVENTS;

	window_collect_events(Class, Obj);

	return TRUE;
}
//+
///Window_Hide
F_METHOD(uint32, Window_Hide)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct Window *win = LOD->window;

	if (win)
	{
		FRender * area_render = _area_render;

		LOD->sys_flags &= ~FF_WINDOW_SYS_NOTIFY_EVENTS;

		if (area_render)
		{
			IFEELIN F_Set(Obj, FA_Window_ActiveObject, 0);

			IFEELIN F_Do(LOD->root, FM_Area_Hide);
			IFEELIN F_Do(LOD->decorator, FM_Area_Hide);

			IFEELIN F_Set(area_render, FA_Render_RPort, 0);
		}

		//
		// clear events
		//
		
		LOD->events = 0;
		
		IFEELIN F_Set(Obj, FA_Window_Events, 0);

		//
		// close window
		//

		LOD->window = NULL;

		IFEELIN F_Do(WinServer, FM_RemMember, Obj);
	}

	return F_SUPERDO();
}
//+
///Window_Draw
F_METHOD(void,Window_Draw)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FRender * area_render = _area_render;

	#ifdef DB_DRAW
	IGRAPHICS SetRast(_area_rp, 3);
	
	IFEELIN F_Log(0,"root (0x%08lx) - forbid (%ld) - rethink (%ld)", LOD->root, F_Get(area_render, FA_Render_Forbid), LOD->RethinkNest);
	#endif

//    IFEELIN F_Log(0, "framebox (%ld : %ld, %ld x %ld)", LOD->FrameBox.x, LOD->FrameBox.y, LOD->FrameBox.w, LOD->FrameBox.h);

	if ((FF_Render_Forbid & area_render->Flags) != 0)
	{
		LOD->sys_flags |= FF_WINDOW_SYS_REFRESH_NEED;
		
		return;
	}

	#ifdef F_ENABLE_BUFFERING

	IFEELIN F_Set(LOD->root, FA_Area_Bufferize, TRUE);
	
	#endif
 
	LOD->sys_flags &= ~FF_WINDOW_SYS_REFRESH_NEED;

	if (_deco_area_public != NULL)
	{
		IFEELIN F_Draw(LOD->decorator, FF_Draw_Object);
	}

	F_SUPERDO();

	if (!LOD->root)
	{
		return;
	}

	/* FF_Render_Complex is disabled  if  FF_WINDOW_COMPLEX_REFRESH  is
	not set, making complete window refresh faster but flashing */
   
	if ((FF_WINDOW_SYS_COMPLEX & LOD->sys_flags) != 0)
	{
		area_render->Flags |= FF_Render_Refreshing | FF_Render_Complex;
	}
	else
	{
		area_render->Flags |= FF_Render_Refreshing;
	}
						
	if ((FF_WINDOW_SYS_REFRESH_SIMPLE & LOD->sys_flags) ||
		((FF_WINDOW_SYS_COMPLEX & LOD->sys_flags) == 0))
	{
		IFEELIN F_Draw(LOD->root, FF_Draw_Object);
	}
	else
	{
		IFEELIN F_Draw(LOD->root, FF_Draw_Damaged | FF_Draw_Object);
	}

	if ((FF_WINDOW_SYS_COMPLEX & LOD->sys_flags) != 0)
	{
		area_render->Flags &= ~FF_Render_Refreshing;
	}
	else
	{
		area_render->Flags &= ~(FF_Render_Refreshing | FF_Render_Complex);
	}
}
//+
