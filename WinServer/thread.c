#include "Private.h"

/************************************************************************************************
*** Configuration *******************************************************************************
************************************************************************************************/

#define F_ENABLE_POST
#define F_ENABLE_LOCK
//#define F_ENABLE_DEBUG

#undef STATIC
#define STATIC

/************************************************************************************************
*** Debug ***************************************************************************************
************************************************************************************************/

#ifdef F_ENABLE_DEBUG

#include <stdarg.h>
#include <graphics/text.h>

//#define DB_WBDROP
//#define DB_POST
//#define DB_MAIN
//#define DB_INPUT_MAIN
//#define DB_WINDOW_ACTIVE
//#define DB_WINDOW_FOCUS
//#define DB_WINDOW_ACTIVE_SAME
//#define DB_KEY
//#define DB_INTUITION
//#define DB_HELP
//#define DB_BUTTON_ACTIVATE

//#define DB_POST_BUTTON

///db_print()

#include <stdlib.h>
 
void db_printA(uint32 line, STRPTR Fmt, int32 *Params)
{
	uint32 len = 0;
	STRPTR str = IFEELIN F_StrNewA(&len, Fmt, Params);

	if (str)
	{
		struct RastPort *rp = &IntuitionBase->ActiveScreen->RastPort;
		
		uint32 apen = IGRAPHICS GetAPen(rp);
		uint32 bpen = IGRAPHICS GetBPen(rp);
		uint32 drmode = IGRAPHICS GetDrMd(rp);

		IGRAPHICS SetABPenDrMd(rp, 1, 1 + (rand() % 25), JAM2);
		
		_Move(0,line * rp->Font->tf_YSize + rp->Font->tf_Baseline + 1);
		_Text(str, len);
		
		IGRAPHICS SetABPenDrMd(rp, apen, bpen, drmode);

		IFEELIN F_Dispose(str);
	}
}

void db_print(uint32 line, STRPTR Fmt, ...)
{
	va_list msg;
	va_start(msg,Fmt);
	
	db_printA(line, Fmt, (int32 *) msg);
}
//+

#endif

/************************************************************************************************
*** Events functions ****************************************************************************
************************************************************************************************/

#ifdef F_ENABLE_INPUT

///events_init
STATIC void events_init(struct LocalObjectData *LOD, FPost *Post, uint32 Qualifier, uint32 Seconds, uint32 Micro, struct in_FamilyNode *node)
{
	F_MAKE_SENDED_MSG(LOD, Post);

	Post->sys.mn_Node.ln_Type = NT_MESSAGE;
	Post->sys.mn_Length = sizeof(FPost) + sizeof (FEvent);
	Post->sys.mn_ReplyPort = LOD->ThreadPublic->Port;

	Post->Type = FV_Application_Post_Type_Event;
	Post->Data = (APTR)((uint32)(Post) + sizeof (FPost));

	_event(Post)->Key = FV_KEY_NONE;

	_event(Post)->Qualifier = Qualifier;
	_event(Post)->Seconds = Seconds;
	_event(Post)->Micros = Micro;
	
	if (node)
	{
		_event(Post)->MouseX = LOD->mouse_x - node->window->LeftEdge;
		_event(Post)->MouseY = LOD->mouse_y - node->window->TopEdge;
		_event(Post)->Target = node->Public.Object;
	}
	else
	{
		_event(Post)->MouseX = LOD->mouse_x;
		_event(Post)->MouseY = LOD->mouse_y;
	}
}
///+
///events_create
STATIC FPost * events_create(struct LocalObjectData *LOD, struct InputEvent *ie, struct in_FamilyNode *node)
{
	FPost *post = IFEELIN F_NewP(LOD->pool,sizeof (FPost) + sizeof (FEvent));

	if (post)
	{
		events_init(LOD, post, ie->ie_Qualifier, ie->ie_TimeStamp.tv_secs, ie->ie_TimeStamp.tv_micro, node);
	}

	return post;
}
///+
///events_post
STATIC void events_post(struct LocalObjectData *LOD,FPost *post)
{
	#ifdef DB_POST
	{
		db_print(2,"  class (0x%08lx) code (0x%04lx) flags (0x%08lx) mouse (%3ld:%3ld) active_node (0x%08lx) app (0x%08lx)  <<  ",_event(post)->Class,_event(post)->Code,_event(post)->Flags,_event(post)->MouseX,_event(post)->MouseY,LOD->window_active_node,LOD->window_active_node->application);
	}
	#endif

	#ifdef F_ENABLE_POST
	
	F_MAKE_SENDED_MSG(LOD,post);
 
	if ((LOD->window_active_node == NULL) ||
		(IFEELIN F_Do(LOD->window_active_node->application, FM_Application_Post, post) == FALSE))
	#endif
	{
		IFEELIN F_Dispose(post);
	}
}
//+

