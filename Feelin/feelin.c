/*
**    feelin.c
**
**    THE library ;-)
**
**    © 2001-2006 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************

$VER: 10.03 (2005/09/29)

    [HOMBRE]
    
    Amiga  OS4  adaptation,  with  use  of  the  OS4  new   library   design
    (interfaces).

    Interface pointers (e.g. : IExec->) are now used in all the  source  and
    must  be used, even for Classic ! Macros in <feelin/machine.h> just void
    the pointers when compiling on anything else than Amiga OS4. So  if  you
    use  a library, just check if the macro voiding the interface pointer is
	created, or create it.

    See OS4 makefiles for compilation options

************************************************************************************************/

#include "Private.h"

#include <exec/initializers.h>
#include <exec/resident.h>

///library tags

#ifdef __amigaos4__

extern CONST struct TagItem libCreateTags[];

#else
 
struct FeelinLID
{
   uint8 i_Type,o_Type,d_Type,p_Type;
   uint16 i_Name,o_Name; STRPTR d_Name;
   uint8 i_Flags,o_Flags,d_Flags,p_Flags;
   uint8 i_Version,o_Version; uint16 d_Version;
   uint8 i_Revision,o_Revision; uint16 d_Revision;
   uint16 i_IdString,o_IdString; STRPTR d_IdString;
   uint32 endmark;
};

extern const uint32 lib_init_table[4];

#endif
//+
/// name and version strings

#if defined(__MORPHOS__)

    #define OS "MorphOS"

#elif defined(__AROS__)

    #define OS "AROS"

#elif defined(__amigaos4__)

    #define OS "AmigaOS 4"

#else

    #define OS "AmigaOS"

#endif

#define LIBNAME "feelin.library"
#define LIBVERS "$VER: Feelin 10.00 (2005/09/30) © Olivier LAVIALE (gofromiel@gofromiel.com) (" OS " version)"

extern const uint8 name[];
extern const uint8 vers[];

//+

/*

GOFROMIEL @ YOMGUI: Quand est-ce qu'on pourra se débarraser de  __abox__  ??
J'aimerais  avoir une version complètement native, qui ne fonctionne qu'avec
des classes natives, et f*ck l'émulation.

*/

#ifdef __MORPHOS__
uint32 __abox__ = 1;
#endif

///libraries bases and interfaces

#if defined(__MORPHOS__) || defined(__AROS__) || defined (__amigaos4__)

struct ExecBase                     *SysBase = NULL;
#ifdef __amigaos4__
struct ExecIFace                    *IExec = NULL;
#endif

struct DosLibrary                   *DOSBase = NULL;
#ifdef __amigaos4__
struct DOSIFace                     *IDOS = NULL;
#endif

struct GfxBase                      *GfxBase = NULL;
#ifdef __amigaos4__
struct GraphicsIFace                *IGraphics = NULL;
#endif

struct IntuitionBase                *IntuitionBase = NULL;
#ifdef __amigaos4__
struct IntuitionIFace               *IIntuition = NULL;
#endif

#ifdef __amigaos4__
struct UtilityBase                  *UtilityBase = NULL;
struct UtilityIFace                 *IUtility = NULL;
#else
struct Library                      *UtilityBase = NULL;
#endif
 
#ifdef __AROS__
struct LocaleBase                   *LocaleBase = NULL;
#else
struct Library                      *LocaleBase = NULL;
#ifdef __amigaos4__
struct LocaleIFace                  *ILocale = NULL;
#endif
#endif

struct in_FeelinBase                *FeelinBase = NULL;

#ifdef __amigaos4__
struct FeelinIFace                  *IFeelin = NULL;
#endif

#endif

//+
 
/************************************************************************************************
*** Magic ***************************************************************************************
************************************************************************************************/

///_start

/*

    AmigaOS4 :

    The system (and compiler) rely on a symbol named _start which marks  the
    beginning  of execution of an ELF file. To prevent others from executing
    this library, and to keep the compiler/linker happy, we define an  empty
    _start symbol here.

    On the classic system (pre-AmigaOS4) this  was  usually  done  by  moveq
    #0,d0 rts

*/

int32 _start(void)
{
    return -1;
}
//+

///romtags
const struct Resident ROMTag
#ifdef __GNUC__
__attribute__((used))
#endif
=
{
    RTC_MATCHWORD,
    (struct Resident *) &ROMTag,
    
    #ifdef __amigaos4__
    (APTR) (&ROMTag + 1),
    #else
    (APTR) &lib_init_table,
    #endif
    
    FF_LIBRARY,
    FV_FEELIN_VERSION,
    NT_LIBRARY,
    0,
    (STRPTR) name,
    (STRPTR) vers,

    #ifdef __amigaos4__
    (APTR) libCreateTags
    #else
    (APTR) &lib_init_table
    #endif

    #if defined(__MORPHOS__) || defined(__AROS__)
    ,FV_FEELIN_REVISION,
    NULL
    #endif
};

const uint8
#ifdef __GNUC__
__attribute__((used))
#endif
name[] = LIBNAME;

const uint8
#ifdef __GNUC__
__attribute__((used))
#endif
vers[] = LIBVERS;

/*** libraries version *************************************************************************/

