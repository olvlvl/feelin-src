/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <exec/memory.h>
#include <utility/tagitem.h>
#include <libraries/feelin.h>

#include <feelin/string.h>
#include <feelin/document.h>

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_POOL,
		FV_ATTRIBUTE_SOURCE,
		FV_ATTRIBUTE_SOURCETYPE,
		FV_ATTRIBUTE_VERSION,
		FV_ATTRIBUTE_REVISION

		};

enum    {

		FV_METHOD_READ,
		FV_METHOD_MERGE,
		FV_METHOD_RESOLVE,

		#if F_CODE_DEPRECATED
		FV_METHOD_FINDNAME,
		FV_METHOD_OBTAINNAME,
		#endif

		FV_METHOD_LOG,
		FV_METHOD_CLEAR,
		FV_METHOD_NUMERIFY,
		FV_METHOD_STRINGIFY,
		FV_METHOD_CREATEPOOL,
		FV_METHOD_PARSE

		};

/************************************************************************************************
*** Structures **********************************************************************************
************************************************************************************************/

struct in_Name
{
	struct in_Name                 *next;
	FAtom                          *atom;
};

struct LocalObjectData
{
	APTR                            pool;

	struct in_Name                 *names;

	uint8                           version;
	uint8                           revision;
	uint16                          _pad0;
	STRPTR                          sourcename;
};
