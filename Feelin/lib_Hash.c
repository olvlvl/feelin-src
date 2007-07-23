/*
**    Hashing functions
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************

$VER: 01.05 (2005/09/30)

    Added AmigaOS 4 support.
 
$VER: 01.04 (2005/08/04)
 
    OLVA: Forgot to increment key in F_HashFind(), resulting in a  hash  key
    computed on the first byte. Bug reported by Michal Schulz.
 
$VER: 01.02 (2005/01/13)

    Converted the whole hashing system to C, even the almighty  F_HashFind()
    function.  I  also update the hash function, not perfect, but better and
    still fast.

    If KeyLength supplied to F_HashAdd() or  F_HashRem()  is  equal  to  ALL
    (-1),  F_StrLen() is used to compute the length of the Key. This is only
    useful if Key is a string.

    F_HashRem() and F_HashRemLink() return TRUE when they succed, FALSE
    otherwise.

*/

#include "Private.h"

/************************************************************************************************
*** Functions ***********************************************************************************
************************************************************************************************/

///f_hash_create
F_LIB_HASH_CREATE
{
    if (Size)
    {
        FHashTable *table;

        if ((table = IFEELIN F_New(sizeof (FHashTable) + sizeof (APTR) * Size)))
        {
            table->Size = Size;
            table->Entries = (APTR)((uint32)(table) + sizeof (FHashTable));

            return table;
        }
    }
    return NULL;
}
//+
///f_hash_delete
F_LIB_HASH_DELETE
{
    IFEELIN F_Dispose(Table);
}
//+
///f_hash_find
F_LIB_HASH_FIND
{
    if (Table && Key && KeyLength)
    {
        uint32 len = KeyLength;
        uint8 *k = Key;
        FHashLink *link;
        uint32 h;
        
        #ifdef F_ENABLE_SAFE_STRING
        
        if (((int32)(Key) < 0xFFFF) || (IEXEC TypeOfMem(Key) == 0))
        {
            IFEELIN F_Log(0,"(db)F_HashFind() invalid string address (0x%08lx)",Key);
            
            return NULL;
        }
        
        #endif 

        for (h = len ; len ; len--)
        {
            h = ((h << 5) ^ (h >> 27)) ^ *k++;
        }

        h = h % Table->Size;

        if (HashPtr)
        {
            *HashPtr = h;
        }

        for (link = Table->Entries[h] ; link ; link = link->Next)
        {
            if (link->KeyLength == KeyLength)
            {
                uint8 *c = link->Key;
                k = Key;

                for (len = KeyLength ; len ; len--)
                {
                    if (*k++ != *c++) break;
                }

                if (!len) return link;
            }
        }
    }
    return NULL;
}
//+
///f_hash_add_link
F_LIB_HASH_ADD_LINK
{
    if (Table && Link && Link->Key && Link->KeyLength)
    {
        volatile uint32 hash=0;

        IFEELIN F_HashFind(Table,Link->Key,Link->KeyLength,&hash);

        Link->Next = Table->Entries[hash];
        Table->Entries[hash] = Link;

        return hash;
    }
    return 0;
}
//+
///f_hash_rem_link
F_LIB_HASH_REM_LINK
{
    volatile uint32 hash;

    if (IFEELIN F_HashFind(Table,Link->Key,Link->KeyLength,&hash))
    {
        FHashLink *node,*prev = NULL;

        for (node = Table->Entries[hash] ; node ; node = node->Next)
        {
            if (node == Link)
            {
                if (prev)
                {
                    prev->Next = node->Next;
                }
                else
                {
                    Table->Entries[hash] = node->Next;
                }

                node->Next = NULL;

                return TRUE;
            }

            prev = node;
        }
    }

    IFEELIN F_Log(FV_LOG_DEV,"F_HashRemLink() Unknown link 0x%08lx - Hash 0x%08lx - Key (%s)",Link,hash,Link->Key);

    return FALSE;
}
//+
///f_hash_add
F_LIB_HASH_ADD
{
    if (Table && Key && KeyLength)
    {
        FHashLink *link = IFEELIN F_NewP(FeelinBase->HashPool,sizeof (FHashLink));

        if (link)
        {
            link->Next      = NULL;
            link->Key       = Key;
            link->KeyLength = (KeyLength == ALL) ? IFEELIN F_StrLen(Key) : KeyLength;
            link->Data      = Data;

            IFEELIN F_HashAddLink(Table,link);

            return link;
        }
    }
    return NULL;
}
//+
///f_hash_rem
F_LIB_HASH_REM
{
    if (Table && Key && KeyLength)
    {
        volatile uint32 hash;
        FHashLink *link = IFEELIN F_HashFind(Table,Key,(KeyLength == ALL) ? IFEELIN F_StrLen(Key) : KeyLength,&hash);

        if (link)
        {
            IFEELIN F_HashRemLink(Table,link);
            IFEELIN F_Dispose(link);

            return TRUE;
        }
    }
    return FALSE;
}
//+

