#include "Private.h"

//#define DB_ADD
//#define DB_FIND
//#define DB_DEPRECATE
//#define DB_REMOVE
//#define DB_CLEAR
//#define DB_WRITE
//#define DB_ASSOCIATED_OBTAIN
//#define DB_ASSOCIATED_RELEASE

#ifndef F_NEW_STYLES

///preference_item_remove

/* The object must be locked before calling this function */

uint32 preference_item_remove(FClass *Class,FObject Obj,struct in_Item *Item,uint32 Hash)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    struct in_Item *node,*prev = NULL;

    if (LOD->table)
    {
        for (node = (struct in_Item *) LOD->table->Entries[Hash] ; node ; node = (struct in_Item *) node->link.Next)
        {
            if ((uint32)(node) == (uint32)(Item))
            {
                if (prev)
                {
                    prev->link.Next = node->link.Next;
                }
                else
                {
                    LOD->table->Entries[Hash] = node->link.Next;
                }

/*** deprecating ********************************************************************************

    If some data are still associated to the item, we still remove it but we
    don't dispose it. A struct in_Deprecated is created and a pointer to the
    item is saved here for later disposal (when the associated  count  drops
    to zero).
 
*/
 
                if (node->associated_count)
                {
                    struct in_Deprecated *deprecated = IFEELIN F_NewP(LOD->pool,sizeof (struct in_Deprecated));
                    
                    if (deprecated)
                    {
                        deprecated->handle = node;
                        deprecated->associated_data = Item->associated_data;
                        deprecated->associated_count = Item->associated_count;

                        #ifdef DB_DEPRECATE
                        IFEELIN F_Log(0,"deprecate: handle (0x%08lx) associated data (0x%08lx) users (%ld)",deprecated->handle,deprecated->associated_data,deprecated->associated_count);
                        #endif

                        deprecated->next = LOD->deprecateds;
                        LOD->deprecateds = deprecated;
                    }
                }
                else
                {
                    #ifdef DB_REMOVE
                    IFEELIN F_Log(0,"item (%s)",node->link.Key);
                    #endif
                    
                    IFEELIN F_Dispose(node);
                }

                return TRUE;
            }

            prev = node;
        }
    }
    return FALSE;
}
//+