#ifdef __amigaos4__
#define FV_VERSION_DOS                          50L
#define FV_VERSION_GRAPHICS                     50L
#define FV_VERSION_LAYERS                       50L
#define FV_VERSION_INTUITION                    50L
#define FV_VERSION_UTILITY                      50L
#define FV_VERSION_LOCALE                       50L
#else
#define FV_VERSION_DOS                          39L
#define FV_VERSION_GRAPHICS                     39L
#define FV_VERSION_LAYERS                       39L
#define FV_VERSION_INTUITION                    39L
#define FV_VERSION_UTILITY                      39L
#define FV_VERSION_LOCALE                       39L
#endif
//+

/************************************************************************************************
*** Library Interface ***************************************************************************
************************************************************************************************/

///lib_null
static uint32 lib_null(void)
{
   return 0;
}
//+
///lib_exit
static uint32 lib_exit
(
   struct in_FeelinBase *FeelinBase
)
{
    struct FeelinClass *Class;
    
    if (!FeelinBase)
    {
        return 0;
    }
 
    DEBUG_FCT(("FeelinBase 0x%08lx <%s> OpenCount %ld\n",
        FeelinBase,
        FeelinBase->Public.Libnode.lib_Node.ln_Name,
        FeelinBase->Public.Libnode.lib_OpenCnt));

/*** remove remaining classes ******************************************************************/

    DEBUG_FCT(("remove remaining classes\n"));
    
    #ifdef F_NEW_SINGLELIST_CLASSES
    Class = F_CLASSES_INDEX;
    #else
    Class = (FClass *) F_CLASSES_TAIL;
    #endif

    while (Class)
    {
        if (Class->UserCount == 0)
        {
            IFEELIN F_DeleteClass(Class);

            #ifdef F_NEW_SINGLELIST_CLASSES
            Class = F_CLASSES_INDEX;
            #else
            Class = (FClass *) F_CLASSES_TAIL;
            #endif
        }
        else
        {
            #ifdef F_NEW_SINGLELIST_CLASSES
            Class = Class->Next;
            #else
            Class = Class->Prev;
            #endif
        }
    }

    #ifdef F_NEW_SINGLELIST_CLASSES
    Class = F_CLASSES_INDEX;
    #else
    Class = (FClass *) F_CLASSES_TAIL;
    #endif

    while (Class)
    {
        if (((struct in_FeelinClass *)(Class))->module)
        {
            DEBUG_FCT(("expunging class\n"));
            
            IFEELIN F_Log
            (
                FV_LOG_USER,
                
                "feelin.expunge() - Class '%s' - UserCount %ld - Super '%s' - Module %s{%08lx}",
                
                Class->Name,
                Class->UserCount,
                (Class->Super) ? Class->Super->Name : (STRPTR)("NONE"),
                ((struct in_FeelinClass *)(Class))->module->lib_Node.ln_Name,
                ((struct in_FeelinClass *)(Class))->module
            );

            IFEELIN F_DeleteClass(Class);

            #ifdef F_NEW_SINGLELIST_CLASSES
            Class = F_CLASSES_INDEX;
            #else
            Class = (FClass *) F_CLASSES_TAIL;
            #endif
        }
        else
        {
            #ifdef F_NEW_SINGLELIST_CLASSES
            Class = Class->Next;
            #else
            Class = (FClass *) Class->Prev;
            #endif
        }
    }

/*** free resources allocated during lib_init() ************************************************/
    
    DEBUG_FCT(("free resources allocated during lib_init()\n"));

    while (FeelinBase->pools)
    {
        IFEELIN F_DeletePool(FeelinBase->pools);
    }

    if (FeelinBase->Public.Console)
    {
        IDOS_ Close(FeelinBase->Public.Console);
        
        FeelinBase->Public.Console = (BPTR)NULL;    /* HOMBRE: Merci de bien vouloir laisser le cast, cause GCC 3.4.4 */
    }

/*** close libraries ***************************************************************************/

    DEBUG_FCT(("close libraries\n"));

    /* dos.library */
 
    #ifdef __amigaos4__
    if (FeelinBase->Public.I_DOS)
    {
        IEXEC DropInterface((struct Interface *)(FeelinBase->Public.I_DOS));
        FeelinBase->Public.I_DOS = NULL;
    }
    #endif
   
    if (FeelinBase->Public.DOS)
    {
        IEXEC CloseLibrary((struct Library *)(FeelinBase->Public.DOS));
        FeelinBase->Public.DOS = NULL;
    }
                    
    /* graphics.library */
    
    #ifdef __amigaos4__
    if (FeelinBase->Public.I_Graphics)
    {
        IEXEC DropInterface((struct Interface *)(FeelinBase->Public.I_Graphics));
        FeelinBase->Public.I_Graphics = NULL;
    }
    #endif

    if (FeelinBase->Public.Graphics)
    {
        IEXEC CloseLibrary((struct Library *)(FeelinBase->Public.Graphics));
        FeelinBase->Public.Graphics = NULL;
    }
            
    /* intuition.library */
    
    #ifdef __amigaos4__
    if (FeelinBase->Public.I_Intuition)
    {
        IEXEC DropInterface((struct Interface *)(FeelinBase->Public.I_Intuition));
        FeelinBase->Public.I_Intuition = NULL;
    }
    #endif
 
    if (FeelinBase->Public.Intuition)
    {
        IEXEC CloseLibrary((struct Library *)(FeelinBase->Public.Intuition));
        FeelinBase->Public.Intuition = NULL;
    }

    /* utility.library */
    
    #ifdef __amigaos4__
    if (FeelinBase->Public.I_Utility)
    {
        IEXEC DropInterface((struct Interface *)(FeelinBase->Public.I_Utility));
        FeelinBase->Public.I_Utility = NULL;
    }
    #endif

    if (FeelinBase->Public.Utility)
    {
        IEXEC CloseLibrary((struct Library *)(FeelinBase->Public.Utility));
        FeelinBase->Public.Utility = NULL;
    }

    /* layers.library */
                        
    #ifdef __amigaos4__
    if (FeelinBase->Public.I_Layers)
    {
        IEXEC DropInterface((struct Interface *)(FeelinBase->Public.I_Layers));
        FeelinBase->Public.I_Layers = NULL;
    }
    #endif
     
    if (FeelinBase->Public.Layers)
    {
        IEXEC CloseLibrary((struct Library *)(FeelinBase->Public.Layers));
        FeelinBase->Public.Layers = NULL;
    }
    
    /* locale.library */
    
    #ifdef __amigaos4__
    if (FeelinBase->Public.I_Locale)
    {
        IEXEC DropInterface((struct Interface *)(FeelinBase->Public.I_Locale));
        FeelinBase->Public.I_Locale = NULL;
    }
    #endif
 
    if (FeelinBase->Public.Locale)
    {
        IEXEC CloseLibrary((struct Library *)(FeelinBase->Public.Locale));
        FeelinBase->Public.Locale = NULL;
    }

    #ifdef __amigaos4__
    if (FeelinBase->Public.I_Feelin)
    {
        IEXEC DropInterface((struct Interface *) FeelinBase->Public.I_Feelin);
        FeelinBase->Public.I_Feelin = NULL;
    }
    #endif
   
    DEBUG_FCT(("bye\n"));
   
    return 0;
}
//+

