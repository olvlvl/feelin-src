#include "Private.h"

//#define DB_COMPUTE_BOX

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///window_setup

/*

	This function create and setup  the  FRender  object,  then  invoke  the
	FM_Element_Setup  method  on  the  object  to  finish  the job and gives
	subclasses the oportunity to setup further thingies.

*/


STATIC bool32 window_setup(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT

	FObject render = RenderObject,

		FA_Render_Application, _element_application,
		FA_Render_Window, Obj,

		End;

	#else

	FObject render = RenderObject,

		FA_Render_Application, _area_parent,
		FA_Render_Window, Obj,

		End;

	#endif

	if (!render)
	{
		IFEELIN F_Log(FV_LOG_USER, "Unable to create Render object");

		return FALSE;
	}

	if (IFEELIN F_Do(Obj, FM_Element_Setup, render) == FALSE)
	{
		IFEELIN F_Do(Obj, FM_Element_Cleanup);

		IFEELIN F_DisposeObj(render);

		return FALSE;
	}

	return TRUE;
}
//+
///window_cleanup
STATIC void window_cleanup(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FObject render = _area_render;

	if (!render)
	{
		return;
	}
	
	IFEELIN F_Do(Obj, FM_Element_Cleanup, render);

	IFEELIN F_DisposeObj(render);
}
//+
///window_compute_box
STATIC void window_compute_box(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	uint16 scr_w = LOD->screen->Width;
	uint16 scr_h = LOD->screen->Height;

	uint16 area_w = _area_w;
	uint16 area_h = _area_h;

	#ifdef DB_COMPUTE_BOX
	IFEELIN F_Log(0,"Dim %4ld,%4ld - Min %4ld,%4ld - Max %4ld,%4ld", area_w, area_h, _area_minw, _area_minh, _area_maxw, _area_maxh);
	#endif

	if ((area_w == 0) && (area_h == 0))
	{
		if ((FF_WINDOW_BOX_WDEFINED & LOD->user_box_flags) != 0)
		{
			if ((FF_WINDOW_BOX_WPERCENT & LOD->user_box_flags) != 0)
			{
				area_w = scr_w * LOD->user_box.w / 100;
			}
			else
			{
				area_w = LOD->user_box.w;
			}

			#ifdef DB_COMPUTE_BOX
			IFEELIN F_Log(0,"w %ld", area_w);
			#endif
		}

		if ((FF_WINDOW_BOX_HDEFINED & LOD->user_box_flags) != 0)
		{
			if ((FF_WINDOW_BOX_HPERCENT & LOD->user_box_flags) != 0)
			{
				area_h = scr_h * LOD->user_box.h / 100;
			}
			else
			{
				area_h = LOD->user_box.h;
			}

			#ifdef DB_COMPUTE_BOX
			IFEELIN F_Log(0,"h %ld", area_h);
			#endif
		}
	}

/* width and height limits */

	area_w = MAX(area_w, _area_minw);
	area_h = MAX(area_h, _area_minh);
	_area_w = area_w = MIN(area_w, _area_maxw);
	_area_h = area_h = MIN(area_h, _area_maxh);

/* x coordinates */
	
	if ((FF_WINDOW_BOX_XDEFINED & LOD->user_box_flags) != 0)
	{
		if ((FF_WINDOW_BOX_XPERCENT & LOD->user_box_flags) != 0)
		{
			_win_x = scr_w * LOD->user_box.x / 100;
		}
		else
		{
			_win_x = LOD->user_box.x;
		}

		if ((FF_WINDOW_BOX_RHANDLE & LOD->user_box_flags) != 0)
		{
			_win_x -= (area_w + 1);
		}
	}

/*	FIXME-060910

	is (-1) still required ?

*/

	else if (_win_x == -1)
	{
		_win_x = (scr_w - area_w) / 2;
	}

/* y coordinate */

	if ((FF_WINDOW_BOX_YDEFINED & LOD->user_box_flags) != 0)
	{
		if ((FF_WINDOW_BOX_YPERCENT & LOD->user_box_flags) != 0)
		{
			_win_y = scr_h * LOD->user_box.y / 100;
		}
		else
		{
			_win_y = LOD->user_box.y;
		}

		if ((FF_WINDOW_BOX_RHANDLE & LOD->user_box_flags) != 0)
		{
			_win_y -= (area_h + 1);
		}
	}

/*	FIXME-060910

	is (-1) still required ?

*/

	else if (_win_y == -1)
	{
		_win_y = (scr_h - area_h) / 2;
	}

	#ifdef DB_COMPUTE_BOX
	IFEELIN F_Log(0,"box (%4ld : %4ld, %4ld x %4ld)", _win_x, _win_y, area_w, area_h);
	#endif
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Window_Open

/*  Cette  Méthode   est   appelée   lorsque   l'application   se   réveille
(FM_Application_Run   ou   FM_Application_Awake),   lorsque  le  système  de
préférence a été modifié, ou encore parce que l'attribut FA_Window_Open  est
devenu TRUE.

La méthode renvoie un pointeur vers la fenêtre qui a été ouverte, ou NULL si
l'ouverture de la fenêtre est impossible pour une raison quelconque. */

F_METHOD(struct Window *,Window_Open)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct Window * win = LOD->window;
	struct Screen * scr;

	uint16 scr_w;
	uint16 scr_h;

	uint16 area_w;
	uint16 area_h;

	//
	// if the window is already opened we return its pointer
	//

	if (win)
	{
		IINTUITION WindowToFront(win);

		if ((FF_WINDOW_WIN_ACTIVABLE & LOD->win_flags) != 0)
		{
			IINTUITION ActivateWindow(win);
		}
		
		return win;
	}

	//
	//
	//

	if (window_setup(Class, Obj) == FALSE)
	{
		return NULL;
	}

	IFEELIN F_Do(Obj, FM_Area_AskMinMax);

	window_compute_box(Class, Obj);

	//
	// would the window fit on the screen ?
	//

	scr = LOD->screen;

	scr_w = scr->Width;
	scr_h = scr->Height;

	area_w = _area_w;
	area_h = _area_h;

	if ((area_w > scr_w) || (area_h > scr_h))
	{
		IFEELIN F_Log(FV_LOG_USER, "Window (%ld x %ld) to big for Screen (%ld x %ld)", area_w, area_h, scr_w, scr_h);

		return NULL;
	}

	IFEELIN F_Do(Obj, FM_Area_Layout);
	
//    IFEELIN F_Log(0,"OPEN %ld : %ld, %ld x %ld",LOD->Box.x,LOD->Box.y,LOD->Box.w,LOD->Box.h);
//    IFEELIN F_Log(0,"winserver addmemeber >> BEGIN");

	if (IFEELIN F_Do(Obj, FM_Area_Show) == FALSE)
	{
		IFEELIN F_Do(Obj, FM_Window_Close);

		return NULL;
	}

	IFEELIN F_Draw(Obj, FF_Draw_Object);

	return LOD->window;
}
//+
///Window_Close
/*

Cette méthode est appelée par l'application lorsque la  fenêtre  doit  être
fermée  ce qui se produit lorsque l'application est réduite ou qu'il y a eu
une modification du système de préférence. Cette méthode est aussi  appelée
lorsque l'attribut FA_Window_Open a été modifié.

*/
F_METHOD(void,Window_Close)
{
	IFEELIN F_Do(Obj, FM_Area_Hide);

	window_cleanup(Class, Obj);
}
//+
