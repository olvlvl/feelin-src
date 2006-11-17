/*
**    lib_Shared.c
**
**    Shared objects management
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************
                
$VER: 02.01 (2005/09/30)

    Added AmigaOS 4 support.
 
************************************************************************************************/

#include "Private.h"

//#define DB_SHARED_FIND
//#define DB_SHARED_CREATE
//#define DB_SHARED_OPEN
//#define DB_SHARED_CLOSE

struct FeelinShared
{
    struct FeelinShared            *next;
    FAtom                          *atom;

    uint32                          user_count;
    FObject                         object;                 // referenced object (shortcut to FC_Shared->object)
};

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///f_shared_findnode
STATIC struct FeelinShared * f_shared_findnode(STRPTR Name,struct in_FeelinBase *FeelinBase)
{
    FAtom *atom = IFEELIN F_AtomFind(Name, ALL);

    if (atom)
    {
        struct FeelinShared *node;

        for (F_SHAREDS_EACH(node))
        {
            if (atom == node->atom)
            {
                break;
            }
        }

        return node;
    }

    return NULL;
}
//+
///f_shared_createnode
STATIC struct FeelinShared * f_shared_createnode(FObject Object,STRPTR Name,struct in_FeelinBase *FeelinBase)
{
///DB_SHARED_CREATE
#ifdef DB_SHARED_CREATE
    IFEELIN F_Log(0,"F_SharedCreate() - Object 0x%08lx - Name '%s'",Object,Name);
#endif
//+

    if (Name && Object)
    {
///DB_SHARED_CREATE
#ifdef DB_SHARED_CREATE
        IFEELIN F_Log(0,"find name '%s'",Name);
#endif
//+

        if (IFEELIN F_SharedFind(Name) == NULL)
        {
            struct FeelinShared *node = IFEELIN F_New(sizeof (struct FeelinShared));

///DB_SHARED_CREATE
#ifdef DB_SHARED_CREATE
            IFEELIN F_Log(0,"allocate node");
#endif
//+

            if (node)
            {
                node->object = Object;
                node->atom = IFEELIN F_AtomObtain(Name, ALL);

                if (node->atom)
                {
                    node->next = F_SHAREDS_INDEX;
                    F_SHAREDS_INDEX = node;

                    return node;
                }

                IFEELIN F_Dispose(node);
            }
        }
    }
    return NULL;
}
//+

/************************************************************************************************
*** API *****************************************************************************************
************************************************************************************************/

///f_shared_find
F_LIB_SHARED_FIND
{
    struct FeelinShared *node;

    #ifdef DB_SHARED_FIND
    IFEELIN F_Log(0,"find: '%s'",Name);
    #endif

    if ((node = f_shared_findnode(Name,FeelinBase)))
    {
        return node->object;
    }
    return NULL;
}
//+
///f_shared_create
F_LIB_SHARED_CREATE
{
    struct FeelinShared *node;

    #ifdef DB_SHARED_CREATE
    IFEELIN F_Log(0,"F_SharedCreate() - Object 0x%08lx - Name '%s'",Object,Name);
    #endif
    
    F_SHAREDS_LOCK;

    node = f_shared_createnode(Object, Name, FeelinBase);
    
    F_SHAREDS_UNLOCK;
    
    if (node)
    {
        return node->object;
    }
    return NULL;
}
//+
///f_shared_delete
F_LIB_SHARED_DELETE
{
    if (Shared)
    {
        struct FeelinShared *prev = NULL;
        struct FeelinShared *node;

        F_SHAREDS_LOCK;

        for (F_SHAREDS_EACH(node))
        {
            if (node->object == Shared)
            {
                if (node->user_count)
                {
                    IFEELIN F_Log(FV_LOG_DEV,"Shared %s{%08lx} refs %ld - forced remove !", node->atom->Key, node->object, node->user_count);
                }

                IFEELIN F_AtomRelease(node->atom);

                if (prev)
                {
                    prev->next = node->next;
                }
                else
                {
                    F_SHAREDS_INDEX = node->next;
                }

                IFEELIN F_DisposeObj(node->object);

                IFEELIN F_Dispose(node);

                break;
            }

            prev = node;
        }
     
        if (!node)
        {
            IFEELIN F_Log(FV_LOG_DEV,"Shared %s{%08lx} not found",_object_classname(Shared),Shared);
        }

        F_SHAREDS_UNLOCK;
    }
}
//+
///f_shared_open
F_LIB_SHARED_OPEN
{
    if (Name)
    {
        struct FeelinShared *node;

        F_SHAREDS_LOCK;

        node = f_shared_findnode(Name, FeelinBase);
        
        if (!node)
        {
            FObject object = IFEELIN F_NewObj(Name, TAG_DONE);

            if (object)
            {
                #ifdef DB_SHARED_OPEN
                IFEELIN F_Log(0,"F_Shared_Open() - Object %s{%08lx}", _object_classname(object), object);
                #endif

                node = f_shared_createnode(object, Name, FeelinBase);

                #ifdef DB_SHARED_OPEN
                IFEELIN F_Log(0,"F_Shared_Open() - Node 0x%08lx",node);
                #endif

                if (!node)
                {
                    IFEELIN F_DisposeObj(object);
                }
            }
            else
            {
                IFEELIN F_Log(FV_LOG_DEV,"F_Shared_Open() - Unable to create object from '%s'",Name);
            }
        }

        if (node)
        {
           node->user_count++;
        }

        F_SHAREDS_UNLOCK;

        if (node)
        {
           return node->object;
        }
    }
    return NULL;
}
//+
///f_shared_close
F_LIB_SHARED_CLOSE
{
    if (Shared)
    {
        struct FeelinShared *node;

        F_SHAREDS_LOCK;

        for (F_SHAREDS_EACH(node))
        {
            if (node->object == Shared)
            {
                if ((--node->user_count) == 0)
                {
                    IFEELIN F_SharedDelete(Shared);
                }

                break;
            }
        }
     
        if (!node)
        {
            IFEELIN F_Log(FV_LOG_DEV,"F_SharedClose() %s{%08lx} not found",_object_classname(Shared),Shared);
        }

        F_SHAREDS_UNLOCK;
    }
}
//+

