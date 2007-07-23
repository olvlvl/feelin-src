/*

$VER: 04.00 (2005/12/29)
 
	Portability update.
   
	Metaclass support.
   
	Some rendering methods introduced e.g. FM_Render_Fill

$VER: 03.01 (2005/01/11)

   Removed useless private memory pool, traces are now allocated  from  the
   default memory pool.

*/

#include "Project.h"

struct ClassUserData               *CUD;

#ifdef F_USE_PICASSO96

	struct Library                     *P96Base;
	#ifdef __amigaos4__
	struct P96IFace                    *IP96;
	#endif

#else

	struct Library                     *CyberGfxBase;
	#ifdef __amigaos4__
	struct CyberGfxIFace               *ICyberGfx;
	#endif

#endif
 
///METHODS
F_METHOD_PROTO(void,Render_New);
F_METHOD_PROTO(void,Render_Get);
F_METHOD_PROTO(void,Render_Set);
F_METHOD_PROTO(void,Render_AddClip);
F_METHOD_PROTO(void,Render_RemClip);
F_METHOD_PROTO(void,Render_AddClipRegion);
F_METHOD_PROTO(void,Render_CreateBuffer);
F_METHOD_PROTO(void,Render_DeleteBuffer);

F_METHOD_PROTO(void,Render_DrawGradient);
F_METHOD_PROTO(void,Render_Fill);
//+

///Class_New
F_METHOD(uint32,Class_New)
{
	CUD = F_LOD(Class,Obj);
	
	RTGFXBASE = IEXEC OpenLibrary(RTGFXLIBNAME,RTGFXLIBVERS);

#ifdef __amigaos4__

	if (RTGFXBASE)
	{
		if (!(RTGFXIFACE = (struct RTGFXIFACESTRUCT *) IEXEC GetInterface((struct Library *) RTGFXBASE,"main",1,NULL)))
		{
			IEXEC CloseLibrary(RTGFXBASE); RTGFXBASE = NULL;
		}
	}

#endif

	IEXEC InitSemaphore(&CUD->arbiter);

	return F_SUPERDO();
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
	CUD = F_LOD(Class,Obj);
  
	if (RTGFXBASE)
	{
		#ifdef __amigaos4__
		if (RTGFXIFACE)
		{
			IEXEC DropInterface((struct Interface *) RTGFXIFACE);

			RTGFXIFACE = NULL;
		}
		#endif
		
		IEXEC CloseLibrary(RTGFXBASE);

		RTGFXBASE = NULL;
	}

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
			   F_METHODS_OVERRIDE_STATIC(Class_New,      FM_New),
			   F_METHODS_OVERRIDE_STATIC(Class_Dispose,  FM_Dispose),

			   F_ARRAY_END
		   };

		   STATIC F_TAGS_ARRAY =
		   {
			   F_TAGS_ADD_SUPER(Class),
			   F_TAGS_ADD(LocalSize, sizeof (struct ClassUserData)),
			   F_TAGS_ADD_METHODS,

			   F_ARRAY_END
		   };

		   return F_TAGS_PTR;
	   }
//+
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD_STATIC("Application", FV_TYPE_OBJECT, FA_Render_Application),
				F_ATTRIBUTES_ADD_STATIC("Display", FV_TYPE_OBJECT, FA_Render_Display),
				F_ATTRIBUTES_ADD_STATIC("Window", FV_TYPE_OBJECT, FA_Render_Window),
				F_ATTRIBUTES_ADD_STATIC("RPort", FV_TYPE_POINTER, FA_Render_RPort),
				F_ATTRIBUTES_ADD_STATIC("Palette", FV_TYPE_POINTER, FA_Render_Palette),
				F_ATTRIBUTES_ADD_STATIC("Friend", FV_TYPE_OBJECT, FA_Render_Friend),
				F_ATTRIBUTES_ADD_STATIC("Forbid", FV_TYPE_BOOLEAN, FA_Render_Forbid),

				F_ARRAY_END
			};

   
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Render_AddClip,        "AddClip",        FM_Render_AddClip),
				F_METHODS_ADD_STATIC(Render_RemClip,        "RemClip",        FM_Render_RemClip),
				F_METHODS_ADD_STATIC(Render_AddClipRegion,  "AddClipRegion",  FM_Render_AddClipRegion),
				F_METHODS_ADD_STATIC(Render_CreateBuffer,   "CreateBuffer",   FM_Render_CreateBuffer),
				F_METHODS_ADD_STATIC(Render_DeleteBuffer,   "DeleteBuffer",   FM_Render_DeleteBuffer),
				F_METHODS_ADD(Render_DrawGradient,   "DrawGradient"),
				F_METHODS_ADD_STATIC(Render_Fill, 			  "Fill", FM_Render_Fill),

				F_METHODS_OVERRIDE_STATIC(Render_New,       FM_New),
				F_METHODS_OVERRIDE_STATIC(Render_Get,       FM_Get),
				F_METHODS_OVERRIDE_STATIC(Render_Set,       FM_Set),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