///preference_associated_obtain
uint32 preference_associated_obtain(FClass *Class, FObject Obj, struct FS_Preference_ObtainAssociated *Msg)
{
    struct LocalObjectData *LOD = F_LOD(Class, Obj);

    struct in_Item *item = NULL;

    APTR rc = NULL;

    if (Msg->Name && Msg->ConstructHook)
    {
        FAtom *atom = IFEELIN F_AtomFind(Msg->Name, ALL);

        if (atom)
        {
            for (item = LOD->items ; item ; item = item->next)
            {
                if (item->atom == atom)
                {
                    break;
                }
            }
        }

        if (item)
        {
            item->associated_count++;

            if (item->associated_count == 1)
            {
                struct FS_Associated_Construct hook_msg;

                hook_msg.Data = item->data;
                hook_msg.Pool = LOD->pool;
                hook_msg.Result = &item->associated_data;
                hook_msg.UserParams = (uint32 *)((uint32)(Msg) + sizeof (struct FS_Preference_ObtainAssociated));

                #ifdef DB_ASSOCIATED_OBTAIN
                IFEELIN F_Log(0,"associated (0x%08lx)(%s) constructor (0x%08lx)",item->associated_data,item->link.Key,Msg->ConstructHook);
                IFEELIN F_Log(0,"params (0x%08lx) : (0x%08lx)(0x%08lx)(0x%08lx)(0x%08lx)", hook_msg.UserParams,hook_msg.UserParams[0],hook_msg.UserParams[1], hook_msg.UserParams[2], hook_msg.UserParams[3], hook_msg.UserParams[4]);
                #endif

                if (IUTILITY CallHookPkt(Msg->ConstructHook, NULL, &hook_msg))
                {
                    rc = item->associated_data;
                }
                else
                {
                    item = NULL;
                }
            }
            else
            {
                rc = item->associated_data;
            }

            #ifdef F_ENABLE_ASSOCIATED_STATS
            
            if (item)
            {
                #ifdef DB_ASSOCIATED_OBTAIN
                IFEELIN F_Log(0,"source (%s) associated (0x%08lx) count (%ld)",Msg->Source,item->associated,item->associated_count);
                #endif

                item->associated_max = MAX(item->associated_max, item->associated_count);
            }
            
            #endif
        }
    }

    if (Msg->Result)
    {
        if (*Msg->Result == NULL)
        {
            *Msg->Result = rc;
        }
        else
        {
            IFEELIN F_Log(0,"Suspicious result pointer (0x%08lx [0x%08lx]). Initialize it to NULL before invoking this method",Msg->Result, *Msg->Result);
        }
    }

    return (uint32) item;
}
//+
///preference_associated_release
uint32 preference_associated_release(FClass *Class,FObject Obj, struct FS_Preference_ReleaseAssociated *Msg)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    uint32 rc = FALSE;

    if (Msg->Handle)
    {

/*** check deprecated ***************************************************************************

    Before we go any further, we check if  the  handler  is  not  deprecated
    (linked  in  the 'deprecateds' chain). If we found it and its user count
    finaly drops to zero, we can finaly dispose it and its associated data.
 
*/

        if (LOD->deprecateds)
        {
            struct in_Deprecated *node;
            struct in_Deprecated *prev = NULL;

            for (node = LOD->deprecateds ; node ; node = node->next)
            {
                if (node->handle == (struct in_Item *) Msg->Handle)
                {
                    #ifdef DB_DEPRECATE
                    IFEELIN F_Log(0,"found a deprecated handle (0x%08lx) users (%ld)",node,node->associated_count);
                    #endif

                    if ((--node->associated_count) == 0)
                    {
                        #ifdef DB_DEPRECATE
                        IFEELIN F_Log(0,"remove deprecated handle (%s)",node->handle->link.Key);
                        #endif

                        if (prev)
                        {
                            prev->next = node->next;
                        }
                        else
                        {
                            LOD->deprecateds = node->next;
                        }

                        if (node->associated_data)
                        {
                            if (Msg->DestructHook)
                            {
                                struct FS_Associated_Destruct hook_msg;

                                hook_msg.Data = node->associated_data;
                                hook_msg.Pool = LOD->pool;
                                hook_msg.UserParams = (uint32 *)((uint32)(Msg) + sizeof (struct FS_Preference_ReleaseAssociated));

                                IUTILITY CallHookPkt(Msg->DestructHook,NULL,&hook_msg);
                            }
                            else
                            {
                                IFEELIN F_Dispose(node->associated_data); node->associated_data = NULL;
                            }
                        }

                        IFEELIN F_AtomRelease(node->handle->atom);

                        IFEELIN F_Dispose(node->handle);
                        IFEELIN F_Dispose(node);
                    }

                    return TRUE;
                }
                prev = node;
            }
        }

/*** remove *************************************************************************************

    New that the handler has been searched within deprecateds  and  was  not
    found, we can try the active list

*/
        {
            struct in_Item *item = NULL;

            #ifdef DB_ASSOCIATED_RELEASE
            IFEELIN F_Log(0,"msg (0x%08lx) handle (0x%08lx) key (0x%08lx) count (%ld)",Msg,Msg->Handle, ((struct in_Item *) Msg->Handle)->link.Key,  ((struct in_Item *) Msg->Handle)->associated_count);
            #endif

            FAtom *atom = IFEELIN F_AtomFind(((struct in_Item *)(Msg->Handle))->atom->Key,((struct in_Item *)(Msg->Handle))->atom->KeyLength);

            if (atom)
            {
                for (item = LOD->items ; item ; item = item->next)
                {
                    if (item->atom == atom)
                    {
                        break;
                    }
                }
            }

            if (item)
            {
                if ((uint32) item == (uint32) Msg->Handle)
                {
                    #ifdef DB_ASSOCIATED_RELEASE
                    IFEELIN F_Log(0,"item (%s) found, count (%ld)",item->link.Key,item->associated_count);
                    #endif

                    if ((--item->associated_count) == 0)
                    {
                        #if defined(DB_ASSOCIATED_RELEASE) || defined(F_ENABLE_ASSOCIATED_STATS)
                            #ifdef F_ENABLE_ASSOCIATED_STATS
                            IFEELIN F_Log(0,"destruct associated (0x%08lx)(%s) max users (%ld)", item->associated_data,item->link.Key, item->associated_max);
                            #else
                            IFEELIN F_Log(0,"destruct associated (0x%08lx)(%s)", item->associated_data,item->link.Key);
                            #endif
                        #endif

                        if (item->associated_data)
                        {
                            if (Msg->DestructHook)
                            {
                                struct FS_Associated_Destruct hook_msg;

                                hook_msg.Data = item->associated_data;
                                hook_msg.Pool = LOD->pool;
                                hook_msg.UserParams = (uint32 *)((uint32)(Msg) + sizeof (struct FS_Preference_ReleaseAssociated));

                                IUTILITY CallHookPkt(Msg->DestructHook,NULL,&hook_msg);
                            }
                            else
                            {
                                IFEELIN F_Dispose(item->associated_data); item->associated_data = NULL;
                            }
                        }
                    }

                    rc = TRUE;
                }
            }

            #if 0
            if (!rc)
            {
                IFEELIN F_Log(FV_LOG_DEV,"Unknown associated handle (0x%08lx)",Msg->Handle);
            }
            #endif
        }
    }

    return rc;
}
//+

