#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Page_New
F_METHOD(uint32,Page_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif
	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;
					
	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Area_Fillable,       FALSE,
		FA_Widget_Chainable,    TRUE,
		FA_Group_MinMaxHook,    &CUD->minmax_hook,
		FA_Group_LayoutHook,    &CUD->layout_hook,

		TAG_MORE, Msg);
}
//+
///Page_Get
F_METHOD(uint32,Page_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ACTIVE:
		{
			F_STORE(LOD->node_active ? LOD->node_active->Widget : NULL);
		}
		break;
	}
	
	return F_SUPERDO();
}
//+
///Page_Set
F_METHOD(uint32,Page_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ACTIVE:
		{
			FWidgetNode *node = LOD->node_active;

			switch (item.ti_Data)
			{
				case FV_Page_Active_Prev:
				{
					if (node)
					{
						node = node->Prev;
					}

					if (!node)
					{
						node = (FWidgetNode *) IFEELIN F_Get(Obj, FA_Family_Tail);
					}
				}
				break;

				case FV_Page_Active_Next:
				{
					if (node)
					{
						node = node->Next;
					}

					if (!node)
					{
						node = (FWidgetNode *) IFEELIN F_Get(Obj, FA_Family_Head);
					}
				}
				break;

				case FV_Page_Active_Last:
				{
					node = (FWidgetNode *) IFEELIN F_Get(Obj,FA_Family_Tail);
				}
				break;

				case FV_Page_Active_First:
				{
					node = (FWidgetNode *) IFEELIN F_Get(Obj,FA_Family_Head);
				}
				break;

				default:
				{
					if ((int32) item.ti_Data > 0xFFFF)
					{
						for (node = (FWidgetNode *) IFEELIN F_Get(Obj,FA_Family_Head) ; node ; node = node->Next)
						{
							if (node->Widget == (FObject) item.ti_Data)
							{
								break;
							}
						}
					}
					else
					{
						uint32 i;

						node = (FWidgetNode *) IFEELIN F_Get(Obj,FA_Family_Head);

						for (i = 0 ; i < item.ti_Data ; i++)
						{
							if (!node->Next)
							{
								break;
							}
							else
							{
								node = node->Next;
							}
						}
					}
				}
				break;
			}
		 
			if ((node != NULL) && (node != LOD->node_active))
			{
				if (LOD->node_active != NULL)
				{
					IFEELIN F_Do(LOD->node_active->Widget, FM_Area_Hide);
				}

				LOD->node_update = LOD->node_active;
				LOD->node_active = node;

				if (LOD->node_active != NULL)
				{
					IFEELIN F_Do(LOD->node_active->Widget, FM_Area_Show);
				}
				
				_area_set_damaged;

				if (_area_render != NULL)
				{
					IFEELIN F_Draw(Obj, FF_Draw_Update);
				}
			}
		}
		break;
	}

	return F_SUPERDO();
}
//+

///Preferences
STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("page",   "padding-left",             FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_PAGE_PADDING_LEFT   ),
	F_PREFERENCES_ADD("page",   "padding-right",            FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_PAGE_PADDING_RIGHT  ),
	F_PREFERENCES_ADD("page",   "padding-top",              FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_PAGE_PADDING_TOP    ),
	F_PREFERENCES_ADD("page",   "padding-bottom",           FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_PAGE_PADDING_BOTTOM ),

	F_PREFERENCES_ADD("page",   "back",                     FV_TYPE_STRING,  "Spec",     NULL ),
	F_PREFERENCES_ADD("page",   "tab-font",                 FV_TYPE_STRING,  "Contents", NULL ),
	F_PREFERENCES_ADD("page",   "tab-back-inactive",        FV_TYPE_STRING,  "Spec",     NULL ),
	F_PREFERENCES_ADD("page",   "tab-back-active",          FV_TYPE_STRING,  "Spec",     NULL ),
	F_PREFERENCES_ADD("page",   "preparse-inactive",        FV_TYPE_STRING,  "Contents", NULL ),
	F_PREFERENCES_ADD("page",   "preparse-active",          FV_TYPE_STRING,  "Contents", NULL ),
	F_PREFERENCES_ADD("page",   "active-padding-top",       FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_TAB_PADDING_TOP     ),
	F_PREFERENCES_ADD("page",   "active-padding-bottom",    FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_TAB_PADDING_BOTTOM  ),
	F_PREFERENCES_ADD("page",   "inactive-padding-top",     FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_ITAB_PADDING_TOP    ),
	F_PREFERENCES_ADD("page",   "inactive-padding-bottom",  FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_ITAB_PADDING_BOTTOM ),
	F_PREFERENCES_ADD("page",   "inactive-margin-top",      FV_TYPE_INTEGER, "Value",    (STRPTR) DEF_ITAB_MARGIN ),

	F_ARRAY_END
};

