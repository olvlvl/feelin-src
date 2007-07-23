/*

$VER: 05.00 (2005/04/06)

	The class is now a subclass of BitMap  class,  which  handle  operations
	such as scaling, tinting, cropping, remapping...

	Whenever possible, we try to  decode  pictures  with  the  corresponding
	Feelin class such as CorePNG for PNG images.

$VER: 04.00 (2005/12/11)
 
	The class has been totaly rewritten. It's  no  longer  an  interface  to
	DataTypes.  The  class  is  able to make bitmap from ARGB pixels arrays;
	scale picture with several methods (nearest, bilinear,  average);  remap
	them with dithering...

$VER: 03.00 (2004/12/07)
 
*/

#include "Project.h"

struct Library                     *DataTypesBase;

#ifdef __amigaos4__
struct DataTypesIFace              *IDataTypes;
#endif

///METHODS
F_METHOD_PROTO(void,Picture_New);
//+

///Class_New
F_METHOD(uint32,Class_New)
{
	DataTypesBase = IEXEC OpenLibrary("datatypes.library", 39);

	#ifdef __amigaos4__

	if (DataTypesBase)
	{
		if (!(IDataTypes = (struct DataTypesIFace *) IEXEC GetInterface((struct Library *) DataTypesBase,"main",1,NULL)))
		{
			IEXEC CloseLibrary(DataTypesBase); DataTypesBase = NULL;
		}
	}

	#endif
	 
	if (DataTypesBase == NULL)
	{
		IFEELIN F_Log(FV_LOG_USER, "Unable to open datatypes.library v%ld", 39);
	}
 
	return F_SUPERDO();
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
	if (DataTypesBase)
	{
		#ifdef __amigaos4__
 
		if (IDataTypes)
		{
			IEXEC DropInterface((struct Interface *) IDataTypes); IDataTypes = NULL;
		}

		#endif

		IEXEC CloseLibrary(DataTypesBase); DataTypesBase = NULL;
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
				F_METHODS_OVERRIDE_STATIC(Class_New, FM_New),
				F_METHODS_OVERRIDE_STATIC(Class_Dispose, FM_Dispose),

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
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD("Name", FV_TYPE_STRING),

				F_ARRAY_END
			};
							  
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_OVERRIDE_STATIC(Picture_New, FM_New),
				
				F_ARRAY_END
			};

			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FA_BitMap_Width"),
				F_RESOLVEDS_ADD("FA_BitMap_Height"),
				F_RESOLVEDS_ADD("FA_BitMap_Depth"),
				F_RESOLVEDS_ADD("FA_BitMap_PixelSize"),
				F_RESOLVEDS_ADD("FA_BitMap_PixelArray"),
				F_RESOLVEDS_ADD("FA_BitMap_PixelArrayMod"),
				F_RESOLVEDS_ADD("FA_BitMap_ColorType"),
				F_RESOLVEDS_ADD("FA_BitMap_ColorArray"),
				F_RESOLVEDS_ADD("FA_BitMap_ColorCount"),
				F_RESOLVEDS_ADD("FM_BitMap_Steal"),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(BitMap),
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_RESOLVEDS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
		break;
//+
   }
   return NULL;
};

