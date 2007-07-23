/*
**    Private.h
**
**    feelin.library global header
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************

$VER: 10.00 (2005/09/30)

	[OLAV]

	The hash table size used by the memory system was  wrong.  I  was  using
	((FV_MEMORY_HASH_SIZE    +    1)    *    sizeof   (APTR))   instead   of
	(FV_MEMORY_HASH_SIZE + 1), resulting in a  structure  size  larger  than
	uint16.

	Added mungwall support. define F_ENABLE_MEMORY_WALL to use it.

	Added F_Dispose() statistics. define F_ENABLE_MEMORY_STATS to use it.

	Removed all SAVEDS. There shall be NO global  variables.  FeelinBase  is
	available  in  the a6 register (or equivalent) of each library functions
	and *must* be passed to private functions.

	Added 'Flags' member to FClass type.

	Added AmigaOS 4 support.

	[GROG]

	Beautify operation, add machine dependent macros, modifiy all  existants
	macros to be compliant with new design. Morphos version macros.

	Undef some macros for MOS.

************************************************************************************************/

#define F_LIB_BASE                              FeelinBase
#define F_LIB_STRUCT                            struct in_FeelinBase
#define F_LIB_IFACE                             FeelinIFace
#define F_LIB_IFACE_PTR                         IFeelin

#define NDEBUG

/************************************************************************************************
*** Configuration *******************************************************************************
************************************************************************************************/

#define F_ENABLE_GLOBALS

	/* use global variables for libraries' base and  interface,  instead  of
	those available in FeelinBase. */

//#define F_ENABLE_MEMORY_WALL

	/* If F_ENABLE_MEMORY_WALL is defined, a wall is added before and  after
	real  allocated  memory.  If the wall is trashed, which may happens with
	undelicate programs, the memory system complains  about  this  when  you
	dispose memory blocks. */

#define FV_MEMORY_WALL_PATTERN                  0xABBAABBA

	/* FV_MEMORY_WALL_PATTERN is the pattern used to fill the  memory  wall.
	The  pattern  is  used  when  memory  blocks are disposed to check their
	integrity. */

#define F_ENABLE_MEMORY_FILL

	/* Disposed memory is filled with FV_MEMORY_FILL_PATTERN, which is  very
	useful for developer to seek usage of deprecated resources. */

#define FV_MEMORY_FILL_PATTERN                  0xDEADBEEF

	/* FV_MEMORY_FILL_PATTERN is the pattern used to fill deprecated  memory
	blocs. */

//#define F_ENABLE_MEMORY_NEW_FILL

	/* If F_ENABLE_MEMORY_NEW_FILL is defined, blocks allocated  from  pools
	without     the     MEMF_CLEAR     flag     set    are    filled    with
	FV_MEMORY_NEW_FILL_PATTERN. This can be usefull to  trace  memory  areas
	which are incorrectly initialized. */

#define FV_MEMORY_NEW_FILL_PATTERN              0xABADFEED

#define F_ENABLE_MEMORY_SHORTCUT

	/* If F_ENABLE_MEMORY_SHORTCUT is defined, an  additional  memory  chunk
	pointer  called  'middle',  which points to the chunk is the middle of a
	puddle, is maintained. This pointer is then used to  speed  up  research
	skipping half of it in most cases. */

//#define F_ENABLE_MEMORY_REARANGE

	/* !! WORK-IN-PROGRESS !!

	if F_ENABLE_MEMORY_REARANGE  is  defined,  puddles  are  rearanged  when
	memory  is allocated and freed to keep the emptiest puddle at the top of
	the pool's chain */

//#define F_ENABLE_MEMORY_STATS

	/* If F_ENABLE_MEMORY_STATS is enabled F_Dispose() memory statistics are
	collected and logged from time to time to the record handler */

//#define F_ENABLE_MEMORY_MARK

	/* if F_ENABLE_MEMORY_MARK is enabled the class name and the method name
	available  in  FeelinBase  are  written  in the memory chunk header .e.g
	[..........Area/Setup.........] (32 bytes long including the \0 char) */



/* Developers may want to define the following symbols to enable  addtionnal
safety  code.  Keep in mind that safety code WILL reduce a lot performances,
especially F_ENABLE_SAFE_STRING.

The F_ENABLE_SAFE_INVOKATION is enabled by  default  because  the  user  can
enable  or  disable  it with the LIB_DEBUG system variable. There is no good
reason to disable it, unless you want Feelin to run at  full  speed  to  see
what she really got ;-) */

#define F_ENABLE_SAFE_INVOKATION

	/* if F_ENABLE_SAFE_INVOKATION is enabled F_DoA() checks if  the  object
	provided  is  really an object before invoking any method. Enabling this
	code reduces performances */

//#define F_ENABLE_SAFE_LINKING

	/* if F_ENABLE_SAFE_LINKING is enabled security mecanisms are  added  to
	FList management functions. */

//#define F_ENABLE_SAFE_STRING

	/*  if  F_ENABLE_SAFE_STRING  is  enabled,  functions  with  strings  as
	parameters  check  if  the  string's  address is valid with TypeOfMem().
	Enabling this code reduces performances a lot ! */