F_METHOD(FObject,Prefs_New)
{
	STATIC F_ENTITIES_ARRAY =
	{
		F_ENTITIES_ADD("locale.font",           "Font"),
		F_ENTITIES_ADD("locale.active",         "Active Tabs"),
		F_ENTITIES_ADD("locale.active.help",    "Preferences for the active tab"),
		F_ENTITIES_ADD("locale.inactive",       "Inactive Tabs"),
		F_ENTITIES_ADD("locale.inactive.help",  "Preferences for the inactive tab"),
		F_ENTITIES_ADD("locale.preparse",       "PreParse"),
		F_ENTITIES_ADD("locale.top",            "Top"),
		F_ENTITIES_ADD("locale.bottom",         "Bottom"),
		F_ENTITIES_ADD("locale.topmarg",        "Top Marg."),
		F_ENTITIES_ADD("locale.active.paddings.help",    "<align=center><b>Top</b> is the number of pixels between the top border and the text.<br><b>Bottom</b> is the number of pixels between the text and the bottom border."),
		F_ENTITIES_ADD("locale.inactive.paddings.help",  "<align=center><b>Top</b> is the number of pixels between the top border and the text.<br><b>Bottom</b> is the number of pixels between the text and the bottom border.<br><b>Top Pad.</b> is the number of pixel to add to lower the inactive tab."),

		F_ENTITIES_ADD("locale.back",           "Back"),
		F_ENTITIES_ADD("locale.general",        "General"),
		F_ENTITIES_ADD("locale.paddings",        "Margins"),
		F_ENTITIES_ADD("locale.paddings.help",   "<align=justify>Use these sliders to adjust the number of pixels<br>to add between the page frame and its contents."),
		F_ENTITIES_ADD("locale.paddings.left",   "Left"),
		F_ENTITIES_ADD("locale.paddings.right",  "Right"),
		F_ENTITIES_ADD("locale.paddings.top",    "Top"),
		F_ENTITIES_ADD("locale.paddings.bottom", "Bottom"),

		F_ARRAY_END
	};

	if (IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Pages",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/page.xml",
		"Entities", F_ENTITIES_PTR,

	TAG_MORE,Msg))
	{
		FObject pl = NULL;
		FObject pr = NULL;
		FObject pt = NULL;
		FObject pb = NULL;

		IFEELIN F_Do
		(
			Obj, (uint32) "GetObjects",

			"page:padding-left",     &pl,
			"page:padding-right",    &pr,
			"page:padding-top",      &pt,
			"page:padding-bottom",   &pb,

			NULL
		);

		IFEELIN F_Do
		( 
			pl, FM_Notify,
			
			"FA_Numeric_Value", FV_Notify_Always,
			
			pr, FM_Set, 2, "FA_Numeric_Value", FV_Notify_Value
		);
		
		IFEELIN F_Do
		(
			pt, FM_Notify,

			"FA_Numeric_Value", FV_Notify_Always,

			pb, FM_Set, 2, "FA_Numeric_Value", FV_Notify_Value
		);
 
		return Obj;
	}
	
	return NULL;
}
//+
