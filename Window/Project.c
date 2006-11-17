/*

$VER: 13.00 (2006/05/27)

	FEventHandler is now deprecated, the structure is no longer provided  to
	the  'AddEventHandler'  method.  Instead,  a taglist must be provided to
	create an  abstract  'handler'.  This  behaviour  has  been  chosen  for
	security purpose because a provided structure could easily be corrupted,
	corrupting the whole handlers list. More over, the 'real' pointer is not
	returned, thus the handler cannot be disposed by mistake.

	Style support.

$VER: 12.00 (2006/01/03)

	Many attributes (e.g. FA_Left...) and methods  (e.g.  FM_Setup...)  were
	moved  from FC_Area to FC_Frame, the class inherit them now. Many former
	FC_Window's attributes and methods are now deprecated.
															
	By default, windows are opened with  their  minimal  width  and  height,
	centered  in  the  screen. Windows with a given ID are able to load/save
	their  previous/current  position  and  dimensions  as  well  as   their
	alternatives position and dimensions.
	
	The window positionning has been extended in many  ways.  The  developer
	can  use  the FA_Left, FA_Top, FA_Area_Width and FA_Area_Height as usual to open a
	window at a specific position with the specific dimensions.
	
	Windows are now handled by the new shared object 'WinServer', which adds
	many new features unavailable with intuition.
	
	Added the FA_Window_Activable attribute, if  the  attribute  is  set  to
	FALSE  the  window  cannot  be made active when you click on it. This is
	very usefull for menus or popups. By default the attribute is TRUE.

*/

#include "Project.h"
#include "_locale/table.h"
												
FObject                             WinServer;

///METHODS
F_METHOD_PROTO(void,Window_New);
F_METHOD_PROTO(void,Window_Dispose);
F_METHOD_PROTO(void,Window_Get);
F_METHOD_PROTO(void,Window_Set);
F_METHOD_PROTO(void,Window_AddMember);
F_METHOD_PROTO(void,Window_RemMember);

#ifdef F_NEW_GLOBALCONNECT
F_METHOD_PROTO(void,Window_GlobalConnect);
F_METHOD_PROTO(void,Window_GlobalDisconnect);
#endif
F_METHOD_PROTO(void,Window_Setup);
F_METHOD_PROTO(void,Window_Cleanup);
F_METHOD_PROTO(void,Window_LoadPersistentAttributes);
F_METHOD_PROTO(void,Window_SavePersistentAttributes);

F_METHOD_PROTO(void,Window_AskMinMax);
F_METHOD_PROTO(void,Window_Layout);
F_METHOD_PROTO(void,Window_Show);
F_METHOD_PROTO(void,Window_Hide);
F_METHOD_PROTO(void,Window_Draw);

F_METHOD_PROTO(void,Window_Open);
F_METHOD_PROTO(void,Window_Close);
F_METHOD_PROTO(void,Window_CreateEventHandler);
F_METHOD_PROTO(void,Window_DeleteEventHandler);
F_METHOD_PROTO(void,Window_AddChainable);
F_METHOD_PROTO(void,Window_RemChainable);
F_METHOD_PROTO(void,Window_DispatchEvent);
F_METHOD_PROTO(void,Window_RequestRethink);
F_METHOD_PROTO(void,Window_Zoom);
F_METHOD_PROTO(void,Window_Depth);

F_METHOD_PROTO(void,Prefs_New);
//+

///Class_New
F_METHOD(uint32,Class_New)
{
	if ((WinServer = IFEELIN F_SharedOpen("WinServer")) != NULL)
	{
		return F_SUPERDO();
	}
 
	return 0;
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
	IFEELIN F_SharedClose(WinServer); WinServer = NULL;

	return F_SUPERDO();
}
//+

