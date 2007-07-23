#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///AppServer_New
F_METHOD(FObject,AppServer_New)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
 
    LOD->Server = Obj;

    if (F_SUPERDO())
    {
        LOD->Preference = PreferenceObject,

            "FA_Preference_Name", F_PREFERENCE_GLOBAL_NAME,

            End;
        
        if (LOD->Preference)
        {
            IFEELIN F_Do(LOD->Preference, (uint32) "FM_Preference_Read", FV_Preference_ENV);

            return Obj;
        }
    }
    return NULL;
}
//+
///AppServer_Dispose
F_METHOD(void,AppServer_Dispose)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
 
    F_SUPERDO();

    IFEELIN F_DisposeObj(LOD->Preference); LOD->Preference = NULL;
}
//+
///AppServer_Get
F_METHOD(void,AppServer_Get)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    struct TagItem *Tags = Msg,item;

    while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
    switch (item.ti_Tag)
    {
        case FV_ATTRIBUTE_PREFERENCE:
        {
            F_STORE(LOD->Preference);
        }
        break;
    }

    F_SUPERDO();
}
//+

