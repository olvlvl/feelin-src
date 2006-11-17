/************************************************************************************************
*** Configuration *******************************************************************************
************************************************************************************************/

//#define F_ENABLE_ASSOCIATED_STATS

/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>
#include <feelin/cssdocument.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_METHOD_GETPROPERTY,
		FV_METHOD_ADDPROPERTY,
		FV_METHOD_PARSE,
		FV_METHOD_CLEAR

		};

enum    {

		FV_ATTRIBUTE_DECLARATIONS

		};

enum    {

		FV_RESOLVED_NUMERIFY,
		FV_RESOLVED_STRINGIFY

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	APTR                            pool;
	APTR                            atoms_pool;
	FCSSDeclaration                *declarations;

	uint32                          line;
};

/************************************************************************************************
*** Macros **************************************************************************************
************************************************************************************************/

#define F_CHAR(str)                             ((c = *str) != '\0')
#define F_IS_ALPHA(c)                           (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || (c == '-') || (c == '_') || (c == '.'))
#define F_IS_WHITE(c)                           ((c == ' ') || (c == '\n') || (c == '\t'))

#define F_SKIP_WHITE(str)                       while (F_CHAR(str) && (c == ' ' || c == '\t' || c == '\n')) { if (c == '\n') { LOD->line++; /*IFEELIN F_Log(0,"line %ld (%16.16s)",LOD->line,str-16);*/ } str++; }
#define F_NEXT_WHITE(str)                       while (F_CHAR(str) && (c != ' ' && c != '\t' && c != '\n')) str++
#define F_CHECKNULL(str)                        if (F_CHAR(str) == '\0') { IFEELIN F_Log(FV_LOG_DEV,"Unexpected end of data"); return NULL; };
