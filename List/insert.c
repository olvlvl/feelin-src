#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///list_lines_create
/*

    Each line is allocated from the LOD->LinePool and  linked  one  to  each
    other  from  an  expressed  previous line. The number of lines insterted
    will be returned.

    The function also call  the  FM_List_Construct  method  to  create  line
    strings, if this call fails the line is disposed and the function return
    with the number of inserted lines. If the line  is  successfuly  created
    the  function will also call the List_LineDims() function to update list
    informations.

*/
STATIC uint32 list_lines_create(FClass *Class,FObject Obj,FLine *Prev,uint32 Num,APTR *Entries,int32 Link)
{
    struct LocalObjectData  *LOD = F_LOD(Class,Obj);
    uint32 prev_entries = LOD->LineCount;
    uint32 prev_visible = LOD->Visible;
    FLine *prev_first = LOD->First;
    FLine *line;

    uint32 p = 0;
    uint32 i;
    
    for (i = 0 ; i < Num ; i++)
    {
        line = IFEELIN F_NewP(LOD->LinePool,
            sizeof (FLine) +                              // Struct
            ((LOD->ColumnCount + 1) * sizeof (STRPTR)) +  // Array of Strings
            ((LOD->ColumnCount + 1) * sizeof (STRPTR)) +  // Array of PreParses
            ((LOD->ColumnCount + 1) * sizeof (uint16)));  // Array of Widths

        if (line)
        {
            line->Strings   = (STRPTR *)((uint32)(line) + sizeof (FLine));
            line->PreParses = (STRPTR *)((uint32)(line->Strings)   + sizeof (STRPTR) * (LOD->ColumnCount + 1));
            line->Widths    = (uint16 *)((uint32)(line->PreParses) + sizeof (STRPTR) * (LOD->ColumnCount + 1));

            #ifdef DB_CREATE
            IFEELIN F_Log(0,"Line 0x%08lx (%03ld) - Prev 0x%08lx - Entry 0x%08lx - Strings 0x%08lx (%ld, %ld)\n",line,((uint32 *)(line))[-1],Prev,line->Entry,line->Strings,(uint32)(line->PreParses) - (uint32)(line->Strings),(uint32)(line->Widths) - (uint32)(line->PreParses));
            #endif

            line->Entry = (APTR) IFEELIN F_Do(Obj, F_METHOD_ID(CONSTRUCT), Entries[i], LOD->pool);

            if (line->Entry)
            {
                LOD->LineCount += 1;

                if (Link == FV_List_Sort_Descending)
                {
                    Prev = (FLine *)(LOD->LineList.Tail);

                    while (Prev)
                    {
                        if ((int32)(IFEELIN F_Do(Obj, F_METHOD_ID(COMPARE), line->Entry, Prev->Entry,FV_List_Sort_Descending,0)) > 0)
                        {
                            break;
                        }
                        Prev = (FLine *)(Prev->Prev);
                    }

                    if (Prev)
                    {
                        IFEELIN F_LinkInsert(&LOD->LineList,(FNode *) line,(FNode *) Prev);
                    }
                    else
                    {
                        IFEELIN F_LinkHead(&LOD->LineList,(FNode *) line);
                    }
                }
                else if (Link == FV_List_Sort_Ascending)
                {
                    Prev = (FLine *)(LOD->LineList.Tail);

                    while (Prev)
                    {
                        if ((int32) IFEELIN F_Do(Obj, F_METHOD_ID(COMPARE), line->Entry, Prev->Entry,FV_List_Sort_Descending,0) < 0)
                        {
                            break;
                        }
                        Prev = Prev->Prev;
                    }

                    if (Prev)
                    {
                        IFEELIN F_LinkInsert(&LOD->LineList,(FNode *) line,(FNode *) Prev);
                    }
                    else
                    {
                        IFEELIN F_LinkHead(&LOD->LineList,(FNode *) line);
                    }
                }
                else
                {
                    IFEELIN F_LinkInsert(&LOD->LineList,(FNode *) line,(FNode *) Prev);
                    Prev = line;
                }
                
            }
            else
            {
                #if 0
                IFEELIN F_Log(FV_LOG_DEV,"Construct of Line %ld failed",LOD->LineCount + 1);
                #endif

                break;
            }
            
            if (_area_render)
            {
                line_compute_dimensions(Class,Obj,line);
            }
        }
        else
        {
            #if 0
            IFEELIN F_Log(FV_LOG_DEV,"Unable to allocate Line");
            #endif
             
            break;
        }
    }

/*** update positions **************************************************************************/

    for (line = (FLine *)(LOD->LineList.Head) ; line ; line = line->Next)
    {
        line->Position = p++;
    }

    if (LOD->Quiet >= 0)
    {
        list_update(Class,Obj,FALSE);
 
        IFEELIN F_SuperDo(Class,Obj,FM_Set,

            (prev_entries != LOD->LineCount) ? F_ATTRIBUTE_ID(ENTRIES) : TAG_IGNORE,LOD->LineCount,
            (prev_visible != LOD->Visible) ? F_ATTRIBUTE_ID(VISIBLE) : TAG_IGNORE,LOD->Visible,
            (prev_first != LOD->First) ? F_ATTRIBUTE_ID(FIRST) : TAG_IGNORE,(LOD->First) ? LOD->First->Position : 0,

            TAG_DONE);

        IFEELIN F_Draw(Obj,FF_Draw_Update);
    }
    else
    {
        LOD->Flags |= FF_LIST_MODIFIED;
    }

    return i;
}
//+
///list_lines_delete
/*

    This function delete 'Number' lines starting with 'Head'.

*/
STATIC void list_lines_delete(FClass *Class,APTR Obj,FLine *Head,uint32 Number)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    if (Head && Number)
    {
        uint32 prev_entries = LOD->LineCount;
        uint32 prev_visible = LOD->Visible;
        FLine *prev_first = LOD->First;
        FLine *line;
        uint32 p=0;

        for ( ; Number ; Number--)
        {
            if ((line = IFEELIN F_NextNode((APTR)(&Head))) != NULL)
            {
                if (LOD->First  == line)  LOD->First  = line->Next;
                if (LOD->Last   == line)  LOD->Last   = line->Prev;
                if (LOD->Active == line)  LOD->Active = NULL;
                if (LOD->LastFirst == line) LOD->LastFirst = NULL;

                IFEELIN F_LinkRemove(&LOD->LineList,(FNode *) line);
                IFEELIN F_Do(Obj, F_METHOD_ID(DESTRUCT), line->Entry, LOD->pool);
                IFEELIN F_Dispose(line);

                LOD->LineCount--;
            }
            else break;
        }

/*** update positions **************************************************************************/

        for (line = (FLine *)(LOD->LineList.Head) ; line ; line = line->Next)
        {
            line->Position = p++;
        }

        if (LOD->Quiet >= 0)
        {
            list_update(Class,Obj,FALSE);

            IFEELIN F_SuperDo(Class,Obj,FM_Set,
            
                (prev_entries != LOD->LineCount) ? F_ATTRIBUTE_ID(ENTRIES) : TAG_IGNORE,LOD->LineCount,
                (prev_visible != LOD->Visible) ? F_ATTRIBUTE_ID(VISIBLE) : TAG_IGNORE,LOD->Visible,
                (prev_first != LOD->First) ? F_ATTRIBUTE_ID(FIRST) : TAG_IGNORE,(LOD->First) ? LOD->First->Position : 0,
                
                TAG_DONE);

            
            IFEELIN F_Draw(Obj,FF_Draw_Update);
        }
        else
        {
            LOD->Flags |= FF_LIST_MODIFIED;
        }
    }
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///List_Insert
F_METHODM(uint32,List_Insert,FS_List_Insert)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    FLine *prev;
    int32 count = Msg->Count;
    int32 link = FV_List_Sort_None;

    if (count == ALL) // Count the number of entries if Count == ALL (-1)
    {
        for (count = 0 ; Msg->Entries[count] ; count++);
    }

    if (count <= 0) return 0;

    switch (Msg->Pos)
    {
        case FV_List_Insert_Top:
        {
            prev = NULL;
        }
        break;

        case FV_List_Insert_Active:
        {
            if (LOD->Active)
            {
                prev = (FLine *)(LOD->Active->Prev);
            }
            else
            {
                prev = NULL;
            }
        }
        break;

        case FV_List_Insert_Sorted:
        {
            prev = NULL;
            link = LOD->SortMode;
        }
        break;

        case FV_List_Insert_Bottom:
        {
            prev = (FLine *)(LOD->LineList.Tail);
        }
        break;

        default:
        {
            if (Msg->Pos > LOD->LineCount)
            {
                prev = (FLine *)(LOD->LineList.Tail);
            }
            else if (Msg->Pos < 0)
            {
                prev = NULL;
            }
            else
            {
                for (prev = (FLine *)(LOD->LineList.Head) ; prev ; prev = prev->Next)
                {
                    if (prev->Position == Msg->Pos) break;
                }
            }
        }
        break;
    }

    if (LOD->TitleBar)
    {
        return list_lines_create(Class,Obj,prev,count,Msg->Entries,link);
    }
    else
    {
        IFEELIN F_Log(0,"Entry has not been added because TitleBar is NULL");
 
        return 0;
    }
}
//+
///List_InsertSingle
F_METHODM(uint32,List_InsertSingle,FS_List_InsertSingle)
{
     return IFEELIN F_Do(Obj, F_METHOD_ID(INSERT), &Msg->Entry, 1, Msg->Pos);
}
//+

