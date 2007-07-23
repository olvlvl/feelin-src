/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <dos/dos.h>
#include <libraries/feelin.h>
#include <feelin/xmlobject.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_REFERENCES,
		FV_ATTRIBUTE_TAGS

		};

enum    {

		FV_METHOD_READ,
		FV_METHOD_RESOLVE,
		FV_METHOD_BUILD,
		FV_METHOD_CREATE
		#ifndef F_NEW_GETELEMENTBYID
		,FV_METHOD_GETOBJECTS
		#endif

		};

enum    {

		FV_RESOLVED_LOG

		};

enum    {

		FV_ATOM_ID,
		FV_ATOM_OBJECTS,
		FV_ATOM_STYLE,
		FV_ATOM_CLASS

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct in_Reference
{
	struct in_Reference            *next;
	FAtom                          *atom;
	FObject                         object;
};

struct LocalObjectData
{
	struct in_Reference            *references;

	struct TagItem                 *tags_initial;
	struct TagItem                 *tags_built;
};
