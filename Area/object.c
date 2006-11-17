#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Area_New
F_METHOD(uint32, Area_New)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = Msg, item;

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif

	_area_flags |= (FF_Area_Fillable | FF_Area_Horizontal);

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class))
	switch (item.ti_Tag)
	{
		case FA_Area_Orientation:
		{
			if (item.ti_Data == FV_Area_Orientation_Vertical)
			{
				_area_flags &= ~FF_Area_Horizontal;
			}
			else
			{
				_area_flags |= FF_Area_Horizontal;
			}
		}
		break;
	}

	return F_SUPERDO();
}
//+
///Area_Get
F_METHOD(uint32, Area_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class))
	switch (item.ti_Tag)
	{
		#ifndef F_NEW_GLOBALCONNECT
		
		case FA_Parent:                 F_STORE(_area_parent); break;

		#endif

		case FA_Area_PublicData:        F_STORE(&LOD->public); break;
		case FA_Area_Orientation:       F_STORE(_area_is_horizontal ? FV_Area_Orientation_Horizontal : FV_Area_Orientation_Vertical); break;
		case FA_Area_Fillable:          F_STORE(_area_is_fillable); break;
		case FA_Area_Font:              F_STORE(_area_font); break;
		case FA_Area_Damaged:			F_STORE(_area_is_damaged); break;
	}

	return F_SUPERDO();
}
//+
///Area_Set
F_METHOD(uint32, Area_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class))
	switch (item.ti_Tag)
	{
		case FA_Area_Fillable:
		{
			if (item.ti_Data != 0)
			{
				_area_set_fillable;
			}
			else
			{
				_area_clear_fillable;
			}
		}
		break;

		case FA_Area_Bufferize:
		{
			if (item.ti_Data != 0)
			{
				_area_flags |= FF_Area_Bufferize;
			}
			else
			{
				_area_flags &= ~FF_Area_Bufferize;
			}
		}
		break;

		/*FIXME-060810: the 'Font' attribute is not  handled  correctly.  It
		should override the 'font' property, and not just change the font on
		the fly *if* the object has already been setuped. */

		case FA_Area_Font:
		{
			if (_area_render != NULL)
			{
				if (_area_font != NULL)
				{
					IGRAPHICS CloseFont(_area_font);
				}
			
				_area_font = (struct TextFont *) IFEELIN F_Do(_area_app, FM_Application_OpenFont, Obj, item.ti_Data);
				
				_area_flags |= FF_Area_Damaged;
				
				IFEELIN F_Do(_area_win, FM_Window_RequestRethink, Obj);
			}
		}
		break;

		/*FIXME-060810: I really don't like this attribute,  but  the  Group
		class  really  needs  it  to  damage  its children when its state is
		modified during the 'SetState' method. */

		case FA_Area_Damaged:
		{
			if (item.ti_Data != 0)
			{
				_area_set_damaged;
			}
			else
			{
				_area_clear_damaged;
			}
		}
		break;
   }

   return F_SUPERDO();
}
//+
#ifndef F_NEW_GLOBALCONNECT
///Area_Connect
F_METHODM(bool32, Area_Connect, FS_Connect)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_area_parent = Msg->Parent;

	return F_SUPERDO();
}
//+
///Area_Disconnect
F_METHOD(bool32, Area_Disconnect)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_area_parent = NULL;

	return F_SUPERDO();
}
//+
#endif

