/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/graphics.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

        FV_ATTRIBUTE_INFO

        };

enum    {

        FV_RESOLVED_VALUE,
        FV_RESOLVED_MIN,
        FV_RESOLVED_MAX

        };

enum    {

        FV_PROPERTY_IMAGE

        };

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
    F_MEMBER_AREA_PUBLIC;

    FObject                         image;
    STRPTR                          info;
};

#if 0

/************************************************************************************************
*** Preferences *********************************************************************************
************************************************************************************************/

struct p_LocalObjectData
{
    FAreaPublic                    *AreaPublic;
    FObject                         example;

    APTR                            timer_handler;

    int32                           Val;
};

#define FM_Gauge_Update                         (FCCM_BASE + 1)

enum    {                                       // Preference Auto

        p_FA_Numeric_Min,
        p_FA_Numeric_Max,
        p_FM_Numeric_Increase

        };

#endif