///preference_write
uint32 preference_write(FClass *Class, FObject Obj, STRPTR Name)
{
    struct LocalObjectData *LOD = F_LOD(Class, Obj);

    uint32 rc = 0;

    if (Name)
    {
        BPTR fh = IDOS_ Open(Name, MODE_NEWFILE);

        #ifdef DB_WRITE
        IFEELIN F_Log(0,"file (%s)(0x%08lx)",Name,fh);
        #endif

        if (fh)
        {
            uint32 i;
            uint32 count = 0;

            struct in_Item *item;

            for (item = LOD->items ; item ; item = item->next)
            {
                count++;
            }

            #ifdef DB_WRITE
            IFEELIN F_Log(0,"%ld items to save",count);
            #endif

            if (count)
            {
                struct in_Item **sort_array = IFEELIN F_NewP(LOD->pool, sizeof (struct in_Item *) * (count + 1));
                struct in_Item **en = sort_array;

                STRPTR family_name = NULL;
                uint32 family_name_length = 0;

                /* build the array, NULL is used as terminator */

                if (sort_array)
                {
                    for (item = LOD->items ; item ; item = item->next)
                    {
                        *en++ = item;
                    }

                    /* sort the array, 'i' is used to count the  number  of
                    changes. if 'i' is zero everything should be sorted */

                    for (;;)
                    {
                        i = 0;
                        en = sort_array;

                        while (en[1])
                        {
                            int32 cmp = IFEELIN F_AtomCmp(en[0]->atom, en[1]->atom, ALL);

                            if (cmp > 0)
                            {
                                APTR tmp = en[1];

                                en[1] = en[0];
                                en[0] = tmp;

                                i++;
                            }

                            en++;
                        }

                        if (i == 0) break;
                    }

                    /* save now */

                    IDOS_ FPrintf(fh, "/* %s generated by Feelin:CSSDocument */\n\n", IDOS_ FilePart(Name));

                    for (en = sort_array ; *en ; en++)
                    {
                        if ((family_name_length == 0) ||
                            ((family_name_length != 0) && (IFEELIN F_StrCmp(family_name, en[0]->atom->Key, family_name_length) != 0)))
                        {

/*

GOFROMIEL @ HOMBRE: Je préfére utiliser 'uint8' plutôt que 'char' parce  que
je  peux être certain du type de 'uint8'. En même temps, je ne pense pas que
cela ai la moindre conséquence non ?

HOMBRE : Si, cela  a  une  conséquence  :  GCC  ne  veut  pas  compiler  car
en[0]->link.Key  est  un  char,  et  que cela pose un problème à GCC dans le
calcul fait ligne 152, sinon on peut faire un cast  en  (char)  just  à  cet
endroit !?

GOFROMIEL: Pff, mais oui, je suis  bête.  J'ai  changé  en  STRPTR  cela  ne
devrait plus poser de problème.

*/

                            STRPTR name = en[0]->atom->Key;
                            uint8 c;

                            while (((c = *name) != '\0') && c != '-' && c != ' ') name++;

                            if (c == '-')
                            {
                                if (family_name)
                                {
                                    IDOS_ FPrintf(fh,"}\n\n");
                                }

                                family_name = en[0]->atom->Key;
                                family_name_length = name - (STRPTR) en[0]->atom->Key + 1; // include the '-' char, it will be used to control

                                #ifdef DB_WRITE
                                IFEELIN F_Log(0,"new family : (%s)(%ld)",en[0]->link.Key, family_name_length);
                                #endif

                                IDOS_ FWrite(fh,family_name,family_name_length-1,1);
                                IDOS_ FPrintf(fh,"\n{\n");
                            }
                            else
                            {
                                family_name = NULL;
                                family_name_length = 0;
                            }
                        }

                        #ifdef DB_WRITE
                        IFEELIN F_Log(0,"use family : (%s) >> (%s)",family_name, en[0]->link.Key + family_name_length);
                        #endif

                        IDOS_ FPrintf(fh, "\t");
                        IDOS_ FPrintf(fh, en[0]->atom->Key + family_name_length);
                        IDOS_ FPrintf(fh, ": ");
                        IDOS_ FPrintf(fh, en[0]->data);
                        IDOS_ FPrintf(fh, ";\n");

                        rc++;
                    }

                    if (family_name)
                    {
                        IDOS_ FPrintf(fh,"}");
                    }

                    IFEELIN F_Dispose(sort_array);
                }
            }

            IDOS_ Close(fh);
        }
    }
    return rc;
}
//+