#ifdef F_ENABLE_EVENT_TICK
///events_kill_tick
STATIC void events_kill_tick(struct LocalObjectData *LOD)
{
	LOD->flags &= ~FF_SENT_TICK;
}
//+
///events_post_tick
STATIC void events_post_tick(struct LocalObjectData *LOD, struct InputEvent *ie)
{
	FPost *post;

	if ((post = events_create(LOD,ie,LOD->window_active_node)) != NULL)
	{
		_event(post)->Class = FF_EVENT_TICK;
		
		LOD->flags |= FF_SENT_TICK;

		events_post(LOD, post);
	}
}
///+
#endif
#ifdef F_ENABLE_EVENT_HELP
///events_kill_help
STATIC void events_kill_help(struct LocalObjectData *LOD)
{
	LOD->flags &= ~FF_SENT_HELP;
}
//+
///events_post_help
STATIC void events_post_help(struct LocalObjectData *LOD, struct InputEvent *ie, uint32 Code)
{
	FPost *post;

	if ((post = events_create(LOD,ie,LOD->window_focus_node)) != NULL)
	{
		_event(post)->Class = FF_EVENT_HELP;
		_event(post)->Code = Code;
									
		if (Code == FV_EVENT_HELP_OPEN)
		{
			LOD->flags |= FF_SENT_HELP | FF_ACTIVE_HELP;
		}
		else
		{
			LOD->flags &= ~FF_ACTIVE_HELP;
		}

		events_post(LOD, post);
	}
}
///+
///events_delay_help
void events_delay_help(struct LocalObjectData *LOD, uint32 Type)
{
	struct timeval now;
	
	ITIMER GetSysTime(&now);
	
	#ifdef DB_HELP

	db_print
	(
		4, "    timer (0x%08lx ?= 0x%08lx) cmp (%ld)    ",

		LOD->time.tv_secs,
		now.tv_secs,
		ITIMER CmpTime(&now, &LOD->time)
	);

	#endif
 
	if (Type == FV_DELAY_HELP_LONG)
	{
		#ifdef DB_HELP
		
		db_print
		(
			5, "    add long period    "
		);
		
		#endif

		LOD->time.tv_secs = DELAY_INITIAL_SECS;
		LOD->time.tv_micro = DELAY_INITIAL_MICRO;

		ITIMER AddTime(&LOD->time, &now);
	}
	else if (ITIMER CmpTime(&now, &LOD->time) < 0)
	{
		#ifdef DB_HELP

		db_print
		(
			5, "    add short period    "
		);

		#endif

		LOD->time.tv_secs = DELAY_NEXT_SECS;
		LOD->time.tv_micro = DELAY_NEXT_MICRO;
		 
		ITIMER AddTime(&LOD->time, &now);
	}
}
//+
#endif
#ifdef F_ENABLE_EVENT_BUTTON
///events_post_button
STATIC void events_post_button(struct LocalObjectData *LOD, struct InputEvent *ie)
{
	FPost *post;
	
	/* I don't know if this happens on other systems / configuration, but on
	my computer, I always receive menu-mouse-button twice. Is it a bug or an
	Intuition trick for its menus ?? */
	
	#if 1

	if (((ie->ie_Code & ~IECODE_UP_PREFIX) == IECODE_RBUTTON) && (LOD->button_clone_bug == ie->ie_Code))
	{
		//db_print(5, "    BUTTON CLONE BUG >> post button skipped    ");
		
		return;
	}
	 
	LOD->button_clone_bug = ie->ie_Code;
	
	#endif
	
	if ((post = events_create(LOD,ie,LOD->window_active_node)) != NULL)
	{
		_event(post)->Class = FF_EVENT_BUTTON;
		
		if ((IECODE_UP_PREFIX & ie->ie_Code) == 0)
		{
			_event(post)->Flags |= FF_EVENT_BUTTON_DOWN;
			
			LOD->flags |= FF_BUTTON_DOWN;
		}
		else
		{
			LOD->flags &= ~FF_BUTTON_DOWN;
		}

		switch (~IECODE_UP_PREFIX & ie->ie_Code)
		{
			case IECODE_LBUTTON: _event(post)->Code = FV_EVENT_BUTTON_SELECT; break;
			case IECODE_RBUTTON: _event(post)->Code = FV_EVENT_BUTTON_MENU; break;
			case IECODE_MBUTTON: _event(post)->Code = FV_EVENT_BUTTON_MIDDLE; break;
			default:             _event(post)->Code = 0xFFFF; break;
		}

		if (FF_EVENT_BUTTON_DOWN & _event(post)->Flags)
		{
			if ((LOD->dc_window  == LOD->window_active) &&
				(LOD->dc_button  == _event(post)->Code) &&
				(LOD->dc_mouse_x == _event(post)->MouseX) &&
				(LOD->dc_mouse_x == _event(post)->MouseY) &&
				
				IINTUITION DoubleClick(LOD->dc_secs,LOD->dc_micros, _event(post)->Seconds,_event(post)->Micros))
			{
				_event(post)->Flags |= FF_EVENT_REPEAT;
				
				LOD->dc_button = 0xFF;
			}
			else
			{
				LOD->dc_window  = LOD->window_active;
				LOD->dc_button  = _event(post)->Code;
				LOD->dc_mouse_x = _event(post)->MouseX;
				LOD->dc_mouse_y = _event(post)->MouseY;
				LOD->dc_secs    = _event(post)->Seconds;
				LOD->dc_micros  = _event(post)->Micros;
			}
		}
	
		#ifdef DB_POST_BUTTON
		
		db_print
		(
			5, "  button (%ld) mouse (%3ld, %3ld) target (0x%08lx)  ",
			
			_event(post)->Code,
			_event(post)->MouseX,
			_event(post)->MouseY,
			_event(post)->Target
		);
		
		#endif
		
		events_post(LOD, post);
	}
}
///+
#endif
#ifdef F_ENABLE_EVENT_MOTION
///events_kill_motion
STATIC void events_kill_motion(struct LocalObjectData *LOD)
{
	LOD->flags &= ~FF_SENT_MOTION;
}
//+
///events_post_motion
STATIC void events_post_motion(struct LocalObjectData *LOD, struct InputEvent *ie)
{
	FPost *post;

	if ((post = events_create(LOD,ie,LOD->window_active_node)) != NULL)
	{
		_event(post)->Class = FF_EVENT_MOTION;

		LOD->flags |= FF_SENT_MOTION;
 
		events_post(LOD, post);
	}
}
///+
#endif
#ifdef F_ENABLE_EVENT_KEY
///events_post_key
STATIC void events_post_key(struct LocalObjectData *LOD, struct InputEvent *ie)
{
	FPost *post;

	#ifdef DB_KEY
	{
		uint32 len = 0;
		STRPTR str = IFEELIN F_StrNew(&len, "  code (0x%04lx) qualifier (%04lx)  <<  ",ie->ie_Code,ie->ie_Qualifier);

		if (str)
		{
			struct RastPort *rp = &IntuitionBase->ActiveScreen->RastPort;

			IGRAPHICS SetABPenDrMd(rp, 1, 2, JAM2);
			_Move(0,10);
			_Text(str, len);

			IFEELIN F_Dispose(str);
		}
	}
	#endif

	if ((post = events_create(LOD,ie,LOD->window_active_node)) != NULL)
	{
		if (ie->ie_Code == 0x7A)
		{
		   _event(post)->Class  = FF_EVENT_BUTTON;
		   _event(post)->Code   = FV_EVENT_BUTTON_WHEEL;
		}
		else if (ie->ie_Code == 0x7B)
		{
		   _event(post)->Class  = FF_EVENT_BUTTON;
		   _event(post)->Code   = FV_EVENT_BUTTON_WHEEL;
		   _event(post)->Flags |= FF_EVENT_BUTTON_DOWN;
		}
		else
		{
			uint32 i;

			_event(post)->Class = FF_EVENT_KEY;
			_event(post)->Code  = ie->ie_Code & ~IECODE_UP_PREFIX;

			if (ie->ie_Code & IECODE_UP_PREFIX)
			{
				_event(post)->Flags |= FF_EVENT_KEY_UP;
			}
			if (ie->ie_Qualifier & IEQUALIFIER_REPEAT)
			{
				_event(post)->Flags |= FF_EVENT_REPEAT;
			}
			
			/* search a matching configurable key */

			for (i = 1 ; i < FV_KEY_COUNT ; i += 1)
			{
				if (ICOMMODITIES MatchIX(ie,&LOD->keys[i - 1]))
				{
					_event(post)->Key = i; break;
				}
			}

			/* try to decode the key as a char */

			if (_event(post)->Key == FV_KEY_NONE)
			{
				uint16 previous_code = ie->ie_Code;
				uint8 buf[4];

				ie->ie_Code = ie->ie_Code & ~IECODE_UP_PREFIX;

				if (IKEYMAP MapRawKey(ie,buf,3,0) == 1)
				{
					_event(post)->DecodedChar = buf[0];
				}
			
				ie->ie_Code = previous_code;
			}
		}

		events_post(LOD, post);
	}
}
///+
#endif
#ifdef F_ENABLE_EVENT_WBDROP
///events_kill_wbdrop
STATIC void events_kill_wbdrop(FPost *post)
{
	struct WBArg *ar = _event(post)->Special;

	if (ar)
	{
		while (ar->wa_Lock)
		{
			#ifdef DB_WBDROP
			IFEELIN F_Log(0,"UnLock (0x%08lx)",ar->wa_Lock);
			#endif

			IDOS_ UnLock(ar->wa_Lock); ar++;
		}
	}
}
///+
///events_post_wbdrop
STATIC FPost * events_post_wbdrop(struct LocalObjectData *LOD,struct AppMessage *AppMsg,struct MsgPort *ReplyPort)
{
	FPost *post;

	uint32 len = sizeof (FPost) + sizeof (FEvent) + (AppMsg->am_NumArgs + 1) * sizeof (struct WBArg);

	if (AppMsg->am_NumArgs && AppMsg->am_ArgList)
	{
		uint32 i;

		for (i = 0 ; i < AppMsg->am_NumArgs ; i++)
		{
			if (AppMsg->am_ArgList[i].wa_Name && *AppMsg->am_ArgList[i].wa_Name)
			{
				uint32 add = IFEELIN F_StrLen(AppMsg->am_ArgList[i].wa_Name) + 1;

				#ifdef DB_WBDROP
				IFEELIN F_Log
				(
					0, "lock (0x%08lx) name (0x%08lx)(%s)(%ld)",
					
					AppMsg->am_ArgList[i].wa_Lock,
					AppMsg->am_ArgList[i].wa_Name,
					AppMsg->am_ArgList[i].wa_Name,add
				);
				#endif

				len += add;
			}
			#ifdef DB_WBDROP
			else
			{
				IFEELIN F_Log(0,"lock (0x%08lx)",AppMsg->am_ArgList[i].wa_Lock);
			}
			#endif
		}
	}

	if ((post = IFEELIN F_NewP(LOD->pool,len)) != NULL)
	{
		struct in_FamilyNode *node = (struct in_FamilyNode *) AppMsg->am_UserData;

		events_init(LOD, post, 0, AppMsg->am_Seconds, AppMsg->am_Micros, node);
 
		/* FEvent */

		_event(post)->Class   = FF_EVENT_WBDROP;
		_event(post)->Code    = AppMsg->am_NumArgs;
		_event(post)->Key     = FV_KEY_NONE,
		_event(post)->MouseX  = AppMsg->am_MouseX;
		_event(post)->MouseY  = AppMsg->am_MouseY;
		_event(post)->Special = (APTR)((uint32)(post->Data) + sizeof (FEvent));

		/* WBArg */

		if (AppMsg->am_NumArgs && AppMsg->am_ArgList)
		{
			struct WBArg *ar = _event(post)->Special;
			STRPTR name = (STRPTR)((uint32)(ar) + (AppMsg->am_NumArgs + 1) * sizeof (struct WBArg));
			uint32 i;

			for (i = 0 ; i < AppMsg->am_NumArgs ; i++)
			{
				uint32 l = IFEELIN F_StrLen(AppMsg->am_ArgList[i].wa_Name);

				ar[i].wa_Lock = IDOS_ DupLock(AppMsg->am_ArgList[i].wa_Lock);

				if (l)
				{
					ar[i].wa_Name = name;

					IEXEC CopyMem(AppMsg->am_ArgList[i].wa_Name, name, l);

					name += l + 1;
				}
			}
		}

		/* Post */

		#ifdef DB_WBDROP
		IFEELIN F_Log
		(
			0,"thread_post_from_appmessage() msg (0x%08lx) win %s{%lx} app %s{%lx}",
			
			AppMsg,
			_object_classname(node->Public.Object),
			node->Public.Object,
			_object_classname(node->application),
			node->application
		);
		#endif

		#ifdef DB_WBDROP
		{
			BPTR fh = IDOS_ Open("ram:post.log",MODE_NEWFILE);

			if (fh)
			{
				IDOS_ Write(fh, post, len);

				IDOS_ Close(fh);
			}
		}
		#endif

		if (IFEELIN F_Do(node->application, FM_Application_Post, post) == FALSE)
		{
			events_kill_wbdrop(post); post = NULL;
		}
	}

	return post;
}
//+
#endif

