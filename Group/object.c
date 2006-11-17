#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///Group_DoA
/*

   used: Group_Set(), Group_Get(), Group_Setup()

*/

void Group_DoA(FClass *Class,FObject Obj,uint32 Method,APTR Msg)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FWidgetNode *node;

	for (_each)
	{
		IFEELIN F_DoA(node->Widget, Method, Msg);
	}
}
//+

#if 0
///group_virtual_cleanup
void group_virtual_cleanup(FClass *Class, FObject Obj)
{
	//struct LocalObjectData *LOD = F_LOD(Class, Obj);
}
//+
///group_virtual_setup
int32 group_virtual_setup(FClass *Class, FObject Obj)
{
//    struct LocalObjectData *LOD = F_LOD(Class, Obj);
	
 
	return TRUE;
}
//+
///group_virtual_delete
void group_virtual_delete(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (LOD->virtual_data != NULL)
	{
		bool32 rethink = FALSE;

		_flags &= ~FF_Group_Virtual;
 
		if (_area_render != NULL)
		{
			group_virtual_cleanup(Class, Obj);

			rethink = TRUE;
		}

		IFEELIN F_Dispose(LOD->virtual_data);

		LOD->virtual_data = NULL;

		if (rethink == TRUE)
		{
			IFEELIN F_Do(_area_win, FM_Window_RequestRethink, Obj);
		}
	}
}
//+
///group_virtual_create
void group_virtual_create(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	
	if (LOD->virtual_data == NULL)
	{
		if ((LOD->virtual_data = IFEELIN F_NewP(CUD->Pool,sizeof (struct FeelinGroupVirtualData))) != NULL)
		{
			_flags |= FF_Group_Virtual;

			if (_area_render)
			{
				if (group_virtual_setup(Class, Obj))
				{
					IFEELIN F_Do(_area_win, FM_Window_RequestRethink, Obj);
				}
				else
				{
					group_virtual_delete(Class, Obj);
				}
			}
		}
		else
		{
			IFEELIN F_Log(0,"Unable to create VirtualData");
		}
	}
}
//+
#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

F_METHOD_PROTO(void,Group_Propagate);

