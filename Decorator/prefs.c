#include "Private.h"

#if 0

//#define DB_NEW
//#define DB_LOAD
//#define DB_SPEC

#define F_DECORATORS_PATH                       "feelin:resources/decorators"
 
STATIC F_PREFERENCES_ARRAY =
{
	F_PREFERENCES_ADD("decorator",  "preparse-active",      FV_TYPE_STRING, "Contents",    NULL),
	F_PREFERENCES_ADD("decorator",  "preparse-inactive",    FV_TYPE_STRING, "Contents",    NULL),
	F_PREFERENCES_ADD("decorator",  "color-scheme-focus",   FV_TYPE_STRING, "Spec",        NULL),
	F_PREFERENCES_ADD("decorator",  "color-scheme",         FV_TYPE_STRING, "Spec",        "<scheme shine='fill' fill='halfshadow' dark='shadow' text='halfdark' highlight='dark' />"),
	F_PREFERENCES_ADD("decorator",  "font",                 FV_TYPE_STRING, "Contents",    NULL),
		
	F_ARRAY_END
};

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

struct FS_SELECT                                { FClass *Class; uint32 Active; };

///code_select_decorator

F_HOOKM(void,code_select_decorator,FS_SELECT)
{
	struct p_LocalObjectData *LOD = F_LOD(Msg->Class,Obj);

	if (Msg->Active == 0)
	{
		if (LOD->active_decorator)
		{
			IFEELIN F_Set(LOD->decorator_group, FA_Group_Quiet, TRUE);
			
			IFEELIN F_Do(LOD->decorator_group, FM_RemMember, LOD->active_decorator);

			LOD->active_decorator = NULL;
			
			IFEELIN F_Set(LOD->decorator_group, FA_Group_Quiet, FALSE);
		}
		
		IFEELIN F_Set(LOD->grp_edit, FA_Widget_Disabled, FF_Disabled_Check | TRUE);
	}
	else
	{
		struct in_deco *node;
		
		for (node = (struct in_deco *) LOD->decoratorslist.Head ; node ; node = node->next)
		{
			if (node->position == Msg->Active)
			{
				break;
			}
		}
	
		if (node->object != LOD->active_decorator)
		{
			IFEELIN F_Set(LOD->decorator_group, FA_Group_Quiet, TRUE);

			IFEELIN F_Do(LOD->decorator_group, FM_RemMember, LOD->active_decorator);

			LOD->active_decorator = NULL;
			
			if (node)
			{
				LOD->active_decorator = node->object;
 
				IFEELIN F_Do(LOD->decorator_group, FM_AddMember, LOD->active_decorator, FV_AddMember_Tail);
			}

			IFEELIN F_Set(LOD->decorator_group, FA_Group_Quiet, FALSE);
		}
		
		IFEELIN F_Set(LOD->grp_edit, FA_Widget_Disabled, FF_Disabled_Check | FALSE);
	}
}
//+

