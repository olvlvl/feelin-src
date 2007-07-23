//#define F_ENABLE_VERBOSE

/************************************************************************************************
*** includes ************************************************************************************
************************************************************************************************/

#include <exec/memory.h>
#include <libraries/feelin.h>

#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/feelin.h>

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Meta ****************************************************************************************
************************************************************************************************/

#ifdef F_ENABLE_VERBOSE

enum    {

        FM_Meta_Debug

        };

#define TEMPLATE                    "ADD/S,REM=REMOVE/S"
#define DEBUGVARNAME                "ENV:Feelin/DBG_FAMILY"

enum  {

      OPT_ADD,
      OPT_REM

      };

#endif

struct ClassUserData
{
    APTR                            NodePool;

    #ifdef F_ENABLE_VERBOSE

    FObject                         db_Notify;
    int32                           db_AddMember;
    int32                           db_RemMember;

    #endif
};

#ifdef F_ENABLE_VERBOSE

#define db_Array                                db_AddMember

#endif

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
    FList                           list;
    FObject                         owner;
    struct Hook                    *create_node_hook;
    struct Hook                    *delete_node_hook;
};

