/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>
#include <feelin/xmldocument.h>
#include <feelin/xmlapplication.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_METHOD_CREATENOTIFY,
		FV_METHOD_RUN,
		FV_METHOD_BUILD

		};

enum    {

		FV_RESOLVED_RESOLVE,
		FV_RESOLVED_LOG,
		FV_RESOLVED_GETOBJECTS

		};

enum    {

		FV_ATOM_APPLICATION,
		FV_ATOM_ATTRIBUTE,
		FV_ATOM_MESSAGE,
		FV_ATOM_METHOD,
		FV_ATOM_NOTIFY,
		FV_ATOM_OBJ,
		FV_ATOM_ROOT,
		FV_ATOM_TARGET,
		FV_ATOM_VALUE,
		FV_ATOM_STYLESHEET,
		FV_ATOM_HREF,
		FV_ATOM_TYPE

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	FObject                         Application;
};

#undef F_ERROR
#define F_ERROR(n)                              { er = FV_ERROR_ ## n; goto __error; }

enum  {

	  FV_ERROR_DUMMY,
	  FV_ERROR_MEMORY,
	  FV_ERROR_TYPE,
	  FV_ERROR_VERSION,
	  FV_ERROR_NULLMarkup

	  };

struct FS_XMLApplication_CreateNotify           { FXMLMarkup *Markup; APTR Pool; };

