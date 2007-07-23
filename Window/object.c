#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

#if 0
///Window_Iterator
STATIC ASM void Window_Iterator(REG_A0 FWindow_EachFunc Func,REG_A1 APTR UserData,REG_A2 FObject Obj)
{
   if (Obj)
   {
	  FObject sub;

	  Func(Obj,UserData);

	  for (sub = (FObject) IFEELIN F_Get(Obj,FA_Family_Head) ; sub ; sub = _next(sub))
	  {
		 Window_Iterator(Func,UserData,Obj);
	  }
   }
}
//+
#endif

///Window_Cycle
FObject Window_Cycle(FClass *Class,FObject Obj,uint32 Direction)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct in_CycleNode *node;

	if (LOD->ActiveObj != NULL)
	{
	  /* first we search the current active object in the cycle list */

		for (node = (struct in_CycleNode *) LOD->CycleChain.Head ; node ; node = node->Next)
		{
			if (node->Widget == LOD->ActiveObj)
			{
				break;
			}
	   }
	}
	else
	{
		node = NULL;
	}

	switch (Direction)
	{
		case FV_Window_ActiveObject_Next:
		{
			if (node != NULL)
			{
				for (node = node->Next ; node ; node = node->Next)
				{
					if (_chained_is_drawable && _chained_is_not_disabled)
					{
						break;
					}
				}

				if (node == NULL)
				{
					for (node = (struct in_CycleNode *) LOD->CycleChain.Head ; node ; node = node->Next)
					{
						if (_chained_is_drawable && _chained_is_not_disabled)
						{
							break;
						}
					}
				}
			}
			else
			{
				node = (struct in_CycleNode *) LOD->CycleChain.Head;
			}
		 }
		 break;

		 case FV_Window_ActiveObject_Prev:
		 {
			if (node)
			{
				for (node = node->Prev ; node ; node = node->Prev)
				{
					if (_chained_is_drawable && _chained_is_not_disabled)
					{
						break;
					}
				}

				if (!node)
				{
					for (node = (struct in_CycleNode *) LOD->CycleChain.Tail ; node ; node = node->Prev)
					{
						if (_chained_is_drawable && _chained_is_not_disabled)
						{
							break;
						}
					}
				}
			}
			else
			{
				node = (struct in_CycleNode *) LOD->CycleChain.Tail;
			}
		}
		break;

		default:
		{
			return (FObject) Direction;
		}
	}

	if (node != NULL)
	{
		return node->Widget;
	}

	return NULL;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Window_New
