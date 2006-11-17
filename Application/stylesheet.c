#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///App_Setup
F_METHOD(bool32, App_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

/*** preferences & stylesheet ******************************************************************/

	FObject reference = (FObject) IFEELIN F_Get(LOD->appserver, (uint32) "FA_AppServer_Preference");

	if ((LOD->base != NULL) || (LOD->css_list.Head != NULL))
	{
		LOD->css = PreferenceObject,

			"FA_Preference_Name",		LOD->base,
			"FA_Preference_Reference",  reference,

		End;

		/* if stylesheets were added with  the  "AddStyleSheet"  method,  we
		merge them with Preference object */

		if (LOD->css_list.Head != NULL)
		{
			struct in_StyleSheet *ss;

			for (ss = (struct in_StyleSheet *) LOD->css_list.Head ; ss ; ss = ss->next)
			{
				#ifdef DB_SETUP
				IFEELIN F_Log(0, "css (0x%08lx) merge (%s) type (0x%08lx)", LOD->css, ss->source, ss->source_type);
				#endif

				IFEELIN F_Do(LOD->css, (uint32) "FM_Document_Merge", ss->source, ss->source_type);
			}
		}

		//IFEELIN F_Log(0, "preferences (0x%08lx) created", LOD->css);
	}

	/* If the Preference object fails to create, or if the  application  has
	no  FA_Application_Base  attribute  defined,  the AppServer's Preference
	object must be used. */

	if (LOD->css == NULL)
	{
		LOD->css = reference;

		_application_set_inherited_css;

		//IFEELIN F_Log(0, "preferences (0x%08lx) inherited", LOD->css);
	}

	/*  In  order  for  the  application  to   be   aware   of   preferences
	modifications,  and  update  itself  in that case, a notification is set
	upon the Preference object's FA_Preference_Update attribute. */

	LOD->css_notify_handler = (FNotifyHandler *) IFEELIN F_Do(LOD->css, FM_Notify, "FA_Preference_Update", TRUE, Obj, FM_Application_Update, 0);

	if (LOD->css_notify_handler == NULL)
	{
		return FALSE;
	}

/*** Display ***********************************************************************************/
	
	/* FIXME: Yeurk ! I don't really like the look of this */

	LOD->display_class = IFEELIN F_OpenClass(FC_Display);
			
	if (LOD->display_class == NULL)
	{
		return FALSE;
	}

	LOD->display = (FObject) IFEELIN F_ClassDo(LOD->display_class,NULL, IFEELIN F_DynamicFindID("FM_Display_Create"), NULL);
 
	if (LOD->display == NULL)
	{
		return FALSE;
	}
	
	LOD->flags |= FF_Application_Setup;

	return TRUE;
}
//+
///App_Cleanup
F_METHOD(bool32,App_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	LOD->flags &= ~FF_Application_Setup;

	if (LOD->display_class != NULL)
	{
		if (LOD->display != NULL)
		{
			IFEELIN F_ClassDo(LOD->display_class,NULL, IFEELIN F_DynamicFindID("FM_Display_Delete"),LOD->display); LOD->display = NULL;
		}

		IFEELIN F_CloseClass(LOD->display_class); LOD->display_class = NULL;
	}

	if (LOD->css != NULL)
	{
		if (LOD->css_notify_handler != NULL)
		{
			IFEELIN F_Do(LOD->css, FM_UnNotify, LOD->css_notify_handler);

			LOD->css_notify_handler = NULL;
		}

		if (_application_isnt_inherited_css)
		{
			IFEELIN F_DisposeObj(LOD->css);
		}
		else
		{
			_application_clear_inherited_css;
		}

		LOD->css = NULL;
	}

	return TRUE;
}
//+
///App_AddStyleSheet
F_METHODM(bool32, App_AddStyleSheet, FS_Application_AddStyleSheet)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_StyleSheet *ss;

	if ((Msg->Source == NULL) || (Msg->SourceType == 0))
	{
		return FALSE;
	}

	for (ss = (struct in_StyleSheet *) LOD->css_list.Head ; ss ; ss = ss->next)
	{
		if (ss->source == Msg->Source)
		{
			break;
		}
	}

	if (ss != NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Source (0x%08lx) already added", Msg->Source);

		return FALSE;
	}

	ss = IFEELIN F_NewP(LOD->pool, sizeof (struct in_StyleSheet));

	if (ss == NULL)
	{
		return FALSE;
	}

	ss->source = IFEELIN F_StrNew(NULL, "%s", Msg->Source);
	ss->source_type = Msg->SourceType;

	IFEELIN F_LinkTail(&LOD->css_list, (FNode *) ss);

	return TRUE;
}
//+
///App_RemStyleSheet
F_METHODM(bool32, App_RemStyleSheet, FS_Application_RemStyleSheet)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_StyleSheet *ss;

	if (Msg->Source == NULL)
	{
		return FALSE;
	}

	for (ss = (struct in_StyleSheet *) LOD->css_list.Head ; ss ; ss = ss->next)
	{
		if (ss->source == Msg->Source)
		{
			break;
		}
	}

	if (ss == NULL)
	{
		IFEELIN F_Log(FV_LOG_DEV, "Unknown Source (0x%08lx)", Msg->Source);

		return FALSE;
	}

	IFEELIN F_LinkRemove(&LOD->css_list, (FNode *) ss);
	IFEELIN F_Dispose(ss);

	return TRUE;
}
//+