#ifdef F_ENABLE_EVENT_WINDOW
///events_post_window
STATIC void events_post_window(struct LocalObjectData *LOD, struct InputEvent *ie, struct in_FamilyNode *node, uint16 Code)
{
	FPost *post;

	if ((post = events_create(LOD,ie,node)) != NULL)
	{
		_event(post)->Class = FF_EVENT_WINDOW;
		_event(post)->Code  = Code;

		events_post(LOD, post);
	}
}
///+
#endif /* F_ENABLE_EVENT_WINDOW */

///events_check_window_active
STATIC void events_check_window_active(struct LocalObjectData *LOD, struct InputEvent *ie)
{
	if (IntuitionBase->ActiveWindow != LOD->window_active)
	{
		struct in_FamilyNode *node = NULL;

		if ((LOD->window_active = IntuitionBase->ActiveWindow) != NULL)
		{
			for (node = (struct in_FamilyNode *) LOD->family.Head ; node ; node = (struct in_FamilyNode *) node->Public.Next)
			{
				if (node->window == LOD->window_active)
				{
					break;
				}
			}
		}

		LOD->window_active_node = node;

#ifdef DB_WINDOW_ACTIVE

		if (node)
		{
			db_print(0,"  new active window (0x%08lx) node (0x%08lx) events (0x%08lx)  <<  ", LOD->window_active,LOD->window_active_node,LOD->window_active_node->events);
		}
		else
		{
			db_print(0,"  no feelin active window (0x%08lx)  <<  ",LOD->window_active);
		}
	}
	#ifdef DB_WINDOW_ACTIVE_SAME
	else
	{
		db_print(0,"  same window (0x%08lx) node (0x%08lx)  <<  ", LOD->window_active, LOD->window_active_node);
	}
	#endif

#else
	
	}

