#include "Private.h"

//#define DB_RESOLVE
 
#define F_PREFERENCE_EXT                        "css"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///preference_resolve_name
STRPTR preference_resolve_name(FClass *Class,FObject Obj,STRPTR Name)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    
    STRPTR path = NULL;

    switch ((uint32)(Name))
    {
        case FV_Preference_ENV:
        {
            if (LOD->name)
            {
                path = IFEELIN F_StrNew(NULL, "ENV:Feelin/%s." F_PREFERENCE_EXT, LOD->name);
            }
        }
        break;

        case FV_Preference_ENVARC:
        {
            if (LOD->name)
            {
                path = IFEELIN F_StrNew(NULL, "ENVARC:Feelin/%s." F_PREFERENCE_EXT, LOD->name);
            }
        }
        break;

        case FV_Preference_BOTH:
        {
            IFEELIN F_Log(FV_LOG_DEV,"FV_Preference_BOTH not valid");
        }
        break;

        default:
        {
            path = IFEELIN F_StrNew(NULL, Name);
        }
        break;
    }

    #ifdef DB_RESOLVE
    
    IFEELIN F_Log(0,"path (%s)",path);
    
    #endif 

    return path;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Prefs_New
F_METHOD(FObject,Prefs_New)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    struct TagItem *Tags = Msg,item;

    while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
    switch (item.ti_Tag)
    {
        case FV_ATTRIBUTE_NAME:
        {
             LOD->name = (STRPTR) item.ti_Data;
        }
        break;

        case FV_ATTRIBUTE_REFERENCE:
        {
             LOD->reference = (FObject) item.ti_Data;
        }
        break;
    }

    LOD->pool = IFEELIN F_CreatePool
    (
        2048,
        
        FA_Pool_Items, 1,
        FA_Pool_Name, "prefrence.data",
        
        TAG_DONE
    );
    
    if (LOD->pool)
    {
        if (F_SUPERDO())
        {
            if (LOD->name)
            {
                LOD->filename = preference_resolve_name(Class,Obj,(STRPTR) FV_Preference_ENV);
                    
                if (LOD->filename)
                {
                    LOD->notify = DOSNotifyObject,
                        
                        "FA_DOSNotify_Name", LOD->filename,
                            
                        End;
                }
                
                if (LOD->notify)
                {
                    IFEELIN F_Do
                    (
                        LOD->notify, FM_Notify,
                            
                        "FA_DOSNotify_Update", FV_Notify_Always,
                            
                        Obj, F_METHOD_ID(CHANGE), 0
                    );
                }
                else
                {
                    IFEELIN F_Log(FV_LOG_USER,"Unable to create notify on '%s' (%s)",LOD->filename,LOD->name);

                    return NULL;
                }
            }

            LOD->reference_notify_handle = IFEELIN F_Do
            (
                LOD->reference, FM_Notify,
                F_ATTRIBUTE_ID(UPDATE), FV_Notify_Always,
                Obj, F_METHOD_ID(CHANGE), 0
                //Obj, FM_Set, 2, F_ATTRIBUTE_ID(UPDATE), FV_Notify_Value
            );

            return Obj;
        }
    }
    return NULL;
}
//+
///Prefs_Dispose
F_METHOD(uint32,Prefs_Dispose)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    if (LOD->reference_notify_handle != NULL)
    {
        IFEELIN F_Do(LOD->reference, FM_UnNotify, LOD->reference_notify_handle);
        
        LOD->reference_notify_handle = NULL;
    }

    IFEELIN F_DisposeObj(LOD->notify); LOD->notify = NULL;
    IFEELIN F_Dispose(LOD->filename); LOD->filename = NULL;

    #ifdef F_NEW_STYLES

    if (LOD->styles != NULL)
    {
        while (style_delete(Class, Obj, LOD->styles));
    }

    if (LOD->declaration_list.Head != NULL)
    {
        while (declaration_delete(Class, Obj, (struct in_Declaration *) LOD->declaration_list.Head));
    }

    #else

    LOD->table = NULL;

    #endif

    IFEELIN F_DeletePool(LOD->pool);

    LOD->pool = NULL;

    return F_SUPERDO();
}
//+
///Prefs_Get
F_METHOD(void,Prefs_Get)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    struct TagItem *Tags = Msg,item;

    while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
    switch (item.ti_Tag)
    {
        case FV_ATTRIBUTE_NAME:         
        {
            F_STORE(LOD->name);
        }
        break;

        case FV_ATTRIBUTE_REFERENCE:    
        {
            F_STORE(LOD->reference);
        }
        break;
    }

    F_SUPERDO();
}
//+
                                
///Prefs_FileChange
F_METHOD(uint32,Prefs_FileChange)
{
    #ifdef DB_FILECHANGE
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    #endif

    F_LOCK_ARBITER;
    
    #ifdef DB_FILECHANGE
    IFEELIN F_Log(0,"set 'update' - reference (0x%08lx)", LOD->reference);
    #endif

    IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(UPDATE), TRUE);
    
    #ifdef DB_FILECHANGE
    IFEELIN F_Log(0,"read from env");
    #endif

    IFEELIN F_Do(Obj, F_METHOD_ID(READ), FV_Preference_ENV);
 
    F_UNLOCK_ARBITER;
     
    return TRUE;
}
//+
