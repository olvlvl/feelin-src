#include "Private.h"

//#define F_ENABLE_SPEC_SAVE

/*************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Image_New
F_METHOD(uint32,Image_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;

	return IFEELIN F_SuperDo(Class, Obj, Method,

		FA_Widget_SetMin, FV_Widget_SetBoth,
		FA_Widget_SetMax, FV_Widget_SetBoth,

		TAG_MORE, Msg);
}
//+
///Image_Dispose
F_METHOD(uint32,Image_Dispose)
{
	#if 0

	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#endif

	return F_SUPERDO();
}
//+
///Image_Set
F_METHOD(uint32,Image_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_SPEC:
		{
			LOD->render_spec = (STRPTR) item.ti_Data;

			/*

			if (LOD->render != NULL)
			{
				IFEELIN F_Set(LOD->render, FA_ImageDisplay_Spec, (uint32) LOD->render_spec);
			}
			else
			{
				LOD->render = ImageDisplayObject,

					FA_ImageDisplay_Spec, LOD->render_spec,
					FA_ImageDisplay_Owner, Obj,
					FA_ImageDisplay_Origin, &_area_content,

					End;

				if (_area_render != NULL)
				{
					if (IFEELIN F_Do(LOD->render, FM_ImageDisplay_Setup, _area_render) == FALSE)
					{
						IFEELIN F_DisposeObj(LOD->render);

						LOD->render = NULL;
					}

					//IFEELIN F_Do(_area_win, FM_Window_RequestRethink, Obj);

					IFEELIN F_Draw(Obj, FF_Draw_Update);
				}
			}
			*/
		}
		break;

		case FV_ATTRIBUTE_ALTSPEC:
		{
			LOD->select_spec = (STRPTR) item.ti_Data;
		}
		break;
	}

	return F_SUPERDO();
}
//+
///Image_Get
F_METHOD(uint32,Image_Get)
{
	#if 0
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_SPEC:
		{
			F_STORE(LOD->image_spec);
		}
		break;
	}
	#endif
	return F_SUPERDO();
}
//+