#endif

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

#ifndef F_NEW_STYLES

///Prefs_Find
F_METHODM(bool32,Prefs_Find,FS_Preference_Find)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    bool32 rc = FALSE;
    uint32 ln = IFEELIN F_StrLen(Msg->Name);

    if (ln)
    {
        struct in_Item *item = NULL;

        FAtom *atom = IFEELIN F_AtomFind(Msg->Name, ln);

        IFEELIN F_Do(CUD->arbiter, FM_Lock, FF_Lock_Shared);

        if (atom)
        {
            for (item = LOD->items ; item ; item = item->next)
            {
                if (item->atom == atom)
                {
                    break;
                }
            }
        }

        if (item)
        {
            if (Msg->DataPtr)
            {
                *Msg->DataPtr = item->data;
            }

            if (Msg->ValuePtr)
            {
                *Msg->ValuePtr = item->value;
            }

            #ifdef DB_FIND
            IFEELIN F_Log(0,"(%s) >> (%s)(%ld)",Msg->Name, item->link.Data, item->value);
            #endif

            rc = TRUE;
        }
        else
        {
            rc = (bool32) F_OBJDO(LOD->reference);
        }

        IFEELIN F_Do(CUD->arbiter, FM_Unlock);
    }

    return rc;
}
//+
///Prefs_Add
F_METHODM(bool32,Prefs_Add,FS_Preference_Add)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);
    
    uint32 name_length = IFEELIN F_StrLen(Msg->Name);
    uint32 value_length = IFEELIN F_StrLen(Msg->Value);
    
    FAtom *atom = IFEELIN F_AtomObtain(Msg->Name, name_length);

    struct in_Item *item = NULL;
    
    F_LOCK_ARBITER;

    if (atom)
    {
        for (item = LOD->items ; item ; item = item->next)
        {
            if (item->atom == atom)
            {
                break;
            }
        }
    }

    if (item)
    {
        #ifdef DB_ADD
        IFEELIN F_Log(0,"item (%s) already exists, it will be overwritten",item->atom->Key);
        #endif 
 
        preference_item_remove(Class,Obj,item);
        
        item = NULL;
    }
 
    if (name_length && value_length)
    {
        item = IFEELIN F_NewP(LOD->pool, sizeof (struct in_Item) + value_length + 1);

        if (item)
        {
            item->next = LOD->items;
            item->atom = atom;
            item->data = (STRPTR)((uint32)(item) + sizeof (struct in_Item));

            IEXEC CopyMem(Msg->Value, item->data, value_length);

            item->value = IFEELIN F_Do(Obj, F_IDR(FM_Document_Numerify), item->data, &item->value_type);

            LOD->items = item;

            #ifdef DB_ADD

            IFEELIN F_Log
            (
                0, "item (0x%08lx) atom (0x%08lx)(%s) value [%-32.32s](%ld) numeric (0x%08lx)",

                item, item->atom, item->atom->Key,
                item->data, value_length, item->value
            );

            #endif
        }
    }

    F_UNLOCK_ARBITER;

    if (item)
    {
        return TRUE;
    }

    IFEELIN F_AtomRelease(atom);

    return FALSE;
}
//+
///Prefs_AddInt
F_METHODM(uint32,Prefs_AddInt,FS_Preference_AddInt)
{
    uint8 buf[32];
    
    /* FIXME: It's not very elaborated. I should add a 'Type' member in  the
    message  to create approriate strings for percents, pixels, metrics... I
    use FM_Document_Stringify instead of this. */

    if ((int32)(Msg->Value) > -256 && (int32)(Msg->Value) < 256)
    {
        IFEELIN F_StrFmt(buf, "%ld", Msg->Value);
    }
    else
    {
        IFEELIN F_StrFmt(buf, "#%lx", Msg->Value);
    }

    return IFEELIN F_Do(Obj, F_IDM(FM_Preference_Add), Msg->Name, &buf);
}
//+
///Prefs_Remove
F_METHODM(uint32,Prefs_Remove,FS_Preference_Remove)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    uint32 rc = FALSE;
    uint32 ln = IFEELIN F_StrLen(Msg->Name);

    if (ln)
    {
        FAtom *atom = IFEELIN F_AtomFind(Msg->Name, ln);

        struct in_Item *item = NULL;

        IFEELIN F_Do(CUD->arbiter, FM_Lock, FF_Lock_Exclusive);

        if (atom)
        {
            for (item = LOD->items ; item ; item = item->next)
            {
                if (item->atom == atom)
                {
                    break;
                }
            }
        }

        if (item)
        {
            rc = preference_item_remove(Class,Obj,item);
        }

        IFEELIN F_Do(CUD->arbiter, FM_Unlock);
    }

    return rc;
}
//+

