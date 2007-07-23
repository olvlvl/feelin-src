#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

#ifdef F_NEW_GLOBALCONNECT
///Window_GlobalConnect
F_METHODM(bool32, Window_GlobalConnect, FS_Element_GlobalConnect)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (IFEELIN F_SuperDo(Class, Obj, Method, Msg->Application, Obj) == FALSE)
	{
		return FALSE;
	}

	if (LOD->root != NULL)
	{
		if (IFEELIN F_Do(LOD->root, Method, Msg->Application, Obj) == FALSE)
		{
			return FALSE;
		}
	}

	if (LOD->decorator != NULL)
	{
		if (IFEELIN F_Do(LOD->root, Method, Msg->Application, Obj) == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}
//+
///Window_GlobalDisconnect
F_METHOD(bool32, Window_GlobalDisconnect)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	if (LOD->root != NULL)
	{
		if (F_SUPERDO() == FALSE)
		{
			return FALSE;
		}
	}

	if (LOD->decorator != NULL)
	{
		if (F_SUPERDO() == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}
//+
#endif
///Window_Setup

/* This method is called by the method FM_Window_Open to setup  the  object
according  to  the  user  preferences  and  the  future environement of the
window. */

F_METHODM(bool32,Window_Setup,FS_Element_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 value;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

/** rendering mode *****************************************************************************/

	#if 0

	/* FIXME: the code to handle simple render  is  currently  broken,  only
	complex  rendering  is  available,  which  should  anyway  be  the  only
	rendering mode, unless... */

	value = TRUE; // default value

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(RENDER_COMPLEX), NULL, &value);

	if (value == TRUE)
	{
		LOD->sys_flags |= FF_WINDOW_SYS_COMPLEX;
	}
	else
	{
		LOD->sys_flags &= ~FF_WINDOW_SYS_COMPLEX;
	}

	#else

	LOD->sys_flags |= FF_WINDOW_SYS_COMPLEX;

	#endif

/** refreshing mode ****************************************************************************/

	value = FALSE; // default value

	IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(REFRESH_SIMPLE), NULL, &value);

	if (value == TRUE)
	{
		LOD->sys_flags |= FF_WINDOW_SYS_REFRESH_SIMPLE;
	}
	else
	{
		LOD->sys_flags &= ~FF_WINDOW_SYS_REFRESH_SIMPLE;
	}

	LOD->sys_flags &= ~FF_WINDOW_SYS_KNOWNMINMAX;

/** chainables pool ****************************************************************************/

	LOD->nodes_pool = IFEELIN F_CreatePool(sizeof (struct in_CycleNode) + sizeof (struct in_EventHandler), FA_Pool_Items,16, FA_Pool_Name,"window.nodes", TAG_DONE);

	if (LOD->nodes_pool == NULL)
	{
		IFEELIN F_Log(FV_LOG_USER, "unable to create nodes pool");

		return FALSE;
	}

/** screen *************************************************************************************/

	LOD->screen = (struct Screen *) IFEELIN F_Get(_area_display, (uint32) "FA_Display_Screen");

/*** Résolution des préférences & Création des objets ******************************************/

	if ((FF_WINDOW_WIN_BORDERLESS & LOD->win_flags) == 0)
	{
		/* the decorator needs to be created before calling MinMax() */

		value = 0;

		IFEELIN F_Do(Obj, FM_Element_GetProperty, NULL, F_PROPERTY_ATOM(DECORATOR), &value, NULL);

		if (value != 0)
		{
			LOD->decorator = IFEELIN F_NewObj((STRPTR) value, TAG_DONE);
	 
			if (LOD->decorator != NULL)
			{
				//_deco_frame_public = F_GET_FRAME_PUBLIC(LOD->decorator);
				_deco_area_public = F_GET_AREA_PUBLIC(LOD->decorator);

				if (/*(_deco_frame_public == NULL) ||*/
					(_deco_area_public == NULL))
				{
					IFEELIN F_Log(FV_LOG_DEV, "decorator %s{%lx} should be a subclass of the Area class !");

					return FALSE;
				}
			}
		}
	}

	/* Setup the root object. If the root object is a  group,  every  single
	object  of  the  group's tree is setup. If there is any problem, even in
	the deepest node of the tree, the method returns FALSE. In this case  we
	send  back  the  method  FM_Cleanup  to give a chance to objects to free
	their datas. */

	if (LOD->root != NULL)
	{
		if (F_OBJDO(LOD->root) != FALSE)
		{
			/* we need to call Window_Resizable() here to know if the window
			is  resizable  (FA_Window_Resizable  == TRUE). This attribute is
			nedeed by decorators to know if they have  to  create  zoom  and
			resize gadgets */

			window_resizable(Class, Obj);
		}
		else
		{

			IFEELIN F_DoA(LOD->root, FM_Element_Cleanup, Msg);

			IFEELIN F_Log(FV_LOG_USER,"Setup of %s{%08lx} failed",_object_classname(LOD->root),LOD->root);

			return FALSE;
		}
	}

	/* now that we know everything about risize capabilities  we  can  setup
	the decorator */

/*	FIXME-061020

	decorator handling is a real mess !

*/

	if (LOD->decorator != NULL)
	{
		#ifdef F_NEW_GLOBALCONNECT
		if (IFEELIN F_Do(LOD->decorator, FM_Element_GlobalConnect, _element_application, Obj) == FALSE)
		{
			IFEELIN F_Log(0, "decorator refused global connection !");

			return FALSE;
		}
		#endif

		IFEELIN F_Do(LOD->decorator, FM_Connect, Obj);

		if (F_OBJDO(LOD->decorator) == FALSE)
		{
			IFEELIN F_DoA(LOD->decorator, FM_Element_Cleanup, Msg);

			IFEELIN F_Do(LOD->decorator, FM_Disconnect);

			IFEELIN F_Do(LOD->decorator, FM_Element_GlobalDisconnect);

			IFEELIN F_DisposeObj(LOD->decorator);

			LOD->decorator = NULL;
		}
	}

	return TRUE;
}
//+
///Window_Cleanup

