/*
**    lib_Link.c
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************
                        
$VER: 04.02 (2005/10/02)

    Added  better  debug  support,  which  can   be   enable   by   defining
    F_ENABLE_SAFE_LINKING.
 
$VER: 04.01 (2005/09/30)

    Added AmigaOS 4 support.

$VER: 04.00 (2003/10/12)

************************************************************************************************/

#include "Private.h"

/************************************************************************************************
*** API *****************************************************************************************
************************************************************************************************/

///f_link_tail
F_LIB_LINK_TAIL
{
    if (List && Node)
    {
        #ifdef F_ENABLE_SAFE_LINKING
        if (Node->Next || Node->Prev)
        {
            IFEELIN F_Log(0,"F_LinkTail() FNode (0x%08lx) seams to be already linked (0x%08lx,0x%08lx)",Node,Node->Next,Node->Prev);
            
            return NULL;
        }
        #endif
            
        if (List->Tail)
        {
            Node->Prev = List->Tail;
            Node->Prev->Next = Node;
            List->Tail = Node;
        }
        else
        {
            List->Head = Node;
            List->Tail = Node;
        }
        return Node;
    }
    return NULL;
}
//+
///f_link_head
F_LIB_LINK_HEAD
{
    if (List && Node)
    {
        #ifdef F_ENABLE_SAFE_LINKING
        if (Node->Next || Node->Prev)
        {
            IFEELIN F_Log(0,"F_LinkHead() FNode (0x%08lx) seams to be already linked (0x%08lx,0x%08lx)",Node,Node->Next,Node->Prev);
        
            return NULL;
        }
        #endif
        
        if (List->Head)
        {
            Node->Next = List->Head;
            Node->Next->Prev = Node;
            List->Head = Node;
        }
        else
        {
            List->Head = Node;
            List->Tail = Node;
        }
        return Node;
    }
    return NULL;
}
//+
///f_link_insert
F_LIB_LINK_INSERT
{
    if (List && Node)
    {
        #ifdef F_ENABLE_SAFE_LINKING
        if (Node->Next || Node->Prev)
        {
            IFEELIN F_Log(0,"F_LinkInsert() FNode (0x%08lx) seams to be already linked (0x%08lx,0x%08lx)",Node,Node->Next,Node->Prev);
            
            return NULL;
        }
        #endif

        if (Prev)
        {
            if (Prev->Next)
            {
                Node->Next = Prev->Next;
                Node->Prev = Prev;
                Prev->Next->Prev = Node;
                Prev->Next = Node;

                return Node;
            }
            else
            {
               return IFEELIN F_LinkTail(List,Node);
            }
        }
        else
        {
            return IFEELIN F_LinkHead(List,Node);
        }
    }
    return NULL;
}
//+
///f_link_move
F_LIB_LINK_MOVE
{
    if (List && Node)
    {
        #ifdef F_ENABLE_SAFE_LINKING
        FNode *node;
        
        for (node = List->Head ; node ; node = node->Next)
        {
            if (node == Node) break;
        }

        if (!node)
        {
            IFEELIN F_Log(0,"F_LinkMove() FNode (0x%08lx) is not member of FList (0x%08lx)",Node,List);
            
            return NULL;
        }
        #endif

        IFEELIN F_LinkRemove(List,Node);
        return IFEELIN F_LinkInsert(List,Node,Prev);
    }
    return NULL;
}
//+
///f_link_remove
F_LIB_LINK_REMOVE
{
    if (List && Node)
    {
        #ifdef F_ENABLE_SAFE_LINKING
        FNode *node;
        
        for (node = List->Head ; node ; node = node->Next)
        {
            if (node == Node) break;
        }

        if (!node)
        {
            IFEELIN F_Log(0,"F_LinkRemove() FNode (0x%08lx) is not member of FList (0x%08lx)",Node,List);
        
            return NULL;
        } 
        #endif

        if (Node->Next)
        {
            if (Node->Prev)
            {
                Node->Prev->Next = Node->Next;
                Node->Next->Prev = Node->Prev;
            }
            else
            {
                List->Head = Node->Next;
                Node->Next->Prev = NULL;
            }
        }
        else
        {
            if (Node->Prev)
            {
                List->Tail = Node->Prev;
                Node->Prev->Next = NULL;
            }
            else
            {
                List->Head = NULL;
                List->Tail = NULL;
            }
        }

        Node->Next = NULL;
        Node->Prev = NULL;

        return Node;
    }
    return NULL;
}
//+
///f_link_member
F_LIB_LINK_MEMBER
{
    if (List && Node)
    {
        FNode *node;

        for (node = List->Head ; node ; node = node->Next)
        {
            if (node == Node) break;
        }

        return node;
    }
    return NULL;
}
//+
///f_next_node
F_LIB_NEXT_NODE
{
    FNode *next = *Nodeptr;

    if (next)
    {
        *Nodeptr = next->Next;
    }
    return next;
}
//+

