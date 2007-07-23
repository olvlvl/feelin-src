/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>

#include <proto/feelin.h>
#include <proto/utility.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

        FV_ATTRIBUTE_DEFAULT,
        FV_ATTRIBUTE_VALUE,
        FV_ATTRIBUTE_MIN,
        FV_ATTRIBUTE_MAX,
        FV_ATTRIBUTE_STEP,
        FV_ATTRIBUTE_STEPS,
        FV_ATTRIBUTE_FORMAT,
        FV_ATTRIBUTE_BUFFER,
        FV_ATTRIBUTE_COMPUTEHOOK,
        FV_ATTRIBUTE_STRINGARRAY

        };

enum    {

        FV_METHOD_INCREASE,
        FV_METHOD_DECREASE,
        FV_METHOD_STRINGIFY,
        FV_METHOD_RESET

        };

enum    {

        FV_ATOM_VALUE

        };

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
    F_MEMBER_AREA_PUBLIC;

    int32                           Value;
    int32                           Min;
    int32                           Max;
    int32                           Default;
    uint32                          Steps;
    STRPTR                          String;
    STRPTR                         *StringArray;
    STRPTR                          Format;
    struct Hook                    *ComputeHook;
};

