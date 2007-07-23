/*

$VER: 01.02 (2007/07/05)

	Update to match Thread class modifications

$VER: 01.00 (2005/12/11)

	Starting  from  Feelin  v10,  windows  are  no  longer  handled  by  the
	application  they  belong to, but a new shared object named "WinServer".
	This server serves as an  anchor  for  various  inputs  such  as  window
	events,  mouse  buttons  and  movements, key stroke... but also pophelp,
	timer, wbdrop... it also add special features which  are  not  available
	with  Intuition  such  as  unactivable  windows  (usefull with menus and
	popups).

************************************************************************************************/

#include "Project.h"

#ifdef F_ENABLE_EVENT_KEY

	struct Library                  *KeymapBase;
	
	#ifdef __amigaos4__
	struct KeymapIFace              *IKeymap;
	#endif

	struct Library                  *CxBase;

	#ifdef __amigaos4__
	struct CommoditiesIFace         *ICommodities;
	#endif

#endif

#ifdef F_ENABLE_EVENT_WBDROP

	struct Library                  *WorkbenchBase;

	#ifdef __amigaos4__
	struct WorkbenchIFace           *IWorkbench;
	#endif

#endif

#ifdef F_ENABLE_EVENT_HELP

	#ifdef __MORPHOS__
	struct Library                  *TimerBase;
	#else
	struct Device                   *TimerBase;
	#endif
	
	#ifdef __amigaos4__
	struct TimerIFace               *ITimer;
	#endif
	
#endif

///METHODS
F_METHOD_PROTO(void,WinServer_New);
F_METHOD_PROTO(void,WinServer_Dispose);
F_METHOD_PROTO(void,WinServer_AddMember);
F_METHOD_PROTO(void,WinServer_RemMember);

F_METHOD_PROTO(void,WinServer_ModifyEvents);

#ifdef F_ENABLE_ZONES
F_METHOD_PROTO(void,WinServer_ModifyZone);
#endif
//+

#if defined(F_ENABLE_EVENT_HELP) || defined(F_ENABLE_EVENT_WBDROP) || defined(F_ENABLE_EVENT_KEY)
///Class_New
F_METHOD(uint32,Class_New)
{

#ifdef F_ENABLE_EVENT_WBDROP
	
	WorkbenchBase = IEXEC OpenLibrary("workbench.library",39);
	
	#ifdef __amigaos4__
  
	if (WorkbenchBase)
	{
		if ((IWorkbench = (struct WorkbenchIFace *) IEXEC GetInterface((struct Library *) WorkbenchBase, "main", 1, NULL)) == NULL)
		{
			IEXEC CloseLibrary(WorkbenchBase); WorkbenchBase = NULL;
		}
	}

	#endif
 
	if (!WorkbenchBase)
	{
		IFEELIN F_Log(0,"Unable to open %s v%ld","workbench.library",39);
		
		return NULL;
	}

#endif

		
#ifdef F_ENABLE_EVENT_KEY
					
	KeymapBase = IEXEC OpenLibrary("keymap.library",37);
				
	#ifdef __amigaos4__

	if (KeymapBase)
	{
		if ((IKeymap = (struct KeymapIFace *) IEXEC GetInterface((struct Library *) KeymapBase, "main", 1, NULL)) == NULL)
		{
			IEXEC CloseLibrary(KeymapBase); KeymapBase = NULL;
		}
	}

	#endif
	
	if (!KeymapBase)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to open %s v%ld","keymap.library",37);
		
		return NULL;
	}


	CxBase = IEXEC OpenLibrary("commodities.library",39);
	
	#ifdef __amigaos4__
	
	if (CxBase)
	{
		if ((ICommodities = (struct CxIFace *) IEXEC GetInterface((struct Library *) CxBase, "main", 1, NULL)) == NULL)
		{
			IEXEC CloseLibrary(CxBase); CxBase = NULL;
		}
	}

	#endif
 
	if (!CxBase)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to open %s v%ld","commodities.library",39);
 
		return NULL;
	}
	
#endif

	
#ifdef F_ENABLE_EVENT_HELP
	
	#ifdef __MORPHOS__
	TimerBase = (struct Library *) IEXEC FindName(&FeelinBase->SYS->DeviceList, "timer.device");
	#else
	TimerBase = (struct Device *) IEXEC FindName(&FeelinBase->SYS->DeviceList, "timer.device");
	#endif

	#ifdef __amigaos4__
	
	if (TimerBase)
	{
		if ((ITimer = (struct TimerIFace *) IEXEC GetInterface((struct Library *) TimerBase, "main", 1, NULL)) == NULL)
		{
			TimerBase = NULL;
		}
	}
	
	#endif
	
	if (!TimerBase)
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to find %s","timer.device");

		return NULL;
	}
 
#endif
 
	return F_SUPERDO();
}
//+
///Class_Dispose
F_METHOD(void,Class_Dispose)
{
#ifdef F_ENABLE_EVENT_WBDROP
	
	if (WorkbenchBase)
	{
		#ifdef __amigaos4__
		
		if (IWorkbench)
		{
			IEXEC DropInterface((struct Interface *) IWorkbench); IWorkbench = NULL;
		}
		
		#endif

		IEXEC CloseLibrary(WorkbenchBase); WorkbenchBase = NULL;
	}

#endif
	

#ifdef F_ENABLE_EVENT_KEY
	
	if (KeymapBase)
	{
		#ifdef __amigaos4__

		if (IKeymap)
		{
			IEXEC DropInterface((struct Interface *) IKeymap); IKeymap = NULL;
		}

		#endif

		IEXEC CloseLibrary(KeymapBase); KeymapBase = NULL;
	}
 
	if (CxBase)
	{
		#ifdef __amigaos4__

		if (ICommodities)
		{
			IEXEC DropInterface((struct Interface *) ICommodities); ICommodities = NULL;
		}

		#endif
		
		IEXEC CloseLibrary(CxBase); CxBase = NULL;
	}

#endif

 
#ifdef F_ENABLE_EVENT_HELP

	if (TimerBase)
	{
		#ifdef __amigaos4__

		if (ITimer)
		{
			IEXEC DropInterface((struct Interface *)ITimer); ITimer = NULL;
		}

		#endif

		TimerBase = NULL;
	}

#endif
 
	F_SUPERDO();
}
//+
#endif

F_QUERY()
{
	switch (Which)
	{
#if defined(F_ENABLE_EVENT_HELP) || defined(F_ENABLE_EVENT_WBDROP) || defined(F_ENABLE_EVENT_KEY)
///Meta
		case FV_Query_MetaClassTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_OVERRIDE_STATIC(Class_New,      FM_New),
				F_METHODS_OVERRIDE_STATIC(Class_Dispose,  FM_Dispose),

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
#endif
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("refresh-simple"),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(WinServer_ModifyEvents, "ModifyEvents"),
				#ifdef F_ENABLE_ZONES
				F_METHODS_ADD(WinServer_ModifyZone, "ModifyZone"),
				#endif
				
				F_METHODS_OVERRIDE_STATIC(WinServer_New,         FM_New),
				F_METHODS_OVERRIDE_STATIC(WinServer_Dispose,     FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(WinServer_AddMember,   FM_AddMember),
				F_METHODS_OVERRIDE_STATIC(WinServer_RemMember,   FM_RemMember),

				F_ARRAY_END
			};
			
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Object),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_PROPERTIES,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
