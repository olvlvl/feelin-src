#include "Private.h"

///Prop_Decrease
F_METHODM(uint32,Prop_Decrease,FS_Prop_Decrease)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    if (LOD->first > 0)
    {
        IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), LOD->first - Msg->Value);
    }

    return LOD->first;
}
//+
///Prop_Increase
F_METHODM(uint32,Prop_Increase,FS_Prop_Increase)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    if (LOD->first + LOD->visible < LOD->entries)
    {
        IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(FIRST), LOD->first + Msg->Value);
    }

    return LOD->first;
}
//+
