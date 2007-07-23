/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <exec/memory.h>

#include <libraries/feelin.h>

#include <proto/exec.h>
#include <proto/layers.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/feelin.h>

#include <feelin/rtg.h>

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

struct ClassUserData
{
	struct SignalSemaphore			arbiter;
	struct FeelinTraceLayer        *TraceLayers;
};

#define F_LOCK_ARBITER 						IEXEC ObtainSemaphore(&CUD->arbiter)
#define F_UNLOCK_ARBITER 					IEXEC ReleaseSemaphore(&CUD->arbiter)

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	FRender                         Public;
	int32                           ForbidNest;
};

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

/*** clipping ***/

struct FeelinTraceLayer
{
	struct FeelinTraceLayer        *Next;
	struct Layer                   *Layer;
	struct FeelinList               TraceClipList;
};

struct FeelinTraceClip
{
	struct FeelinTraceClip         *Next;
	struct FeelinTraceClip         *Prev;
	struct Region                  *Region;
	struct Region                  *PreviousRegion;
};