///Group_New
F_METHOD(uint32,Group_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,*real,item;
	
	bool32 orientation = FV_Area_Orientation_Horizontal;

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif
	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;
					
	LOD->rows = 1;
	LOD->columns = 1;

	_widget_flags |= FF_Widget_Group;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Area_Orientation:
		{
			orientation = item.ti_Data;
		}
		break;

		case FA_Group_HSpacing:
		{
			LOD->hspacing = item.ti_Data;
			LOD->flags |= FF_GROUP_USER_HSPACING;
		}
		break;
		
		case FA_Group_VSpacing:
		{
			LOD->vspacing = item.ti_Data;
			LOD->flags |= FF_GROUP_USER_VSPACING;
		}
		break;
		
		case FA_Group_RelSizing:
		{
			if (item.ti_Data)
			{
				LOD->flags |= FF_GROUP_RELSIZING;
			}
			else
			{
				LOD->flags &= ~FF_GROUP_RELSIZING;
			}
		}
		break;
		
		case FA_Group_SameWidth:
		{
			if (item.ti_Data)
			{
				LOD->flags |= FF_GROUP_SAMEWIDTH;
			}
			else
			{
				LOD->flags &= ~FF_GROUP_SAMEWIDTH;
			}
		}
		break;
		
		case FA_Group_SameHeight:
		{
			if (item.ti_Data)
			{
				LOD->flags |= FF_GROUP_SAMEHEIGHT;;
			}
			else
			{
				LOD->flags &= ~FF_GROUP_SAMEHEIGHT;
			}
		}
		break;
		
		case FA_Group_SameSize:
		{
			if (item.ti_Data)
			{
				LOD->flags |= FF_GROUP_SAMESIZE;
			}
			else
			{
				LOD->flags &= ~FF_GROUP_SAMESIZE;
			}
		}
		break;
		
		case FA_Group_Rows:
		{
			LOD->rows = MAX(item.ti_Data, 1);

			_group_clear_columns;
		}
		break;

		case FA_Group_Columns:
		{
			LOD->columns = MAX(item.ti_Data, 1);

			_group_set_columns;
		}
		break;

		case FA_Group_LayoutHook:
		{
			LOD->LayoutHook = (struct Hook *)(item.ti_Data);
		}
		break;
		
		case FA_Group_MinMaxHook:
		{
			LOD->MinMaxHook = (struct Hook *)(item.ti_Data);
		}
		break;
	}

	if (LOD->LayoutHook == NULL)
	{
		if ((LOD->rows > 1) || (LOD->columns > 1))
		{
			LOD->LayoutHook = &CUD->Hook_ALayout;
		}
		else
		{
			if (orientation == FV_Area_Orientation_Horizontal)
			{
				LOD->LayoutHook = &CUD->Hook_HLayout;
			}
			else
			{
				LOD->LayoutHook = &CUD->Hook_VLayout;
			}
		}
	}

	if (FamilyObject,
		
		FA_Family_Owner, Obj,
		FA_Family_CreateNodeHook, &CUD->Hook_CreateFamilyNode,
  
		TAG_MORE,Msg, End)
	{
		return IFEELIN F_SuperDo(Class, Obj, Method,
			
			FA_Widget_Chainable, FALSE,

			TAG_MORE, Msg);
	}

	Tags = Msg;

	while ((real = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
	{
		if (item.ti_Tag == FA_Child)
		{
			IFEELIN F_DisposeObj((FObject)(item.ti_Data)); real->ti_Tag = TAG_IGNORE; real->ti_Data = NULL;
		}
	}

	return 0;
}
//+
///Group_Dispose
F_METHOD(uint32, Group_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->family)
	{
		IFEELIN F_DisposeObj(LOD->family);

		LOD->family = NULL;
	}

	#ifdef F_GROUP_BUFFERED_REGIONS_ENABLED
	if (LOD->region != NULL)
	{
		IGRAPHICS DisposeRegion(LOD->region);

		LOD->region = NULL;
	}
	#endif

	if (LOD->virtual_data != NULL)
	{
		IFEELIN F_Dispose(LOD->virtual_data);

		LOD->virtual_data = NULL;
	}

	return F_SUPERDO();
}
//+
///Group_Get
F_METHOD(void,Group_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	F_OBJDO(LOD->family);

	if (IUTILITY GetTagData(FA_Group_Forward,FALSE,Tags))
	{
		Group_DoA(Class,Obj,Method,Msg);
	}

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Group_HSpacing:     F_STORE(LOD->hspacing); break;
		case FA_Group_VSpacing:     F_STORE(LOD->vspacing); break;
		case FA_Group_Name:         F_STORE(LOD->name); break;
	}
	F_SUPERDO();
}
//+
///Group_Set
F_METHOD(uint32, Group_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = Msg,item;

	bool32 forward = FALSE;

	while (IFEELIN F_DynamicNTI(&Tags, &item, Class))
	switch (item.ti_Tag)
	{
		case FA_Widget_Disabled:
		{
			FWidgetNode *node;

			for (_each)
			{
				IFEELIN F_Do(node->Widget, Method, item.ti_Tag, item.ti_Data, TAG_DONE);
			}
		}
		break;

		case FA_Group_Name:
		{
			LOD->name = (STRPTR) item.ti_Data;
		}
		break;

		case FA_Group_Forward:
		{
		   forward = item.ti_Data;
		}
		break;
#if 0
		case FA_Group_Virtual:
		{
			if (item.ti_Data)
			{
				group_virtual_create(Class, Obj);
			}
			else
			{
				group_virtual_delete(Class, Obj);
			}
		}
		break;
#endif
		case FA_Group_Quiet:
		{
			if (item.ti_Data)
			{
				LOD->quiet--;
			}
			else
			{
				LOD->quiet++;
			}

			if (LOD->quiet == 0 && _area_render)
			{
				IFEELIN F_Do(_area_win, FM_Window_RequestRethink);
			}
		}
		break;

		case FA_Family:
		{
			LOD->family = (FObject) item.ti_Data;
		}
		break;
	}

	if (forward)
	{
		Group_DoA(Class, Obj, Method, Msg);
	}

	return F_SUPERDO();
}
//+
///Group_AddMember
F_METHODM(bool32,Group_AddMember,FS_AddMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *node = (FWidgetNode *) F_OBJDO(LOD->family);

/* FIXME-0601019

  	Check is the orphan is a subclass of the Widget class.

*/

	if (node == NULL)
	{
		return FALSE;
	}

	LOD->members++;

	#ifdef F_NEW_GLOBALCONNECT

	if (_element_application)
	{
		IFEELIN F_Do(Msg->Orphan, FM_Element_GlobalConnect, _element_application, _element_window);
	}

	#endif

	/* if we are already setup, we also setup our new member,  otherwise  we
	return TRUE. */

	if (_area_render == NULL)
	{
		return TRUE;
	}

	if (IFEELIN F_Do(Msg->Orphan, FM_Element_Setup, _area_render) == FALSE)
	{
		IFEELIN F_Log(FV_LOG_DEV, "setup of member %s{%lx} failed", _object_classname(Msg->Orphan), Msg->Orphan);

		IFEELIN F_Do(Msg->Orphan, FM_Element_Cleanup);
		
		IFEELIN F_Do(LOD->family, FM_RemMember, Msg->Orphan);

		return FALSE;
	}

	/* if we are showable and our new member too, we show it */

	if (_widget_is_showable && _sub_is_showable)
	{
		if (IFEELIN F_Do(Msg->Orphan, FM_Area_Show) == FALSE)
		{
			IFEELIN F_Log(FV_LOG_DEV, "show of member %s{%lx} failed", _object_classname(Msg->Orphan), Msg->Orphan);

			IFEELIN F_Do(Msg->Orphan, FM_Element_Cleanup);
			IFEELIN F_Do(Msg->Orphan, FM_Area_Hide);

			IFEELIN F_Do(LOD->family, FM_RemMember, Msg->Orphan);

			return FALSE;
		}

		if (LOD->quiet >= 0)
		{
			#ifdef DB_ADDMEMBER
			IFEELIN F_Log(0,"Quiet (%ld) >> Request Rethink", LOD->quiet);
			#endif

			IFEELIN F_Do(_area_win, FM_Window_RequestRethink);
		}
	}

	return TRUE;
}
//+
///Group_RemMember
F_METHODM(uint32,Group_RemMember,FS_RemMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 rc;

	if (Msg->Member == NULL)
	{
		return TRUE;
	}

	if (_area_render != NULL)
	{
		IFEELIN F_Do(Msg->Member, FM_Area_Hide);
		IFEELIN F_Do(Msg->Member, FM_Element_Cleanup);

		if (LOD->quiet >= 0)
		{
			#ifdef DB_ADDMEMBER
			IFEELIN F_Log(0,"Quiet (%ld) >> Request Rethink", LOD->quiet);
			#endif

			IFEELIN F_Do(_area_win, FM_Window_RequestRethink);
		}
	}

	/* I don't need to invoke the  'GlobalDisconnect'  method  too,  because
	it's  invoked  automatically  when  the  'Disconnect' method reaches the
	Element class */

	rc = F_OBJDO(LOD->family);

	if (rc != 0)
	{
		LOD->members--;
	}

	return rc;
}
//+