#endif
}
//+
///events_check_window_focus
STATIC void events_check_window_focus(struct LocalObjectData *LOD, struct InputEvent *ie)
{
	#ifdef DB_WINDOW_FOCUS
	STRPTR str = NULL;      
	uint32 len = 0;
	#endif

	if (LOD->screen)
	{
		struct Layer *layer = ILAYERS WhichLayer(&LOD->screen->LayerInfo,LOD->mouse_x,LOD->mouse_y);
		struct Window *layer_window = (layer) ? layer->Window : NULL;

		if (layer_window != LOD->window_focus)
		{
			struct in_FamilyNode *node = NULL;

			if ((LOD->window_focus = layer_window) != NULL)
			{
				for (node = (struct in_FamilyNode *) LOD->family.Head ; node ; node = (struct in_FamilyNode *) node->Public.Next)
				{
					if (node->window == LOD->window_focus)
					{
						break;
					}
				}
			}

			LOD->window_focus_node = node;

#ifdef DB_WINDOW_FOCUS

			if (node)
			{
				str = IFEELIN F_StrNew(&len,"  new focus window (0x%08lx) node (0x%08lx) events (0x%08lx)  <<", LOD->window_focus,LOD->window_focus_node,LOD->window_focus_node->events);
			}
			else
			{
				str = IFEELIN F_StrNew(&len,"  no feelin focus window (0x%08lx)  <<",LOD->window_focus);
			}
		}
	#ifdef DB_WINDOW_FOCUS_SAME
		else
		{
			str = IFEELIN F_StrNew(&len,"  same focus window (0x%08lx) node (0x%08lx)  <<", LOD->window_focus, LOD->window_focus_node);
		}
	#endif
	}

	if (str)
	{
		{
			struct RastPort *rp = &LOD->screen->RastPort;

			IGRAPHICS SetABPenDrMd(rp, 1, 2, JAM2);
			_Move(0,9);
			_Text(str, len);

			IFEELIN F_Dispose(str);
		}
	#else
		}
#endif

	}
}
//+

