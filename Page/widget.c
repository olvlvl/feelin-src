#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Page_HandleEvent
F_METHODM(bits32,Page_HandleEvent,FS_Widget_HandleEvent)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    FEvent *fev = Msg->Event;

    if (fev->Key)
    {
        uint32 page;

        switch (fev->Key)
        {
            case FV_KEY_LEFT:    page = FV_Page_Active_Prev;  break;
            case FV_KEY_RIGHT:   page = FV_Page_Active_Next;  break;
            case FV_KEY_LAST:    page = FV_Page_Active_Last;  break;
            case FV_KEY_FIRST:   page = FV_Page_Active_First; break;
            default:             return 0;
        }

        IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(ACTIVE), page);

        return FF_HandleEvent_Eat;
    }
    else if (fev->Class == FF_EVENT_BUTTON)
    {
        int16 x = _area_x;
        int16 y = _area_y;

        if (fev->MouseY >= y &&
            fev->MouseX >= x &&
            fev->MouseY <= y + LOD->tabs_height - 1 &&
            fev->MouseX <= x + _area_w - 1)
        {
            if ((fev->Code == FV_EVENT_BUTTON_SELECT) && (FF_EVENT_BUTTON_DOWN & fev->Flags))
            {
                if (IFEELIN F_Get(Obj, FA_Family_Head))
                {
                    FWidgetNode *node;

                    uint16 i = 0, n = 0, w;

                    for (_each)
                    {
                        i++;
                    }

                    for (_each)
                    {
                        w = (_area_x2 - x + 1) / i-- - 1;

                        if (node == LOD->node_active)
                        {
                            if (fev->MouseX >= x && fev->MouseX <= x + w - 1)
                            {
                                IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(ACTIVE), n); break;
                            }
                        }
                        else
                        {
                            if (fev->MouseX >= x && fev->MouseX <= x + w - 1 && fev->MouseY >= y + LOD->style->inactive_margin_top)
                            {
                                IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(ACTIVE), n); break;
                            }
                        }

                        x += w; n++;
                    }
                }
                return FF_HandleEvent_Eat;
            }
            else if (fev->Code == FV_EVENT_BUTTON_WHEEL)
            {
                IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(ACTIVE), (FF_EVENT_BUTTON_DOWN & fev->Flags) ? FV_Page_Active_Next : FV_Page_Active_Prev);

                return FF_HandleEvent_Eat;
            }
        }
    }
    return 0;
}
//+
