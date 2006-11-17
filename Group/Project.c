/*

$VER: 12.00 (2006/05/28)

	Now a subclass of Widget class.

	Style support

$VER: 11.00 (2005/12/10)

	Metaclass support
 
	FA_Group_Rows and FA_Group_Columns are now initial attributes only [I..]

	The FC_Family object is now created *before* passing the  FM_New  method
	to the superclass. Thus, the FM_Set method won't be invoked by FC_Object
	if the family failed to be  created  (preventing  setting  attribute  to
	disposed object, whom pointers have been saved in the LocalObjectData of
	subclasses).
   
	Implements the new *damaged* technique. The FM_RethinkLayout  method  is
	now deprecated.
	
	Replaced both FM_Group_InitChange and FM_Group_ExitChange methods by the
	FA_Group_Quiet attribute.
	
	First steps in virtualization.
	
	FA_Family_DeleteNodeHook is no longer implemented since the F_DisposeP()
	function is deprecated.
	
	Implements the FM_Area_Move method.
	
	The page related stuff has been moved to a standalone class.
 
*/

#include "Project.h"
#include "_locale/table.h"

struct ClassUserData      *CUD;

///METHODS
F_METHOD_PROTO(void,Group_New);
F_METHOD_PROTO(void,Group_Dispose);
F_METHOD_PROTO(void,Group_Get);
F_METHOD_PROTO(void,Group_Set);
F_METHOD_PROTO(void,Group_AddMember);
F_METHOD_PROTO(void,Group_RemMember);

#ifdef F_NEW_GLOBALCONNECT
F_METHOD_PROTO(void,Group_GlobalConnect);
F_METHOD_PROTO(void,Group_GlobalDisconnect);
#endif

F_METHOD_PROTO(void,Group_Setup);
F_METHOD_PROTO(void,Group_Cleanup);
F_METHOD_PROTO(void,Group_Show);
F_METHOD_PROTO(void,Group_Hide);
F_METHOD_PROTO(void,Group_AskMinMax);
F_METHOD_PROTO(void,Group_Layout);
F_METHOD_PROTO(void,Group_Move);
F_METHOD_PROTO(void,Group_Draw);
F_METHOD_PROTO(void,Group_SetState);
F_METHOD_PROTO(void,Group_HandleEvent);
F_METHOD_PROTO(void,Group_BuildContextHelp);
F_METHOD_PROTO(void,Group_Forward);
F_METHOD_PROTO(void,Group_TreeUp);
F_METHOD_PROTO(void,Group_Propagate);

F_METHOD_PROTO(void,Prefs_New);
//+

///code_family_create_node
F_HOOKM(APTR,code_family_create_node,FS_Family_CreateNode)
{
	FAreaPublic *apd = F_GET_AREA_PUBLIC(Msg->Object);
	FWidgetPublic *wpd = F_GET_WIDGET_PUBLIC(Msg->Object);

	#ifdef DB_CREATENODE
	IFEELIN F_Log(0,"create_family_node() Member %s{%08lx}", _object_classname(Msg->Object), Msg->Object);
	#endif

	if ((apd != NULL) && (wpd != NULL))
	{
		FWidgetNode *node = IFEELIN F_NewP(CUD->Pool, sizeof (FWidgetNode));

		if (node != NULL)
		{
			node->AreaPublic = apd;
			node->WidgetPublic = wpd;

			return node;
		}
	}
	else if (Msg->Object != NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "%s{%08lx) is not a subclass of the Widget class", _object_classname(Msg->Object), Msg->Object);
	}

	return NULL;
}
//+

