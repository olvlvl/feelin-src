#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Text_New
F_METHOD(uint32,Text_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_SAVE_AREA_PUBLIC;
	F_SAVE_WIDGET_PUBLIC;

	/* Default values */

	LOD->Flags = FF_Text_Static | FF_Text_Shortcut;

	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Widget_SetMin, FV_Widget_SetBoth,

		TAG_MORE, Msg);
}
//+
///Text_Dispose
F_METHOD(uint32,Text_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (!(FF_Text_Static & LOD->Flags))
	{
		IFEELIN F_Dispose(LOD->text);
	}

	LOD->text = NULL;

	return F_SUPERDO();
}
//+
///Text_Set

#define FF_UPDATE_CONTENTS                      (1 << 0)
#define FF_UPDATE_PREP                          (1 << 1)
#define FF_UPDATE_ALTPREP                       (1 << 2)
#define FF_UPDATE_STATE                         (1 << 3)
#define FF_UPDATE_FONT                          (1 << 4)
#define FF_UPDATE_SHORTCUT                      (1 << 5)

F_METHOD(uint32,Text_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	bits32 update=0;
	//bool32 selected = _widget_is_selected;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Widget_Selected:
		{
			#if 1

			/*
			STRPTR prep = LOD->Prep[(item.ti_Data != FALSE) ? 1 : 0];
			
			if (prep == NULL)
			{
				prep = LOD->Prep[0];
			}

			IFEELIN F_Do(LOD->TextDisplay, FM_Set,

				FA_TextDisplay_PreParse, prep,

				TAG_DONE);
			*/

			#else

			selected = item.ti_Data;

			if (LOD->Prep[0] != LOD->Prep[1] && LOD->Prep[1] != NULL)
			{
				update |= FF_UPDATE_STATE;
			}

			#endif
		}
		break;

		case FA_Area_Font:
		{
			update |= FF_UPDATE_FONT;
		}
		break;

		/****/

		case FA_Text_Contents:
		{
			update |= FF_UPDATE_CONTENTS;
			
			if (FF_Text_Static & LOD->Flags)
			{
				LOD->text = (STRPTR)(item.ti_Data);
			}
			else
			{
				IFEELIN F_Dispose(LOD->text); LOD->text = NULL;
				
				LOD->text = IFEELIN F_StrNew(NULL,"%s",item.ti_Data);
			}
		}
		break;

		case FA_Text_PreParse:
		{
			update |= FF_UPDATE_PREP;
			
			LOD->preparse = LOD->preparse_user = (STRPTR) item.ti_Data;
		}
		break;

		#if 0
		
		case FA_Text_AltPreParse:
		{
			update |= FF_UPDATE_ALTPREP;

			LOD->Prep[1] = (STRPTR)(item.ti_Data);
		}
		break;

		#endif

		case FA_Text_Shortcut:
		{
			update |= FF_UPDATE_SHORTCUT;

			if (item.ti_Data)
			{
				LOD->Flags |= FF_Text_Shortcut;
			}
			else
			{
				LOD->Flags &= ~FF_Text_Shortcut;
			}
		}
		break;

		case FA_Text_HCenter:
		{
			if (item.ti_Data)
			{
				LOD->Flags |= FF_Text_HCenter;
			}
			else
			{
				LOD->Flags &= ~FF_Text_HCenter;
			}
		}
		break;

		case FA_Text_VCenter:
		{
			if (item.ti_Data)
			{
				LOD->Flags |= FF_Text_VCenter;
			}
			else
			{
				LOD->Flags &= ~FF_Text_VCenter;
			}
		}
		break;
/*FIXME:

	Si pas encore de buffer cr�er et copier. Si buffer mettre � z�ro et d�truire

*/
		case FA_Text_Static:
		{
			if (item.ti_Data)
			{
				LOD->Flags |= FF_Text_Static;
			}
			else
			{
				LOD->Flags &= ~FF_Text_Static;
			}
		}
		break;
	}

	F_SUPERDO();

	if ((update != 0) && (_area_render != NULL))
	{
		#if 1

		IFEELIN F_Do(LOD->TextDisplay, FM_Set,

			FA_TextDisplay_Font,     _area_font,
			FA_TextDisplay_PreParse, LOD->preparse,
			FA_TextDisplay_Contents, LOD->text,
			FA_TextDisplay_Shortcut, (0 != (FF_Text_Shortcut & LOD->Flags)),

			TAG_DONE);

		#else

		STRPTR prep;

		prep = LOD->Prep[selected ? 1 : 0];
		if (!prep) prep = LOD->Prep[0];

		IFEELIN F_Do(LOD->TextDisplay, FM_Set,
			
			FA_TextDisplay_Font,     _area_font,
			FA_TextDisplay_PreParse, prep,
			FA_TextDisplay_Contents, LOD->Text,
			FA_TextDisplay_Shortcut, (0 != (FF_Text_Shortcut & LOD->Flags)),
			
			TAG_DONE);

		#endif

		_area_set_damaged;

		IFEELIN F_Set(Obj, FA_Widget_Accel, IFEELIN F_Get(LOD->TextDisplay, FA_TextDisplay_Shortcut));
		
		#ifdef DB_SET
		IFEELIN F_Log(0,"Text (%s) Short (%ld)(%lc),",LOD->Text,(0 != (FF_Text_Shortcut & LOD->Flags)),F_Get(LOD->TextDisplay,FA_TextDisplay_Shortcut));
		#endif

		IFEELIN F_Draw(Obj, FF_Draw_Update);
	}

	return 0;
}
//+
///Text_Get
F_METHOD(void,Text_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	F_SUPERDO();

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FA_Text_Contents:             F_STORE(LOD->text);    break;
	/*
		case FA_Text_PreParse:    F_STORE(LOD->Prep[0]); break;
		case FA_Text_AltPreParse: F_STORE(LOD->Prep[1]); break;
	*/
		case FA_Text_HCenter:     F_STORE(FALSE != (FF_Text_HCenter & LOD->Flags)); break;
		case FA_Text_VCenter:     F_STORE(FALSE != (FF_Text_VCenter & LOD->Flags)); break;
		case FA_Text_Static:      F_STORE(FALSE != (FF_Text_Static  & LOD->Flags)); break;
	}
}
//+