///lib_init
static F_LIB_INIT
{
    F_LIB_INIT_ARGS
    
/*

    GOFROMIEL: Because the  *classic*  version  of  feelin.library  uses  no
    global   variable,   portability  was  some  time  a  problem.  To  help
    portability (and global variables management), the parameter  giving  us
    the  pointer  to  the  base of our library (FeelinBase) was renamed from
    'F_LIB_BASE' to 'SelfBase'.
 
*/

    #if defined(__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
    
        FeelinBase = SelfBase;

    #else

        #ifdef __SASC
        
            #define FeelinBase SelfBase
        
        #else
        
            F_LIB_STRUCT *F_LIB_BASE = (F_LIB_STRUCT *) SelfBase;
        
        #endif
    
    #endif
    
#ifdef __amigaos4__
    
    DEBUG_INIT(FeelinBase, SegList, (struct ExecBase *)(SYS->Data.LibBase));
 
    /* The ROMTAG Initialisation */
    
    FeelinBase->Public.Libnode.lib_Node.ln_Type = NT_LIBRARY;
    FeelinBase->Public.Libnode.lib_Node.ln_Pri  = 0;
    FeelinBase->Public.Libnode.lib_Node.ln_Name = LIBNAME;
    FeelinBase->Public.Libnode.lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
    FeelinBase->Public.Libnode.lib_Version      = FV_FEELIN_VERSION;
    FeelinBase->Public.Libnode.lib_Revision     = FV_FEELIN_REVISION;
    FeelinBase->Public.Libnode.lib_IdString     = LIBVERS;

    /* Get the ExecBase and ExecIFace pointers for OS4 */
    
    FeelinBase->Public.SYS   = (struct ExecBase *) SYS->Data.LibBase;
    FeelinBase->Public.I_SYS = (struct ExecIFace *) SYS;

#else
    
    DEBUG_INIT(FeelinBase, SegList, SYS);
 
    /* Get the ExecBase pointer for others */
    
    FeelinBase->Public.SYS = SYS;
    
#endif
    
    #if defined(__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
    
    SysBase = FeelinBase->Public.SYS;
    
        #ifdef __amigaos4__
        IExec = FeelinBase->Public.I_SYS;
        #endif
        
    #endif

    FeelinBase->seglist = SegList;
    FeelinBase->debug_log_level = FV_LOG_CORE;

/************************************************************************************************
*** Init Semaphores *****************************************************************************
************************************************************************************************/

    IEXEC InitSemaphore(&FeelinBase->Henes);
    IEXEC InitSemaphore(&F_SHAREDS_ARBITER);
    IEXEC InitSemaphore(F_ATOMS_ARBITER);

/************************************************************************************************
*** Open System Libraries ***********************************************************************
************************************************************************************************/

    #ifdef __amigaos4__
    if (!(FeelinBase->Public.I_Feelin = (struct FeelinIFace *) IEXEC GetInterface((struct Library *) FeelinBase, "main", 1, NULL)))
    {
        goto __error;
    }

    IFeelin = FeelinBase->Public.I_Feelin;
    #endif


    /* dos.library */
    
    if (!(FeelinBase->Public.DOS = (void *) IEXEC OpenLibrary("dos.library", FV_VERSION_DOS)))
    {
        IEXEC Alert(AG_OpenLib | AO_DOSLib); goto __error;
    }
    #ifdef __amigaos4__
    else
    {
        if (!(FeelinBase->Public.I_DOS = (struct DOSIFace *) IEXEC GetInterface((struct Library *) FeelinBase->Public.DOS, "main", 1, NULL)))
        {
            IEXEC Alert(AG_OpenLib | AO_DOSLib); goto __error;
        }
    
        IDOS = FeelinBase->Public.I_DOS;
    }
    #endif
 
    #if defined (__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
    DOSBase = FeelinBase->Public.DOS;
    #endif

 
    /* graphics.library */
    
    if (!(FeelinBase->Public.Graphics = (void *) IEXEC OpenLibrary("graphics.library", FV_VERSION_GRAPHICS)))
    {
        IEXEC Alert(AG_OpenLib | AO_GraphicsLib); goto __error;
    }
    #ifdef __amigaos4__
    else
    {
        if (!(FeelinBase->Public.I_Graphics = (struct GraphicsIFace *) IEXEC GetInterface((struct Library *) FeelinBase->Public.Graphics, "main", 1, NULL)))
        {
            IEXEC Alert(AG_OpenLib | AO_GraphicsLib); goto __error;
        }
    
        IGraphics = FeelinBase->Public.I_Graphics;
    }
    #endif

    #if defined (__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
    GfxBase = FeelinBase->Public.Graphics;
    #endif
        
    
    /* intuition.library */

    if (!(FeelinBase->Public.Intuition = (void *) IEXEC OpenLibrary("intuition.library", FV_VERSION_INTUITION)))
    {
        IEXEC Alert(AG_OpenLib | AO_Intuition); goto __error;
    }
    #ifdef __amigaos4__
    else
    {
        if (!(FeelinBase->Public.I_Intuition = (struct IntuitionIFace *) IEXEC GetInterface((struct Library *) FeelinBase->Public.Intuition, "main", 1, NULL)))
        {
            IEXEC Alert(AG_OpenLib | AO_Intuition); goto __error;
        }
    
        IIntuition = FeelinBase->Public.I_Intuition;
    }
    #endif

    #if defined (__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
    IntuitionBase = FeelinBase->Public.Intuition;
    #endif

    
    /* utility.library */
 
    if (!(FeelinBase->Public.Utility = (void *) IEXEC OpenLibrary("utility.library", FV_VERSION_UTILITY)))
    {
       IEXEC Alert(AG_OpenLib | AO_UtilityLib); goto __error;
    }
    #ifdef __amigaos4__
    else
    {
        if (!(FeelinBase->Public.I_Utility = (struct UtilityIFace *) IEXEC GetInterface((struct Library *) FeelinBase->Public.Utility, "main", 1, NULL)))
        {
            IEXEC Alert(AG_OpenLib | AO_UtilityLib); goto __error;
        }
    
        IUtility = FeelinBase->Public.I_Utility;
    }
    #endif
    
    #if defined (__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
    UtilityBase = FeelinBase->Public.Utility;
    #endif


    /* layers.library *

        This library is not used within feelin.library,  we  don't  need  to
        setup its base nor its interface
        
    */
 
    if (!(FeelinBase->Public.Layers = (void *) IEXEC OpenLibrary("layers.library", FV_VERSION_LAYERS)))
    {
       IEXEC Alert(AG_OpenLib | AO_LayersLib); goto __error;
    }
    #ifdef __amigaos4__
    else
    {
        if (!(FeelinBase->Public.I_Layers = (struct LayersIFace *) IEXEC GetInterface((struct Library *) FeelinBase->Public.Layers, "main", 1, NULL)))
        {
            IEXEC Alert(AG_OpenLib | AO_LayersLib); goto __error;
        }
    
        #if 0
        ILayers = FeelinBase->Public.I_Layers;
        #endif
    }
    #endif
    
    #if 0 // layers.library is not used within feelin.library
    #if defined (__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
    LayersBase = FeelinBase->Public.Layers;
    #endif
    #endif
 
/************************************************************************************************
*** Create Memory Pools *************************************************************************
************************************************************************************************/

    /*** This is the default memory pool, used by F_New() **************************************/

    FeelinBase->DefaultPool = IFEELIN F_CreatePool(8192,

        FA_Pool_Items, 1,
        FA_Pool_Name, "feelin-default",

        TAG_DONE);

    if (!FeelinBase->DefaultPool)
    {
       goto __error;
    }


    /*** This pool is used to allocate atoms ***************************************************/

    F_ATOMS_POOL = IFEELIN F_CreatePool(1024,

        FA_Pool_Items, 1,
        FA_Pool_Name, "atoms",

        TAG_DONE);

    if (F_ATOMS_POOL == NULL)
    {
       goto __error;
    }


    /*** This pool is used to create hash links ************************************************/

/*

HOMBRE : Ce bassin est bien réservé à l'usage exclusif de  feelin.library  ?
si  oui,  on  peut  la  demandé  en  tant  que  mémoire  protégé  pour  OS4:
FA_Pool_Attributes, MEMF_PRIVATE.
    
GOFROMIEL: Ui, mais ça fait quoi ? C'est drôle tu dis 'bassin'  moi  je  dis
'piscine' ;-)

*/

    FeelinBase->HashPool = IFEELIN F_CreatePool(sizeof (FHashLink),
    
        FA_Pool_Attributes, 0,
        FA_Pool_Items, 128,
        FA_Pool_Name, "feelin-hash",

        TAG_DONE);

    if (!FeelinBase->HashPool)
    {
       goto __error;
    }

    /*** This pool is used by FC_Class to create instances *************************************/

    F_CLASSES_POOL = IFEELIN F_CreatePool(sizeof (struct in_FeelinClass) + sizeof (void *),

        FA_Pool_Name, "feelin-classes",

        TAG_DONE);

    if (F_CLASSES_POOL == NULL)
    {
       goto __error;
    }

    /*** This pool is used by the Dynamic system ***********************************************/

    F_DYNAMIC_POOL = IFEELIN F_CreatePool(1024,

        FA_Pool_Items, 1,
        FA_Pool_Name, "feelin-dynamic",

        TAG_DONE);

    if (F_DYNAMIC_POOL == NULL)
    {
       goto __error;
    }

    /*** This pool is used by FC_Object for notifycation handlers ******************************/

    FeelinBase->NotifyPool = IFEELIN F_CreatePool(1024,

        FA_Pool_Items, 1,
        FA_Pool_Name, "object-notify",

        TAG_DONE);

    if (!FeelinBase->NotifyPool)
    {
       goto __error;
    }

/************************************************************************************************
*** Create Hash Tables **************************************************************************
************************************************************************************************/

    if ((F_ATOMS_TABLE = IFEELIN F_HashCreate(FV_HASH_MEDIUM)) == NULL)
    {
        goto __error;
    }

/************************************************************************************************
*** Create Classes ******************************************************************************
************************************************************************************************/
             
/*

HOMBRE @ GOFROMIEL : Tu appelles la fonction en static ici et en dessous ?

GOFROMIEL: Hein ? C'est génant ?

HOMBRE : Pas du tout ; c'est juste que ça me surprend, puisque tu fait des
appels à la bibliothèque partout ailleurs...

GOFROMIEL: A croire que je sois un homme de paradoxes ;-)

*/
 
    if (!(fc_class_create(FeelinBase))) goto __error;
    if (!(fc_object_create(FeelinBase))) goto __error;

    return (struct Library *)(FeelinBase);

__error:

    lib_exit(FeelinBase);

    return NULL;
}
//+
///lib_expunge
static F_LIB_EXPUNGE
{
    F_LIB_EXPUNGE_ARGS
    
    DEBUG_EXPUNGE(FeelinBase);

    if (!(FeelinBase->Public.Libnode.lib_OpenCnt))
    {
        #ifdef __amigaos4__
        struct ExecIFace *ISYS = FeelinBase->Public.I_SYS;
        #endif
        
        APTR seglist = FeelinBase->seglist;

        lib_exit(FeelinBase);

        IEXEC Remove((struct Node *)(FeelinBase));
        
#ifdef __amigaos4__
        
        /* DeleteLibrary seems to be buggy, or the conditions are not filled to call it
        ISYS->DeleteLibrary((struct Library *)FeelinBase);*/

        ISYS->FreeMem((APTR)((uint32)(FeelinBase) - FeelinBase->Public.Libnode.lib_NegSize),
            FeelinBase->Public.Libnode.lib_NegSize +
            FeelinBase->Public.Libnode.lib_PosSize);

#else
        
        FreeMem((APTR)((uint32)(FeelinBase) - FeelinBase->Public.Libnode.lib_NegSize),
            FeelinBase->Public.Libnode.lib_NegSize +
            FeelinBase->Public.Libnode.lib_PosSize);

#endif
        
        DEBUG_FCT(("bye!\n"));

        return seglist;
    }
    else
    {
       FeelinBase->Public.Libnode.lib_Flags |= LIBF_DELEXP;
    }
    return 0;
}
//+
///lib_close
static F_LIB_CLOSE
{
    F_LIB_CLOSE_ARGS

/*

HOMBRE @ GOFROMIEL: Pourquoi utiliser le  type  uint32  pour  la  valeur  de
retour de lib_close ?

GOFROMIEL: Parce que des fois je fais n'importe quoi... mais je me  ratrappe
;-)

*/
    
    #ifdef __amigaos4__
    struct ExecIFace *ISYS = FeelinBase->Public.I_SYS;
    #endif
     
    APTR rc = 0;
    
    IEXEC ObtainSemaphore(&FeelinBase->Henes);
 
    DEBUG_CLOSE(FeelinBase);
 
    #ifdef F_ENABLE_MEMORY_STATS
    {
        BPTR record = IDOS_ Open("T:feelin.log",MODE_READWRITE);

        if (record)
        {
            struct DateTime dt;
            char buf_date[32];
            char buf_time[16];

            IDOS_ DateStamp(&dt.dat_Stamp);

            dt.dat_Format  = FORMAT_INT;
            dt.dat_Flags   = 0;
            dt.dat_StrDay  = NULL;
            dt.dat_StrDate = (STRPTR) buf_date;
            dt.dat_StrTime = (STRPTR) buf_time;

            IDOS_ DateToStr(&dt);

            IDOS_ Seek(record,0,OFFSET_END);

            IDOS_ FPrintf
            (
                record,
                
                "\n"
                "***********************************\n"
                "* F_Dispose() statistics: *********\n"
                "***********************************\n"
                "Immediately found:       %10ld\n"
                "In chain:                %10ld\n"
                "In a previous chain:     %10ld (distance max: %ld)\n"
                "In a following chain:    %10ld\n"
                "Unknown:                 %10ld\n"
                "***********************************\n\n",
                
                FeelinBase->memory_stats_immediate,
                FeelinBase->memory_stats_link,
                FeelinBase->memory_stats_prev,
                FeelinBase->memory_stats_prev_distance_max,
                FeelinBase->memory_stats_next,
                FeelinBase->memory_stats_fucked
            );
        
            IDOS_ Close(record);
        }
    }
    #endif

    if (FeelinBase->Public.Libnode.lib_OpenCnt > 0)
    {
       FeelinBase->Public.Libnode.lib_OpenCnt--;
    }
 
    if (FeelinBase->Public.Libnode.lib_OpenCnt == 0)
    {
       if (LIBF_DELEXP & FeelinBase->Public.Libnode.lib_Flags)
       {
#if defined(__MORPHOS__)

            REG_A6 = (uint32) FeelinBase;

            rc = lib_expunge();

#elif defined(__amigaos4__)

            rc = lib_expunge(SelfIFace);

#else

            rc = lib_expunge(FeelinBase);

#endif
        }
    }

    #ifdef __amigaos4__
    ISYS->ReleaseSemaphore(&FeelinBase->Henes);
    #else
    ReleaseSemaphore(&FeelinBase->Henes);
    #endif

    return rc;
}
//+
///lib_open
#define TEMPLATE                    "LEVEL=L/K,MEMWALLSIZE=WALL/N/K,VERBOSE_NEW=VBNEW/S,VERBOSE_DISPOSE=VBDISPOSE/S,PUBSEMAPHORES=PUBSEM/S"
#define CONSOLE                     "CON:0/30//130/Feelin Debug/INACTIVE/AUTO"

