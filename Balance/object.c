#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Balance_New
F_METHOD(uint32,Balance_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif
	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;

	return F_SUPERDO();
}
//+
///Balance_Set
F_METHOD(uint32,Balance_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_QUICKDRAW:
		{
			if (item.ti_Data != 0)
			{
				_balance_set_quickdraw;
			}
			else
			{
				_balance_clear_quickdraw;
			}
		}
		break;
	}

	return F_SUPERDO();
}
//+