///decorator_create_spec
FObject decorator_create_spec(struct in_deco *node)
{
	if (node->name)
	{
		STRPTR render_name = IFEELIN F_StrNew(NULL,F_DECORATORS_PATH "/%s/render.png",node->name);
		STRPTR select_name = IFEELIN F_StrNew(NULL,F_DECORATORS_PATH "/%s/select.png",node->name);

		BPTR render_lock = IDOS_ Lock(render_name, ACCESS_READ);
		BPTR select_lock = IDOS_ Lock(select_name, ACCESS_READ);

		STRPTR spec=NULL;

		if (render_lock && select_lock)
		{
			spec = IFEELIN F_StrNew(NULL,"<image type='picture' src='%s' />" F_IMAGEDISPLAY_SEPARATOR "<image type='picture' src='%s' />",render_name, select_name);

			IDOS_ UnLock(render_lock);
			IDOS_ UnLock(select_lock);
		}
		else if (render_lock)
		{
			spec = IFEELIN F_StrNew(NULL,"<image type='picture' src='%s' />", render_name);

			IDOS_ UnLock(render_lock);
		}
		else if (select_lock)
		{
			IDOS_ UnLock(select_lock);
		}

		#ifdef DB_SPEC
		IFEELIN F_Log(0,"name (%s) spec (%s)",node->name, spec);
		#endif

		node->spec = spec;
 
		IFEELIN F_Dispose(render_name);
		IFEELIN F_Dispose(select_name);
	}

	return node->spec;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Prefs_New
F_METHOD(FObject,Prefs_New)
{
	struct p_LocalObjectData *LOD = F_LOD(Class,Obj);
	
	if ((LOD->moduleslist = IFEELIN F_SharedOpen("ModulesList")) != NULL)
	{
		struct in_deco *gnode;
		FModuleCreated *cnode;
		
		for (cnode = (FModuleCreated *) IFEELIN F_Get(LOD->moduleslist, (uint32) "FA_ModulesList_DecoratorPrefs") ; cnode ; cnode = cnode->Next)
		{
			gnode = IFEELIN F_New(sizeof (struct in_deco));
			
			if (gnode)
			{
				gnode->object = IFEELIN F_NewObj(cnode->Class->Name,TAG_DONE);
				
				gnode->classname = IFEELIN F_StrNew(NULL,"%s",cnode->Module->lib_Node.ln_Name);
				
				if (gnode->object && gnode->classname)
				{
					struct in_deco *gprev;
					STRPTR c = gnode->classname;
					
					while (*c && *c != '.') c++;
						
					if (*c == '.')
					{
						*c = '\0';
					}

					gnode->name = (STRPTR) IFEELIN F_Get(gnode->object, FA_Group_Name);
										
					decorator_create_spec(gnode);
 
					for (gprev = (struct in_deco *) LOD->decoratorslist.Tail ; gprev ; gprev = gprev->prev)
					{
						if (IFEELIN F_StrCmp(gnode->name,gprev->name,ALL) > 0) break;
					}
					
					#ifdef DB_NEW
					IFEELIN F_Log(0,"gnode (0x%08lx) label (%s) image (%s)",gnode,gnode->name,gnode->spec);
					#endif
 
					gnode->item = ItemObject,
					
						"Label", gnode->name,
						"Image", gnode->spec,
						
						End;
						 
					IFEELIN F_LinkInsert(&LOD->decoratorslist,(FNode *) gnode,(FNode *) gprev);
			
					#ifdef DB_NEW
					IFEELIN F_Log(0,"gnode (0x%08lx) name (%s) class (%s)",gnode,gnode->name,gnode->classname);
					#endif
					
					continue;
				}
				
				IFEELIN F_Log(FV_LOG_DEV,"Unable to create object from %s",cnode->Module->lib_Node.ln_Name);
			
				IFEELIN F_DisposeObj(gnode->object);
				IFEELIN F_Dispose(gnode->classname);
				IFEELIN F_Dispose(gnode);
			}
		}
	
		gnode = IFEELIN F_New(sizeof (struct in_deco));
		
		if (gnode)
		{
			gnode->name = "Intuition";
			
			decorator_create_spec(gnode);
					
			gnode->item = ItemObject,

				"Label", gnode->name,
				"Image", gnode->spec,

				End;
 
			IFEELIN F_LinkHead(&LOD->decoratorslist,(FNode *) gnode);
		}
	}

	if (IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Group_Name, "Decorator",

		"Script", F_PREFERENCES_PTR,
		"Source", "feelin/preference/decorator.xml",

		TAG_MORE,Msg))
	{
		struct in_deco *node;
		uint32 i = 0;

		IFEELIN F_Do
		(
			Obj, F_RESOLVED_ID(GETOBJECTS),

			"decorator-class",  &LOD->chooser,
			"decorator-group",  &LOD->decorator_group,
			"group-edit",       &LOD->grp_edit,

			NULL
		);

		#ifdef DB_NEW

		IFEELIN F_Log
		(
			0, "decorator-class (0x%08lx) decorator-group (0x%08lx) group-edit (0x%08lx)",

			LOD->chooser,
			LOD->decorator_group,
			LOD->grp_edit
		);

		#endif

		for (node = (struct in_deco *) LOD->decoratorslist.Head ; node ; node = node->next)
		{
			node->position = i++;
			
			IFEELIN F_Do(LOD->chooser, FM_AddMember, node->item, FV_AddMember_Tail);
		}
			
		IFEELIN F_Do(LOD->chooser, FM_Notify, "FA_Radio_Active",FV_Notify_Always, Obj,FM_CallHookEntry,3, code_select_decorator,Class,FV_Notify_Value);
		
		return Obj;
	}
 
	return NULL;
}
//+
///Prefs_Dispose
F_METHOD(void,Prefs_Dispose)
{
	struct p_LocalObjectData *LOD = F_LOD(Class,Obj);
	struct in_deco *gnode;

	while ((gnode = IFEELIN F_LinkRemove(&LOD->decoratorslist, LOD->decoratorslist.Head)) != NULL)
	{
		IFEELIN F_Dispose(gnode->spec);
		IFEELIN F_DisposeObj(gnode->object);
		IFEELIN F_Dispose(gnode->classname);
		IFEELIN F_Dispose(gnode);
	}

	F_SUPERDO();
	
	IFEELIN F_SharedClose(LOD->moduleslist); LOD->moduleslist = NULL;
}
//+

///Prefs_Load
F_METHODM(uint32,Prefs_Load,FS_PreferenceGroup_Load)
{
	struct p_LocalObjectData *LOD = F_LOD(Class,Obj);
	struct in_deco *gnode;

	uint32 active = 0;
	FCSSProperty *property = (FCSSProperty *) IFEELIN F_Do(Msg->Prefs, Msg->id_GetProperty, "window", "decorator");

	if (property)
	{
		for (gnode = (struct in_deco *) LOD->decoratorslist.Head ; gnode ; gnode = gnode->next)
		{

			/* while we pass the method to decorator preference objets  we  look
			for the previously selected one */

			if (IFEELIN F_StrCmp(property->Value, gnode->classname, ALL) == 0)
			{
				active = gnode->position;
			}

			F_OBJDO(gnode->object);
		}
	}

	IFEELIN F_Set(LOD->chooser, (uint32) "FA_Radio_Active", active);

	return F_SUPERDO();
}
//+
///Prefs_Save
F_METHODM(uint32,Prefs_Save,FS_PreferenceGroup_Save)
{
	struct p_LocalObjectData *LOD = F_LOD(Class,Obj);
	struct in_deco *gnode;

	uint32 active = IFEELIN F_Get(LOD->chooser, (uint32) "FA_Radio_Active");
	STRPTR name = NULL;

	for (gnode = (struct in_deco *) LOD->decoratorslist.Head ; gnode ; gnode = gnode->next)
	{
		if (active == gnode->position)
		{
			name = gnode->classname;
		}

		F_OBJDO(gnode->object);
	}

	IFEELIN F_Do(Msg->Prefs, Msg->id_AddProperty, "window", "decorator", name, FV_TYPE_STRING);

	return F_SUPERDO();
}
//+

#endif
