#define F_ENABLE_CLONE_WORKAROUND

/*

	Strangely, copying a file to a notified one triggers 2  notifies,  while
	modifiying    a    file    triggers    4    notifies.    Enabling    the
	F_ENABLE_CLONE_WORKAROUND symbol changes the way messages are  read  and
	handled.  If  the  symbol  is enabled only the first message is proceed,
	following message with the same notify requester are ignored.

*/

/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <dos/notify.h>
#include <libraries/feelin.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/feelin.h>

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

struct ClassUserData
{
	FObject                         thread;
	struct Hook						thread_hook;

	uint32                          id_Send;
	uint32                          id_Update;
};

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_NAME

		}; 

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	struct NotifyRequest            Notify;
};

/************************************************************************************************
*** Thread **************************************************************************************
************************************************************************************************/

F_HOOKM(FThreadMsg *, Thread_Main, FS_Thread_Run);

enum    {

		FV_Thread_AddNotify = FV_Thread_Dummy,
		FV_Thread_RemNotify

		};

struct  FS_Thread_AddNotify                     { struct NotifyRequest *NReq; FObject Object; };
struct  FS_Thread_RemNotify                     { struct NotifyRequest *NReq; };