///input_handler

#if defined(__MORPHOS__)
						
struct InputEvent * input_handler(void)
{
	struct InputEvent *ie = (struct InputEvent *) REG_A0;
	struct LocalObjectData *LOD = (struct LocalObjectData *) REG_A1;

#elif defined(__amigaos4__)

struct InputEvent *input_handler(struct InputEvent *ie, struct LocalObjectData *LOD)
{

#else /* classic */
 
SAVEDS ASM(struct InputEvent *) input_handler
(
	REG(a0, struct InputEvent *ie),
	REG(a1, struct LocalObjectData *LOD)
)
{
	
#endif
 
	uint32 eat = FALSE;
	int8 update_screen_active = FALSE;
 
	if (((FF_THREAD_READY & LOD->flags) == 0) || (ie->ie_Class == IECLASS_NULL))
	{
		return ie;
	}

	#ifdef F_ENABLE_LOCK
	IFEELIN F_Do(LOD->Server, FM_Lock, FF_Lock_Shared);
	#endif
	
	if (LOD->screen != IntuitionBase->ActiveScreen)
	{
		LOD->screen = IntuitionBase->ActiveScreen;

		update_screen_active = TRUE;
	}

/*** check bubble help **************************************************************************

	The bubble help must be closed if  an  event  other  than  IECLASS_TIMER
	reaches our function.
	
	FIXME: we may need to check more than just IECLASS_TIMER...

*/

	#ifdef F_ENABLE_EVENT_HELP

	if (ie->ie_Class != IECLASS_TIMER)
	{
		events_delay_help(LOD, FV_DELAY_HELP_LONG);

		if (FF_ACTIVE_HELP & LOD->flags)
		{
			events_post_help(LOD, ie, FV_EVENT_HELP_CLOSE);
		}
	}

	#endif

/*** check active window ************************************************************************

	If F_ENABLE_EVENT_WINDOW_ACTIVE is defined, the active state of  windows
	is sended when it changes.
 
*/
		
	if ((FF_EVENT_TICK | FF_EVENT_KEY | FF_EVENT_MOTION | FF_EVENT_BUTTON | FF_EVENT_WINDOW) & LOD->events)
	{
		#ifdef F_ENABLE_EVENT_WINDOW_ACTIVE
		
		struct Window *previous_active = LOD->window_active;
		struct in_FamilyNode *previous_node = LOD->window_active_node;
										
		events_check_window_active(LOD,ie);
 
		if (previous_active != LOD->window_active)
		{
			if (previous_node)
			{
				events_post_window(LOD, ie, previous_node, FV_EVENT_WINDOW_INACTIVE);
			}

			if (LOD->window_active_node)
			{
				events_post_window(LOD, ie, LOD->window_active_node, FV_EVENT_WINDOW_ACTIVE);
			}
		}
		
		#else
			
		events_check_window_active(LOD,ie);
 
		#endif
	}
	else
	{
		LOD->window_active = NULL;
		LOD->window_active_node = NULL;
	}

/*** check mouse motion *************************************************************************

	Here  we  check  mouse  coordinates  which  may  have  changed   without
	triggering a IECLASS_RAWMOUSE event.
 
*/
 
	if (LOD->screen)
	{
		if (LOD->mouse_x != LOD->screen->MouseX ||
			LOD->mouse_y != LOD->screen->MouseY || (update_screen_active))
		{
			LOD->mouse_x = LOD->screen->MouseX;
			LOD->mouse_y = LOD->screen->MouseY;

			#ifdef F_ENABLE_EVENT_MOTION

			if (ie->ie_Class != IECLASS_RAWMOUSE)
			{
				if ((LOD->window_active_node != NULL) && (FF_EVENT_MOTION & LOD->window_active_node->events))
				{
					events_post_motion(LOD, ie);
				}
			}

			#endif
		}
	}
 
/*** proceed with the other events *************************************************************/
	
#if defined(F_ENABLE_EVENT_TICK) || defined(F_ENABLE_EVENT_HELP) || defined(F_ENABLE_EVENT_MOTION) || defined(F_ENABLE_EVENT_BUTTON) || defined(F_ENABLE_EVENT_KEY) || defined(F_ENABLE_EVENT_WINDOW_ACTIVE)
	
	switch (ie->ie_Class)
	{
///TICK | HELP
		#if defined(F_ENABLE_EVENT_TICK) || defined(F_ENABLE_EVENT_HELP)
		case IECLASS_TIMER:
		{
			if ((LOD->window_active_node != NULL) && (FF_EVENT_TICK & LOD->window_active_node->events))
			{
				if ((FF_SENT_TICK & LOD->flags) == 0)
				{
					events_post_tick(LOD, ie);
				}
			}
		
			#ifdef F_ENABLE_EVENT_HELP
					
			if (((FF_SENT_HELP | FF_ACTIVE_HELP) & LOD->flags) == 0)
			{
				if (FF_BUTTON_DOWN & LOD->flags)
				{

					/* one of the mouse button is still down, we cannot open
					the pop help */
 
					events_delay_help(LOD, FV_DELAY_HELP_LONG);
				}
				else
				{
					#ifdef DB_HELP
					
					db_print
					(
						0, "    waiting right time to open pop help: timer (0x%08lx ?= 0x%08lx) cmp (%ld)    ",
						
						LOD->time.tv_secs,
						ie->ie_TimeStamp.tv_secs,
						ITIMER CmpTime(&ie->ie_TimeStamp, &LOD->time)
					);
					
					#endif

					if (ITIMER CmpTime(&ie->ie_TimeStamp, &LOD->time) < 0)
					{
						events_check_window_focus(LOD,ie);
						
						if ((LOD->window_focus_node != NULL) && (FF_EVENT_HELP & LOD->window_focus_node->events))
						{
							events_post_help(LOD, ie, FV_EVENT_HELP_OPEN);
						}
					}

				}
			}
 
			#endif /* F_ENABLE_EVENT_HELP */
		}
		break;
		#endif /* F_ENABLE_EVENT_TICK */
//+
///MOTION | BUTTON
		#if defined(F_ENABLE_EVENT_MOTION) || defined(F_ENABLE_EVENT_BUTTON)
		case IECLASS_RAWMOUSE:
		{
			#ifdef F_ENABLE_EVENT_MOTION
			if (ie->ie_Code == IECODE_NOBUTTON)
			{
				if ((LOD->window_active_node != NULL) && (FF_EVENT_MOTION & LOD->window_active_node->events))
				{
					if ((FF_SENT_MOTION & LOD->flags) == 0)
					{
						events_post_motion(LOD, ie);
					}
				}
			}
			#endif /* F_ENABLE_EVENT_MOTION */

			#if defined(F_ENABLE_EVENT_BUTTON) || defined(F_ENABLE_EVENT_ACTIVE)
			#ifdef F_ENABLE_EVENT_MOTION
			else
			#else
			if (ie->ie_Code != IECODE_NOBUTTON)
			#endif
			{
				#ifdef F_ENABLE_EVENT_WINDOW_ACTIVE
				
				if (ie->ie_Code == IECODE_LBUTTON)
				{
					struct Window *previous_focus = LOD->window_focus;

					#ifdef DB_WINDOW_ACTIVE
					struct in_FamilyNode *previous_focus_node = LOD->window_focus_node;
						
					events_check_window_focus(LOD,ie);
 
					db_print
					(
						1, "    active (0x%08lx)/(0x%08lx.0x%08lx) - focus (0x%08lx)/(0x%08lx.0x%08lx) - check (0x%08lx)/(0x%08lx.0x%08lx)   ",
						
						LOD->window_active,
						LOD->window_active_node,
					   (LOD->window_active_node) ? LOD->window_active_node->window : NULL,
						
						previous_focus,
						previous_focus_node,
					   (previous_focus_node) ? previous_focus_node->window : NULL,
 
						LOD->window_focus,
						LOD->window_focus_node,
					   (LOD->window_focus_node) ? LOD->window_focus_node->window : NULL
					);
					#else

					events_check_window_focus(LOD,ie);
					
					#endif
					
					if (previous_focus != LOD->window_focus)
					{
						#ifdef DB_WINDOW_ACTIVE
						
						db_print
						(
							2,
							
							"    NEW FOCUS: window (0x%08lx) node (0x%08lx) - mouse (%3ld:%3ld)    ",
							
							LOD->window_focus, LOD->window_focus_node,
							LOD->mouse_x, LOD->mouse_y
						);
						
						#endif
					
						if (LOD->window_active != LOD->window_focus)
						{
							if (LOD->window_active_node)
							{
								if (FF_NODE_ACTIVABLE & LOD->window_active_node->flags)
								{
									events_post_window(LOD, ie, LOD->window_active_node, FV_EVENT_WINDOW_INACTIVE);
								}
							}
						
							LOD->window_active = LOD->window_focus;
							LOD->window_active_node = LOD->window_focus_node;

							if (LOD->window_active_node)
							{
								if (FF_NODE_ACTIVABLE & LOD->window_active_node->flags)
								{
									events_post_window(LOD, ie, LOD->window_active_node, FV_EVENT_WINDOW_ACTIVE);
								}
								else
								{
									eat = TRUE;
								}
							}
						}
					}
				}
			
				#endif /* F_ENABLE_EVENT_WINDOW_ACTIVE */

				#ifdef F_ENABLE_EVENT_BUTTON
				 
				if ((LOD->window_active_node != NULL) && (FF_EVENT_BUTTON & LOD->window_active_node->events))
				{
					events_post_button(LOD, ie);
				}
			
				#endif /* F_ENABLE_EVENT_BUTTON */
			}
			#endif
		}
		break;

		#endif /* F_ENABLE_EVENT_MOTION || F_ENABLE_EVENT_BUTTON */
//+
///KEY
		#ifdef F_ENABLE_EVENT_KEY
		
		case IECLASS_RAWKEY:
		{
			if ((LOD->window_active_node != NULL) && (FF_EVENT_KEY & LOD->window_active_node->events))
			{
				events_post_key(LOD, ie);
			}
		}
		break;

		#endif /* F_ENABLE_EVENT_KEY */
//+
	}
	
#endif
	
	#ifdef F_ENABLE_LOCK
	IFEELIN F_Do(LOD->Server, FM_Unlock);
	#endif
 
	return (eat) ? NULL : ie;
}
#ifdef __MORPHOS__
_MAKEGATE(input_handler, input_handler);
#endif

//+

#endif /* F_ENABLE_INPUT */

#if defined(F_ENABLE_INPUT) || defined(F_ENABLE_INTUITION)

///thread_init
STATIC uint32 thread_init(struct LocalObjectData *LOD)
{
	LOD->flags &= ~FF_THREAD_READY;

	#ifdef F_ENABLE_INTUITION
	if ((LOD->intuition_port = IEXEC CreateMsgPort()) != NULL)
	#endif
	{
		#ifdef F_ENABLE_INPUT
		if ((LOD->input_port = IEXEC CreateMsgPort()))
		{
			if ((LOD->input_request = IEXEC CreateIORequest(LOD->input_port,sizeof (struct IOStdReq))))
			{
				if (!IEXEC OpenDevice("input.device",0,(struct IORequest *) LOD->input_request,0))
				{
					LOD->input_request->io_Data = (APTR) &LOD->input_handler;
					LOD->input_request->io_Command = IND_ADDHANDLER;
					IEXEC DoIO((struct IORequest *) LOD->input_request);
		#endif
					LOD->flags |= FF_THREAD_READY;
					
					return TRUE;
		#ifdef F_ENABLE_INPUT
				}
				IEXEC DeleteIORequest(LOD->input_request); LOD->input_request = NULL;
			}
			IEXEC DeleteMsgPort(LOD->input_port); LOD->input_port = NULL;
		}
		
		#ifdef F_ENABLE_INTUITION
		
		IEXEC DeleteMsgPort(LOD->intuition_port); 
		
		#endif
		#endif
	}
	return FALSE;
}
//+
///thread_exit
STATIC void thread_exit(struct LocalObjectData *LOD)
{
	if (FF_THREAD_READY & LOD->flags)
	{
		LOD->flags &= ~FF_THREAD_READY;

		#ifdef F_ENABLE_INPUT
		LOD->input_request->io_Data = (APTR) &LOD->input_handler;
		LOD->input_request->io_Command = IND_REMHANDLER;

		IEXEC DoIO((struct IORequest *) LOD->input_request);
		IEXEC CloseDevice((struct IORequest *) LOD->input_request);
		IEXEC DeleteIORequest(LOD->input_request); LOD->input_request = NULL;
		IEXEC DeleteMsgPort(LOD->input_port); LOD->input_port = NULL;
		#endif
		
		#ifdef F_ENABLE_INTUITION
		IEXEC DeleteMsgPort(LOD->intuition_port); LOD->intuition_port = NULL;
		#endif
	}
}
//+

#endif /* F_ENABLE_INPUT || F_ENABLE_INTUITION */

///Thread_Main
F_THREAD_ENTRY(Thread_Main)
{
	struct LocalObjectData *LOD = UserData;

	uint8 read_again = FALSE;

	LOD->ThreadPublic = Public;

	#ifdef F_ENABLE_INPUT

	LOD->input_handler.is_Code = (void (* )()) F_FUNCTION_GATE(input_handler);
	LOD->input_handler.is_Data = LOD;
	LOD->input_handler.is_Node.ln_Pri = 54; /* should be enough to be before Intuition */
	LOD->input_handler.is_Node.ln_Name = "winserver.handler";
	
	#ifdef __amigaos4__
	LOD->input_handler.is_Node.ln_Type = NT_EXTINTERRUPT;
	#else
	LOD->input_handler.is_Node.ln_Type = NT_INTERRUPT;
	#endif

	#ifdef F_ENABLE_EVENT_HELP
		
	ITIMER GetSysTime(&LOD->time);
   
	#endif
 
	#endif
 
	for (;;)
	{
		FThreadMsg *msg;
		
		if ((msg = (FThreadMsg *) IFEELIN F_Do(Thread, Public->id_Pop)) != NULL)
		{
			read_again = TRUE;
 
			#ifdef DB_MAIN
			IFEELIN F_Log(0,"message (0x%08lx) thread_msg (%s)",msg,F_IS_THREAD_MSG(msg) ? "yes" : "no");
			#endif
 
			if (F_IS_THREAD_MSG(msg))
			{
				switch (msg->Action)
				{
					case FV_Thread_Hello:
					{
						#if defined(F_ENABLE_INPUT) || defined(F_ENABLE_INTUITION)
						
						msg->Return = thread_init(LOD);
						
						#else
							
						msg->Return = TRUE;

						#endif
					}
					break;

					case FV_Thread_Bye:
					{
						#if defined(F_ENABLE_INPUT) || defined(F_ENABLE_INTUITION)
						
						thread_exit(LOD);
						
						#endif
 
						msg->Return = TRUE;

						return msg;
					}
					break;
				}
			}
			else if (F_IS_SENDED_MSG(LOD,msg))
			{
				#ifdef DB_MAIN
				IFEELIN F_Log(0,"disposed sended message (0x%08lx)",msg);
				#endif
				
				#ifdef F_ENABLE_EVENT_TICK
				if (_event(msg)->Class == FF_EVENT_TICK)
				{
					events_kill_tick(LOD);
				}
				#ifdef F_ENABLE_EVENT_HELP
				else
				#endif
				#endif
				
				#ifdef F_ENABLE_EVENT_HELP
				if (_event(msg)->Class == FF_EVENT_HELP)
				{
					events_kill_help(LOD);
				}
				#ifdef F_ENABLE_EVENT_MOTION
				else
				#endif
				#endif
				
				#ifdef F_ENABLE_EVENT_MOTION
				if (_event(msg)->Class == FF_EVENT_MOTION)
				{
					events_kill_motion(LOD);
				}
				#ifdef F_ENABLE_EVENT_WBDROP
				else
				#endif
				#endif
				
				#ifdef F_ENABLE_EVENT_WBDROP
				if (_event(msg)->Class == FF_EVENT_WBDROP)
				{
					events_kill_wbdrop((FPost *) msg);
				}
				#endif

				IFEELIN F_Dispose(msg);
				
				msg = NULL;
			}
			#ifdef F_ENABLE_EVENT_WBDROP
			else
			{
				if (((((struct AppMessage *)(msg))->am_Type == AMTYPE_APPWINDOW) ||
					 (((struct AppMessage *)(msg))->am_Type == AMTYPE_APPICON) ||
					 (((struct AppMessage *)(msg))->am_Type == AMTYPE_APPMENUITEM)) &&
					(((struct AppMessage *)(msg))->am_ID == MAKE_ID('A','P','P','W')))
				{
					events_post_wbdrop(LOD, (struct AppMessage *) msg, Public->Port);
				}
			}
			#endif
			
			if (msg)
			{
				#ifdef DB_MAIN
				IFEELIN F_Log(0,"replymsg (0x%08lx)",msg);
				#endif
 
				IEXEC ReplyMsg((struct Message *) msg);
			}
		}
		
		#ifdef F_ENABLE_INTUITION
		
		#ifdef F_ENABLE_LOCK
		IFEELIN F_Do(LOD->Server, FM_Lock, FF_Lock_Shared);
		#endif
		
		if (LOD->intuition_port)
		{
			struct IntuiMessage *msg = (struct IntuiMessage *) IEXEC GetMsg(LOD->intuition_port);
			
			if (msg)
			{
				FPost *post = IFEELIN F_NewP(LOD->pool,sizeof (FPost) + sizeof (FEvent));
				
				read_again = TRUE;
				
				#ifdef F_ENABLE_EVENT_HELP
					
				events_delay_help(LOD, FV_DELAY_HELP_LONG);
   
				#endif
				
				if (post)
				{
					#ifdef DB_INTUITION
					STRPTR class_name = "unsupported";
					#endif
					struct in_FamilyNode *node = (struct in_FamilyNode *) msg->IDCMPWindow->UserData;

					if (node)
					{
						events_init(LOD, post, msg->Qualifier, msg->Seconds, msg->Micros, node);

						_event(post)->Class  = FF_EVENT_WINDOW;
						_event(post)->MouseX = msg->MouseX;
						_event(post)->MouseY = msg->MouseY;
						_event(post)->Target = node->Public.Object;
							
						switch (msg->Class)
						{
							case IDCMP_CLOSEWINDOW:
							{
								_event(post)->Code  = FV_EVENT_WINDOW_CLOSE;
								
								#ifdef DB_INTUITION
								class_name = "close";
								#endif
							}
							break;
							
							case IDCMP_ACTIVEWINDOW:
							{
								_event(post)->Code  = FV_EVENT_WINDOW_ACTIVE;

								#ifdef DB_INTUITION
								class_name = "active";
								#endif
							}
							break;
							
							case IDCMP_INACTIVEWINDOW:
							{
								_event(post)->Code  = FV_EVENT_WINDOW_INACTIVE;

								#ifdef DB_INTUITION
								class_name = "inactive";
								#endif
							}
							break;
							
							case IDCMP_NEWSIZE:
							{
								_event(post)->Code  = FV_EVENT_WINDOW_SIZE;

								#ifdef DB_INTUITION
								class_name = "newisze";
								#endif
							}
							break;
							
							case IDCMP_REFRESHWINDOW:
							{
								_event(post)->Code  = FV_EVENT_WINDOW_REFRESH;

								#ifdef DB_INTUITION
								class_name = "refresh";
								#endif
							}
							break;

							case IDCMP_CHANGEWINDOW:
							{
								if (msg->Code)
								{
									_event(post)->Code  = FV_EVENT_WINDOW_DEPTH;
								}
								else
								{
									_event(post)->Code  = FV_EVENT_WINDOW_CHANGE;
								}

								#ifdef DB_INTUITION
								class_name = "change";
								#endif
							}
							break;
						
							#ifdef DB_INTUITION
							default:
							{
								IFEELIN F_Log(0,"unsupported class received (0x%08lx) code (0x%08lx) IDCMPWindow (0x%08lx)",msg->Class,msg->Code,msg->IDCMPWindow);
								
								_event(post)->Class = 0xFFFFFFFF;
								_event(post)->Code = 0xFFFF;
							}
							break;
							#endif
						}
					
						#ifdef DB_INTUITION
						IFEELIN F_Log(0,"intuition: class (%s) code (%04lx) node (0x%08lx) app (0x%08lx)",class_name,_event(post)->Code,node,node->application);
						#endif
					
						#ifdef F_ENABLE_POST

						if (IFEELIN F_Do(node->application, FM_Application_Post, post) == FALSE)
						#endif
						{
							IFEELIN F_Dispose(post);
						}
					}
				}
 
				IEXEC ReplyMsg((struct Message *) msg);
			}
		}
		
		#ifdef F_ENABLE_LOCK
		IFEELIN F_Do(LOD->Server, FM_Unlock);
		#endif
 
		#endif
 
		if (read_again)
		{
			read_again = FALSE;
		}
		else
		{
			#ifdef DB_MAIN
			IFEELIN F_Log(0,"waiting...");
			#endif
 
			#ifdef F_ENABLE_INTUITION
			
			IFEELIN F_Do(Thread, Public->id_Wait, (LOD->intuition_port) ? (1 << LOD->intuition_port->mp_SigBit) : 0);
			
			#else
			
			IFEELIN F_Do(Thread, Public->id_Wait, 0);
			
			#endif
		}
	}
}
//+