#if 0
///Preferences
STATIC F_PREFERENCES_ARRAY =
{
	#ifdef F_NEW_STYLES

	F_PREFERENCES_ADD("box",    "back",                 FV_TYPE_STRING,  "Spec",     DEF_GROUP_BACK),
	F_PREFERENCES_ADD("box",    "frame",                FV_TYPE_STRING,  "Spec",     DEF_GROUP_FRAME),
	F_PREFERENCES_ADD("box",    "frame-position",       FV_TYPE_INTEGER, "Active",   (STRPTR) DEF_GROUP_POSITION),
	F_PREFERENCES_ADD("box",    "frame-preparse",       FV_TYPE_STRING,  "Contents", DEF_GROUP_PREPARSE),
	F_PREFERENCES_ADD("box",    "frame-font",           FV_TYPE_STRING,  "Contents", DEF_GROUP_FONT),

	#else

	F_PREFERENCES_ADD("group",  "back",                 FV_TYPE_STRING,  "Spec",     DEF_GROUP_BACK),
	F_PREFERENCES_ADD("group",  "frame",                FV_TYPE_STRING,  "Spec",     DEF_GROUP_FRAME),
	F_PREFERENCES_ADD("group",  "frame-position",       FV_TYPE_INTEGER, "Active",   (STRPTR) DEF_GROUP_POSITION),
	F_PREFERENCES_ADD("group",  "frame-preparse",       FV_TYPE_STRING,  "Contents", DEF_GROUP_PREPARSE),
	F_PREFERENCES_ADD("group",  "frame-font",           FV_TYPE_STRING,  "Contents", DEF_GROUP_FONT),

	#endif

	F_PREFERENCES_ADD("group",  "spacing-horizontal",   FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_GROUP_VSPACING),
	F_PREFERENCES_ADD("group",  "spacing-vertical",     FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_GROUP_HSPACING),

	F_PREFERENCES_ADD("preferencegroup", "frame",       FV_TYPE_STRING,  "Spec",    DEF_PREFERENCE_FRAME),
		
	F_ARRAY_END
};

