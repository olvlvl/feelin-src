#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///List_New
F_METHOD(FObject,List_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;
	APTR *entries = NULL;
	bool32 readonly = FALSE;

	uint32 pool_item_size = 1024;
	uint32 pool_item_number = 4;
 
	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;
	
	LOD->ColumnCount      = 1;
	LOD->SortMode         = FV_List_Sort_Descending;
	LOD->Activation       = FV_List_Activation_Unknown;
	LOD->steps            = 1;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_POOL:
		{
			LOD->pool = (APTR)(item.ti_Data);
		}
		break;

		case FV_ATTRIBUTE_POOLITEMSIZE:
		{
			pool_item_size = item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_POOLITEMNUMBER:
		{
			pool_item_number = item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_SOURCEARRAY:
		{
			entries  = (APTR *)(item.ti_Data);
		}
		break;

		case FV_ATTRIBUTE_READONLY:
		{
			readonly = item.ti_Data;
		}
		break;
	}

/*** create / obtain memory pools **************************************************************/

	LOD->LinePool = IFEELIN F_CreatePool(1024,FA_Pool_Items,1,FA_Pool_Name,"list-lines",TAG_DONE);

	if (!LOD->LinePool)
	{
		IFEELIN F_Log(FV_LOG_DEV,"Unable to create Lines Pool");
		
		return NULL;
	}

	if (!LOD->pool)
	{
		LOD->pool = IFEELIN F_CreatePool(pool_item_size,
			
			FA_Pool_Items, pool_item_number,
			FA_Pool_Name, "List.Data",
			
			TAG_DONE);
		
		LOD->Flags |= FF_LIST_OWNPOOL;
	}

	if (!LOD->pool)
	{
		IFEELIN F_Log(FV_LOG_DEV,"Unable to CreatePool");
		
		return NULL;
	}

	LOD->TDObj = TextDisplayObject,
	
		FA_TextDisplay_Shortcut, FALSE,

		End;
		
	if (LOD->TDObj == NULL)
	{
		return NULL;
	}

	if (IFEELIN F_SuperDo(Class,Obj,Method,

		//FA_Area_MinWidth, 20,
		FA_Area_Fillable, FALSE,
		FA_Widget_Chainable, FALSE,

		(readonly) ? FA_Element_Class : TAG_IGNORE, "list-read",

		F_ATTRIBUTE_ID(FORMAT), "<col />",
		
		TAG_MORE, Msg))
	{
		if (LOD->TitleBar != NULL)
		{
			if (entries != NULL)
			{
				IFEELIN F_Do(Obj, F_METHOD_ID(INSERT), entries, ALL, FV_List_Insert_Top);
			}
			return Obj;
		}
	}
	return NULL;
}
//+
///List_Dispose
F_METHOD(void,List_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	titlebar_dispose(Class,Obj);

	if (LOD->TDObj)
	{
		IFEELIN F_DisposeObj(LOD->TDObj); LOD->TDObj = NULL;
	}

	if (LOD->pool && (FF_LIST_OWNPOOL & LOD->Flags))
	{
		IFEELIN F_DeletePool(LOD->pool); LOD->pool = NULL;
	}

	if (LOD->LinePool)
	{
		IFEELIN F_DeletePool(LOD->LinePool); LOD->LinePool = NULL;
	}

	F_SUPERDO();
}
//+
///List_Get
F_METHOD(void,List_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ACTIVE:       F_STORE((LOD->Active) ? LOD->Active->Position : FV_List_NotVisible); break;
		case FV_ATTRIBUTE_ACTIVATION:   F_STORE(LOD->Activation); break;
		case FV_ATTRIBUTE_ENTRIES:      F_STORE(LOD->LineCount); break;
		case FV_ATTRIBUTE_VISIBLE:      F_STORE(LOD->Visible); break;
		case FV_ATTRIBUTE_FIRST:        F_STORE((LOD->First) ? LOD->First->Position : FV_List_NotVisible); break;
		case FV_ATTRIBUTE_LAST:         F_STORE((LOD->Last) ? LOD->Last->Position : FV_List_NotVisible); break;
		case FV_ATTRIBUTE_TITLE:        F_STORE((LOD->TitleBar && LOD->ColumnCount == 1) ? (LOD->TitleBar->Strings[0]) : NULL); break;
		case FV_ATTRIBUTE_STEPS:        F_STORE(LOD->steps); break;
		case FV_ATTRIBUTE_SORTMODE:     F_STORE(LOD->SortMode); break;
	}

	F_SUPERDO();
}
//+
///List_Set
F_METHOD(void,List_Set)
{
	struct LocalObjectData *LOD  = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,*tag,item;

	while ((tag = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_COMPAREHOOK:
		{
			LOD->Hook_Compare = (struct Hook *)(item.ti_Data);
		}
		break;

		case FV_ATTRIBUTE_CONSTRUCTHOOK:
		{
			LOD->Hook_Construct = (struct Hook *)(item.ti_Data);
		}
		break;
		
		case FV_ATTRIBUTE_DESTRUCTHOOK:
		{
			LOD->Hook_Destruct = (struct Hook *)(item.ti_Data);
		}
		break;
		
		case FV_ATTRIBUTE_DISPLAYHOOK:
		{
			LOD->Hook_Display = (struct Hook *)(item.ti_Data);
		}
		break;

		case FV_ATTRIBUTE_FORMAT:
		{
			LOD->LastFirst = NULL;
		   
			titlebar_new(Class,Obj,(STRPTR)(item.ti_Data));
		}
		break;

		case FV_ATTRIBUTE_TITLE:
		{
			if (LOD->TitleBar && LOD->ColumnCount == 1)
			{
				LOD->TitleBar->Strings[0] = (STRPTR)(item.ti_Data);
			}
		}
		break;

		case FV_ATTRIBUTE_FIRST:
		{
			FLine *line = LOD->First;
			int32 pos;

			pos = MAX((int32)(item.ti_Data),0);
			pos = MIN(pos,LOD->LineCount - 1);
			 
			list_adjust_first(Class,Obj,pos);

			if (LOD->Quiet >= 0 && line != LOD->First)
			{
				IFEELIN F_Draw(Obj,FF_Draw_Update);
			}
		}
		break;

		case FV_ATTRIBUTE_ACTIVE:
		{
			FLine *active = LOD->Active;

			if (!(FF_LIST_ACTIVATION_INTERNAL & LOD->Flags))
			{
				LOD->Activation = FV_List_Activation_External;
			   
				LOD->Flags &= ~FF_LIST_ACTIVATION_INTERNAL;
			}
	
			#ifdef DB_SET
			IFEELIN F_Log(0,"ACTIVE %ld - VISIBLE_FIRST 0x%08lx (%ld) - VISIBLE_LAST 0x%08lx (%ld)",item.ti_Data,LOD->First,LOD->First->Position,LOD->Last,LOD->Last->Position);
			#endif

			if (item.ti_Data == FV_List_Active_None)
			{
				if (LOD->Quiet >= 0)
				{
					if ((LOD->Update = LOD->Active) != NULL)
					{
						LOD->Active->Flags &= ~FF_LINE_SELECTED;
						
						IFEELIN F_Draw(Obj,FF_Draw_Update | FF_Draw_Line);
					}
				}
				LOD->Active = NULL;
			}
			else
			{
				int32 pos = (LOD->Active) ? LOD->Active->Position : 0;

				switch (item.ti_Data)
				{
					case FV_List_Active_PageDown:
					{
						if (LOD->Last)
						{
							if (active == LOD->Last)
							{
								pos += LOD->Visible - 1;
							}
							else
							{
								pos = LOD->Last->Position;
							}
						}
						else
						{
							pos += LOD->Visible - 1;
						}
					}
					break;

					case FV_List_Active_PageUp:
					{
						if (LOD->First)
						{
							if (active == LOD->First)
							{
								pos -= LOD->Visible - 1;
							}
							else
							{
								pos = LOD->First->Position;
							}
						}
						else
						{
							pos -= LOD->Visible - 1;
						}
					}
					break;

					case FV_List_Active_Down:       pos += 1; break;
					case FV_List_Active_Up:         pos -= 1; break;
					case FV_List_Active_Bottom:     pos  = LOD->LineCount - 1; break;
					case FV_List_Active_Top:        pos  = 0; break;
					default:                        pos  = item.ti_Data;
				}

				pos = MAX(pos,0);
				pos = MIN(pos,LOD->LineCount - 1);

				tag->ti_Data = pos;

				for (active = (FLine *)(LOD->LineList.Head) ; active ; active = active->Next)
				{
				   if (active->Position == pos) break;
				}

				if (active == LOD->Active) break;

/*** new active * clear previous one ***********************************************************/

				if ((LOD->Update = LOD->Active) != NULL)
				{
					LOD->Active->Flags &= ~FF_LINE_SELECTED;

					if (LOD->Quiet >= 0)
					{
						IFEELIN F_Draw(Obj,FF_Draw_Update | FF_Draw_Line);
					}
				}

				LOD->Active = active;
				
				if (LOD->Active)
				{
					LOD->Active->Flags |= FF_LINE_SELECTED;
				}

				if (LOD->First && LOD->Last)
				{
					if (pos >= LOD->First->Position && pos <= LOD->Last->Position)
					{
						if ((LOD->Update = LOD->Active) != NULL)
						{
							IFEELIN F_Draw(Obj,FF_Draw_Update | FF_Draw_Line);
						}
					}
					else
					{
						if (pos > LOD->Last->Position)
						{
							pos = pos - LOD->Visible + 1;
						}

						if (LOD->Quiet >= 0)
						{
							IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), pos);
						}
						else
						{
							IFEELIN F_Do(Obj, FM_Set, FA_NoNotify, TRUE, F_ATTRIBUTE_ID(FIRST), pos, TAG_DONE);
						}
					}
				}
			}
		}
		break;
	 
		case FV_ATTRIBUTE_ACTIVATION:
		{
			LOD->Activation = item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_QUIET:
		{
			if (item.ti_Data) LOD->Quiet--;
			else LOD->Quiet++;

			if ((LOD->Quiet == 0) && (FF_LIST_MODIFIED & LOD->Flags))
			{
				list_update(Class,Obj,FALSE);

				#ifdef DB_SET
				IFEELIN F_Log(0,"entries (%ld) visible (%ld) first (0x%08lx)(%ld)",LOD->LineCount,LOD->Visible,LOD->First,LOD->First->Position);
				#endif
	 
				IFEELIN F_SuperDo(Class,Obj,FM_Set,
				   
				   F_ATTRIBUTE_ID(ENTRIES), LOD->LineCount,
				   F_ATTRIBUTE_ID(VISIBLE), LOD->Visible,
				   F_ATTRIBUTE_ID(FIRST),   LOD->First ? LOD->First->Position : 0,
				   
				   TAG_DONE);
				   
				IFEELIN F_Draw(Obj,FF_Draw_Update);
			}
		}
		break;
	 
		case FV_ATTRIBUTE_STEPS:
		{
			LOD->steps = item.ti_Data;
		}
		break;
	}

	F_SUPERDO();
}
//+
///List_GetEntry
F_METHODM(APTR,List_GetEntry,FS_List_GetEntry)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	switch (Msg->Position)
	{
		case FV_List_GetEntry_First:  return LOD->LineList.Head;
		case FV_List_GetEntry_Last:   return LOD->LineList.Tail;
		case FV_List_GetEntry_Prev:   return (LOD->Active) ? LOD->Active->Prev : NULL;
		case FV_List_GetEntry_Next:   return (LOD->Active) ? LOD->Active->Next : NULL;
		case FV_List_GetEntry_Active: return LOD->Active;
		
		default:
		{
			FLine *line;

			for (line = (FLine *)(LOD->LineList.Head) ; line ; line = line->Next)
			{
				if (line->Position == Msg->Position)
				{
					break;
				}
			}

			if (line)
			{
				return line->Entry;
			}
		}
	}
	return NULL;
}
//+