enum  {

      OPT_LEVEL,
      OPT_MEMORYWALL,
      OPT_VERBOSE_NEW,
      OPT_VERBOSE_DISPOSE,
      OPT_PUBSEMAPHORES,

      OPT_COUNT

      };

STATIC F_LIB_OPEN
{
    F_LIB_OPEN_ARGS
   
    STRPTR buf;
    int32 ar[OPT_COUNT];

    DEBUG_OPEN(FeelinBase);

    IEXEC ObtainSemaphore(&FeelinBase->Henes);

    FeelinBase->Public.Libnode.lib_OpenCnt++;
    FeelinBase->Public.Libnode.lib_Flags &= ~LIBF_DELEXP;
    
    if (!FeelinBase->Public.Locale)
    {
        FeelinBase->Public.Locale = (void *) IEXEC OpenLibrary("locale.library", FV_VERSION_LOCALE);
        
        #if defined(__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
        LocaleBase = (void *)FeelinBase->Public.Locale;
        #endif
 
        #ifdef __amigaos4__
        if (FeelinBase->Public.Locale)
        {
            FeelinBase->Public.I_Locale = (struct LocaleIFace *) IEXEC GetInterface((struct Library *) FeelinBase->Public.Locale, "main", 1, NULL);
            ILocale = FeelinBase->Public.I_Locale;
        }
        #endif
    }
    
    ar[OPT_LEVEL]           = (int32)("USER");
    ar[OPT_MEMORYWALL]      = 0;
    ar[OPT_VERBOSE_NEW]     = FALSE;
    ar[OPT_VERBOSE_DISPOSE] = FALSE;
    ar[OPT_PUBSEMAPHORES]   = FALSE;
                                                        
    #ifdef __amigaos4__
    if ((buf = IEXEC AllocMem(1024,MEMF_CLEAR | MEMF_VIRTUAL)) != NULL)
    #else
    if ((buf = IEXEC AllocMem(1024,MEMF_CLEAR)) != NULL)
    #endif
    {
        BPTR record = IDOS_ Open("T:feelin.log",MODE_READWRITE);
        uint32 len;

        if (record)
        {
            struct DateTime dt;
            char buf_date[32];
            char buf_time[16];

            IDOS_ DateStamp(&dt.dat_Stamp);

            dt.dat_Format  = FORMAT_INT;
            dt.dat_Flags   = 0;
            dt.dat_StrDay  = NULL;
            dt.dat_StrDate = (STRPTR) buf_date;
            dt.dat_StrTime = (STRPTR) buf_time;

            IDOS_ DateToStr(&dt);

            IDOS_ Seek(record,0,OFFSET_END);

            IDOS_ FPuts(record, "\n\n\n");
            IDOS_ FPuts(record, buf_date);
            IDOS_ FPutC(record, ' ');
            IDOS_ FPuts(record, buf_date);
            IDOS_ FPuts(record, " NEW SESSION ***\n");

            IDOS_ Close(record);
        }
       
        /* update debug level, debug flags... */

        IDOS_ GetVar("Feelin/LIB_DEBUG",buf,1023,0);
                
        len = IFEELIN F_StrLen(buf);
        
        if (len)
        {
            struct RDArgs *rda = (struct RDArgs *) IDOS_ AllocDosObject(DOS_RDARGS,NULL);
                
            if (rda)
            {
                rda->RDA_Source.CS_Buffer = buf;
                rda->RDA_Source.CS_Length = len;

                if (IDOS_ ReadArgs(TEMPLATE,ar,rda))
                {
                    #if 0
                    if (ar[OPT_MEMORYWALL])
                    {
                        IDOS_ Printf("wall size (%ld)(0x%08lx)\n",*((uint32 *)(ar[OPT_MEMORYWALL])), ar[OPT_MEMORYWALL]);
                    }
                    #endif

                    IDOS_ FreeArgs(rda);
                }
                #if 0
                else
                {
                    IDOS_ Printf("Readargs failed\n");
                }
                #endif

                IDOS_ FreeDosObject(DOS_RDARGS,rda);
            }
        }
        
        /* open console */

        if (!FeelinBase->Public.Console)
        {
            IEXEC CopyMem(CONSOLE,buf, IFEELIN F_StrLen(CONSOLE));
            
            IDOS_ GetVar("Feelin/LIB_CONSOLE",buf,1023,0);

            FeelinBase->Public.Console = IDOS_ Open(buf,MODE_NEWFILE);
        }

        IEXEC FreeMem(buf,1024);
    }

    if (ar[OPT_LEVEL])
    {
        if (IUTILITY Stricmp((STRPTR)(ar[OPT_LEVEL]),"USER") == 0)        FeelinBase->debug_log_level = FV_LOG_USER;
        else if (IUTILITY Stricmp((STRPTR)(ar[OPT_LEVEL]),"DEV") == 0)    FeelinBase->debug_log_level = FV_LOG_DEV;
        else if (IUTILITY Stricmp((STRPTR)(ar[OPT_LEVEL]),"CLASS") == 0)  FeelinBase->debug_log_level = FV_LOG_CLASS;
        else if (IUTILITY Stricmp((STRPTR)(ar[OPT_LEVEL]),"CORE") == 0)   FeelinBase->debug_log_level = FV_LOG_CORE;
    }

    if (ar[OPT_PUBSEMAPHORES])    FeelinBase->debug_flags   |=  FF_DEBUG_PUBLICSEMAPHORES;
    else                          FeelinBase->debug_flags   &= ~FF_DEBUG_PUBLICSEMAPHORES;
    if (ar[OPT_VERBOSE_NEW])      FeelinBase->verbose_flags |=  FF_VERBOSE_NEW;
    else                          FeelinBase->verbose_flags &= ~FF_VERBOSE_NEW;
    if (ar[OPT_VERBOSE_DISPOSE])  FeelinBase->verbose_flags |=  FF_VERBOSE_DISPOSE;
    else                          FeelinBase->verbose_flags &= ~FF_VERBOSE_DISPOSE;

    #ifdef F_ENABLE_MEMORY_WALL
    FeelinBase->memory_wall_size = 32;
    //FeelinBase->debug_memory_wall_size = *((uint32 *)(ar[OPT_MEMORYWALL]));
    #endif
    
    IEXEC ReleaseSemaphore(&FeelinBase->Henes);
    
    return (struct Library *)(FeelinBase);
}
//+