F_METHOD(uint32,Window_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif
	F_SAVE_AREA_PUBLIC;

	/* set default values */

	LOD->GADFlags = FF_WINDOW_GAD_DRAG  | FF_WINDOW_GAD_CLOSE |
					FF_WINDOW_GAD_DEPTH | FF_WINDOW_GAD_ICONIFY;

	LOD->win_flags = FF_WINDOW_WIN_ACTIVABLE | FF_WINDOW_WIN_ACTIVE;

	_win_x = -1;
	_win_y = -1;

	/* Parse initial attributes */

	while (IFEELIN F_DynamicNTI(&Tags,&item,NULL))
	switch (item.ti_Tag)
	{
		case FA_Child:              if (!item.ti_Data) return NULL; break;

		case FA_Window_GadNone:     if (item.ti_Data) LOD->GADFlags  = NULL; break;
		case FA_Window_GadDragbar:  if (item.ti_Data) LOD->GADFlags |= FF_WINDOW_GAD_DRAG;       else LOD->GADFlags &= ~FF_WINDOW_GAD_DRAG;       break;
		case FA_Window_GadClose:    if (item.ti_Data) LOD->GADFlags |= FF_WINDOW_GAD_CLOSE;      else LOD->GADFlags &= ~FF_WINDOW_GAD_CLOSE;      break;
		case FA_Window_GadDepth:    if (item.ti_Data) LOD->GADFlags |= FF_WINDOW_GAD_DEPTH;      else LOD->GADFlags &= ~FF_WINDOW_GAD_DEPTH;      break;
		case FA_Window_GadIconify:  if (item.ti_Data) LOD->GADFlags |= FF_WINDOW_GAD_ICONIFY;    else LOD->GADFlags &= ~FF_WINDOW_GAD_ICONIFY;    break;

		case FA_Window_Active:      if (item.ti_Data) LOD->win_flags |= FF_WINDOW_WIN_ACTIVE;     else LOD->win_flags &= ~FF_WINDOW_WIN_ACTIVE;     break;
		case FA_Window_Activable:   if (item.ti_Data) LOD->win_flags |= FF_WINDOW_WIN_ACTIVABLE;    else LOD->win_flags &= ~FF_WINDOW_WIN_ACTIVABLE;    break;
		case FA_Window_Backdrop:    if (item.ti_Data) LOD->win_flags |= FF_WINDOW_WIN_BACKDROP;     else LOD->win_flags &= ~FF_WINDOW_WIN_BACKDROP;   break;
		case FA_Window_Borderless:  if (item.ti_Data) LOD->win_flags |= FF_WINDOW_WIN_BORDERLESS;   else LOD->win_flags &= ~FF_WINDOW_WIN_BORDERLESS; break;

/*** coordinates *******************************************************************************/

		/* Par défaut, les  coordonées  de  la  fenêtre  sont  calculées  en
		ajoutant la largeur de la fenêtre à son côté gauche, et sa hauteur à
		son côté haut. En définissant les attributs 'Right' et 'Bottom' vous
		pouvez  positionner  votre fenêtre differement. Par exemple, si vous
		définissez  l'attribut  'Right',  la  largeur  de  la  fenêtre  sera
		soustraite  pour  obtenir  la  coordonée du côté gauche. De même, si
		vous définissez l'attribut 'Bottom', la hauteur de la  fenêtre  sera
		soustraite  pour  obtenir  les  coordonées  du  bord  haut. Ainsi en
		définissant  les  attributs  'Right'   et   'Bottom'   vous   pouvez
		positionner  la  fenêtre  relativement  aux côtés droit / bas plutôt
		qu'aux côtés gauche / haut habituels.

		Notez que si  vous  ne  définissez  ni  'Left',  'Right',  'Top'  ni
		'Bottom', la fenêtre sera centrée à l'écran. */

		case FA_Window_Left:
		{
			LOD->user_box_flags &= ~(FF_WINDOW_BOX_XPERCENT | FF_WINDOW_BOX_RHANDLE);
			LOD->user_box_flags |= FF_WINDOW_BOX_XDEFINED;

			if ((int32) item.ti_Data > 0xFFFF)
			{
				STRPTR str = (STRPTR) item.ti_Data;
				int32 val;

				str += f_stcd(str, &val);

				LOD->user_box.x = val;

				if (*str == '%')
				{
					LOD->user_box_flags |= FF_WINDOW_BOX_XPERCENT;
				}
			}
			else
			{
				LOD->user_box.x = item.ti_Data;
			}
		}
		break;

		case FA_Window_Right:
		{
			LOD->user_box_flags &= ~FF_WINDOW_BOX_XPERCENT;
			LOD->user_box_flags |= (FF_WINDOW_BOX_XDEFINED | FF_WINDOW_BOX_RHANDLE);

			if ((int32) item.ti_Data > 0xFFFF)
			{
				STRPTR str = (STRPTR) item.ti_Data;
				int32 val;

				str += f_stcd(str, &val);

				LOD->user_box.x = val;

				if (*str == '%')
				{
					LOD->user_box_flags |= FF_WINDOW_BOX_XPERCENT;
				}
			}
			else
			{
				LOD->user_box.x = item.ti_Data;
			}
		}
		break;

		case FA_Window_Top:
		{
			LOD->user_box_flags &= ~(FF_WINDOW_BOX_YPERCENT | FF_WINDOW_BOX_BHANDLE);
			LOD->user_box_flags |= FF_WINDOW_BOX_YDEFINED;

			if ((int32) item.ti_Data > 0xFFFF)
			{
				STRPTR str = (STRPTR)(item.ti_Data);
				int32 val;

				str += f_stcd(str, &val);

				LOD->user_box.y = val;

				if (*str == '%')
				{
					LOD->user_box_flags |= FF_WINDOW_BOX_YPERCENT;
				}
			}
			else
			{
				LOD->user_box.y = item.ti_Data;
			}
		}
		break;

		case FA_Window_Bottom:
		{
			LOD->user_box_flags &= ~FF_WINDOW_BOX_YPERCENT;
			LOD->user_box_flags |= (FF_WINDOW_BOX_YDEFINED | FF_WINDOW_BOX_BHANDLE);

			if ((int32) item.ti_Data > 0xFFFF)
			{
				STRPTR str = (STRPTR)(item.ti_Data);
				int32 val;

				str += f_stcd(str, &val);

				LOD->user_box.y = val;

				if (*str == '%')
				{
					LOD->user_box_flags |= FF_WINDOW_BOX_YPERCENT;
				}
			}
			else
			{
				LOD->user_box.y = item.ti_Data;
			}
		}
		break;

		case FA_Window_Width:
		{
			LOD->user_box_flags &= ~FF_WINDOW_BOX_WPERCENT;
			LOD->user_box_flags |= FF_WINDOW_BOX_WDEFINED;

			if ((int32) item.ti_Data > 0xFFFF)
			{
				STRPTR str = (STRPTR)(item.ti_Data);
				int32 val;

				str += f_stcd(str, &val);

				LOD->user_box.w = val;

				if (*str == '%')
				{
					LOD->user_box_flags |= FF_WINDOW_BOX_WPERCENT;
				}
			}
			else
			{
				LOD->user_box.w = item.ti_Data;
			}
		}
		break;

		case FA_Window_Height:
		{
			LOD->user_box_flags &= ~FF_WINDOW_BOX_HPERCENT;
			LOD->user_box_flags |= FF_WINDOW_BOX_HDEFINED;

			if ((int32) item.ti_Data > 0xFFFF)
			{
				STRPTR str = (STRPTR)(item.ti_Data);
				int32 val;

				str += f_stcd(str, &val);

				LOD->user_box.h = val;

				if (*str == '%')
				{
					LOD->user_box_flags |= FF_WINDOW_BOX_HPERCENT;
				}
			}
			else
			{
				LOD->user_box.h = item.ti_Data;
			}
		}
		break;
	}

	if (FF_WINDOW_WIN_BORDERLESS & LOD->win_flags)
	{
		LOD->GADFlags = 0;
	}
			
	if ((FF_WINDOW_WIN_ACTIVABLE & LOD->win_flags) == 0)
	{
		LOD->win_flags &= ~FF_WINDOW_WIN_ACTIVABLE;
	}

	return F_SUPERDO();
}
//+
///Window_Dispose
F_METHOD(void,Window_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->window != NULL)
	{
		IFEELIN F_Do(Obj, FM_Window_Close);
	}

	if (LOD->root != NULL)
	{
		IFEELIN F_DisposeObj(LOD->root);

		LOD->root = NULL;
	}
 
	F_SUPERDO();
}
//+
///Window_Get
F_METHOD(void,Window_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;
	struct Window *win = LOD->window;

	F_SUPERDO();

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Child:                F_STORE(LOD->root);		break;
  //         case FA_ContextHelp:       F_STORE(self.contexthelp
  //         CASE FA_ContextMenu         ; ^save := self.contextmenu

		case FA_Window_System:          F_STORE(win); break;
		case FA_Window_Open:            F_STORE(0 != (FF_WINDOW_SYS_OPENREQUEST & LOD->sys_flags)); break;
		case FA_Window_Title:           F_STORE(LOD->WinTitle);  break;
		case FA_Window_ScreenTitle:     F_STORE(LOD->ScrTitle);  break;
		case FA_Window_ActiveObject:    F_STORE(LOD->ActiveObj); break;

		case FA_Window_GadDragbar:      F_STORE(0 != (FF_WINDOW_GAD_DRAG       & LOD->GADFlags)); break;
		case FA_Window_GadClose:        F_STORE(0 != (FF_WINDOW_GAD_CLOSE      & LOD->GADFlags)); break;
		case FA_Window_GadDepth:        F_STORE(0 != (FF_WINDOW_GAD_DEPTH      & LOD->GADFlags)); break;
		case FA_Window_GadIconify:      F_STORE(0 != (FF_WINDOW_GAD_ICONIFY    & LOD->GADFlags)); break;

		case FA_Window_Active:          F_STORE(0 != (FF_WINDOW_WIN_ACTIVE     & LOD->win_flags)); break;
		case FA_Window_Activable:       F_STORE(0 != (FF_WINDOW_WIN_ACTIVABLE & LOD->win_flags)); break;
		case FA_Window_Resizable:       F_STORE(0 != (FF_WINDOW_WIN_RESIZABLE  & LOD->win_flags)); break;
		case FA_Window_Borderless:      F_STORE(0 != (FF_WINDOW_WIN_BORDERLESS & LOD->win_flags)); break;
	
		case FA_Window_Events:          F_STORE(LOD->events); break;
		case FA_Window_Decorator:       F_STORE(LOD->decorator); break;

		case FA_Window_Left:            F_STORE(_win_x); break;
		case FA_Window_Right:           F_STORE(_win_x + _area_w - 1); break;
		case FA_Window_Top:             F_STORE(_win_y); break;
		case FA_Window_Bottom:          F_STORE(_win_y + _area_h - 1); break;
		case FA_Window_Width:           F_STORE(_area_w); break;
		case FA_Window_Height:          F_STORE(_area_h); break;
	}
}
//+
///Window_Set
F_METHOD(void,Window_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,NULL))
	switch (item.ti_Tag)
	{
		
		case FA_Child:
		{
			IFEELIN F_Do(Obj,FM_AddMember,item.ti_Data,FV_AddMember_Tail);
		}
		break;
/*
		case FA_ContextHelp:
		{
			LOD->ContextHelp = (STRPTR)(item.ti_Data);
		}
		break;
*/
		case FA_Window_Open:
		{
			if (item.ti_Data != 0)
			{
				LOD->sys_flags |= FF_WINDOW_SYS_OPENREQUEST;
			}
			else
			{
				LOD->sys_flags &= ~FF_WINDOW_SYS_OPENREQUEST;
			}

			#ifdef F_NEW_GLOBALCONNECT

			if (_element_application != NULL)
			{
				if (IFEELIN F_Get(_element_application, FA_Application_Sleep) == FALSE)
				{
					if ((FF_WINDOW_SYS_OPENREQUEST & LOD->sys_flags) != 0)
					{
						IFEELIN F_Do(_element_application, FM_Application_PushMethod, Obj, FM_Window_Open, 0);
					}
					else
					{
						IFEELIN F_Do(_element_application, FM_Application_PushMethod, Obj, FM_Window_Close, 0);
					}
				}
			}

			#else

			if (_area_parent != NULL)
			{
				if (IFEELIN F_Get(_area_parent, FA_Application_Sleep) == FALSE)
				{
					if ((FF_WINDOW_SYS_OPENREQUEST & LOD->sys_flags) != 0)
					{
						IFEELIN F_Do(_area_parent, FM_Application_PushMethod, Obj, FM_Window_Open, 0);
					}
					else
					{
						IFEELIN F_Do(_area_parent, FM_Application_PushMethod, Obj, FM_Window_Close, 0);
					}
				}
			}

			#endif
		}
		break;

		case FA_Window_Title:
		{
			LOD->WinTitle = (STRPTR)(item.ti_Data);

			if (LOD->window != NULL)
			{
				if (!LOD->decorator && (FF_WINDOW_GAD_DRAG & LOD->GADFlags))
				{
					IINTUITION SetWindowTitles(LOD->window, LOD->WinTitle, (STRPTR) -1);
				}
				else
				{
					LOD->window->Title = LOD->WinTitle;
				}
			}
		}
		break;

		case FA_Window_ScreenTitle:
		{
			if ((LOD->ScrTitle = (STRPTR)(item.ti_Data)) != NULL)
			{
				if (LOD->window != NULL)
				{
					IINTUITION SetWindowTitles(LOD->window, (STRPTR) -1, LOD->ScrTitle);
				}
			}
		}
		break;

		case FA_Window_Active:
		{
			if (LOD->window != NULL)
			{
				IINTUITION WindowToFront(LOD->window);
				
				if (FF_WINDOW_WIN_ACTIVABLE & LOD->win_flags)
				{
					IINTUITION ActivateWindow(LOD->window);
				}
			}
				
			if ((item.ti_Data != FALSE) && (FF_WINDOW_WIN_ACTIVABLE & LOD->win_flags))
			{
				LOD->win_flags |= FF_WINDOW_WIN_ACTIVE;
			}
			else
			{
				LOD->win_flags &= ~FF_WINDOW_WIN_ACTIVE;
			}
		}
		break;

		case FA_Window_ActiveObject:
		{
			FObject ao = Window_Cycle(Class,Obj,item.ti_Data);

			if (ao != LOD->ActiveObj)
			{
				IFEELIN F_Set(LOD->ActiveObj,     FA_Widget_Active,FALSE);
				IFEELIN F_Set(LOD->ActiveObj = ao,FA_Widget_Active,TRUE);
			}
		}
		break;
	}

	F_SUPERDO();
}
//+
///Window_AddMember
F_METHODM(FObject,Window_AddMember,FS_AddMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Orphan == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Orphan is NULL !");

		return NULL;
	}

	if (LOD->root != NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Child alreay added: %s{%08lx} ! Orphan %s{%08mx} refused", _object_classname(LOD->root), LOD->root, _object_classname(Msg->Orphan), Msg->Orphan);

		return NULL;
	}

	_root_public = F_GET_AREA_PUBLIC(Msg->Orphan);

	if (_root_public == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Orphan %s{%08lx} should be a subclass of the Area class", _object_classname(Msg->Orphan), Msg->Orphan);

		return NULL;
	}

	#ifdef F_NEW_GLOBALCONNECT

	if (_element_application)
	{
		if (IFEELIN F_Do(Msg->Orphan, FM_Element_GlobalConnect, _element_application, Obj) == FALSE)
		{
			IFEELIN F_Log(FV_LOG_DEV, "Orphan %s{%08lx} refused global connection", _object_classname(Msg->Orphan), Msg->Orphan);

			return NULL;
		}
	}

	#endif

	if (IFEELIN F_Do(Msg->Orphan, FM_Connect, Obj) == FALSE)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Orphan %s{%08lx} refused connection", _object_classname(Msg->Orphan), Msg->Orphan);

		return NULL;
	}

	LOD->root = Msg->Orphan;

	IFEELIN F_Set(Obj, FA_Area_Fillable, FALSE);

	return LOD->root;
}
//+
///Window_RemMember
F_METHODM(uint32,Window_RemMember,FS_RemMember)
{
   struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Member == NULL)
	{
		return 0;
	}

	/* we don't need to call  the  'GlobalDisconnect'  method,  it's  called
	automatically when the Disconnect method reaches the Element class */

	if (Msg->Member == LOD->root)
	{
		IFEELIN F_Do(LOD->root, FM_Disconnect);

		LOD->root = NULL;
		_root_public = NULL;

		IFEELIN F_Set(Obj, FA_Area_Fillable, TRUE);
	}
	else if (Msg->Member == LOD->decorator)
	{
		IFEELIN F_Do(LOD->decorator, FM_Disconnect);

		LOD->decorator = NULL;
	}
	else
	{
		IFEELIN F_Log(0,"%s{%08lx} is not root nor decorator",_object_classname(Msg->Member),Msg->Member);
	}

	return 0;
}
//+

