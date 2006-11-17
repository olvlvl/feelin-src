//#define F_ENABLE_DISPLAY

/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <exec/memory.h>
#include <utility/tagitem.h>
#include <libraries/feelin.h>

#include <feelin/string.h>
#include <feelin/xmldocument.h>

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_MARKUPS,
		FV_ATTRIBUTE_ATTRIBUTES,
		FV_ATTRIBUTE_INSTRUCTIONS,
		FV_ATTRIBUTE_ENTITIES

		};
		
enum    {

		FV_METHOD_PUSH,
		FV_METHOD_POP,
		FV_METHOD_ADD,
		FV_METHOD_ADDINT,
		FV_METHOD_ADDENTITY,
		FV_METHOD_ADDENTITIES,
		FV_METHOD_FINDENTITY,
		FV_METHOD_WRITE,
		FV_METHOD_PARSE

		};

enum    {

		FV_RESOLVED_POOL,
		FV_RESOLVED_SOURCE,
		FV_RESOLVED_SOURCETYPE,
		FV_RESOLVED_VERSION,
		FV_RESOLVED_REVISION,
		FV_RESOLVED_READ,
		FV_RESOLVED_RESOLVE,
		FV_RESOLVED_LOG,
		FV_RESOLVED_NUMERIFY,
		FV_RESOLVED_CREATEPOOL

		};

enum    {

		FV_XML_ERROR_NONE,
		FV_XML_ERROR_NULL,
		FV_XML_ERROR_MEMORY,
		FV_XML_ERROR_SYNTAX,
		FV_XML_ERROR_HEADER,
		FV_XML_ERROR_OPEN,
		FV_XML_ERROR_CLOSE,
		FV_XML_ERROR_INTEROGATION,
		FV_XML_ERROR_EQUAL,
		FV_XML_ERROR_QUOTE,
		FV_XML_ERROR_NAME,
		FV_XML_ERROR_ATTRIBUTE,
		FV_XML_ERROR_ENTITY,
		FV_XML_ERROR_ENTITY_TYPE

		};

enum    {

		FV_ATOM_VERSION

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	FList							pi_list;
	FList                           attributes_list;
	FList                           markups_list;
	APTR                            pool;
	APTR                            atomspool;
	uint32                          line;

	FXMLMarkup                     *markup_cursor;

	FHashTable                     *entities;
};

#define QUOTE_SINGLE                            39
#define QUOTE_DOUBLE                            34

#if 1
#define F_SKIPWHITE(str)                        while (*str && (*str == ' ' || *str == '\n')) { if (*str == '\n') { LOD -> line++; /*IFEELIN F_Log(0,"line %ld (%16.16s)",LOD->line,str-16);*/ } str++; }
#define F_NEXTWHITE(str)                        while (*str && (*str != ' ' && *str != '\n')) str++
#else
#define F_SKIPWHITE(str)                        while (*str && (*str == ' ' || *str == '\t' || *str == '\n')) { if (*str == '\n') { LOD -> line++; /*IFEELIN F_Log(0,"line %ld (%16.16s)",LOD->line,str-16);*/ } str++; }
#define F_NEXTWHITE(str)                        while (*str && (*str != ' ' && *str != '\t' && *str != '\n')) str++
#endif

#define F_ERROR(n)                              { er = FV_XML_ERROR_ ## n; goto __error; }

