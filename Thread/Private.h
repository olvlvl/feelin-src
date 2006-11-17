/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <dos/dostags.h>
#include <libraries/feelin.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

        FV_ATTRIBUTE_ENTRY,
        FV_ATTRIBUTE_STACKSIZE,
        FV_ATTRIBUTE_PRIORITY,
        FV_ATTRIBUTE_NAME,
        FV_ATTRIBUTE_PROCESS,
        FV_ATTRIBUTE_USERDATA,
        FV_ATTRIBUTE_PORT

        };

enum    {

        FV_METHOD_SEND,
        FV_METHOD_POP,
        FV_METHOD_WAIT

        };

#define FV_Thread_Init                          FV_Thread_Reserved1

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

struct FS_Thread_Init
{
    FClass                         *Class;
    FObject                         Object;
    F_THREAD_ENTRY_PROTO         ((*UserEntry));
    APTR                            UserData;
};

#define F_MSG_THREAD_INIT(cl,obj,entry,data)    cl,obj,entry,data

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
    FThreadPublic                   pub;
    struct SignalSemaphore          arbitrer;
};