/* This method is called by the  method  FM_Window_Close  just  before  the
window is closed. */

F_METHOD(bool32,Window_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (_area_render != NULL)
	{
		F_OBJDO(LOD->root);

		if (LOD->PopHelp != NULL)
		{
			IFEELIN F_DisposeObj(LOD->PopHelp);

			LOD->PopHelp = NULL;
		}

		if (LOD->decorator != NULL)
		{
			F_OBJDO(LOD->decorator);

			#ifdef F_NEW_GLOBALCONNECT
			IFEELIN F_Do(LOD->decorator, FM_Element_GlobalDisconnect);
			#endif

			IFEELIN F_DisposeObj(LOD->decorator);

			LOD->decorator = NULL;
			//_deco_frame_public = NULL;
			_deco_area_public = NULL;
		}
	}

	LOD->screen = NULL;

	if (LOD->nodes_pool != NULL)
	{
		LOD->CycleChain.Head = NULL;
		LOD->CycleChain.Tail = NULL;

		LOD->event_handlers_list.Head = NULL;
		LOD->event_handlers_list.Tail = NULL;

		IFEELIN F_DeletePool(LOD->nodes_pool);

		LOD->nodes_pool = NULL;
	}

	return F_SUPERDO();
}
//+
///Window_LoadPersistentAttributes
F_METHOD(FPDRHandle *, Window_LoadPersistentAttributes)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FPDRHandle *handle = (FPDRHandle *) F_SUPERDO();

	if (handle)
	{
		FPDRAttribute *attr;

		for (attr = handle->Attributes ; attr ; attr = attr->Next)
		{
			if (attr->Atom == F_ATOM(LEFT))
			{
				LOD->user_box_flags &= ~FF_WINDOW_BOX_XDEFINED;

				_win_x = F_PDRDOCUMENT_GET_NUMERIC(attr);
			}
			else if (attr->Atom == F_ATOM(TOP))
			{
				LOD->user_box_flags &= ~FF_WINDOW_BOX_YDEFINED;

				_win_y = F_PDRDOCUMENT_GET_NUMERIC(attr);
			}
			else if (attr->Atom == F_ATOM(WIDTH))
			{
				LOD->user_box_flags &= ~FF_WINDOW_BOX_WDEFINED;

				_area_w = F_PDRDOCUMENT_GET_NUMERIC(attr);
			}
			else if (attr->Atom == F_ATOM(HEIGHT))
			{
				LOD->user_box_flags &= ~FF_WINDOW_BOX_HDEFINED;

				_area_h = F_PDRDOCUMENT_GET_NUMERIC(attr);
			}
		}
	}

	F_OBJDO(LOD->root);

	return handle;
}
//+
///Window_SavePersistentAttributes
F_METHOD(FPDRHandle *, Window_SavePersistentAttributes)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FPDRHandle *handle = (FPDRHandle *) F_SUPERDO();

	if (handle)
	{
		FPDRAttribute *attr;

		for (attr = handle->Attributes ; attr ; attr = attr->Next)
		{
			if (attr->Atom == F_ATOM(LEFT))
			{
				F_PDRDOCUMENT_SET_TYPED(attr, _win_x, FV_TYPE_PIXEL);
			}
			else if (attr->Atom == F_ATOM(TOP))
			{
				F_PDRDOCUMENT_SET_TYPED(attr, _win_y, FV_TYPE_PIXEL);
			}
			else if (attr->Atom == F_ATOM(WIDTH))
			{
				F_PDRDOCUMENT_SET_TYPED(attr, _area_w, FV_TYPE_PIXEL);
			}
			else if (attr->Atom == F_ATOM(HEIGHT))
			{
				F_PDRDOCUMENT_SET_TYPED(attr, _area_h, FV_TYPE_PIXEL);
			}
		}
	}

	F_OBJDO(LOD->root);

	return handle;
}
//+