///Class_New
F_METHOD(uint32,Class_New)
{
	CUD = F_LOD(Class,Obj);
	
	CUD->Hook_HLayout.h_Entry = (HOOKFUNC) F_FUNCTION_GATE(code_layout_horizontal);
	CUD->Hook_HLayout.h_Data  = Obj;
	CUD->Hook_VLayout.h_Entry = (HOOKFUNC) F_FUNCTION_GATE(code_layout_vertical);
	CUD->Hook_VLayout.h_Data  = Obj;
	CUD->Hook_ALayout.h_Entry = (HOOKFUNC) F_FUNCTION_GATE(code_layout_array);
	CUD->Hook_ALayout.h_Data  = Obj;
	CUD->Hook_CreateFamilyNode.h_Entry = (HOOKFUNC) F_FUNCTION_GATE(code_family_create_node);

	if ((CUD->Pool = IFEELIN F_CreatePool(1024, FA_Pool_Items,1, FA_Pool_Name,"group-data", TAG_DONE)))
	{
		return F_SUPERDO();
	}

	return 0;
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
	IFEELIN F_DeletePool(CUD->Pool); CUD->Pool = NULL;
   
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
				F_METHODS_ADD_STATIC(Class_New, FM_New),
				F_METHODS_ADD_STATIC(Class_Dispose, FM_Dispose),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Class),
				F_TAGS_ADD(LODSize, sizeof (struct ClassUserData)),
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
				F_ATTRIBUTES_ADD_STATIC("HSpacing",     FV_TYPE_INTEGER, FA_Group_HSpacing),
				F_ATTRIBUTES_ADD_STATIC("VSpacing",     FV_TYPE_INTEGER, FA_Group_VSpacing),
				F_ATTRIBUTES_ADD_STATIC("SameWidth",    FV_TYPE_BOOLEAN, FA_Group_SameWidth),
				F_ATTRIBUTES_ADD_STATIC("SameHeight",   FV_TYPE_BOOLEAN, FA_Group_SameHeight),
				F_ATTRIBUTES_ADD_STATIC("SameSize",     FV_TYPE_BOOLEAN, FA_Group_SameSize),
				F_ATTRIBUTES_ADD_STATIC("RelSizing",    FV_TYPE_BOOLEAN, FA_Group_RelSizing),
				F_ATTRIBUTES_ADD_STATIC("Forward",      FV_TYPE_BOOLEAN, FA_Group_Forward),
				F_ATTRIBUTES_ADD_STATIC("Name",         FV_TYPE_STRING, FA_Group_Name),
				F_ATTRIBUTES_ADD_STATIC("Rows",         FV_TYPE_INTEGER, FA_Group_Rows),
				F_ATTRIBUTES_ADD_STATIC("Columns",      FV_TYPE_INTEGER, FA_Group_Columns),
				F_ATTRIBUTES_ADD_STATIC("LayoutHook",   FV_TYPE_POINTER, FA_Group_LayoutHook),
				F_ATTRIBUTES_ADD_STATIC("MinMaxHook",   FV_TYPE_POINTER, FA_Group_MinMaxHook),
				F_ATTRIBUTES_ADD_STATIC("Virtual",      FV_TYPE_BOOLEAN, FA_Group_Virtual),
				F_ATTRIBUTES_ADD_STATIC("Quiet",        FV_TYPE_BOOLEAN, FA_Group_Quiet),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Group_New,             FM_New),
				F_METHODS_ADD_STATIC(Group_Dispose,         FM_Dispose),
				F_METHODS_ADD_STATIC(Group_Get,             FM_Get),
				F_METHODS_ADD_STATIC(Group_Set,             FM_Set),
				F_METHODS_ADD_STATIC(Group_AddMember,       FM_AddMember),
				F_METHODS_ADD_STATIC(Group_RemMember,       FM_RemMember),

				#ifdef F_NEW_GLOBALCONNECT
				F_METHODS_ADD_STATIC(Group_GlobalConnect,		FM_Element_GlobalConnect),
				F_METHODS_ADD_STATIC(Group_GlobalDisconnect,	FM_Element_GlobalDisconnect),
				#endif

				F_METHODS_ADD_STATIC(Group_Setup,           FM_Element_Setup),
				F_METHODS_ADD_STATIC(Group_Cleanup,         FM_Element_Cleanup),
				F_METHODS_ADD_STATIC(Group_Propagate,       FM_Element_LoadPersistentAttributes),
				F_METHODS_ADD_STATIC(Group_Propagate,       FM_Element_SavePersistentAttributes),

				F_METHODS_ADD_STATIC(Group_AskMinMax,       FM_Area_AskMinMax),
				F_METHODS_ADD_STATIC(Group_Layout,          FM_Area_Layout),

				F_METHODS_ADD_STATIC(Group_Move,            FM_Area_Move),
				F_METHODS_ADD_STATIC(Group_Show,            FM_Area_Show),
				F_METHODS_ADD_STATIC(Group_Hide,            FM_Area_Hide),
				F_METHODS_ADD_STATIC(Group_Draw,            FM_Area_Draw),
		        F_METHODS_ADD_STATIC(Group_SetState,   	    FM_Area_SetState),

				F_METHODS_ADD_STATIC(Group_TreeUp,          FM_Widget_DnDQuery),

				F_METHODS_ADD_BOTH(Group_Forward,           "Forward", FM_Group_Forward),
/*
				F_METHODS_ADD_STATIC(Group_TreeUp,          FM_BuildContextHelp),
				F_METHODS_ADD_STATIC(Group_TreeUp,          FM_BuildContextMenu),
*/
				F_ARRAY_END
			};

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("spacing-horizontal"),
				F_PROPERTIES_ADD("spacing-vertical"),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Widget),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_CATALOG,
				F_TAGS_ADD_PROPERTIES,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
#if 0
///Prefs
		case FV_Query_PrefsTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Prefs_New, FM_New),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(PreferenceGroup),
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
#endif
   }
   return NULL;
};
//+
