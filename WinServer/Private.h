/************************************************************************************************
*** Configuration *******************************************************************************
************************************************************************************************/

#define F_ENABLE_EVENT_TICK
#define F_ENABLE_EVENT_BUTTON
#define F_ENABLE_EVENT_MOTION
#define F_ENABLE_EVENT_WBDROP
#define F_ENABLE_EVENT_KEY
#define F_ENABLE_EVENT_WINDOW
//#define F_ENABLE_EVENT_WINDOW_ACTIVE

/* if F_ENABLE_EVENT_WINDOW_ACTIVE is defined, WinServer's  custom  code  is
used   instead   Intuition   one  (IDCMP)  to  handle  window  activation  /
desactication. It's best for now to leave this undefined. */

#define F_ENABLE_EVENT_HELP

#define F_ENABLE_INPUT
#define F_ENABLE_INTUITION
#define F_ENABLE_ACTIVABLE

/* use intuition active and inactive events instead of custom code */

//#define F_ENABLE_ZONES

/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <exec/interrupts.h>
#include <devices/input.h>
#include <intuition/intuitionbase.h>
#include <libraries/feelin.h>

#ifdef F_ENABLE_EVENT_KEY
#include <libraries/commodities.h>
#endif

#ifdef F_ENABLE_EVENT_WBDROP
#include <workbench/startup.h>
#include <workbench/workbench.h>
#endif

#include <proto/exec.h>
#include <proto/dos.h> /* only used for debug */
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/layers.h>

#ifdef F_ENABLE_EVENT_KEY

	#include <proto/keymap.h>

	#ifdef __amigaos4__
	#define IKEYMAP                             IKeymap ->
	#else
	#define IKEYMAP
	#endif

	#include <proto/commodities.h>

	#ifdef __amigaos4__
	#define ICOMMODITIES                        ICommodities ->
	#else
	#define ICOMMODITIES
	#endif

#endif

#ifdef F_ENABLE_EVENT_WBDROP

	#include <proto/wb.h>

	#ifdef __amigaos4__
	#define IWORKBENCH                          IWorkbench ->
	#else
	#define IWORKBENCH
	#endif

#endif

#ifdef F_ENABLE_EVENT_HELP

	#include <proto/timer.h>

	#ifdef __amigaos4__
	#define ITIMER                              ITimer ->
	#else
	#define ITIMER
	#endif

#endif

#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_METHOD_MODIFYEVENTS

		};

enum    {

		FV_PROPERTY_REFRESH_SIMPLE

		};

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

#if 0
#ifdef F_ENABLE_ZONES

struct FeelinZone
{
	struct FeelinZone              *next;
	FObject                         target;
	FBox                           *box;
	bits32                          events;
};

#endif
#endif

struct in_FamilyNode
{
	FFamilyNode                     Public;
	FObject                         application;
	struct Window                  *window;
	bits32                          events;
	uint32                          events_notify;

	#ifdef F_ENABLE_EVENT_WBDROP
	struct AppWindow               *wbdrop_handler;
	#endif

	#ifdef F_ENABLE_ACTIVABLE
	bits32                          flags;
	#endif

	#if 0
	#ifdef F_ENABLE_ZONES
	struct FeelinZone              *zones;
	#endif
	#endif
};

#ifdef F_ENABLE_ACTIVABLE
#define FF_NODE_ACTIVABLE           (1 << 0)
#endif

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	FObject                         Server;

	FObject                         Thread;
	
	struct Hook						thread_hook;

	FThreadPublic                  *ThreadPublic;

	bits32                          flags;
	bits32                          events;

	FList                           family;

	APTR                            pool;

	#ifdef F_ENABLE_INTUITION

	struct MsgPort                 *intuition_port;

	#endif

	#ifdef F_ENABLE_INPUT

	// input core

	struct Interrupt                input_handler;
	struct MsgPort                 *input_port;
	struct IOStdReq                *input_request;

	struct Screen                  *screen;
	uint16                          mouse_x;
	uint16                          mouse_y;

	#ifdef F_ENABLE_EVENT_HELP

	struct timeval                  time;

	#endif

	// active

	struct Window                  *window_active;
	struct in_FamilyNode           *window_active_node;

	// focus

	struct Window                  *window_focus;
	struct in_FamilyNode           *window_focus_node;

	#ifdef F_ENABLE_EVENT_BUTTON
	// double-click

	struct Window                  *dc_window;
	uint8                           dc_button;
	uint16                          dc_mouse_x;
	uint16                          dc_mouse_y;
	uint32                          dc_secs;
	uint32                          dc_micros;
	#endif

	#ifdef F_ENABLE_EVENT_KEY
	// keys

	IX                              keys[FV_KEY_COUNT];
	#endif

	// misc

	uint16                          button_clone_bug;

	#endif
};

#define FF_THREAD_READY                         (1 << 0)
#ifdef F_ENABLE_EVENT_TICK
#define FF_SENT_TICK                            (1 << 1)
#endif
#ifdef F_ENABLE_EVENT_MOTION
#define FF_SENT_MOTION                          (1 << 2)
#endif
#ifdef F_ENABLE_EVENT_BUTTON
#define FF_BUTTON_DOWN                          (1 << 3)
#endif
#ifdef F_ENABLE_EVENT_HELP
#define FF_SENT_HELP                            (1 << 4)
#define FF_ACTIVE_HELP                          (1 << 5)
#endif

#ifdef F_ENABLE_EVENT_HELP

#define DELAY_INITIAL_SECS                      2
#define DELAY_INITIAL_MICRO                     0

#define DELAY_NEXT_SECS                         1
#define DELAY_NEXT_MICRO                        0

enum    {

		FV_DELAY_HELP_SHORT,
		FV_DELAY_HELP_LONG

		};

#endif

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

F_HOOKM(FThreadMsg *, Thread_Main, FS_Thread_Run);

/************************************************************************************************
*** Macros **************************************************************************************
************************************************************************************************/

#define _event(post)                            (((FEvent *) ((FPost *) post)->Data))

#define F_MAKE_SENDED_MSG(lod,msg)              ((struct Message *) msg)->mn_Node.ln_Name = (STRPTR) (LOD)
#define F_IS_SENDED_MSG(lod,msg)                (((uint32) (lod)) == ((uint32)((struct Message *) msg)->mn_Node.ln_Name))