/************************************************************************************************
*** AmigaOS 4 IFace *****************************************************************************
************************************************************************************************/

#ifdef __amigaos4__

#include "lib_OS4vararg_functions.h"

/*** Manager Interface *************************************************************************/

///_manager_Obtain
STATIC int32 _manager_Obtain(struct LibraryManagerInterface *Self)
{
    return ++Self->Data.RefCount;
}
//+
///_manager_Release
STATIC uint32 _manager_Release(struct LibraryManagerInterface *Self)
{
    return --Self->Data.RefCount;
}
//+

///lib_manager_vectors
/* Manager interface vectors */
STATIC CONST APTR lib_manager_vectors[] =
{
    _manager_Obtain,
    _manager_Release,
    NULL,
    NULL,
    lib_open,
    lib_close,
    lib_expunge,
    NULL,
    (APTR)-1
};
//+
///lib_managerTags
/* "__library" interface tag list */
STATIC CONST struct TagItem lib_managerTags[] =
{
    { MIT_Name,        (Tag)"__library"         },
    { MIT_VectorTable, (Tag)lib_manager_vectors },
    { MIT_Version,     1                        },
    { TAG_DONE,        0                        }
};
//+

/*** Library Interface(s) **********************************************************************/

///_Feelin_Obtain
uint32 _Feelin_Obtain(struct FeelinIFace *Self)
{
    /* Increment the reference count and return it */
    Self->Data.RefCount++;
    return Self->Data.RefCount;
}
//+
///_Feelin_Release
uint32 _Feelin_Release(struct FeelinIFace *Self)
{
    /* Decrement reference count */
    Self->Data.RefCount--;
    
    /* Self-destruction on the last Release() call: */
    if (Self->Data.RefCount == 0)
    {
        Self->Expunge();
        return 0;
    }

    /* This is usually only required for non-permanent interfaces.*/
    return Self->Data.RefCount;
}
//+