#endif

///Prefs_Clear
F_METHOD(uint32,Prefs_Clear)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    F_LOCK_ARBITER;

    #ifdef F_NEW_STYLES

    if (LOD->styles != NULL)
    {
        style_flush(Class, Obj);

        #ifdef DB_CLEAR

        if (LOD->styles != NULL)
        {
            IFEELIN F_Log(0,"some styles remain !");
        }

        #endif
    }

    if (LOD->declaration_list.Head != NULL)
    {
        declaration_flush(Class, Obj);

        #ifdef DB_CLEAR

        if (LOD->declaration_list.Head != NULL)
        {
            IFEELIN F_Log(0,"some declarations remain !");
        }

        #endif
    }

    IFEELIN F_SuperDo(Class, Obj, F_RESOLVED_ID(CLEAR));

    #else

    while (LOD->items)
    {
        preference_item_remove(Class,Obj,LOD->items);
    }

    #endif

    F_UNLOCK_ARBITER;

    return 0;
}
//+
///Prefs_Read
F_METHODM(bool32,Prefs_Read,FS_Preference_Read)
{
    bool32 rc = FALSE;
    
    STRPTR name = preference_resolve_name(Class,Obj,Msg->Name);

    F_LOCK_ARBITER;

    IFEELIN F_Do(Obj, F_METHOD_ID(CLEAR));

/** read ****************************************************************************************

    Reading new preference items is done by clearing the document's contents
    and merging the new one.
 
*/
    
    if (name)
    {
        rc = IFEELIN F_Do(Obj, F_METHOD_ID(MERGE), name, FV_Document_SourceType_File);

        IFEELIN F_Dispose(name);

    }

    F_UNLOCK_ARBITER;

    return rc;
}
//+
///Prefs_Write
F_METHODM(bool32,Prefs_Write,FS_Preference_Write)
{
    bool32 rc = FALSE;
 
    STRPTR name = Msg->Name;

    if ((uint32)(name) == FV_Preference_BOTH)
    {
       IFEELIN F_Do(Obj,Method,FV_Preference_ENVARC);

       name = (STRPTR)(FV_Preference_ENV);
    }

    name = preference_resolve_name(Class,Obj,name);

    if (name)
    {
        IFEELIN F_Do(CUD->arbiter, FM_Lock, FF_Lock_Shared);

        #ifdef F_NEW_STYLES
		rc = IFEELIN F_SuperDo(Class, Obj, F_RESOLVED_ID(WRITE), name);
        #else
        rc = preference_write(Class,Obj,name);
        #endif
 
        IFEELIN F_Do(CUD->arbiter, FM_Unlock);
        
        IFEELIN F_Dispose(name);
    }
    
    return rc;
}
//+