#if 0
///Preferences
STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("list",       "frame",                FV_TYPE_STRING,  "Spec",     "<frame id='23' padding-width='2' padding-height='1' />"),
	F_PREFERENCES_ADD("list",       "back",                 FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("list",       "color-scheme",         FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("list-title", "back",                 FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("list-title", "frame",                FV_TYPE_STRING,  "Spec",     DEF_CELL_FRAME),
	F_PREFERENCES_ADD("list-title", "color-scheme",         FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("list-read",  "frame",                FV_TYPE_STRING,  "Spec",     "<frame id='24' padding-width='2' padding-height='1' />"),
	F_PREFERENCES_ADD("list-read",  "back",                 FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("list-read",  "color-scheme",         FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("list-read-title",    "back",           FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("list-read-title",    "frame",          FV_TYPE_STRING,  "Spec",     DEF_CELL_FRAME),
	F_PREFERENCES_ADD("list-read-title",    "color-scheme",   FV_TYPE_STRING,  "Spec",     NULL),

	F_PREFERENCES_ADD("list",       "cursor-active",        FV_TYPE_STRING,  "Spec",     "highlight"),
	F_PREFERENCES_ADD("list",       "cursor-selected",      FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("list",       "font-prop",            FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("list",       "font-fixed",           FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("list",       "spacing",              FV_TYPE_INTEGER, "Value",    (STRPTR) 1),
	F_PREFERENCES_ADD("list",       "steps",                FV_TYPE_INTEGER, "Value",    (STRPTR) 3),
		
	F_ARRAY_END
};

F_METHOD(uint32,Prefs_New)
{
	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Listview",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/listview.xml",

	TAG_MORE,Msg);
}
//+
#endif