#undef  F_CAT
#define F_CAT(n)                (FCC_CatalogTable[CAT_P_ ## n].String)

F_METHOD(uint32,Prefs_New)
{
	STATIC F_ENTITIES_ARRAY =
	{
		F_ENTITIES_ADD_EMPTY("locale.title"),
		F_ENTITIES_ADD_EMPTY("locale.position"),
		F_ENTITIES_ADD_EMPTY("locale.positions"),
		F_ENTITIES_ADD_EMPTY("locale.preparse"),
		F_ENTITIES_ADD_EMPTY("locale.font"),
		F_ENTITIES_ADD_EMPTY("locale.frames"),
		F_ENTITIES_ADD_EMPTY("locale.normal"),
		F_ENTITIES_ADD_EMPTY("locale.virtual"),
		F_ENTITIES_ADD_EMPTY("locale.settings"),
		F_ENTITIES_ADD_EMPTY("locale.backs"),
		F_ENTITIES_ADD_EMPTY("locale.spacing"),
		F_ENTITIES_ADD_EMPTY("locale.horiz"),
		F_ENTITIES_ADD_EMPTY("locale.verti"),
		
		F_ARRAY_END
	};
	
	enum    {
		
			FV_ENTITY_LOCALE_TITLE,
			FV_ENTITY_LOCALE_POSITION,
			FV_ENTITY_LOCALE_POSITIONS,
			FV_ENTITY_LOCALE_PREPARSE,
			FV_ENTITY_LOCALE_FONT,
			FV_ENTITY_LOCALE_FRAMES,
			FV_ENTITY_LOCALE_NORMAL,
			FV_ENTITY_LOCALE_VIRTUAL,
			FV_ENTITY_LOCALE_SETTINGS,
			FV_ENTITY_LOCALE_BACKS,
			FV_ENTITY_LOCALE_SPACING,
			FV_ENTITY_LOCALE_HORIZ,
			FV_ENTITY_LOCALE_VERTI
	
			};

	F_ENTITIES_SET(FV_ENTITY_LOCALE_TITLE, F_CAT(FTITLE));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_POSITION, F_CAT(POSITION));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_POSITIONS, F_CAT(POSITIONS));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_PREPARSE, F_CAT(PREPARSE));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_FONT, F_CAT(FONT));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_FRAMES, F_CAT(FRAMES));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_NORMAL, F_CAT(NORMAL));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_VIRTUAL, F_CAT(VIRTUAL));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_SETTINGS, F_CAT(SETTINGS));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_BACKS, F_CAT(BACKS));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_SPACING, F_CAT(SPACING));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_HORIZ, F_CAT(HORIZ));
	F_ENTITIES_SET(FV_ENTITY_LOCALE_VERTI, F_CAT(VERTI));

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Group",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/group.xml",
		"Entities", F_ENTITIES_PTR,

	TAG_MORE,Msg);
}
//+
#endif