#ifndef F_NEW_STYLES

///Prefs_ObtainAssociated
F_METHODM(uint32,Prefs_ObtainAssociated,FS_Preference_ObtainAssociated)
{
    struct LocalObjectData *LOD = F_LOD(Class, Obj);

    uint32 rc;

    IFEELIN F_Do(CUD->arbiter, FM_Lock, FF_Lock_Exclusive);

    rc = preference_associated_obtain(Class,Obj,Msg);
 
    if (!rc)
    {
        rc = F_OBJDO(LOD->reference);
    }

    IFEELIN F_Do(CUD->arbiter, FM_Unlock);

    return rc;
}
//+
///Prefs_ReleaseAssociated
F_METHODM(uint32,Prefs_ReleaseAssociated,FS_Preference_ReleaseAssociated)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    uint32 rc = FALSE;

    IFEELIN F_Do(CUD->arbiter, FM_Lock, FF_Lock_Exclusive);

    if (Msg->Handle)
    {
        rc = preference_associated_release(Class,Obj,Msg);

        if (!rc)
        {
            rc = F_OBJDO(LOD->reference);
        }
    }

    IFEELIN F_Do(CUD->arbiter, FM_Unlock);

    return rc;
}
//+

#endif

///Prefs_Merge
F_METHODM(uint32,Prefs_Merge,FS_Document_Merge)
{
    uint32 rc;
    
    F_LOCK_ARBITER;
        
/*** merge contents ****************************************************************************/
 
    rc = F_SUPERDO();

    if (rc != 0)
    {
        APTR pool = NULL;
        FCSSDeclaration *declaration = NULL;
    
/*** parse CSSDeclarations and create preference items *****************************************/
 
        IFEELIN F_Do
        (
            Obj, FM_Get,

            F_RESOLVED_ID(POOL), &pool,
            F_RESOLVED_ID(DECLARATIONS), &declaration,

            TAG_DONE
        );

        if ((pool != NULL) && (declaration != NULL))
        {
            for ( ; declaration ; declaration = declaration->Next)
            {
                declaration_create(Class, Obj, declaration, NULL);

                #ifdef F_NEW_STYLES_EXTENDED

                if (declaration->Pseudos != NULL)
                {
                    FCSSDeclaration *pseudo;

                    for (pseudo = declaration->Pseudos ; pseudo ; pseudo = pseudo->Next)
                    {
                        declaration_create(Class, Obj, pseudo, declaration);
                    }
                }

                #endif
            }

            rc = TRUE;
        }
    }

    F_UNLOCK_ARBITER;
    
    return rc;
}
//+
