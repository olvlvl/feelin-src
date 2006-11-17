#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Numeric_Reset
F_METHOD(int32,Numeric_Reset)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(VALUE), LOD->Default);

    return LOD->Value;
}
//+
///Numeric_Increase
F_METHODM(int32,Numeric_Increase,FS_Numeric_Increase)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(VALUE), LOD->Value + Msg->Value);

    return LOD->Value;
}
//+
///Numeric_Decrease
F_METHODM(int32,Numeric_Decrease,FS_Numeric_Decrease)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(VALUE), LOD->Value - Msg->Value);

    return LOD->Value;
}
//+
///Numeric_Stringify
F_METHODM(STRPTR,Numeric_Stringify,FS_Numeric_Stringify)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    if (LOD->StringArray)
    {
        IFEELIN F_Dispose(LOD->String);
        return LOD->String = IFEELIN F_StrNew(NULL,LOD->StringArray[Msg->Value]);
    }
    else if (LOD->Format)
    {
        IFEELIN F_Dispose(LOD->String);
        return LOD->String = IFEELIN F_StrNew(NULL, LOD->Format, Msg->Value);
    }
    return NULL;
}
//+

///Numeric_HandleEvent
F_METHODM(uint32,Numeric_HandleEvent,FS_Widget_HandleEvent)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    if (Msg->Event->Key)
    {
        uint32 step = MAX(1,(LOD->Max - LOD->Min) / LOD->Steps);
        uint32 val;

        if (_area_is_horizontal)
        {
            switch (Msg->Event->Key)
            {
                case FV_KEY_LEFT:       val = LOD->Value - 1;    break;
                case FV_KEY_RIGHT:      val = LOD->Value + 1;    break;
                case FV_KEY_STEPLEFT:   val = LOD->Value - step; break;
                case FV_KEY_STEPRIGHT:  val = LOD->Value + step; break;
                case FV_KEY_FIRST:      val = LOD->Min;          break;
                case FV_KEY_LAST:       val = LOD->Max;          break;
                default:                return 0;
            }
        }
        else
        {
            switch (Msg->Event->Key)
            {
                case FV_KEY_UP:         val = LOD->Value - 1;    break;
                case FV_KEY_DOWN:       val = LOD->Value + 1;    break;
                case FV_KEY_STEPUP:     val = LOD->Value - step; break;
                case FV_KEY_STEPDOWN:   val = LOD->Value + step; break;
                case FV_KEY_TOP:        val = LOD->Min;          break;
                case FV_KEY_BOTTOM:     val = LOD->Max;          break;
                default:                return 0;
            }
        }

        IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(VALUE), val);

        return FF_HandleEvent_Eat;
    }
    else if (Msg->Event->Class == FF_EVENT_BUTTON)
    {
        if (Msg->Event->Code == FV_EVENT_BUTTON_WHEEL)
        {
            if (F_IS_INSIDE(Msg->Event->MouseX,_area_cx,_area_cx2) &&
                F_IS_INSIDE(Msg->Event->MouseY,_area_cy,_area_cy2))
            {
                uint32 step = MAX(1,(LOD->Max - LOD->Min) / LOD->Steps);
 
                if (FF_EVENT_BUTTON_DOWN & Msg->Event->Flags)
                {
                    IFEELIN F_Do(Obj, F_METHOD_ID(INCREASE), step);
                }
                else
                {
                    IFEELIN F_Do(Obj, F_METHOD_ID(DECREASE), step);
                }
            }
        }
    }
    return 0;
}
//+