#define F_ENABLE_ATOM_GARBAGE

	/* if F_ENABLE_ATOM_GARBAGE is defined, atoms which reference count drop
	to  zero  are  not  delete  right  away.  Instead,  a 'garbage count' is
	incremented,   and   when   it   reaches   the    limit    defined    by
	FV_ATOM_GARBAGE_LIMIT,  all  atom  which  reference  count  are zero are
	deleted in a single step. */

#define FV_ATOM_GARBAGE_LIMIT                   50

//#define F_ENABLE_SAVE_LOG

	/* if F_ENABLE_SAFE_LOG is enable,  messages  logged  with  the  F_Log()
	function are also saved to "T:feelin.log" */

/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <stdlib.h>

#include <feelin/string.h>

/* FIXME: C'est un peu pourrit ça */
//#if defined(__MORPHOS__) || defined(__AROS__) || defined (__amigaos4__)
#ifdef F_ENABLE_GLOBALS
#include <proto/exec.h>
#else
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#endif

#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/locale.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

/* We make sure __NOLIBBASE__ is defined, and store it's current state */

#ifndef __NOLIBBASE__
#define __WASNT_DEFINED__
#define __NOLIBBASE__
#endif

#include <proto/feelin.h>

/* We restore the previous state of __NOLIBBASE__ */

#ifdef __WASNT_DEFINED__
#undef  __NOLIBBASE__
#undef  __WASNT_DEFINED__
#endif

#include <proto/feelinclass.h>

#ifdef __amigaos4__
#include "feelin_function_alias.h"
#endif

#include <exec/alerts.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/locale.h>
#include <libraries/feelin.h>

#ifndef __AROS__
#ifndef NDEBUG
#include <clib/debug_protos.h>
#endif
#endif

#include <feelin/debug.h>

/************************************************************************************************
*** Libraries base ******************************************************************************
************************************************************************************************/

//#if defined(__MORPHOS__) || defined(__AROS__) || defined(__amigaos4__)
#ifdef F_ENABLE_GLOBALS

extern struct in_FeelinBase                *FeelinBase;

#ifdef __amigaos4__
extern struct FeelinIFace                  *IFeelin;
#endif

extern struct ExecBase                     *SysBase;
#ifdef __amigaos4__
extern struct ExecIFace                    *IExec;
#endif

extern struct DosLibrary                   *DOSBase;
#ifdef __amigaos4__
extern struct DOSIFace                     *IDOS;
#endif

extern struct GfxBase                      *GfxBase;
#ifdef __amigaos4__
extern struct GraphicsIFace                *IGraphics;
#endif

extern struct IntuitionBase                *IntuitionBase;
#ifdef __amigaos4__
extern struct IntuitionIFace               *IIntuition;
#endif

extern struct UtilityBase                  *UtilityBase;
#ifdef __amigaos4__
extern struct UtilityIFace                 *IUtility;
#endif

extern struct LocaleBase                   *LocaleBase;
#ifdef __amigaos4__
extern struct LocaleIFace                  *ILocale;
#endif

#else

/* The classic version of feelin.library uses no global  variables.  Because
FeelinBase  is  available  in a6 whenever a function is called, I don't keep
any track of the library  base  nor  any  other.  Instead  of  using  global
variables  to store these base I use macros to use the ones available within
the FeelinBase structure. DON'T USE SAVEDS IN ANY PLACE !*/

#define DOSBase                     F_LIB_BASE->Public.DOS
#define UtilityBase                 F_LIB_BASE->Public.Utility
#define LocaleBase                  F_LIB_BASE->Public.Locale
#define GfxBase                     F_LIB_BASE->Public.Graphics
#define IntuitionBase               F_LIB_BASE->Public.Intuition

#endif

/************************************************************************************************
*** Machine dependent macros ********************************************************************
************************************************************************************************/

#undef  F_METHOD   
#undef  F_METHODM     

#if defined(__MORPHOS__)

#define F_METHOD(rt,name)                       _DEFFUNC5(name, rt, \
													a2, FClass *, Class, \
													a0, FObject, Obj, \
													d0, uint32, Method, \
													a1, APTR, Msg, \
													a6, F_LIB_STRUCT *, F_LIB_BASE)

#define F_METHODM(rt,name,sn)                   _DEFFUNC5(name, rt, \
													a2, FClass *, Class, \
													a0, FObject, Obj, \
													d0, uint32, Method, \
													a1, struct sn *, Msg, \
													a6, F_LIB_STRUCT *, F_LIB_BASE)

#elif defined(__amigaos4__)

#define F_METHOD(rc,name)                       rc name(FClass *Class, FObject Obj, uint32 Method, APTR Msg)
#define F_METHODM(rc,name,sn)                   rc name(FClass *Class, FObject Obj, uint32 Method, struct sn *Msg)

#else

#define F_METHOD(rc,name)                       ASM(rc) name(REG(a2,FClass *Class),REG(a0,FObject Obj),REG(d0,uint32 Method),REG(a1,APTR Msg),REG(a6,F_LIB_STRUCT *F_LIB_BASE))
#define F_METHODM(rc,name,sn)                   ASM(rc) name(REG(a2,FClass *Class),REG(a0,FObject Obj),REG(d0,uint32 Method),REG(a1,struct sn *Msg),REG(a6,F_LIB_STRUCT *F_LIB_BASE))

#endif
  
/************************************************************************************************
*** Flags ***************************************************************************************
************************************************************************************************/

//#define DEBUG_CODE

/*  DEBUG_CODE

	Enable debug codes e.g. safe invokation, public semaphores, or  various
	verbose.

*/

