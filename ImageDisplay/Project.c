/*

$VER: 06.00 (2006/07/19)

	Specifications are no longer in XML format. They are now compatible with
	CSS,  with  some extensions to create patterns, gradients and hooks. All
	CSS color definitions are handled as well as all bitmap positions.

	** AUTO ORIENTATION **

	Gradients have been extended to automatically rotate  depending  on  the
	FA_Area_Orientation  attribute of their owner. Thus, a second definition
	for vertically oriented object is no longer required.

	For example, "white black" creates a  gradient  which  starts  with  the
	'white'  color  and ends with the 'black' color. The default orientation
	is horizontal: gradients are drawn from left to right. If an object with
	a  vertical  orientation  draw the gradient, 90 degrees are added to the
	gradient: it  is  drawn  from  top  to  bottom.  To  complete  this,  an
	'anticlockwise'  switch  can  be used to substract 90 degrees instead of
	adding them.

$VER: 05.00 (2005/12/10)
 
	Metaclass support.
	
	*associated data* support.
 
	All specifications have been changed they all are in XML format,  expect
	for colors which are more like CSS items.
   
	Added a third and fourth color (aqua style) to create gradients.
	
	The picture and brush support has been totaly rewritten to use  the  new
	FC_Picture and FC_BitMap classes.
	
	Rasters support completely rewritten: You can use  any  color  type  for
	both pens, 16 different patterns are available.
 
*/

#include "Project.h"

struct ClassUserData *CUD;

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

#ifdef __amigaos4__
	struct FeelinIFace                 *IFeelin;
#endif

///METHODS
F_METHOD_PROTO(void,ID_Dispose);
F_METHOD_PROTO(void,ID_Set);

F_METHOD_PROTO(void,ID_Get);
F_METHOD_PROTO(void,ID_Setup);
F_METHOD_PROTO(void,ID_Cleanup);
F_METHOD_PROTO(void,ID_Draw);
//+

///Class_New
F_METHOD(uint32,Class_New)
{
	CUD = F_LOD(Class,Obj);

	CUD->pool = IFEELIN F_CreatePool(1024,

		FA_Pool_Items, 1,
		FA_Pool_Name, "imagedisplay:data",

		TAG_DONE);

	if (CUD->pool == NULL)
	{
		return NULL;
	}

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
	CUD = F_LOD(Class,Obj);

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

	if (CUD->pool)
	{
		IFEELIN F_DeletePool(CUD->pool);

		CUD->pool = NULL;
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
				F_ATTRIBUTES_ADD_STATIC("Spec",   FV_TYPE_STRING,  FA_ImageDisplay_Spec),
				F_ATTRIBUTES_ADD_STATIC("Width",  FV_TYPE_INTEGER, FA_ImageDisplay_Width),
				F_ATTRIBUTES_ADD_STATIC("Height", FV_TYPE_INTEGER, FA_ImageDisplay_Height),
				F_ATTRIBUTES_ADD_STATIC("Mask",   FV_TYPE_POINTER, FA_ImageDisplay_Mask),
				F_ATTRIBUTES_ADD_STATIC("Origin", FV_TYPE_POINTER, FA_ImageDisplay_Origin),
				F_ATTRIBUTES_ADD_STATIC("Owner",  FV_TYPE_OBJECT,  FA_ImageDisplay_Owner),
			 
				F_ARRAY_END
			};
	
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_OVERRIDE_STATIC(ID_Dispose,    FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(ID_Set,        FM_Set),
				F_METHODS_OVERRIDE_STATIC(ID_Get, 		 FM_Get),

				F_METHODS_ADD_STATIC(ID_Draw,         "Draw",     FM_ImageDisplay_Draw),
				F_METHODS_ADD_STATIC(ID_Setup,        "Setup",    FM_ImageDisplay_Setup),
				F_METHODS_ADD_STATIC(ID_Cleanup,      "Cleanup",  FM_ImageDisplay_Cleanup),
				
				F_ARRAY_END
			};
			
			STATIC F_AUTOS_ARRAY =
			{
				F_AUTOS_ADD("FM_BitMap_Render"),
				F_AUTOS_ADD("FM_BitMap_Blit"),

				F_AUTOS_ADD("FA_BitMap_Width"),
				F_AUTOS_ADD("FA_BitMap_Height"),
				F_AUTOS_ADD("FA_BitMap_ColorType"),
				F_AUTOS_ADD("FA_BitMap_ColorArray"),
				F_AUTOS_ADD("FA_BitMap_ColorCount"),
				
				F_AUTOS_ADD("FA_BitMap_BlitMode"),
				F_AUTOS_ADD("FA_BitMap_Background"),

				#if 0
				F_AUTOS_ADD("FA_BitMap_ScaleFilter"),
				#endif

				F_AUTOS_ADD("FA_Display_Screen"),
				
				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Object),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_AUTOS,
				
				F_ARRAY_END
			};
	
			return F_TAGS_PTR;
		}
//+
   }
   return NULL;
};