F_QUERY()
{
	switch (Which)
	{
///Meta
		case FV_Query_MetaClassTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Class_New,      FM_New),
				F_METHODS_ADD_STATIC(Class_Dispose,  FM_Dispose),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Class),
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(ActiveObject) =
			{
				F_VALUES_ADD("prev",  FV_Window_ActiveObject_Prev),
				F_VALUES_ADD("next",  FV_Window_ActiveObject_Next),
				F_VALUES_ADD("none",  FV_Window_ActiveObject_None),

				F_ARRAY_END
			};
			
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_STATIC("System",         FV_TYPE_POINTER, FA_Window_System),
				F_ATTRIBUTES_ADD_STATIC("Left",				FV_TYPE_STRING, FA_Window_Left),
				F_ATTRIBUTES_ADD_STATIC("Right",			FV_TYPE_STRING, FA_Window_Right),
				F_ATTRIBUTES_ADD_STATIC("Top",				FV_TYPE_STRING, FA_Window_Top),
				F_ATTRIBUTES_ADD_STATIC("Bottom",			FV_TYPE_STRING, FA_Window_Bottom),
				F_ATTRIBUTES_ADD_STATIC("Width",			FV_TYPE_STRING, FA_Window_Width),
				F_ATTRIBUTES_ADD_STATIC("Height",			FV_TYPE_STRING, FA_Window_Height),
				F_ATTRIBUTES_ADD_STATIC("Title",          FV_TYPE_STRING,  FA_Window_Title),
				F_ATTRIBUTES_ADD_STATIC("ScreenTitle",    FV_TYPE_STRING,  FA_Window_ScreenTitle),
				F_ATTRIBUTES_ADD_STATIC("Open",           FV_TYPE_BOOLEAN, FA_Window_Open),
				F_ATTRIBUTES_ADD_STATIC("CloseRequest",   FV_TYPE_BOOLEAN, FA_Window_CloseRequest),
				F_ATTRIBUTES_ADD_STATIC("Active",         FV_TYPE_BOOLEAN, FA_Window_Active),
				F_ATTRIBUTES_ADD_STATIC_VALUES("ActiveObject",   FV_TYPE_OBJECT,  FA_Window_ActiveObject, ActiveObject),
				F_ATTRIBUTES_ADD_STATIC("Backdrop",       FV_TYPE_OBJECT,  FA_Window_Backdrop),
				F_ATTRIBUTES_ADD_STATIC("Borderless",     FV_TYPE_BOOLEAN, FA_Window_Borderless),
				F_ATTRIBUTES_ADD_STATIC("Resizable",      FV_TYPE_BOOLEAN, FA_Window_Resizable),
				
				F_ATTRIBUTES_ADD_STATIC("GadNone",        FV_TYPE_BOOLEAN, FA_Window_GadNone),
				F_ATTRIBUTES_ADD_STATIC("GadDragbar",     FV_TYPE_BOOLEAN, FA_Window_GadDragbar),
				F_ATTRIBUTES_ADD_STATIC("GadClose",       FV_TYPE_BOOLEAN, FA_Window_GadClose),
				F_ATTRIBUTES_ADD_STATIC("GadDepth",       FV_TYPE_BOOLEAN, FA_Window_GadDepth),
				F_ATTRIBUTES_ADD_STATIC("GadIconify",     FV_TYPE_BOOLEAN, FA_Window_GadIconify),
				F_ATTRIBUTES_ADD_STATIC("Events",         FV_TYPE_INTEGER, FA_Window_Events),
				F_ATTRIBUTES_ADD_STATIC("Activable",      FV_TYPE_BOOLEAN, FA_Window_Activable),

				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("render-complex"),
				F_PROPERTIES_ADD("refresh-simple"),
				F_PROPERTIES_ADD("decorator"),

				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD("left"),
				F_ATOMS_ADD("top"),
				F_ATOMS_ADD("width"),
				F_ATOMS_ADD("height"),
				F_ATOMS_ADD("box"),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Window_New,               FM_New),
				F_METHODS_ADD_STATIC(Window_Dispose,           FM_Dispose),
				F_METHODS_ADD_STATIC(Window_Get,               FM_Get),
				F_METHODS_ADD_STATIC(Window_Set,               FM_Set),
				F_METHODS_ADD_STATIC(Window_AddMember,         FM_AddMember),
				F_METHODS_ADD_STATIC(Window_RemMember,         FM_RemMember),

				#ifdef F_NEW_GLOBALCONNECT
				F_METHODS_ADD_STATIC(Window_GlobalConnect,		FM_Element_GlobalConnect),
				F_METHODS_ADD_STATIC(Window_GlobalDisconnect,	FM_Element_GlobalDisconnect),
				#endif
				
				F_METHODS_ADD_STATIC(Window_Setup,              FM_Element_Setup),
				F_METHODS_ADD_STATIC(Window_Cleanup,            FM_Element_Cleanup),
				F_METHODS_ADD_STATIC(Window_LoadPersistentAttributes, FM_Element_LoadPersistentAttributes),
				F_METHODS_ADD_STATIC(Window_SavePersistentAttributes, FM_Element_SavePersistentAttributes),

				F_METHODS_ADD_STATIC(Window_AskMinMax,          FM_Area_AskMinMax),
				F_METHODS_ADD_STATIC(Window_Layout,             FM_Area_Layout),
				F_METHODS_ADD_STATIC(Window_Show,               FM_Area_Show),
				F_METHODS_ADD_STATIC(Window_Hide,               FM_Area_Hide),
				F_METHODS_ADD_STATIC(Window_Draw,               FM_Area_Draw),
				
				F_METHODS_ADD_BOTH(Window_Open,             	"Open",             	FM_Window_Open),
				F_METHODS_ADD_BOTH(Window_Close,            	"Close",            	FM_Window_Close),
				F_METHODS_ADD_BOTH(Window_CreateEventHandler,  	"CreateEventHandler",  	FM_Window_CreateEventHandler),
				F_METHODS_ADD_BOTH(Window_DeleteEventHandler,	"DeleteEventHandler",  	FM_Window_DeleteEventHandler),
				F_METHODS_ADD_BOTH(Window_AddChainable,     	"AddChainable",     	FM_Window_AddChainable),
				F_METHODS_ADD_BOTH(Window_RemChainable,     	"RemChainable",     	FM_Window_RemChainable),
				F_METHODS_ADD_BOTH(Window_DispatchEvent,    	"DispatchEvent",    	FM_Window_DispatchEvent),
				F_METHODS_ADD_BOTH(Window_RequestRethink,   	"RequestRethink",   	FM_Window_RequestRethink),
				F_METHODS_ADD_BOTH(Window_Zoom,             	"Zoom",             	FM_Window_Zoom),
				F_METHODS_ADD_BOTH(Window_Depth,            	"Depth",            	FM_Window_Depth),
				
				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Border),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_CATALOG,
				F_TAGS_ADD_ATOMS,
				F_TAGS_ADD_PROPERTIES,
				
				F_ARRAY_END
			};
	
			return F_TAGS_PTR;
		}
//+
#if 0
///Prefs
		case FV_Query_PrefsTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Prefs_New, FM_New),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(PreferenceGroup),
				F_TAGS_ADD_METHODS,
/*
				F_TAGS_ADD_CATALOG,
				F_TAGS_ADD_CATALOGNAME(FC_Window),
*/
				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
#endif
	}
	return NULL;
};