/* the following flags are adjustables by the user  through  the  LIB_DEBUG
system variable. Flags are updated each time the library is opened */

#define FF_DEBUG_PUBLICSEMAPHORES               (1 << 1)
#define FF_VERBOSE_NEW                          (1 << 0)
#define FF_VERBOSE_DISPOSE                      (1 << 1)

/************************************************************************************************
*** Typedefs and structures *********************************************************************
************************************************************************************************/

///Memory
#define FV_MEMORY_HASH_SIZE                     0xFFF
#define FV_MEMORY_HASH_SHFT                     12
#define F_MEMORY_HASH(mem)                      (((FV_MEMORY_HASH_SIZE << FV_MEMORY_HASH_SHFT) & (uint32)(mem)) >> FV_MEMORY_HASH_SHFT)

/* Shouldn't memory hashing macro be modified for little endian  machines  ?
*/

typedef struct FeelinMemChunk
{
	struct FeelinMemChunk          *next;
	uint32                          size; /* Complete size (Struct + Alloc) */
	#ifdef F_ENABLE_MEMORY_MARK
	uint8                           mark[32];
	#endif
	/* Memory area follows */
}
FMemChunk;

typedef struct FeelinPuddle
{
	struct FeelinPuddle            *next;
	struct FeelinPuddle            *prev; /* needed since memory hashing */

	struct FeelinPuddle            *hash_next;

	struct FeelinMemChunk          *chunks;
	#ifdef F_ENABLE_MEMORY_SHORTCUT
	struct FeelinMemChunk          *middle;
	#endif

	APTR                            lower;
	APTR                            upper;
	uint32                          size;   /* Real size of allocation */
	uint32                          free;   

	struct FeelinPool              *pool;   /* needed since memory hashing */

	/* memory area follows */
}
FPuddle;

typedef struct FeelinPool
{
	struct FeelinPool              *next;

	struct FeelinPuddle            *puddles;

	struct SignalSemaphore          Semaphore;

	bits32                          flags;
	uint32                          puddle_size;
	uint32                          thresh_size;
	#ifdef F_ENABLE_MEMORY_WALL
	uint32                          wall_size;
	#endif
}
FPool;

//+
///FeelinClass
struct in_FeelinClass
{
	FClass                          public;

	struct Library                 *module;
	#ifdef __amigaos4__
	struct FeelinClassIFace        *module_iface;
	#endif
	struct FeelinPool              *pool;

	FMethod                         dispatcher;

	uint32                          id;
	bits32                          flags;

	struct Catalog                 *catalog;
};

#define FF_CLASS_INHERITED_POOL                 (1 << 0)

#define FV_Class_DynamicSpace                   64

/* FV_Class_DynamicSpace is the maximum number of methods or attributes  per
class */

//+
///FeelinBase
struct in_FeelinBase
{
	struct FeelinBase               Public;

	//
	// private
	//

	APTR                            seglist;
	struct SignalSemaphore          Henes;

	/* the 'Henes' semaphore is used  during  library  opening  to  allocate
	resources that could not be allocated during the initialization time. */

	struct FeelinPool              *pools;

	struct FeelinPool              *DefaultPool;
	struct FeelinPool              *HashPool;
	
	struct FeelinPool              *NotifyPool;
	struct SignalSemaphore			objects_arbiter;

	/* the 'objects_arbiter' is used by objects during the  Lock  method  in
	order to create their arbiter safely. */

	struct FeelinPuddle            *hash_puddles[FV_MEMORY_HASH_SIZE + 1];

	FList                           dynamic_list;
	struct FeelinPool              *dynamic_pool;

	struct SignalSemaphore          shareds_arbiter;

	struct FeelinShared            *shareds;

	#ifdef F_NEW_SINGLELIST_CLASSES
	FClass                         *classes;
	#else
	FHashTable                     *classes_table;
	FList                           classes_list;
	#endif
	struct FeelinPool              *classes_pool;

	APTR                            atoms_pool;
	FHashTable                     *atoms_table;
	struct SignalSemaphore          atoms_arbiter;
	#ifdef F_ENABLE_ATOM_GARBAGE
	uint32                          atoms_garbage_count;
	#endif

	// debug stuff

	bits32                          debug_flags;
	bits32                          verbose_flags;

	FClass                         *debug_classdo_class;
	uint32                          debug_classdo_method;
	FObject                         debug_classdo_object;

	uint8                           debug_log_level;

	#ifdef F_ENABLE_MEMORY_WALL
	uint8                           memory_wall_size;
	#endif

	#ifdef F_ENABLE_MEMORY_STATS
	uint32                          memory_stats_numpuddles;
	uint32                          memory_stats_immediate;
	uint32                          memory_stats_link;
	uint32                          memory_stats_prev;
	uint32                          memory_stats_prev_distance_max;
	uint32                          memory_stats_next;
	uint32                          memory_stats_fucked;
//    uint32                          memory_stats_step;
	#endif
};

#define F_ATOMS_TABLE                           FeelinBase->atoms_table
#define F_ATOMS_POOL                            FeelinBase->atoms_pool
#define F_ATOMS_ARBITER                         &FeelinBase->atoms_arbiter
#define F_ATOMS_LOCK                            IEXEC ObtainSemaphore(F_ATOMS_ARBITER)
#define F_ATOMS_UNLOCK                          IEXEC ReleaseSemaphore(F_ATOMS_ARBITER)