#ifdef F_NEW_GETELEMENTBYID
///Window_GetElementById
F_METHODM(FObject, Window_GetElementById, FS_GetElementById)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FObject rc = (FObject) F_SUPERDO();

	if (!rc)
	{
		rc = (FObject) F_OBJDO(LOD->root);
	}

	return rc;
}
//+
#endif


#if 0
///Window_ForEach
F_METHODM(void,Window_ForEach,FS_Window_ForEach)
{
   Window_Iterator(Msg->Func,Msg->UserData,LOD->root);
}
//+
#endif

#if 0
/************************************************************************************************
*** Preferences *********************************************************************************
************************************************************************************************/

STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("window",     "back",                 FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("window",     "font",                 FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("window",     "frame",                FV_TYPE_STRING,  "Spec",     "<frame id='50' padding-left='3' padding-top='3' padding-right='5' padding-bottom='5'/>"),
	F_PREFERENCES_ADD("window",     "color-scheme",         FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("window",     "color-scheme-ghost",   FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("filedialog", "back",                 FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("filedialog", "font",                 FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("filedialog", "frame",                FV_TYPE_STRING,  "Spec",     "<frame id='50' padding-left='3' padding-top='3' padding-right='5' padding-bottom='5'/>"),
	F_PREFERENCES_ADD("filedialog", "color-scheme",         FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("filedialog", "color-scheme-ghost",   FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("fontdialog", "back",                 FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("fontdialog", "font",                 FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("fontdialog", "frame",                FV_TYPE_STRING,  "Spec",     "<frame id='50' padding-left='3' padding-top='3' padding-right='5' padding-bottom='5'/>"),
	F_PREFERENCES_ADD("fontdialog", "color-scheme",         FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("fontdialog", "color-scheme-ghost",   FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("progressdialog", "back",                 FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("progressdialog", "font",                 FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("progressdialog", "frame",                FV_TYPE_STRING,  "Spec",     "<frame id='50' padding-left='3' padding-top='3' padding-right='5' padding-bottom='5'/>"),
	F_PREFERENCES_ADD("progressdialog", "color-scheme",         FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("progressdialog", "color-scheme-ghost",   FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("infodialog", "back",                 FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("infodialog", "font",                 FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("infodialog", "frame",                FV_TYPE_STRING,  "Spec",     "<frame id='50' padding-left='3' padding-top='3' padding-right='5' padding-bottom='5'/>"),
	F_PREFERENCES_ADD("infodialog", "color-scheme",         FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("infodialog", "color-scheme-ghost",   FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("window",     "render-complex",   FV_TYPE_BOOLEAN, "Selected", (STRPTR) TRUE),
	F_PREFERENCES_ADD("window",     "refresh-simple",   FV_TYPE_BOOLEAN, "Selected", (STRPTR) FALSE),
		
	F_ARRAY_END
};

extern struct FeelinCatalogEntry Catalog[];

F_METHOD(uint32,Prefs_New)
{
	F_ENTITIES_ARRAY =
	{
		F_ENTITIES_ADD_EMPTY("locale.general"),
		F_ENTITIES_ADD_EMPTY("locale.filedialog"),
		F_ENTITIES_ADD_EMPTY("locale.fontdialog"),
		F_ENTITIES_ADD_EMPTY("locale.render"),
		F_ENTITIES_ADD_EMPTY("locale.refresh"),

		F_ARRAY_END
	};
	
	enum    {
		
			FV_ENTITY_LOCALE_GENERAL,
			FV_ENTITY_LOCALE_FILEDIALOG,
			FV_ENTITY_LOCALE_FONTDIALOG,
			FV_ENTITY_LOCALE_RENDER,
			FV_ENTITY_LOCALE_REFRESH
	
			};
			
	F_ENTITIES_SET(FV_ENTITY_LOCALE_GENERAL,      F_CAT(P_GENERAL));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_FILEDIALOG,   F_CAT(P_FILEDIALOG));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_FONTDIALOG,   F_CAT(P_FONTDIALOG));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_RENDER,       F_CAT(P_RENDER));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_REFRESH,      F_CAT(P_REFRESH));

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, F_CAT(P_TITLE),

		"Script", F_PREFERENCES_PTR,
		"Entities", F_ENTITIES_PTR,
		"Source", "feelin/preference/window.xml",

		TAG_MORE, Msg);
}
#endif
