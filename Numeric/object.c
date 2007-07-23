#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Numeric_New
F_METHOD(uint32,Numeric_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	F_SAVE_AREA_PUBLIC;
 
	LOD->Min    = 0;
	LOD->Max    = 100;
	LOD->Steps  = 10;
	LOD->Format = "%ld";
	
	return F_SUPERDO();
}
//+
///Numeric_Dispose
F_METHOD(void,Numeric_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->String != NULL)
	{
		IFEELIN F_Dispose(LOD->String);

		LOD->String = NULL;
	}
	
	F_SUPERDO();
}
//+
///Numeric_Set
F_METHOD(void,Numeric_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,*tag,item;

	bool32 update=FALSE;

	struct TagItem *value_tag = NULL;
 
	while ((tag = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_DEFAULT:
		{
			LOD->Default = (int32)(item.ti_Data);
		}
		break;

		case FV_ATTRIBUTE_MIN:
		{
			LOD->Min = (int32) item.ti_Data;

			update = TRUE;
		}
		break;

		case FV_ATTRIBUTE_MAX:
		{
			LOD->Max = (int32) item.ti_Data;

			update = TRUE;
		}
		break;

		case FV_ATTRIBUTE_STEPS:
		{
			LOD->Steps = (int32) item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_FORMAT:
		{
			LOD->Format = (STRPTR) item.ti_Data;

			update = TRUE;
		}
		break;

		case FV_ATTRIBUTE_COMPUTEHOOK:
		{
			LOD->ComputeHook = (APTR) item.ti_Data;

			update = TRUE;
		}
		break;

		case FV_ATTRIBUTE_STRINGARRAY:
		{
			LOD->StringArray = (STRPTR *) item.ti_Data;

			update = TRUE;
		}
		break;
 
		case FV_ATTRIBUTE_VALUE:
		{
			if (value_tag != NULL)
			{
				value_tag->ti_Tag = TAG_IGNORE;
			}

			value_tag = tag;

			update = TRUE;
		}
		break;
	}

	if (update)
	{
		int32 val;

 		if (value_tag != NULL)
		{
			val = (int32) value_tag->ti_Data;
		}
		else
		{
			val = LOD->Value;
		}

		if (LOD->ComputeHook != NULL)
		{
			val = IUTILITY CallHookPkt(LOD->ComputeHook, Obj, &val);
		}

		val = MAX(val, LOD->Min);
		val = MIN(val, LOD->Max);

		if (value_tag != NULL)
		{
			value_tag->ti_Data = val;
		}

		if (LOD->Value != val)
		{
			LOD->Value = val;

			if ((LOD->Format != NULL) || (LOD->StringArray != NULL))
			{
			   IFEELIN F_Do(Obj, F_METHOD_ID(STRINGIFY), (uint32) val);
			}
				
			IFEELIN F_Draw(Obj, FF_Draw_Update | FF_Draw_Damaged);
		}
	}

	F_SUPERDO();
}
//+
///Numeric_Get
F_METHOD(void,Numeric_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_DEFAULT:      F_STORE(LOD->Default); break;
		case FV_ATTRIBUTE_VALUE:        F_STORE(LOD->Value); break;
		case FV_ATTRIBUTE_MIN:          F_STORE(LOD->Min); break;
		case FV_ATTRIBUTE_MAX:          F_STORE(LOD->Max); break;
		case FV_ATTRIBUTE_STEP:         F_STORE(MAX(1,(LOD->Max - LOD->Min) / LOD->Steps)); break;
		case FV_ATTRIBUTE_BUFFER:       F_STORE(LOD->String); break;
		case FV_ATTRIBUTE_STRINGARRAY:  F_STORE(LOD->StringArray); break;
	}

	F_SUPERDO();
}
//+