#define F_OBJECTS_ARBITER						(&FeelinBase->objects_arbiter)
#define F_OBJECTS_LOCK()						IEXEC ObtainSemaphore(F_OBJECTS_ARBITER)
#define F_OBJECTS_UNLOCK()						IEXEC ReleaseSemaphore(F_OBJECTS_ARBITER)

#define F_SHAREDS_ARBITER                       FeelinBase->shareds_arbiter
#define F_SHAREDS_LOCK                          IEXEC ObtainSemaphore(&F_SHAREDS_ARBITER)
#define F_SHAREDS_UNLOCK                        IEXEC ReleaseSemaphore(&F_SHAREDS_ARBITER)

#define F_SHAREDS_INDEX                         FeelinBase->shareds
#define F_SHAREDS_EACH(entry)                   entry = F_SHAREDS_INDEX ; entry ; entry = entry->next

#define F_DYNAMIC_LIST                          FeelinBase->dynamic_list
#define F_DYNAMIC_POOL                          FeelinBase->dynamic_pool
#define F_DYNAMIC_LOCK                          IFEELIN F_OPool(F_DYNAMIC_POOL)
#define F_DYNAMIC_UNLOCK                        IFEELIN F_RPool(F_DYNAMIC_POOL)

#ifdef F_NEW_SINGLELIST_CLASSES
#define F_CLASSES_INDEX                         FeelinBase->classes
#define F_CLASSES_EACH(node)                    node = F_CLASSES_INDEX ; node ; node = node->Next
#else
#define F_CLASSES_LIST                          FeelinBase->classes_list
#define F_CLASSES_HEAD                          F_CLASSES_LIST.Head
#define F_CLASSES_TAIL                          F_CLASSES_LIST.Tail
#define F_CLASSES_EACH(node)                    node = (FClass *) F_CLASSES_HEAD ; node ; node = node->Next
#endif
#define F_CLASSES_POOL                          FeelinBase->classes_pool
#define F_CLASSES_LOCK                          IFEELIN F_OPool(F_CLASSES_POOL)
#define F_CLASSES_UNLOCK                        IFEELIN F_RPool(F_CLASSES_POOL)
//+

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

STRPTR            f_find_attribute_name         (uint32 id,struct in_FeelinBase *FeelinBase);
STRPTR            f_find_method_name            (uint32 id,struct in_FeelinBase *FeelinBase);
FClass *          f_dynamic_find_name           (STRPTR String,struct in_FeelinBase *FeelinBase);
void              f_dynamic_auto_add_class      (FClass *Class,struct in_FeelinBase *FeelinBase);

FClass *          fc_class_create               (struct in_FeelinBase *FeelinBase);
FClass *          fc_object_create              (struct in_FeelinBase *FeelinBase);

/************************************************************************************************
*** Public Functions ****************************************************************************
************************************************************************************************/

///Memory Management

/* F_CreatePool() */

#define F_LIB_POOL_CREATE                       LIB_DEFFUNC2(f_pool_create_a, FPool *, \
													d0, uint32, ItemSize, \
													a0, struct TagItem *, Tagl)
LIB_PROTO2(f_pool_create_a, FPool *,
	d0, uint32 ItemSize,
	a0, struct TagItem *Tagl);

/* F_DeletePool() */

#define F_LIB_POOL_DELETE                       LIB_DEFFUNC1(f_pool_delete, uint32, \
													a0, FPool *,Pool)
LIB_PROTO1(f_pool_delete, uint32,
	a0, FPool * Pool);

/* F_New() */

#define F_LIB_NEW                               LIB_DEFFUNC1(f_new, APTR, \
													d0, uint32, Size)
LIB_PROTO1(f_new, APTR,
	d0, uint32 Size);

/* F_NewP() */

#define F_LIB_NEWP                              LIB_DEFFUNC2(f_newp, APTR, \
													a0, FPool *,Pool, \
													d0, uint32, Size)
LIB_PROTO2(f_newp, APTR,
	a0, FPool * Pool,
	d0, uint32 Size);

/* F_Dispose() */

#define F_LIB_DISPOSE                           LIB_DEFFUNC1(f_dispose, uint32, \
													a1, APTR, Mem)
LIB_PROTO1(f_dispose, uint32,
	a1, APTR Mem);

/* F_OPool() */

#define F_LIB_OPOOL                             LIB_DEFFUNC1NR(f_opool, \
													a0, FPool *, Pool)
LIB_PROTO1NR(f_opool,
	a0, FPool * Pool);

/* F_SPool() */

#define F_LIB_SPOOL                             LIB_DEFFUNC1NR(f_spool, \
													a0, FPool *, Pool)
LIB_PROTO1NR(f_spool,
	a0, FPool * Pool);

/* F_RPool() */

#define F_LIB_RPOOL                             LIB_DEFFUNC1NR(f_rpool, \
													a0, FPool *, Pool)
LIB_PROTO1NR(f_rpool,
	a0, FPool * Pool);

//+
///Linking
#define F_LIB_LINK_TAIL                         LIB_DEFFUNC2(f_link_tail, FNode *, \
													a0, FList *, List, \
													a1, FNode *, Node)
LIB_PROTO2(f_link_tail, FNode *,
	a0, FList * List,
	a1, FNode * Node);

