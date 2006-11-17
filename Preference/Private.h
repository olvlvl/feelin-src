/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/feelin.h>

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_METHOD_CLEAR,
		FV_METHOD_READ,
		FV_METHOD_WRITE,
		FV_METHOD_CHANGE,
		FV_METHOD_MERGE

		};

enum    {

		FV_ATTRIBUTE_NAME,
		FV_ATTRIBUTE_REFERENCE,
		FV_ATTRIBUTE_UPDATE

		};

enum    {

		FV_RESOLVED_POOL,
		FV_RESOLVED_SOURCE,
		FV_RESOLVED_SOURCETYPE,
		FV_RESOLVED_CLEAR,
		FV_RESOLVED_NUMERIFY,
		FV_RESOLVED_WRITE,
		FV_RESOLVED_DECLARATIONS

		};

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

struct ClassUserData
{
	FObject                         arbiter;
};

#define F_LOCK_ARBITER                          IFEELIN F_Do(CUD->arbiter, FM_Lock, FF_Lock_Exclusive)
#define F_LOCK_ARBITER_SHARED                   IFEELIN F_Do(CUD->arbiter, FM_Lock, FF_Lock_Shared)
#define F_UNLOCK_ARBITER                        IFEELIN F_Do(CUD->arbiter, FM_Unlock)

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	APTR                            pool;

	struct in_Item                 *items;

	FObject                         notify;
	STRPTR                          name;
	STRPTR                          filename;
	FObject                         reference;
	uint32                          reference_notify_handle;

	FList                           declaration_list;
	struct in_Style                *styles;
};

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

STRPTR preference_resolve_name(FClass *Class,FObject Obj,STRPTR Name);

bool32 						declaration_create	(FClass *Class, FObject Obj, FCSSDeclaration *CSSDeclaration, FCSSDeclaration *CSSMasterDeclaration);
struct in_Declaration * 	declaration_delete	(FClass *Class, FObject Obj, struct in_Declaration *Declaration);
void 						declaration_flush	(FClass *Class, FObject Obj);

bool32 	style_delete	(FClass *Class, FObject Obj, struct in_Style *Style);
void 	style_flush		(FClass *Class, FObject Obj);

