#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Numeric_LoadPersistentAttributes
F_METHOD(FPDRHandle *, Numeric_LoadPersistentAttributes)
{
    FPDRHandle *handle = (FPDRHandle *) F_SUPERDO();

    if (handle)
    {
        FPDRAttribute *attr;

        for (attr = handle->Attributes ; attr ; attr = attr->Next)
        {
            if (attr->Atom == F_ATOM(VALUE))
            {
                IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(VALUE), F_PDRDOCUMENT_GET_NUMERIC(attr));
            }
        }
    }

    return handle;
}
//+
///Numeric_SavePersistentAttributes
F_METHOD(FPDRHandle *, Numeric_SavePersistentAttributes)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    FPDRHandle *handle = (FPDRHandle *) F_SUPERDO();

    if (handle)
    {
        FPDRAttribute *attr;

        for (attr = handle->Attributes ; attr ; attr = attr->Next)
        {
            if (attr->Atom == F_ATOM(VALUE))
            {
                F_PDRDOCUMENT_SET_TYPED(attr, LOD->Value, FV_TYPE_INTEGER);
            }
        }
    }

    return handle;
}
//+