#define F_LIB_LINK_HEAD                         LIB_DEFFUNC2(f_link_head, FNode *, \
													a0, FList *, List, \
													a1, FNode *, Node)
LIB_PROTO2(f_link_head, FNode *,
	a0, FList * List,
	a1, FNode * Node);

#define F_LIB_LINK_INSERT                       LIB_DEFFUNC3(f_link_insert, FNode *, \
													a0, FList *, List, \
													a1, FNode *, Node, \
													a2, FNode *, Prev)
LIB_PROTO3(f_link_insert, FNode *,
	a0, FList * List,
	a1, FNode * Node,
	a2, FNode * Prev);

#define F_LIB_LINK_MOVE                         LIB_DEFFUNC3(f_link_move, FNode *, \
													a0, FList *, List, \
													a1, FNode *, Node, \
													a2, FNode *, Prev)
LIB_PROTO3(f_link_move, FNode *,
	a0, FList * List,
	a1, FNode * Node,
	a2, FNode * Prev);

#define F_LIB_LINK_REMOVE                       LIB_DEFFUNC2(f_link_remove, FNode *, \
													a0, FList *, List, \
													a1, FNode *, Node)
LIB_PROTO2(f_link_remove, FNode *,
	a0, FList * List,
	a1, FNode * Node);

#define F_LIB_LINK_MEMBER                       LIB_DEFFUNC2(f_link_member, FNode *, \
													a0, FList *, List, \
													a1, FNode *, Node)
LIB_PROTO2(f_link_member, FNode *,
	a0, FList * List,
	a1, FNode * Node);

#define F_LIB_NEXT_NODE                         LIB_DEFFUNC1(f_next_node, FNode *, \
													a0, FNode **, Nodeptr)
LIB_PROTO1(f_next_node, FNode *,
	a0, FNode ** Nodeptr);

//+
///Strings manipulation

/* F_StrFmt() */

#define F_LIB_STR_FMT                           LIB_DEFFUNC3(f_str_fmtA, APTR, \
													a0, STRPTR, Buffer, \
													a1, STRPTR, Fmt, \
													a2, uint32 *,Params)
LIB_PROTO3(f_str_fmtA, APTR,
	a0, STRPTR Buffer,
	a1, STRPTR Fmt,
	a2, uint32 *Params);

/* F_NewStrA() */

#define F_LIB_STR_NEW                           LIB_DEFFUNC3(f_str_newA, STRPTR, \
													a0, uint32 *, LenPtr, \
													a1, STRPTR, Fmt, \
													a2, uint32 *,Params)
LIB_PROTO3(f_str_newA, STRPTR,
	a0, uint32 *LenPtr,
	a1, STRPTR Fmt,
	a2, uint32 *Params);

/* F_NewStrPA() */

#define F_LIB_STR_NEWP                          LIB_DEFFUNC4(f_str_newpA, STRPTR, \
													a0, APTR, Pool, \
													a1, uint32 *, LenPtr, \
													a2, STRPTR, Fmt, \
													a3, uint32 *,Params)
LIB_PROTO4(f_str_newpA, STRPTR,
	a0, APTR Pool,
	a1, uint32 *LenPtr,
	a2, STRPTR Fmt,
	a3, uint32 *Params);

/* F_StrLen() */

#define F_LIB_STR_LEN                           LIB_DEFFUNC1(f_str_len, uint32, \
													a0, STRPTR, Str)
LIB_PROTO1(f_str_len, uint32,
	a0, STRPTR Str);

/* F_StrCmp() */

#define F_LIB_STR_CMP                           LIB_DEFFUNC3(f_str_cmp, int32, \
													a0, STRPTR, Str1, \
													a1, STRPTR, Str2, \
													d1, uint32, Length)
LIB_PROTO3(f_str_cmp, int32,
	a0, STRPTR Str1,
	a1, STRPTR Str2,
	d1, uint32 Length);
//+
///Alert
#define F_LIB_LOG                               LIB_DEFFUNC3NR(f_logA, \
													d0, uint32, Level, \
													a0, STRPTR, Fmt, \
													a1, APTR, Msg)
LIB_PROTO3NR(f_logA,
	d0, uint32 Level,
	a0, STRPTR Fmt,
	a1, APTR Msg);

#define F_LIB_ALERT                             LIB_DEFFUNC3(f_alerta, int32, \
													a0, STRPTR, Title, \
													a1, STRPTR, Body, \
													a2, APTR, Params)
LIB_PROTO3(f_alerta, int32,
	a0, STRPTR Title,
	a1, STRPTR Body,
	a2, APTR Params);

//+
///Hashing
#define F_LIB_HASH_CREATE                       LIB_DEFFUNC1(f_hash_create, FHashTable *, \
													d0, uint32, Size)
LIB_PROTO1(f_hash_create, FHashTable *,
	d0, uint32 Size);

#define F_LIB_HASH_DELETE                       LIB_DEFFUNC1NR(f_hash_delete, \
													a0, FHashTable *, Table)
LIB_PROTO1NR(f_hash_delete,
	a0, FHashTable * Table);

#define F_LIB_HASH_FIND                         LIB_DEFFUNC4(f_hash_find, FHashLink *, \
													a0, FHashTable *, Table, \
													a1, uint8 *, Key, \
													d1, uint32, KeyLength, \
													a2, uint32 *, HashPtr)
