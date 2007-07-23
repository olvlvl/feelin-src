/*

$VER: 05.00 (2005/10/15)

	The 'CreateColorScheme' method is removed. The  palette  scheme  is  now
	created by the Area class, since it knows better its requirements.

$VER: 04.00 (2005/12/09)
		
	Metaclass support
 
	New FM_Display_PixelRead and FM_Display_PixelWrite  methods,  which  are
	used to read and write pixel using CyberGfx whenever possible.
   
	The FA_Diplay_Depth attribute was always 8 for HiColors screens.
   
	Color scheme is now XML formated.  Color  scheme  attributes  have  been
	extended to support 'contrast' and 'saturation'.
   
	NEW COLOR TYPES
	---------------
	
		"s:", "p:" and "c:" and now deprecated and replaced with  CSS  style
		ones.
	
		For example, the color scheme color "s:3" is now defined by  "fill".
		The  following  color  scheme  references  are  available : "shine",
		"halfshine", "fill",  "halfshadow",  "shadow",  "halfdark",  "dark",
		"text" and "highlight".
		
		The colormap pen entry "p:1" is now defined by  "1".  The  possible
		values range from -128 to 127.
		
		The RGB value "c:FF00FF" is now defined by "#FF00FF".
   
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
	
/* GOFROMIEL @ HOMBRE: Pourquoi tu le mets ici ? N'est-il pas censé se créer
comme une grand dans <feelin/class_startup.c>
 
#ifdef __amigaos4__
	struct FeelinIFace                 *IFeelin;
#endif

*/

///METHODS
F_METHOD_PROTO(void,D_New);
F_METHOD_PROTO(void,D_Dispose);
F_METHOD_PROTO(void,D_Get);
F_METHOD_PROTO(void,D_Set);
F_METHOD_PROTO(void,D_Find);
F_METHOD_PROTO(void,D_Create);
F_METHOD_PROTO(void,D_Delete);
F_METHOD_PROTO(void,D_AddMember);
F_METHOD_PROTO(void,D_RemMember);
F_METHOD_PROTO(void,D_AddColor);
F_METHOD_PROTO(void,D_RemColor);
F_METHOD_PROTO(void,D_AddPalette);
F_METHOD_PROTO(void,D_RemPalette);
F_METHOD_PROTO(void,D_CreateColor);
F_METHOD_PROTO(void,D_PixelRead);
F_METHOD_PROTO(void,D_PixelWrite);
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

	CUD->colors_pool = IFEELIN F_CreatePool(sizeof (struct in_Color),

	   FA_Pool_Name,"display-color",

	   TAG_DONE);

	if (CUD->colors_pool == NULL)
	{
		return NULL;
	}

	CUD->palettes_pool = IFEELIN F_CreatePool(1024,

	   FA_Pool_Items,  1,
	   FA_Pool_Name,   "display-palette",

	   TAG_DONE);

	if (CUD->palettes_pool == NULL)
	{
		return NULL;
	}

	CUD->Semaphore = IFEELIN F_NewObj(FC_Object,
		  
	   //FA_Semaphore_Name,"Display.Semaphore",
			 
	   End;

	if (CUD->Semaphore == NULL)
	{
		return NULL;
	}
			  
	CUD->document = XMLDocumentObject, End;

	if (CUD->document == NULL)
	{
		return NULL;
	}

	CUD->document_source_id = IFEELIN F_DynamicFindID("FA_Document_Source");
	CUD->document_sourcetype_id = IFEELIN F_DynamicFindID("FA_Document_SourceType");
	CUD->document_markups_id = IFEELIN F_DynamicFindID("FA_XMLDocument_Markups");

	return F_SUPERDO();
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
	CUD = F_LOD(Class,Obj);

	// graphics library
 
	if (RTGFXBASE)
	{
		#ifdef __amigaos4__

		if (RTGFXIFACE)
		{
			IEXEC DropInterface((struct Interface *) RTGFXIFACE); RTGFXIFACE = NULL;
		}

		#endif

		IEXEC CloseLibrary(RTGFXBASE); RTGFXBASE = NULL;
	}

	IFEELIN F_DisposeObj(CUD->document); CUD->document = NULL;
	IFEELIN F_DisposeObj(CUD->Semaphore); CUD->Semaphore = NULL;
	IFEELIN F_DeletePool(CUD->palettes_pool); CUD->palettes_pool = NULL;
	IFEELIN F_DeletePool(CUD->colors_pool); CUD->colors_pool = NULL;
	
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
				F_ATTRIBUTES_ADD("Background",  FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("ColorMap",    FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Depth",       FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("BitMap",      FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("DrawInfo",    FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("Height",      FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Name",        FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("Screen",      FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("ScreenMode",  FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Spec",        FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("Title",       FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("Width",       FV_TYPE_INTEGER),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(D_Find,                "Find"),
				F_METHODS_ADD(D_Create,              "Create"),
				F_METHODS_ADD(D_Delete,              "Delete"),
				
				F_METHODS_ADD_STATIC(D_AddColor,            "AddColor",          FM_Display_AddColor),
				F_METHODS_ADD_STATIC(D_RemColor,            "RemColor",          FM_Display_RemColor),
				F_METHODS_ADD_STATIC(D_AddPalette,          "AddPalette",        FM_Display_AddPalette),
				F_METHODS_ADD_STATIC(D_RemPalette,          "RemPalette",        FM_Display_RemPalette),
				F_METHODS_ADD_STATIC(D_CreateColor,         "CreateColor",       FM_Display_CreateColor),
				
				F_METHODS_ADD(D_PixelRead,    "PixelRead"),
				F_METHODS_ADD(D_PixelWrite,   "PixelWrite"),
				
				F_METHODS_OVERRIDE_STATIC(D_New,        FM_New),
				F_METHODS_OVERRIDE_STATIC(D_Dispose,    FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(D_Get,        FM_Get),
				F_METHODS_OVERRIDE_STATIC(D_AddMember,  FM_AddMember),
				F_METHODS_OVERRIDE_STATIC(D_RemMember,  FM_RemMember),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
			   F_TAGS_ADD_SUPER(Object),
			   F_TAGS_ADD_LOD,
			   F_TAGS_ADD_METHODS,
			   F_TAGS_ADD_ATTRIBUTES,

			   F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	
	return NULL;
}