///List_Remove
F_METHODM(void,List_Remove,FS_List_Remove)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    switch (Msg->Position)
    {
        case FV_List_Remove_First:
        {
            list_lines_delete(Class,Obj,(FLine *)(LOD->LineList.Head),1);
        }
        break;
        
        case FV_List_Remove_Last:
        {
            list_lines_delete(Class,Obj,(FLine *)(LOD->LineList.Head),1);
        }
        break;
    
        case FV_List_Remove_Prev:
        {
            list_lines_delete(Class,Obj,(LOD->Active) ? LOD->Active->Prev : (FLine *)(LOD->LineList.Head),1);
        }
        break;
        
        case FV_List_Remove_Next:
        {
            list_lines_delete(Class,Obj,(LOD->Active) ? LOD->Active->Next : (FLine *)(LOD->LineList.Tail),1);
        }
        break;
    
        case FV_List_Remove_Active:
        {
            if (LOD->Active)
            {
                list_lines_delete(Class,Obj,LOD->Active,1);
            }
        }
        break;

        case FV_List_Remove_Others:
        {
            if (LOD->Active)
            {
                list_lines_delete(Class,Obj,(FLine *)(LOD->LineList.Head),LOD->Active->Position - 1);
                
                if (LOD->Active && LOD->Active->Next)
                {
                    list_lines_delete(Class,Obj,LOD->Active->Next,((FLine *)(LOD->LineList.Tail))->Position - LOD->Active->Position + 1);
                }
            }
        }
        break;
          
        case FV_List_Remove_ToBottom:
        {
            if (LOD->Active)
            {
                list_lines_delete(Class,Obj,LOD->Active->Next,((FLine *)(LOD->LineList.Tail))->Position - LOD->Active->Position);
            }
        }
        break;
         
        case FV_List_Remove_FromTop:
        {
            if (LOD->Active)
            {
                list_lines_delete(Class,Obj,(FLine *)(LOD->LineList.Head),LOD->Active->Position - 1);
            }
        }
        break;
          
        case FV_List_Remove_All:
        {
            list_lines_delete(Class,Obj,(FLine *)(LOD->LineList.Head),ALL);
        }
        break;
    
        default:
        {
            FLine *line;
            
            for (line = (FLine *)(LOD->LineList.Head) ; line ; line = line->Next)
            {
                if (line->Position == Msg->Position)
                {
                    list_lines_delete(Class,Obj,line,1); break;
                }
            }
        }
        break;
    }
}
//+