LIB_PROTO4(f_hash_find, FHashLink *,
	a0, FHashTable * Table,
	a1, uint8 *Key,
	d1, uint32 KeyLength,
	a2, uint32 *HashPtr);

#define F_LIB_HASH_ADD_LINK                     LIB_DEFFUNC2(f_hash_add_link, uint32, \
													a0, FHashTable *, Table, \
													a1, FHashLink *, Link)
LIB_PROTO2(f_hash_add_link, uint32,
	a0, FHashTable * Table,
	a1, FHashLink * Link);

#define F_LIB_HASH_REM_LINK                     LIB_DEFFUNC2(f_hash_rem_link, int32, \
													a0, FHashTable *, Table, \
													a1, FHashLink *, Link)
LIB_PROTO2(f_hash_rem_link, int32,
	a0, FHashTable * Table,
	a1, FHashLink * Link);

#define F_LIB_HASH_ADD                          LIB_DEFFUNC4(f_hash_add, FHashLink *, \
													a0, FHashTable *, Table, \
													a1, uint8 *, Key, \
													d0, uint32, KeyLength, \
													a2, APTR, Data)
LIB_PROTO4(f_hash_add, FHashLink *,
	a0, FHashTable * Table,
	a1, uint8 *Key,
	d0, uint32 KeyLength,
	a2, APTR Data);

#define F_LIB_HASH_REM                          LIB_DEFFUNC3(f_hash_rem, int32, \
													a0, FHashTable *, Table, \
													a1, uint8 *, Key, \
													d0, uint32, KeyLength)
LIB_PROTO3(f_hash_rem, int32,
	a0, FHashTable * Table,
	a1, uint8 *Key,
	d0, uint32 KeyLength);
//+
///Dynamic System
#define F_LIB_DYNAMIC_CREATE                    LIB_DEFFUNC1(f_dynamic_create, int32, \
													a0, FClass *, Class)
LIB_PROTO1(f_dynamic_create, int32,
	a0, FClass *Class);

#define F_LIB_DYNAMIC_DELETE                    LIB_DEFFUNC1NR(f_dynamic_delete, \
													a0, FClass *, Class)
LIB_PROTO1NR(f_dynamic_delete,
	a0, FClass *Class);

#define F_LIB_DYNAMIC_FIND_ATTRIBUTE            LIB_DEFFUNC3(f_dynamic_find_attribute, FClassAttribute *, \
													a0, STRPTR, Name, \
													a1, FClass *, FromClass, \
													a2, FClass **, RealClass)
LIB_PROTO3(f_dynamic_find_attribute, FClassAttribute *,
	a0, STRPTR Name,
	a1, FClass * FromClass,
	a2, FClass ** RealClass);

#define F_LIB_DYNAMIC_FIND_METHOD               LIB_DEFFUNC3(f_dynamic_find_method, FClassMethod *, \
													a0, STRPTR, Name, \
													a1, FClass *, FromClass, \
													a2, FClass **, RealClass)
LIB_PROTO3(f_dynamic_find_method, FClassMethod *,
	a0, STRPTR Name,
	a1, FClass * FromClass,
	a2, FClass ** RealClass);

#define F_LIB_DYNAMIC_FIND_ID                   LIB_DEFFUNC1(f_dynamic_find_id, uint32, \
													a0, STRPTR, Name)
LIB_PROTO1(f_dynamic_find_id, uint32,
	a0, STRPTR Name);

#define F_LIB_DYNAMIC_RESOLVE_TABLE             LIB_DEFFUNC1(f_dynamic_resolve_table, uint32, \
													a0, FDynamicEntry *, Entries)
LIB_PROTO1(f_dynamic_resolve_table, uint32,
	a0, FDynamicEntry * Entries);

#define F_LIB_DYNAMIC_ADD_AUTO_TABLE            LIB_DEFFUNC1(f_dynamic_add_auto_table, int32, \
													a0, FDynamicEntry *, Table)
LIB_PROTO1(f_dynamic_add_auto_table, int32,
	a0, FDynamicEntry * Table);

#define F_LIB_DYNAMIC_REM_AUTO_TABLE            LIB_DEFFUNC1NR(f_dynamic_rem_auto_table, \
													a0, FDynamicEntry *, Table)
LIB_PROTO1NR(f_dynamic_rem_auto_table,
	a0, FDynamicEntry * Table);

#define F_LIB_DYNAMIC_NTI                       LIB_DEFFUNC3(f_dynamic_nti, struct TagItem *, \
													a0, struct TagItem **, TLP, \
													a1, struct TagItem *, item, \
													a2, struct in_FeelinClass *, Class)
LIB_PROTO3(f_dynamic_nti, struct TagItem *,
	a0, struct TagItem **TLP,
	a1, struct TagItem *item,
	a2, struct in_FeelinClass *Class);

#define F_LIB_DYNAMIC_FTI                       LIB_DEFFUNC2(f_dynamic_fti, struct TagItem *, \
													d0, uint32, Attribute, \
													a0, struct TagItem *, Tags)
LIB_PROTO2(f_dynamic_fti, struct TagItem *,
	d0, uint32 Attribute,
	a0, struct TagItem *Tags);

#define F_LIB_DYNAMIC_GTD                       LIB_DEFFUNC3(f_dynamic_gtd, uint32, \
													d0, uint32, Attribute, \
													d1, uint32, Default, \
													a0, struct TagItem *, Tags)
