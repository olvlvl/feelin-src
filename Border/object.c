#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Border_New
F_METHOD(uint32,Border_New)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	STRPTR title = NULL;

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif
	F_SAVE_AREA_PUBLIC;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Border_Caption:
		{
			title = (STRPTR)(item.ti_Data);
		}
		break;
	}

	if (title != NULL)
	{
		LOD->caption = IFEELIN F_New(sizeof (struct in_Caption));

		if (LOD->caption == NULL)
		{
			return 0;
		}

		LOD->caption->label = title;
	}

	return F_SUPERDO();
}
//+
///Border_Dispose
F_METHOD(uint32,Border_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->caption != NULL)
	{
		IFEELIN F_Dispose(LOD->caption);

		LOD->caption = NULL;
	}

	return F_SUPERDO();
}
//+
///Border_Get
F_METHOD(uint32,Border_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Border_Caption:
		{
			F_STORE(LOD->caption ? LOD->caption->label : NULL);
		}
		break;
	}

	return F_SUPERDO();
}
//+