extern const APTR lib_func_table[];

///mainTags
STATIC CONST struct TagItem mainTags[] =
{
    { MIT_Name,        (Tag)"main"              },
    { MIT_VectorTable, (Tag)lib_func_table      },
    { MIT_Version,     1                        },
    { TAG_DONE,        0                        }
};
//+
///libInterfaces
STATIC CONST CONST_APTR libInterfaces[] =
{
    lib_managerTags,
    mainTags,
    NULL
};
//+

///libCreateTags
const struct TagItem libCreateTags[] =
{
    { CLT_DataSize,    sizeof(struct in_FeelinBase) },
    { CLT_InitFunc,    (Tag)lib_init                },
    { CLT_Interfaces,  (Tag)libInterfaces           },
    { TAG_DONE,         0                           }
};
//+

#endif

/************************************************************************************************
*** Functions Table *****************************************************************************
************************************************************************************************/

///lib_func_table
const APTR lib_func_table[] =
{
#ifdef __MORPHOS__
    (APTR) FUNCARRAY_BEGIN,
    (APTR) FUNCARRAY_32BIT_NATIVE,
#endif /* __MORPHOS__ */
                    
#ifdef __amigaos4__ 
    
    /*** interface management ***/
    
    _Feelin_Obtain,
    _Feelin_Release,
    NULL,
    NULL,

#else
 
 
    /*** Library interface ***/

    lib_open,
    lib_close,
    lib_expunge,
    
#endif
 
    lib_null,

    /*** Memory related functions ***/

    f_pool_create_a,
#ifdef __amigaos4__

   F_CreatePool,
#endif
    f_pool_delete,
    lib_null,
    lib_null,
    lib_null,
    lib_null,
    lib_null,

    f_new,
    f_newp,
    f_dispose,
    #if F_CODE_DEPRECATED
    f_disposep,
    #else
    lib_null,
    #endif

    f_opool,
    f_spool,
    f_rpool,

    lib_null,

    /*** List related functions ***/

    f_link_tail,
    f_link_head,
    f_link_insert,
    f_link_move,
    f_link_remove,
    f_link_member,
    f_next_node,

    /*** String functions ***/

    f_str_fmtA,
#ifdef __amigaos4__
    F_StrFmt,
#endif
    
    f_str_newA,
#ifdef __amigaos4__
    F_StrNew,
#endif

    f_str_len,
    f_str_cmp,

    /*** Bug report functions ***/

    f_logA,
#ifdef __amigaos4__
    F_Log,
#endif
    f_alerta,
#ifdef __amigaos4__
    F_Alert,
#endif

    /*** Hash functions ***/
          
    f_hash_create,
    f_hash_delete,
    f_hash_find,
    f_hash_add_link,
    f_hash_rem_link,
    f_hash_add,
    f_hash_rem,

    /*** Dynamic IDs allocation system ***/

    f_dynamic_create,
    f_dynamic_delete,
    f_dynamic_find_attribute,
    f_dynamic_find_method,
    f_dynamic_find_id,
    f_dynamic_resolve_table,
    f_dynamic_add_auto_table,
    f_dynamic_rem_auto_table,
    f_dynamic_nti,
    f_dynamic_fti,
    f_dynamic_gtd,

    /*** Object-oriented system functions ***/

    f_find_class,
    f_open_class,
    f_close_class,
    f_create_classA,
#ifdef __amigaos4__
    F_CreateClass,
#endif
    f_dispose_obj,

    f_doa,
#ifdef __amigaos4__
    F_Do,
#endif
    f_classdoa,
#ifdef __amigaos4__
    F_ClassDo,
#endif
    f_superdoa,
#ifdef __amigaos4__
    F_SuperDo,
#endif

    f_new_obja,
#ifdef __amigaos4__
    F_NewObj,
#endif
    
    f_make_obja,
#ifdef __amigaos4__
    F_MakeObj,
#endif

    f_dispose_obj,

    f_get,
    f_set,

    /*** FC_Area support ***/

    f_draw,
    f_layout,
    f_erase,

    /*** Shared objects functions ***/

    f_shared_find,
    f_shared_create,
    f_shared_delete,
    f_shared_open,
    f_shared_close,
                
    /*** added at v10 ***/

    f_str_newpA,
#ifdef __amigaos4__
    F_StrNewP,
#endif

    f_atom_find,
    f_atom_obtain,
    f_atom_release,
    f_atom_create_pool,
    f_atom_delete_pool,
    f_atom_findp,
    f_atom_obtainp,
    f_atom_releasep,

    (APTR) -1
#ifdef __MORPHOS__
    ,(APTR)  FUNCARRAY_END
#endif /* __MORPHOS__ */
};
//+
    
#ifndef __amigaos4__

#ifdef __MORPHOS__

#define lib_init_data                           NULL

#else

static const struct FeelinLID lib_init_data[] =
{
    0xA0,  8, NT_LIBRARY, 0,
    0x80, 10, LIBNAME,
    0xA0, 14, LIBF_SUMUSED|LIBF_CHANGED, 0,
    0x90, 20, FV_FEELIN_VERSION,
    0x90, 22, FV_FEELIN_REVISION,
    0x80, 24, LIBVERS,

    NULL
};

#endif

const uint32 lib_init_table[4] =
{
    (uint32) sizeof (struct in_FeelinBase),
    (uint32) lib_func_table,
    (uint32) lib_init_data,
    (uint32) lib_init
};

#endif