LIB_PROTO3(f_dynamic_gtd, uint32,
	d0, uint32 Attribute,
	d1, uint32 Default,
	a0, struct TagItem *Tags);
//+
///OOS
#define F_LIB_FIND_CLASS                        LIB_DEFFUNC1(f_find_class, FClass *, \
													a0, STRPTR, Name)
LIB_PROTO1(f_find_class, FClass *,
	a0, STRPTR Name);

#define F_LIB_OPEN_CLASS                        LIB_DEFFUNC1(f_open_class, FClass *, \
													a0, STRPTR, Name)
LIB_PROTO1(f_open_class, FClass *,
	a0, STRPTR Name);

#define F_LIB_CLOSE_CLASS                       LIB_DEFFUNC1(f_close_class, uint32, \
													a0, struct in_FeelinClass *, Class)
LIB_PROTO1(f_close_class, uint32,
	a0, struct in_FeelinClass *Class);

#define F_LIB_CREATE_CLASS                      LIB_DEFFUNC2(f_create_classA, FClass *, \
													a0, STRPTR, Name, \
													a1, struct TagItem *, Tags)
LIB_PROTO2(f_create_classA, FClass *,
	a0, STRPTR Name,
	a1, struct TagItem *Tags);

#define F_LIB_DO                                LIB_DEFFUNC3(f_doa, uint32, \
													a0, FObject, Obj, \
													d0, uint32, Method, \
													a1, APTR, Msg)
LIB_PROTO3(f_doa, uint32,
	a0, FObject Obj,
	d0, uint32 Method,
	a1, APTR Msg);

#define F_LIB_CLASS_DO                          LIB_DEFFUNC4(f_classdoa, uint32, \
													a2, FClass *, Class, \
													a0, FObject, Obj, \
													d0, uint32, Method, \
													a1, APTR, Msg)
LIB_PROTO4(f_classdoa, uint32,
	a2, FClass * Class,
	a0, FObject Obj,
	d0, uint32 Method,
	a1, APTR Msg);

#define F_LIB_SUPER_DO                          LIB_DEFFUNC4(f_superdoa, uint32, \
													a2, FClass *, Class, \
													a0, FObject, Obj, \
													d0, uint32, Method, \
													a1, APTR, Msg)
LIB_PROTO4(f_superdoa, uint32,
	a2, FClass * Class,
	a0, FObject Obj,
	d0, uint32 Method,
	a1, APTR Msg);

#define F_LIB_NEW_OBJ                           LIB_DEFFUNC2(f_new_obja, FObject, \
													a0, STRPTR, Name, \
													a1, struct TagItem *, Tags)
LIB_PROTO2(f_new_obja, FObject,
	a0, STRPTR Name,
	a1, struct TagItem *Tags);

#define F_LIB_MAKE_OBJ                          LIB_DEFFUNC2(f_make_obja, FObject, \
													d0, uint32, Type, \
													a0, uint32 *, Params)
LIB_PROTO2(f_make_obja, FObject,
	d0, uint32 Type,
	a0, uint32 *Params);

#define F_LIB_DISPOSE_OBJ                       LIB_DEFFUNC1(f_dispose_obj, uint32, \
													a0, FObject, Obj)
LIB_PROTO1(f_dispose_obj, uint32,
	a0, FObject Obj);

#define F_LIB_GET                               LIB_DEFFUNC2(f_get, uint32, \
													a0, FObject, Obj, \
													d1, uint32, Tag)
LIB_PROTO2(f_get, uint32,
	a0, FObject Obj,
	d1, uint32 Tag);

#define F_LIB_SET                               LIB_DEFFUNC3NR(f_set, \
													a0, FObject, Obj, \
													d1, uint32, Tag, \
													d2, uint32, Data)
LIB_PROTO3NR(f_set,
	a0, FObject Obj,
	d1, uint32 Tag,
	d2, uint32 Data);

#define F_LIB_DRAW                              LIB_DEFFUNC2NR(f_draw, \
													a0, FObject, Obj, \
													d0, uint32, Flags)
LIB_PROTO2NR(f_draw,
	a0, FObject Obj,
	d0, uint32 Flags);


#define F_LIB_LAYOUT                            LIB_DEFFUNC6NR(f_layout, \
													a0, FObject, Obj, \
													d0, int16, x, \
													d1, int16, y, \
													d2, uint16, w, \
													d3, uint16, h, \
													d4, uint32, Flags)
LIB_PROTO6NR(f_layout,
	a0, FObject Obj,
	d0, int16 x,
	d1, int16 y,
	d2, uint16 w,
	d3, uint16 h,
	d4, bits32 Flags);

#define F_LIB_ERASE                             LIB_DEFFUNC6NR(f_erase, \
													a0, FObject, Obj, \
													d0, int16, x1, \
													d1, int16, y1, \
													d2, int16, x2, \
													d3, int16, y2, \
													d4, bits32, Flags)
LIB_PROTO6NR(f_erase,
	a0, FObject Obj,
	d0, int16 x1,
	d1, int16 y1,
	d2, int16 x2,
	d3, int16 y2,
	d4, bits32 Flags);
//+
///Shared objects management
#define F_LIB_SHARED_FIND                       LIB_DEFFUNC1(f_shared_find, FObject, \
													a0, STRPTR, Name)
