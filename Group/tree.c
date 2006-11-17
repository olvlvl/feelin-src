#include "Private.h"

struct FS_Group_Tree                            { int32 MouseX, MouseY; };
struct FS_Group_Forward                         { uint32 Method; APTR MsgStart; };

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///Group_TreeUp
F_METHODM(uint32,Group_TreeUp,FS_Group_Tree)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *node;
	
	if (Msg->MouseX >= _area_x && Msg->MouseX <= _area_x2 &&
		Msg->MouseY >= _area_y && Msg->MouseY <= _area_y2)
	{
		uint32 rc;
		
		for (_each)
		{
			if (Msg->MouseX >= _sub_x && Msg->MouseX <= _sub_x2 &&
				Msg->MouseY >= _sub_y && Msg->MouseY <= _sub_y2)
			{
				rc = F_OBJDO(node->Widget);

				if (rc)
				{
					return rc;
				}
			}
		}
		return F_SUPERDO();
	}
	return 0;
}
//+
///Group_Forward
F_METHODM(uint32,Group_Forward,FS_Group_Forward)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_sub_declare_all;

	for (_each)
	{
		if (_sub_is_group)
		{
			IFEELIN F_DoA(node->Widget, Method, Msg);
		}
		else
		{
			IFEELIN F_DoA(node->Widget, Msg->Method, &Msg->MsgStart);
		}
	}

	return IFEELIN F_DoA(Obj,Msg->Method,&Msg->MsgStart);
}
//+
///Group_Propagate
F_METHOD(uint32,Group_Propagate)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_sub_declare_all;

	for (_each)
	{
		F_OBJDO(node->Widget);
	}
	
	return F_SUPERDO();
}
//+

