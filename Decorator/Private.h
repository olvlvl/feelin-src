/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/dos.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

        FV_RESOLVED_GETOBJECTS

        };

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct in_deco
{
    struct in_deco                 *next;
    struct in_deco                 *prev;

    FObject                         object;
    STRPTR                          name;
    STRPTR                          spec;
    STRPTR                          classname;
    uint32                          position;

    FObject                         item;
};

struct p_LocalObjectData
{
    FObject                         grp_edit;
    FObject                         chooser;

    FObject                         moduleslist;
    FList                           decoratorslist;

    FObject                         decorator_group;
    FObject                         active_decorator;
};

