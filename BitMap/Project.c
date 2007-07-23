/*
							
$VER: 01.00 (2005/12/08)

	The FA_BitMap_ColorArray attribute can now be used during FM_BitMap_Blit
	to  replace  bitmap's color array. This attribute is only effective with
	FV_COLOR_TYPE_PALETTE bitmaps.


** TODO **

	Ajouter  toute  une  chaîne  de  rendu,  redimensionnage,  tinte,  type,
	remapage... ne jamais modifier la source.

	La chaine de processing  est  créer  avec  la  méthode  'AddProcessing'.
	Ainsi,  on  peut  ajouter  n'importe  quelle méthode dans n'importe quel
	ordre.

	La méthode 'Render' est appellée la dernière pour créer un nouvel  objet
	BitMap.

   
*/

#include "Project.h"

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
F_METHOD_PROTO(void,BitMap_Blit);
F_METHOD_PROTO(void,BitMap_Clone);
F_METHOD_PROTO(void,BitMap_Steal);
F_METHOD_PROTO(void,BitMap_Render);
F_METHOD_PROTO(void,BitMap_AddProcessing);
F_METHOD_PROTO(void,BitMap_RemProcessing);

F_METHOD_PROTO(void,BitMap_New);
F_METHOD_PROTO(void,BitMap_Dispose);
F_METHOD_PROTO(void,BitMap_Get);
//+

///Class_New
F_METHOD(uint32,Class_New)
{
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
   
	return F_SUPERDO();
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
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
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(BlitMode) =
			{
				F_VALUES_ADD("Copy", FV_BitMap_BlitMode_Copy),
				F_VALUES_ADD("Tile", FV_BitMap_BlitMode_Tile),
				F_VALUES_ADD("Scale", FV_BitMap_BlitMode_Scale),
				F_VALUES_ADD("Frame", FV_BitMap_BlitMode_Frame),
				
				F_ARRAY_END
			};
			
			#if 0

			STATIC F_VALUES_ARRAY(ScaleFilter) =
			{
				F_VALUES_ADD("Nearest",     FV_BitMap_ScaleFilter_Nearest),
				F_VALUES_ADD("Bilinear",    FV_BitMap_ScaleFilter_Bilinear),
				F_VALUES_ADD("Average",     FV_BitMap_ScaleFilter_Average),

				F_ARRAY_END
			};

			#endif

			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD("Width",               FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Height",              FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("Depth",               FV_TYPE_INTEGER),
				
				F_ATTRIBUTES_ADD("PixelSize",           FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("PixelArray",          FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("PixelArrayMod",       FV_TYPE_POINTER),

				F_ATTRIBUTES_ADD("ColorType",           FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("ColorCount",          FV_TYPE_INTEGER),
				F_ATTRIBUTES_ADD("ColorArray",          FV_TYPE_INTEGER),

				F_ATTRIBUTES_ADD_VALUES("BlitMode",     FV_TYPE_INTEGER, BlitMode),
				F_ATTRIBUTES_ADD("Background",          FV_TYPE_INTEGER),

				#if 0
				
				F_ATTRIBUTES_ADD("Pens", FV_TYPE_POINTER),
				F_ATTRIBUTES_ADD("NumPens", FV_TYPE_INTEGER),
				
				F_ATTRIBUTES_ADD_VALUES("ScaleFilter", FV_TYPE_INTEGER, ScaleFilter),
 
				F_ATTRIBUTES_ADD("Margins", FV_TYPE_POINTER),

				#endif

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(BitMap_Blit,              "Blit"),
				F_METHODS_ADD(BitMap_Clone,             "Clone"),
				F_METHODS_ADD(BitMap_Steal,             "Steal"),
				F_METHODS_ADD(BitMap_Render,            "Render"),
				F_METHODS_ADD(BitMap_AddProcessing,     "AddProcessing"),
				F_METHODS_ADD(BitMap_RemProcessing,     "RemProcessing"),
				
				F_METHODS_OVERRIDE_STATIC(BitMap_New,         FM_New),
				F_METHODS_OVERRIDE_STATIC(BitMap_Dispose,     FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(BitMap_Get,         FM_Get),
				
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
