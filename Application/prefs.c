#include "Private.h"

//#define DB_OPENFONT
 
/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///app_create_ta
STATIC struct TextAttr *app_create_ta(STRPTR Define,struct TextAttr *TA)
{
	if (Define)
	{
		STRPTR name = IFEELIN F_New(256);

		if (name)
		{
			STRPTR back = name;

			while ((*Define != NULL) && (*Define != '/'))
			{
				*name++ = *Define++;
			}

			IEXEC CopyMem(".font",name,5);
			
			name = back;

			if (*Define++ == '/')
			{
				uint16 size = atol(Define);
				
				if (size)
				{
					TA->ta_Name  = name;
					TA->ta_YSize = size;
					TA->ta_Style = FS_NORMAL;
					TA->ta_Flags = NULL;

					return TA;
				}
			}
			IFEELIN F_Dispose(name);
		}
	}
	return NULL;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

#ifndef F_NEW_STYLES

///App_Resolve
F_METHODM(APTR,App_Resolve,FS_Application_Resolve)
{
	#ifdef F_NEW_STYLES

	if (Msg->Name != NULL)
	{
		IFEELIN F_Log(0,"FM_Application_Resolve is deprecated, please use Styles (%s)", Msg->Name);
	}

	return (APTR) Msg->Default;

	#else

	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (((int32)(Msg->Name) > 0xFFFF) && (*Msg->Name == FV_Preference_Magic))
	{
		APTR data = (APTR) Msg->Default;

		IFEELIN F_Do(LOD->preferences, F_IDO(FM_Preference_Find), Msg->Name + 1, &data, NULL);

		return data;
	}
	
	return Msg->Name;
	
	#endif
}
//+
///App_ResolveInt
F_METHODM(uint32,App_ResolveInt,FS_Application_Resolve)
{
	#ifdef F_NEW_STYLES

	IFEELIN F_Log(0,"FM_Application_Resolve is deprecated, please use Styles (%s)", ((int32)(Msg->Name) > 0xFFFF) ? Msg->Name : (STRPTR) "numeric");

	return Msg->Default;

	#else
	
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (((int32)(Msg->Name) > 0xFFFF) && (*Msg->Name == FV_Preference_Magic))
	{
		uint32 value = Msg->Default;

		IFEELIN F_Do(LOD->preferences, F_IDO(FM_Preference_Find), Msg->Name + 1, NULL, &value);

		return value;
	}
	
	return (uint32) Msg->Name;
	
	#endif
}
//+
///App_ObtainAssociated
F_METHODM(uint32, App_ObtainAssociated, FS_Preference_ObtainAssociated)
{
	#ifdef F_NEW_STYLES

	IFEELIN F_Log(0,"*** disabled (%s)", Msg->Name);

	return 0;

	#else
	
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	uint32 rc = 0;

	if (((int32)(Msg->Name) > 0xFFFF) && (*Msg->Name == FV_Preference_Magic))
	{
		Msg->Name++;
 
		rc = IFEELIN F_DoA(LOD->preferences, F_IDO(FM_Preference_ObtainAssociated), Msg);
		
		Msg->Name--;
	}
	else if (Msg->ConstructHook)
	{
		struct FS_Associated_Construct hook_msg;
		
		hook_msg.Data = Msg->Name;
		hook_msg.Pool = LOD->pool;
		hook_msg.Result = Msg->Result;
		hook_msg.UserParams = (uint32 *)((uint32)(Msg) + sizeof (struct FS_Preference_ObtainAssociated));

		#ifdef DB_ASSOCIATED
		IFEELIN F_Log(0,"custom (0x%08lx)(%s) constructor (0x%08lx)",Msg->Name,Msg->Name,Msg->ConstructHook);
		#endif

		IUTILITY CallHookPkt(Msg->ConstructHook, NULL, &hook_msg);
	}

	return rc;
	
	#endif
}
//+
///App_ReleaseAssociated
F_METHODM(uint32,App_ReleaseAssociated,FS_Preference_ReleaseAssociated)
{
	#ifdef F_NEW_STYLES

	IFEELIN F_Log(0,"*** disabled (0x%08lx)", Msg->Handle);

	return 0;

	#else
 
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (Msg->Handle)
	{
		return IFEELIN F_DoA(LOD->preferences, F_IDO(FM_Preference_ReleaseAssociated), Msg);
	}
	else if (Msg->Data)
	{
		#ifdef DB_ASSOCIATED
		IFEELIN F_Log(0,"no handle but data (0x%08lx)",Msg->Data);
		#endif

		if (Msg->DestructHook)
		{
			struct FS_Associated_Destruct hook_msg;

			hook_msg.Data = Msg->Data;
			hook_msg.Pool = LOD->pool;
			hook_msg.UserParams = (uint32 *)((uint32)(Msg) + sizeof (struct FS_Preference_ReleaseAssociated));

			IUTILITY CallHookPkt(Msg->DestructHook,NULL,&hook_msg);
		}
		else
		{
			IFEELIN F_Dispose(Msg->Data);
		}

		return TRUE;
	}

	return FALSE;
	
	#endif
}
//+

#endif

///App_OpenFont
F_METHODM(struct TextFont *,App_OpenFont,FS_Application_OpenFont)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	#ifndef F_NEW_STYLES
	STRPTR Define = Msg->Spec;
	#endif
	struct TextFont *font = NULL;

/*** check inheritance *************************************************************************/

	/* FIXME: inheritance should be handled by FC_Area */

	#ifdef F_NEW_STYLES

	if (Msg->Spec != NULL)
	{
		struct TextAttr ta;

		if (app_create_ta(Msg->Spec, &ta) != NULL)
		{
			font = IDISKFONT OpenDiskFont(&ta);

			IFEELIN F_Dispose(ta.ta_Name);
		}
	}

	#else

	if ((uint32)(Define) == FV_Font_Inherit)
	{
		Define = NULL;

		if ((font = (struct TextFont *) IFEELIN F_Get((FObject) IFEELIN F_Get(Msg->Object, FA_Parent), FA_Area_Font)) != NULL)
		{
			struct TextAttr ta;

			ta.ta_Name  = font->tf_Message.mn_Node.ln_Name;
			ta.ta_YSize = font->tf_YSize;
			ta.ta_Style = font->tf_Style;
			ta.ta_Flags = font->tf_Flags;

			font = IGRAPHICS OpenFont(&ta);
		}
	}

/*** preference item ***************************************************************************/

	if (!font)
	{
		if ((Define = (STRPTR) IFEELIN F_Do(Obj,FM_Application_Resolve,Define,NULL)) != NULL)
		{
			struct TextAttr ta;

			if (app_create_ta((STRPTR)(Define),&ta))
			{
				font = IDISKFONT OpenDiskFont(&ta); IFEELIN F_Dispose(ta.ta_Name);
			}
		}
	}


/*** preference item : $font-normal ************************************************************/

	if (!font)
	{
		if ((Define = (STRPTR) IFEELIN F_Do(Obj,FM_Application_Resolve,"$font-normal",NULL)) != NULL)
		{
			struct TextAttr ta;

			if (app_create_ta((STRPTR)(Define),&ta))
			{
				font = IDISKFONT OpenDiskFont(&ta); IFEELIN F_Dispose(ta.ta_Name);
			}
		}
	}

	#endif

/*** last fall back : screen font **************************************************************/
 
	if (!font)
	{
		struct Screen *scr;

		if ((scr = (struct Screen *) IFEELIN F_Get(LOD->display,(uint32) "FA_Display_Screen")) != NULL)
		{
			font = IGRAPHICS OpenFont(scr->Font);
		}
	}

	#ifdef DB_OPENFONT
	IFEELIN F_Log(0,"FONT (%s)(0x%08lx) - YSize %ld",font->tf_Message.mn_Node.ln_Name,font,font->tf_YSize);
	#endif

	return font;
}
//+

#ifndef F_NEW_STYLES

/************************************************************************************************
*** Preferences *********************************************************************************
************************************************************************************************/

STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("application", "color-scheme", FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("application", "color-scheme-ghost", FV_TYPE_STRING,  "Spec",     DEF_DISABLEDSCHEME),
	F_PREFERENCES_ADD("application", "font-tiny", FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("application", "font-big", FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("application", "font-normal", FV_TYPE_STRING,  "Contents", NULL),
	F_PREFERENCES_ADD("application", "font-fixed", FV_TYPE_STRING,  "Contents", NULL),

	F_PREFERENCES_ADD("pophelp", "scheme", FV_TYPE_STRING,  "Spec",     NULL),
	F_PREFERENCES_ADD("pophelp", "font", FV_TYPE_STRING,  "Contents", NULL),

	F_ARRAY_END
};
 
///Prefs_New
F_METHOD(uint32,Prefs_New)
{
	return IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Application",

		"Script",   F_PREFERENCES_PTR,
		"Source",   "feelin/preference/application.xml",
		
	TAG_MORE,Msg);
}
//+

#endif