LIB_PROTO1(f_shared_find, FObject,
	a0, STRPTR Name);

#define F_LIB_SHARED_CREATE                     LIB_DEFFUNC2(f_shared_create, FObject, \
													a0, FObject, Object, \
													a1, STRPTR, Name)
LIB_PROTO2(f_shared_create, FObject,
	a0, FObject Object,
	a1, STRPTR Name);

#define F_LIB_SHARED_DELETE                     LIB_DEFFUNC1NR(f_shared_delete, \
													a0, FObject, Shared)
LIB_PROTO1NR(f_shared_delete,
	a0, FObject Shared);

#define F_LIB_SHARED_OPEN                       LIB_DEFFUNC1(f_shared_open, FObject, \
													a0, STRPTR, Name)
LIB_PROTO1(f_shared_open, FObject,
	a0, STRPTR Name);

#define F_LIB_SHARED_CLOSE                      LIB_DEFFUNC1NR(f_shared_close, \
													a0, FObject, Shared)
LIB_PROTO1NR(f_shared_close,
	a0, FObject Shared);
//+

///Atoms

#include "lib_Atoms.h"

/* F_AtomFind() */

#define F_LIB_ATOM_FIND                         LIB_DEFFUNC2(f_atom_find, FAtom *, \
													a0, STRPTR, Str, \
													d0, uint32, Length)

LIB_PROTO2(f_atom_find, FAtom *,
	a0, STRPTR Str,
	d0, uint32 Length);

/* F_AtomObtain() */

#define F_LIB_ATOM_OBTAIN                       LIB_DEFFUNC2(f_atom_obtain, FAtom *, \
													a0, STRPTR, Str, \
													d0, uint32, Length)
LIB_PROTO2(f_atom_obtain, FAtom *,
	a0, STRPTR Str,
	d0, uint32 Length);

/* F_AtomRelease() */

#define F_LIB_ATOM_RELEASE                      LIB_DEFFUNC1NR(f_atom_release, \
													a0, FAtom *, Atom)

LIB_PROTO1NR(f_atom_release,
	a0, FAtom *Atom);

/* F_AtomCreatePool() */

#define F_LIB_ATOM_CREATE_POOL                  LIB_DEFFUNC0(f_atom_create_pool, APTR)

LIB_PROTO0(f_atom_create_pool, APTR);

/* F_AtomDeletePool() */

#define F_LIB_ATOM_DELETE_POOL                  LIB_DEFFUNC1NR(f_atom_delete_pool, \
													a0, struct in_LocalAtomPool *, Pool)

LIB_PROTO1NR(f_atom_delete_pool,
	a0, struct in_LocalAtomPool *Pool);

/* F_AtomFindP() */

#define F_LIB_ATOM_FINDP                        LIB_DEFFUNC3(f_atom_findp, FAtom *, \
													a0, struct in_LocalAtomPool *, Pool, \
													a1, STRPTR, Key, \
													d0, uint32, KeyLength)

LIB_PROTO3(f_atom_findp, FAtom *, \
	a0, struct in_LocalAtomPool *Pool, \
	a1, STRPTR Key, \
	d0, uint32 KeyLength);

/* F_AtomObtainP() */

#define F_LIB_ATOM_OBTAINP                      LIB_DEFFUNC3(f_atom_obtainp, FAtom *, \
													a0, struct in_LocalAtomPool *, Pool, \
													a1, STRPTR, Key, \
													d0, uint32, KeyLength)

LIB_PROTO3(f_atom_obtainp, FAtom *, \
	a0, struct in_LocalAtomPool *Pool, \
	a1, STRPTR Key, \
	d0, uint32 KeyLength);

/* F_AtomReleaseP() */

#define F_LIB_ATOM_RELEASEP                     LIB_DEFFUNC2NR(f_atom_releasep, \
													a0, struct in_LocalAtomPool *, Pool, \
													a1, FAtom *, Atom)

LIB_PROTO2NR(f_atom_releasep, \
	a0, struct in_LocalAtomPool * Pool, \
	a1, FAtom * Atom);

//+

#ifdef __amigaos4__

/*** Vararg functions proto ********************************************************************/

APTR                 F_CreatePool(struct FeelinIFace *Self, uint32 ItemSize, ...);
STRPTR               F_StrNew(struct FeelinIFace *Self, uint32 * Len, STRPTR Fmt, ...);
STRPTR               F_StrFmt(struct FeelinIFace *Self, APTR Buf, STRPTR Str, ...);
void                 F_Log(struct FeelinIFace *Self, uint32 Level, STRPTR Fmt, ...);
void                 F_Alert(struct FeelinIFace *Self, STRPTR Title, STRPTR Body, ...);
FClass *             F_CreateClass(struct FeelinIFace *Self, STRPTR Name, ...);
uint32               F_Do(struct FeelinIFace *Self, FObject Obj, uint32 Method, ...);
uint32               F_ClassDo(struct FeelinIFace *Self, FClass * feelin_class, FObject Obj, uint32 Method, ...);
uint32               F_SuperDo(struct FeelinIFace *Self, FClass * feelin_class, FObject Obj, uint32 Method, ...);
FObject              F_NewObj(struct FeelinIFace *Self, STRPTR Name, ...);
FObject              F_MakeObj(struct FeelinIFace *Self, uint32 Type, ...);

#endif

