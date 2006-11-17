#define F_ENABLE_PREFERENCES

/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <stdlib.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/rexxsyslib.h>
#include <proto/commodities.h>
#include <proto/iffparse.h>
#include <proto/feelin.h>

#include <intuition/intuition.h>
#include <libraries/commodities.h>
#include <libraries/iffparse.h>
#include <rexx/storage.h>
#include <workbench/workbench.h>
#include <libraries/feelin.h>
#include <feelin/preference.h>

extern struct ClassUserData *CUD;

#ifdef __amigaos4__
#define IREXXSYS                                IRexxSys ->
#else
#define IREXXSYS
#endif

#ifdef __amigaos4__
#define IDISKFONT                               IDiskfont ->
#else
#define IDISKFONT
#endif

#ifdef __amigaos4__
#define ICOMMODITIES                            ICommodities ->
#else
#define ICOMMODITIES
#endif

#ifdef __amigaos4__
#define IIFFPARSE                               IIFFParse ->
#else
#define IIFFPARSE
#endif

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		#ifndef F_NEW_PERSISTENT

		FM_Dataspace_Add,
		FM_Dataspace_Find,
		FM_Dataspace_Clear,
		FM_Dataspace_ReadIFF,
		FM_Dataspace_WriteIFF,

		#endif

		FV_AUTO_READ,

		#ifndef F_NEW_STYLES
		FM_Preference_Find,
		FM_Preference_ObtainAssociated,
		FM_Preference_ReleaseAssociated
		#endif

		};

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

#define TEMPLATE                    "PUSH/S,EVENTS/S"

enum  {

	  OPT_PUSH,
	  OPT_EVENTS

	  };

struct ClassUserData
{
	FObject                         db_Notify;

	bool32                          db_Push;
	bool32                          db_Events;
};

#define db_Array                                db_Push

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

struct in_SignalHandler
{
	FNode                           node;

	FObject                         target;
	uint32                          method;

	bits32                          signals;
};

struct in_TimerHandler
{
	FNode                           node;

	FObject                         target;
	uint32                          method;

	uint32                          seconds;
	uint32                          micros;
	struct timerequest              request;
};

typedef struct FeelinAppPush
{
	struct FeelinAppPush           *Next;
	struct FeelinAppPush           *Prev;
//  end of FeelinNode header
	FObject                         Target;
	uint32                          Method;
	APTR                            Msg;
//  message data below
}
FPush;

struct in_StyleSheet
{
	struct in_StyleSheet		   *next;
	struct in_StyleSheet		   *prev;

	APTR							source;
	uint32							source_type;
};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	bits32                          flags;

	STRPTR                          title;
	STRPTR                          version;
	STRPTR                          copyright;
	STRPTR                          author;
	STRPTR                          description;
	STRPTR                          base;

	FObject                         appserver;

/** stylesheet **/

	FObject                         css;

	/* stylesheet(s) added with  the  AddStyleSheet  method  are  linked  in
	'css_list' */

	FList							css_list;
	FNotifyHandler                 *css_notify_handler;















	FObject                         menu;
	
	APTR                            pool;

	
	APTR                            broker;
	struct NewBroker               *broker_init;
	struct MsgPort                 *app_port;
	struct MsgPort                 *timers_port;
	struct MsgPort                 *broker_port;

	int32                           sleep_count;
	bits32                          wait_signals;
	bits32                          user_signals;

	FList                           pushed_methods_list;
	
	struct timerequest             *time_request;

	FList                           timer_handlers_list;
	FList                           signal_handlers_list;

	FClass                         *display_class;          /* FIXME: use display server as fast as I can */
	FObject                         display;
};

#define FF_Application_Run                      (1 << 0)

/* The FF_Application_Run flags is set by  the  method  FM_Application_Run.
The  flag is checked within the main loop of the application. When the flag
is cleared the method ends (the application quits the main loop. */

#define FF_APPLICATION_UPDATE                   (1 << 1)
#define FF_APPLICATION_INHERITED_CSS            (1 << 2)

/* The FF_Application_Update flag is set by the  notification  set  on  the
FA_Preference_Update  attribute  of the FC_Preference object. When the flag
is  set  the  application  invoke  the  methods  FM_Application_Sleep   and
FM_Application_Awake to read new preferences */

#define FF_Application_Setup                    (1 << 3) // setup done

#define _application_is_inherited_css			((FF_APPLICATION_INHERITED_CSS & LOD->flags) != 0)
#define _application_isnt_inherited_css			((FF_APPLICATION_INHERITED_CSS & LOD->flags) == 0)
#define _application_set_inherited_css			LOD->flags |= FF_APPLICATION_INHERITED_CSS
#define _application_clear_inherited_css		LOD->flags &= ~FF_APPLICATION_INHERITED_CSS

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

bits32 app_collect_signals(FClass *Class,FObject Obj);

