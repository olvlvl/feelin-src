#include "Private.h"

#if 0
///group_is_object_visible
int32 group_is_object_visible(FClass *Class, FObject Obj, FWidgetNode *node)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
					
	if ((_area_render != NULL) && (node != NULL))
	{
		FObject win = _area_win;
		FObject par = node->Widget;

		int32 x = _sub_x;
		int32 y = _sub_y;
 
		while ((par = (FObject) IFEELIN F_Get(par, FA_Element_Parent)) != NULL)
		{
			if (par == win)
			{
				break;
			}
			else
			{
				FAreaPublic *pub = (FAreaPublic *) IFEELIN F_Get(par, FA_Area_PublicData);
				
				if (x < (pub->Box.x + pub->Padding.l) ||
					x > (pub->Box.x + pub->Box.w - 1 - pub->Padding.l - pub->Padding.r) ||
					y < (pub->Box.y + pub->Padding.t) ||
					y > (pub->Box.y + pub->Box.h - 1 - pub->Padding.t - pub->Padding.b))
				{
					return FALSE;
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}
//+
#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Group_Show
F_METHOD(bool32,Group_Show)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *node;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	#if 0
///
	if (LOD->virtual_data)
	{
		for (_each)
		{
			if (_sub_is_showable)
			{
				if (group_is_object_visible(Class, Obj, node))
				{
					IFEELIN F_Do(node->Object,FM_Show);
				}
				else
				{
					IFEELIN F_Log(0,"%s{%08lx} is not visible",_object_classname(node->Object),node->Object);
				}
			}
		}
	}
	else
//+
	#endif
	{
		for (_each)
		{
			if (_sub_is_showable)
			{
				if (F_OBJDO(node->Widget) == FALSE)
				{
					IFEELIN F_Log(FV_LOG_DEV, "Widget %s{%08lx} refuses to show", _object_classname(node->Widget), node->Widget);

					return FALSE;
				}
			}
		}
	}

	return TRUE;
}
//+
///Group_Hide
F_METHOD(bool32,Group_Hide)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *node;

	for (_each)
	{
		if (_sub_is_showable)
		{
			F_OBJDO(node->Widget);
		}
	}

	return F_SUPERDO();
}
//+
///Group_Move
F_METHODM(void,Group_Move,FS_Area_Move)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_sub_declare_all;
	
	if (FF_Move_Relative & Msg->Flags)
	{
		F_SUPERDO();

		for (_each)
		{
			F_OBJDO(node->Widget);
		}
	}
	else
	{
		int32 relative_x = Msg->x - _area_x;
		int32 relative_y = Msg->y - _area_y;
		
		F_SUPERDO();

		for (_each)
		{
			IFEELIN F_Do(node->Widget, Method, relative_x, relative_y, FF_Move_Relative);
		}
	}
}
//+
///Group_SetState
F_METHOD(uint32,Group_SetState)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_sub_declare_all;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	/* damage all children to force them the redraw */

	for (_each)
	{
		#if 1

		IFEELIN F_Do(node->Widget, FM_Set,

			FA_Area_Damaged, TRUE,
			FA_Group_Forward, TRUE,

			TAG_DONE);

		#else

		IFEELIN F_Do(node->Widget, Method, node->AreaPublic->State);

		#endif
	}

	return TRUE;
}
//+

