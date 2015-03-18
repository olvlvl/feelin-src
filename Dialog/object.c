#include "Private.h"

struct FS_HOOK_RESPONSE                         { FClass *Class; uint32 Response; };

/************************************************************************************************
**** Private ************************************************************************************
************************************************************************************************/

///code_response
STATIC F_HOOKM(void,code_response,FS_HOOK_RESPONSE)
{
	struct FeelinClass *Class = Msg->Class;

	IFEELIN F_Set(Obj,FA_Window_Open,FALSE);
 
	IFEELIN F_Do
	(
		#ifdef F_NEW_GLOBALCONNECT

	    (FObject) IFEELIN F_Get(Obj, FA_Element_Application), FM_Application_PushMethod,

		#else

	    (FObject) IFEELIN F_Get(Obj, FA_Application), FM_Application_PushMethod,

		#endif

		Obj,FM_Set,2,

		F_ATTRIBUTE_ID(RESPONSE), Msg->Response
	);
}
//+

/************************************************************************************************
**** Methods ************************************************************************************
************************************************************************************************/

///Dialog_New
F_METHOD(FObject,Dialog_New)
{
	struct TagItem *Tags = Msg,item;

	uint32 buttons=FV_Dialog_Buttons_None;
	bool32 separator=FALSE;
	FObject grp,ok=NULL,cancel=NULL,apply=NULL,yes=NULL,no=NULL,all=NULL,save=NULL,use=NULL;
	STRPTR applytext=F_CAT(APPLY);

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_BUTTONS:
		{
			buttons = item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_SEPARATOR:
		{
			separator = item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_APPLYTEXT:
		{
			applytext = (STRPTR) item.ti_Data;
		}
		break;
	}

	switch (buttons)
	{
		case FV_Dialog_Buttons_Ok:
		{
			grp = HGroup,
				Child, ok = Button(F_CAT(OK)),
			End;
		}
		break;
		
		case FV_Dialog_Buttons_Boolean:
		{
			//grp = HGroup, FA_Group_SameSize,TRUE, FA_Widget_SetMax,FV_Widget_SetWidth, FA_Align, FV_Align_End,
			grp = HGroup,
				Child, ok = Button(F_CAT(OK)),
				Child, cancel = Button(F_CAT(CANCEL)),
			End;
		}
		break;
		  
		case FV_Dialog_Buttons_Confirm:
		{     
			grp = HGroup,
				Child, ok = Button(F_CAT(OK)),
				Child, apply = Button(applytext),
				Child, cancel = Button(F_CAT(CANCEL)),
			End;
		}
		break;

		case FV_Dialog_Buttons_Always:
		{
			grp = HGroup,
				Child, yes = Button(F_CAT(YES)),
				Child, all = Button(F_CAT(ALL)),
				Child, no = Button(F_CAT(NO)),
				Child, cancel = Button(F_CAT(CANCEL)),
			End;
		}
		break;
	
		case FV_Dialog_Buttons_Preference:
		{
			grp = HGroup,
				Child, save = Button(F_CAT(SAVE)),
				Child, use = Button(F_CAT(USE)),
				Child, cancel = Button(F_CAT(CANCEL)),
			End;
		}
		break;
	
		case FV_Dialog_Buttons_PreferenceTest:
		{
			grp = HGroup, FA_Widget_SetMax,FV_Widget_SetHeight,
				Child, save = Button(F_CAT(SAVE)),
				Child, use = Button(F_CAT(USE)),
				Child, apply = Button(applytext),
				Child, BarObject, End,
				Child, cancel = Button(F_CAT(CANCEL)),
			End;
		}
		break;

		default:
		{
			IFEELIN F_Log(0, "unknown buttons request (%ld)", buttons);

			grp = (FObject) -1;
		}
		break;
	}

	if (IFEELIN F_SuperDo(Class,Obj,Method,
		
		FA_Window_Open, TRUE,
		
		Child, VGroup,
			(separator) ? FA_Child : TAG_IGNORE, (separator) ? BarObject, End : NULL,
			((uint32)(grp) == -1) ? TAG_IGNORE : FA_Child, grp,
		End,

		TAG_MORE,Msg))
	{
		IFEELIN F_Do(ok,FM_Notify,FA_Widget_Pressed,FALSE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_Ok);
		IFEELIN F_Do(cancel,FM_Notify,FA_Widget_Pressed,FALSE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_Cancel);
		IFEELIN F_Do(apply,FM_Notify,FA_Widget_Pressed,FALSE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_Apply);
		IFEELIN F_Do(yes,FM_Notify,FA_Widget_Pressed,FALSE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_Yes);
		IFEELIN F_Do(no,FM_Notify,FA_Widget_Pressed,FALSE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_No);
		IFEELIN F_Do(all,FM_Notify,FA_Widget_Pressed,FALSE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_All);
		IFEELIN F_Do(save,FM_Notify,FA_Widget_Pressed,FALSE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_Save);
		IFEELIN F_Do(use,FM_Notify,FA_Widget_Pressed,FALSE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_Use);

		return Obj;
	}
	return NULL;
}
//+
///Dialog_Open
F_METHOD(uint32,Dialog_Open)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	uint32 rc = F_SUPERDO();
  
	if (rc)
	{
		LOD->CloseRequest_NH = (APTR) IFEELIN F_Do(Obj,FM_Notify,FA_Window_CloseRequest,TRUE,Obj,FM_CallHookEntry,3,F_FUNCTION_GATE(code_response),Class,FV_Dialog_Response_Cancel);
	}

	return rc;
}
//+
///Dialog_Close
F_METHOD(uint32,Dialog_Close)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->CloseRequest_NH)
	{
		IFEELIN F_Do(Obj,FM_UnNotify,LOD->CloseRequest_NH); LOD->CloseRequest_NH = NULL;
	}

	return F_SUPERDO();
}
//+

