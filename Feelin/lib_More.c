/*
**    Miscellaneous...
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************
			
$VER: 10.00 (2005/12/01)

	Updated F_Draw() to support the new 'damaged' refreshing technique.
	
	Updated F_Layout() to support the new 'damaged' technique.
	
	Added the F_StrNewP() function.
	
	Added AmigaOS 4 support.
	
	F_LogA() is now more secure. For example, class and object  are  checked
	before  reading  datas  to create the additionnal string. A semaphore is
	also used to avoid simultaneous outputs.
 
$VER: 09.00 (2005/06/09)
 
	F_StrFmt() now returns a pointer to the end  of  the  formated  string,
	which may be used to add further data.
	
	F_LogA()  wasn't  displaying  developer  message   when   called   from
	"input.device" task.
 
$VER: 08.02 (2005/01/13)

	Improved F_LogA() : if the  task  is  not  a  process  the  message  is
	displayed  in  an  EZRequest  window, with the location of the error as
	window title. This time, I also check the level of  the  log,  which  I
	never did before... oops !

************************************************************************************************/

#include "Private.h"

#define F_CODE_DRAW_CHECK_RENDER
#define F_CODE_DRAW_CHECK_RPORT
#define F_CODE_DRAW_CHECK_BITMAP
#define F_CODE_DRAW_CHECK_COORDINATES
//#define F_CODE_DRAW_BUFFERIZE_MOVE
//#define F_CODE_DRAW_VERBOSE_FORBID
//#define F_CODE_DRAW_VERBOSE_DRAWABLE
//#define F_CODE_DRAW_VERBOSE_NOTDAMAGED
#define F_CODE_DRAW_VERBOSE_COORDINATES
//#define F_CODE_DRAW_ONLYDAMAGED

#define F_ENABLE_LOG_LOCKING
 
/************************************************************************************************
*** Shared **************************************************************************************
************************************************************************************************/

///f_find_attribute_name
STRPTR f_find_attribute_name(uint32 id,struct in_FeelinBase *FeelinBase)
{
	FClass *node;
	STRPTR name=NULL;

	for (F_CLASSES_EACH(node))
	{
		if (node->Attributes)
		{
			FClassAttribute *e;

			for (F_ATTRIBUTES_EACH(node->Attributes, e))
			{
				if (_attribute_name(e))
				{
					if (id == e->ID)
					{
						name = _attribute_name(e); break;
					}
				}
			}
		}
		if (name) break;
	}
	return name;
}
//+
///f_find_method_name
STRPTR f_find_method_name(uint32 id,struct in_FeelinBase *FeelinBase)
{
	FClass *node;
	STRPTR name=NULL;

	for (F_CLASSES_EACH(node))
	{
		if (node->Methods)
		{
			FClassMethod *e;

			for (F_METHODS_EACH(node->Methods, e))
			{
				if (_method_name(e))
				{
					if (id == e->ID)
					{
						name = _method_name(e); break;
					}
				}
			}
		}
		if (name) break;
	}
	return name;
}
//+

/************************************************************************************************
*** API - string ********************************************************************************
************************************************************************************************/

///put_func & len_func

#if defined (__AROS__)
#include <aros/asmcall.h>

AROS_UFH2S(void, put_func,
	AROS_UFHA(uint8, chr, D0),
	AROS_UFHA(STRPTR *, strPtrPtr, A3))
{
	AROS_USERFUNC_INIT

	*(*strPtrPtr)++ = chr;

	AROS_USERFUNC_EXIT
}

AROS_UFH2S(void, len_func,
	AROS_UFHA(uint8, chr, D0),
	AROS_UFHA(uint32 *, lenPtr, A3))
{
	AROS_USERFUNC_INIT

	(*lenPtr)++;

	AROS_USERFUNC_EXIT
}

#elif defined (__amigaos4__)

void len_func(char currChar, uint32 *length)
{
	(*length)++;
	
	return;
}

#else /* classic && MorphOS */

STATIC const uint32 put_func = 0x16C04E75; /* MOVE.B d0,(a3)+ ; RTS */
STATIC const uint32 len_func = 0x52934E75; /* ADDQ.L #1,(a3)  ; RTS */

#endif
//+

///f_str_newA
F_LIB_STR_NEW
{
	STRPTR string = NULL;
	uint32 len = 0;

	if (Fmt == NULL)
	{
		if (LenPtr)
		{
			*LenPtr = 0;
		}

		return NULL;
	}

	#ifdef F_ENABLE_SAFE_STRING

	if (((int32)(Fmt) < 0xFFFF) || (IEXEC TypeOfMem(Fmt) == 0))
	{
		IFEELIN F_Log(0,"(db)F_StrNew() invalid address: Fmt (0x%08lx)",Fmt);

		if (LenPtr)
		{
			*LenPtr = 0;
		}

		return NULL;
	}

	#endif
 
 
	#ifdef __AROS__
	IEXEC RawDoFmt(Fmt,Params,(VOID_FUNC)AROS_ASMSYMNAME(len_func),&len);
	#else
	IEXEC RawDoFmt(Fmt,Params,(void *)(&len_func),&len);
	#endif

	if (len > 1)
	{
		if ((string = IFEELIN F_New(len)) != NULL)
		{
			#if defined (__AROS__)
			STRPTR stringptr = string;
			IEXEC RawDoFmt(Fmt,Params,(VOID_FUNC)AROS_ASMSYMNAME(put_func),&stringptr);
			#elif defined (__amigaos4__)
			IEXEC RawDoFmt(Fmt,Params,NULL,string);
			#else
			IEXEC RawDoFmt(Fmt,Params,(void *)(&put_func),string);
			#endif
		}
	}

	if (LenPtr)
	{
		*LenPtr = len - 1;
	}

	return string;
}
//+
///f_str_newpA
F_LIB_STR_NEWP
{
	STRPTR string = NULL;
	uint32 len = 0;
	
	if ((Pool == NULL) || (Fmt == NULL))
	{
		if (LenPtr)
		{
			*LenPtr = 0;
		}
 
		return NULL;
	}
	
	#ifdef F_ENABLE_SAFE_STRING

	if (((int32)(Fmt) < 0xFFFF) || (IEXEC TypeOfMem(Fmt) == 0))
	{
		IFEELIN F_Log(0,"(db)F_StrNewP() invalid address: Fmt (0x%08lx)",Fmt);
				
		if (LenPtr)
		{
			*LenPtr = 0;
		}

		return NULL;
	}

	#endif
 
	#ifdef __AROS__
	IEXEC RawDoFmt(Fmt,Params,(VOID_FUNC)AROS_ASMSYMNAME(len_func),&len);
	#else
	IEXEC RawDoFmt(Fmt,Params,(void *)(&len_func),&len);
	#endif

	if (len > 1)
	{
		if ((string = IFEELIN F_NewP(Pool, len)) != NULL)
		{
			#if defined(__AROS__)
			STRPTR stringptr = string;
			IEXEC RawDoFmt(Fmt,Params,(VOID_FUNC)AROS_ASMSYMNAME(put_func),&stringptr);
			#elif defined(__amigaos4__)
			IEXEC RawDoFmt(Fmt,Params, NULL, string);
			#else
			IEXEC RawDoFmt(Fmt,Params,(void *)(&put_func),string);
			#endif
		}
	}

	if (LenPtr)
	{
		*LenPtr = len - 1;
	}

	return string;
}
//+
///f_str_fmtA
F_LIB_STR_FMT
{
	if (Buffer && Fmt)
	{
		#ifdef F_ENABLE_SAFE_STRING

		if (((int32)(Buffer) < 0xFFFF) || (IEXEC TypeOfMem(Buffer) == 0) ||
			((int32)(Fmt) < 0xFFFF) || (IEXEC TypeOfMem(Fmt) == 0))

		{
			IFEELIN F_Log(0,"(db)F_StrFmt() invalid address: Buffer (0x%08lx) Fmt (0x%08lx)",Buffer,Fmt);

			return 0;
		}

		#endif

		#if defined(__AROS__)
		STRPTR stringptr = Buffer;
		 
		IEXEC RawDoFmt(Fmt,Params,(VOID_FUNC)AROS_ASMSYMNAME(put_func),&stringptr);

		#elif defined(__amigaos4__)
		
		IEXEC RawDoFmt(Fmt,Params,NULL,Buffer);
 
		#else
		
		IEXEC RawDoFmt(Fmt,Params,(void *)(&put_func),Buffer);
		
		#endif
	
		Buffer += IFEELIN F_StrLen(Buffer);
	}
	return Buffer;
}
//+
///f_str_len
F_LIB_STR_LEN
{
	if (Str)
	{
		STRPTR start = Str;
		
		#ifdef F_ENABLE_SAFE_STRING
		
		if (((int32)(Str) < 0xFFFF) || (IEXEC TypeOfMem(Str) == 0))
		{
			IFEELIN F_Log(0,"(db)F_StrLen() invalid address: Str (0x%08lx)",Str);
			
			return 0;
		}
		
		#endif 

		while (*Str++);

		return ((uint32)(Str) - (uint32)(start) - 1);
	}
	return 0;
}
//+
///f_str_cmp
F_LIB_STR_CMP
{
	uint8 c1,c2;

	if (!Length)
	{
		return 0;
	}
	else if (!Str1)
	{
		return -1;
	}
	else if (!Str2)
	{
		return 1;
	}

	#ifdef F_ENABLE_SAFE_STRING
	
	if (((int32)(Str1) < 0xFFFF) || (IEXEC TypeOfMem(Str1) == 0) || ((uint32)(Str1) == FV_Notify_Always) ||
		((int32)(Str2) < 0xFFFF) || (IEXEC TypeOfMem(Str2) == 0) || ((uint32)(Str2) == FV_Notify_Always))
	{
		IFEELIN F_Log(0,"(db)F_StrCmp() invalid address: Str1 (0x%08lx) Str2 (0x%08lx)",Str1,Str2);
		
		return -1;
	}

	#endif

	#if 0

	IFEELIN F_Log(0,"F_StrCmp() [%16.16s] [%16.16s] (%ld)", Str1, Str2, Length);

	#endif
 
	do
	{
		c1 = *Str1++;
		c2 = *Str2++;
	}
	while (c1 == c2 && c1 && --Length);

	return (int32)(c1) - (int32)(c2);
}
//+

/************************************************************************************************
*** API - logging *******************************************************************************
************************************************************************************************/

///f_alerta
F_LIB_ALERT
{
	struct EasyStruct ez;

	ez.es_StructSize   = sizeof (struct EasyStruct);
	ez.es_Flags        = 0;
	ez.es_Title        = Title;
	ez.es_TextFormat   = Body;
	ez.es_GadgetFormat = "Damned";

	return IINTUITION EasyRequestArgs(NULL,&ez,NULL,Params);
}
//+
///f_logA
F_LIB_LOG
{
	FClass *cl = FeelinBase->debug_classdo_class;
	FObject Obj = FeelinBase->debug_classdo_object;
	uint32 Method = FeelinBase->debug_classdo_method;
	STRPTR str;

	if (Level > FeelinBase->debug_log_level)
	{
		return;
	}

/*

GOFROMIEL: il y avait marqué à supprimer... alors c'est ce que j'ai fais ;-)
J'ai viré le truc de vérification du sémpahore.

*/
 
	#ifdef F_ENABLE_LOG_LOCKING
   
	IEXEC ObtainSemaphore(&FeelinBase->Henes);
	
	#endif

	/* I use AllocMem() because F_New() and F_NewP()  may  call  F_Log()  to
	complain about memory shortage. */
				
	#ifdef __amigaos4__
	str = IEXEC AllocMem(1024,MEMF_SHARED);
	#else
	str = IEXEC AllocMem(1024,0);
	#endif
	
	if (str)
	{
		struct Task *task = IEXEC FindTask(NULL);
		STRPTR buf = str;

		if (task)
		{
			*buf++ = '[';
			
			if (task->tc_Node.ln_Type == NT_PROCESS)
			{
				IDOS_ GetProgramName(buf, 256);
			}

			if (*buf == '\0')
			{
				if (task->tc_Node.ln_Name)
				{
					buf = IFEELIN F_StrFmt(buf,"%s",task->tc_Node.ln_Name);
				}
				else
				{
					buf = IFEELIN F_StrFmt(buf,"0x%08lx",task);
				}
			}
			else
			{
				while (*buf) buf++;
			}
			
			*buf++ = ']';
			*buf++ = ' ';
		}
	
		if (cl && Obj)
		{
			FClass *node;
			
			for (F_CLASSES_EACH(node))
			{
				if (node == _object_class(Obj)) break;
			}

			if (node)
			{
				buf = IFEELIN F_StrFmt(buf,"%s{%08lx}",_object_classname(Obj),Obj);
			
				if (_object_class(Obj) != cl)
				{
					for (F_CLASSES_EACH(node))
					{
						if (node == cl) break;
					}

					if (node)
					{
						buf = IFEELIN F_StrFmt(buf," @ %s",cl->Name);
					}
				}
			}
		
			if (Method)
			{
				STRPTR method_name = NULL;
 
				for (F_CLASSES_EACH(node))
				{
					if (node->Methods)
					{
						FClassMethod *me;

						for (F_METHODS_EACH(node->Methods, me))
						{
							if (_method_name(me))
							{
								if (Method == me->ID)
								{
									method_name = _method_name(me); break;
								}
							}
						}
					}
					
					if (method_name) break;
				}

				if (method_name)
				{
					buf = IFEELIN F_StrFmt(buf,".%s>",method_name);
				}
				else
				{
					buf = IFEELIN F_StrFmt(buf,".0x%08lx>",Method);
				}
			}

			*buf++ = ' ';
		}

		*buf = '\0';
		
/*** output string to selected targets *********************************************************/

		if ((task != NULL) && (task->tc_Node.ln_Type == NT_PROCESS) && (FeelinBase->Public.Console))
		{
			#ifdef F_ENABLE_SAVE_LOG

			BPTR record;
		
			if ((record = IDOS_ Open("T:feelin.log",MODE_READWRITE)))
			{
				struct DateTime dt;
				char buf_date[32];
				char buf_time[16];
				
				IDOS_ DateStamp(&dt.dat_Stamp);
				
				dt.dat_Format  = FORMAT_INT;
				dt.dat_Flags   = 0;
				dt.dat_StrDay  = NULL;
				dt.dat_StrDate = (STRPTR) &buf_date;
				dt.dat_StrTime = (STRPTR) &buf_time;
				
				IDOS_ DateToStr(&dt);

				IDOS_ Seek(record,0,OFFSET_END);
 
				IDOS_ FPrintf(record,"%s %s ", (int32) &buf_date, (int32) &buf_time);
				IDOS_ VFPrintf(record,str,NULL);
				IDOS_ VFPrintf(record,Fmt,Msg);
				IDOS_ VFPrintf(record,"\n",NULL);
				
				IDOS_ Close(record);
			}

			#endif

			IDOS_ VFPrintf(FeelinBase->Public.Console,str,NULL);
			IDOS_ VFPrintf(FeelinBase->Public.Console,Fmt,Msg);
			IDOS_ VFPrintf(FeelinBase->Public.Console,"\n",NULL);
		}

		#if 0
		else if (IFEELIN F_StrCmp("input.device",task->tc_Node.ln_Name,ALL) == 0)
		{
			STRPTR buf = IEXEC AllocMem(2048,MEMF_CLEAR);
			
			if (buf)
			{
				struct RastPort *rp = IntuitionBase->ActiveWindow->RPort;
		
				IFEELIN F_StrFmtA(buf,Fmt,Msg);
				
				IGRAPHICS SetABPenDrMd(rp,2,1,JAM2);
				IGRAPHICS Move(rp,10,10);
				IGRAPHICS Text(rp,str,IFEELIN F_StrLen(str));
				IGRAPHICS Move(rp,10,10 + rp->Font->tf_YSize);
				IGRAPHICS Text(rp,buf,IFEELIN F_StrLen(buf));
				
				IEXEC FreeMem(buf,2048);
			}
		}
		else
		{
			IFEELIN F_AlertA(str,Fmt,Msg);
		}
		#endif

		IEXEC FreeMem(str,1024);
	}

	#ifdef F_ENABLE_LOG_LOCKING
	IEXEC ReleaseSemaphore(&FeelinBase->Henes);
	#endif
}
//+

/************************************************************************************************
*** API - area support **************************************************************************
************************************************************************************************/

///f_draw
F_LIB_DRAW
{
	if (Obj)
	{
		FAreaPublic *pub = (FAreaPublic *) IFEELIN F_Get(Obj, FA_Area_PublicData);

		#undef _area_public
		#define _area_public pub->

		if (pub)
		{
			FRender *render = _area_render;

			if (!_area_is_drawable)
			{
				#ifdef F_CODE_DRAW_VERBOSE_DRAWABLE
				IFEELIN F_Log(0,"F_Draw() %s{%lx} is not drawable.", _object_classname(Obj), Obj);
				#endif

				return;
			}

			if ((FF_Draw_Damaged & Flags) != 0)
			{
				//IFEELIN F_Log(0, "F_Draw() damage set to %s{%lx}.", _object_classname(Obj), Obj);

				_area_set_damaged;
			}

			#ifdef F_CODE_DRAW_ONLYDAMAGED

			if (_area_isnt_damaged)
			{
				#ifdef F_CODE_DRAW_VERBOSE_NOTDAMAGED
				IFEELIN F_Log(0, "F_Draw() %s{%lx} is not damaged.", _object_classname(Obj), Obj);
				#endif

				return;
			}

			#endif

#ifdef F_CODE_DRAW_CHECK_RENDER
			
			if (render == NULL)
			{
				IFEELIN F_Log(0,"F_Draw() Render is NULL !!");
				
				return;
			}
		
#endif

			if (FF_Render_Forbid & render->Flags)
			{
				#ifdef F_CODE_DRAW_VERBOSE_FORBID
				IFEELIN F_Log(0,"F_Draw() rendering forbiden: %s{%lx}", _object_classname(Obj), Obj);
				#endif
				
				return;
			}

#ifdef F_CODE_DRAW_CHECK_RPORT

			if (render->RPort == NULL)
			{
				IFEELIN F_Log(0,"F_Draw() RPort is NULL !!");

				return;
			}
		
#endif

#ifdef F_CODE_DRAW_CHECK_BITMAP

			if (render->RPort->BitMap == NULL)
			{
				IFEELIN F_Log(0,"F_Draw() BitMap is NULL !!");
				
				return;
			}
		
#endif

#ifdef F_CODE_DRAW_CHECK_COORDINATES

			if ((_area_w == 0) || (_area_w >= FV_Area_Max) || (_area_h == 0) || (_area_h >= FV_Area_Max))
			{
				#ifdef F_CODE_DRAW_VERBOSE_COORDINATES
				
				IFEELIN F_Log
				(
					FV_LOG_DEV, "F_Draw() %s{%lx} coordinates error (%ld:%ld %ldx%ld) (please report)",

					_object_classname(Obj), Obj,
					_area_x, _area_y, _area_w, _area_h
				);
				
				#endif

				return;
			}
#endif

			if (_area_is_bufferize)
			{
				struct RastPort *rp = IFEELIN F_New(sizeof (struct RastPort));

				if (rp)
				{
					#ifdef F_CODE_DRAW_BUFFERIZE_MOVE
					uint16 w = _area_w;
					uint16 h = _area_h;
					#else
					uint16 w = _area_x + _area_w/* - 1*/;
					uint16 h = _area_y + _area_h/* - 1*/;
					#endif
					
					struct BitMap *bmp;
					
					IGRAPHICS InitRastPort(rp);
 
					if ((bmp = IGRAPHICS AllocBitMap(w, h, IFEELIN F_Get(render->Display,(uint32) "FA_Display_Depth"),0,render->RPort->BitMap)))
					{
						FRect r;
						uint32 clip;
						struct RastPort *old_rp = render->RPort;

						r.x1 = _area_x; r.x2 = r.x1 + _area_w - 1;
						r.y1 = _area_y; r.y2 = r.y1 + _area_h - 1;
							  
						rp->BitMap = bmp;
						render->RPort = rp;
						
						clip = IFEELIN F_Do(render, FM_Render_AddClip, &r);

						if (clip)
						{
							#ifdef F_CODE_DRAW_BUFFERIZE_MOVE
							IGRAPHICS ClipBlit(old_rp,r.x1, r.y1, rp, 0,0,_area_w,_area_h,0x0C0);
							#else
							IGRAPHICS ClipBlit(old_rp,0,0, rp, 0,0,w,h,0x0C0);
							#endif
							
							#ifdef F_CODE_DRAW_BUFFERIZE_MOVE
							IFEELIN F_Layout(Obj,0,0,w,h,0);
							#endif
							IFEELIN F_Do(Obj, FM_Area_Draw, Flags);

							#ifdef F_CODE_DRAW_BUFFERIZE_MOVE
							IGRAPHICS ClipBlit(rp,0,0,old_rp,r.x1,r.y1,w,h,0x0C0);
							IFEELIN F_Layout(Obj,r.x1,r.y1,w,h,0);
							#else
							
							IGRAPHICS ClipBlit(rp,r.x1,r.y1,old_rp,r.x1,r.y1,_area_w,_area_h,0x0C0);
							
							#endif
							
							IFEELIN F_Do(render,FM_Render_RemClip,clip);
						}

						render->RPort = old_rp;
						
						IGRAPHICS FreeBitMap(bmp);
					}

					#ifdef __AROS__
					DeinitRastPort(rp);
					#endif
					
					IFEELIN F_Dispose(rp);
				}
			}
			else
			{
				IFEELIN F_Do(Obj, FM_Area_Draw, Flags);
			}
		}
		else
		{
			IFEELIN F_Log(0,"F_Draw() %s{%lx} is not a subclass of FC_Area",_object_classname(Obj),Obj);
		}
	}
}
//+
///f_erase
F_LIB_ERASE
/*

	I created the  function  because  I  was  tired  of  creating  temporary
	structures  to  store  coordinates...  Lazyness  rarely ends with clever
	choices :-)

*/
{
	FRect r;

	r.x1 = x1;
	r.y1 = y1;
	r.x2 = x2;
	r.y2 = y2;

	IFEELIN F_Do(Obj, FM_Area_Erase, &r, Flags);
}
//+
///f_layout
F_LIB_LAYOUT
{
	if (Obj)
	{
		FAreaPublic *pub = (FAreaPublic *) IFEELIN F_Get(Obj, FA_Area_PublicData);
		
		#undef _area_public
		#define _area_public pub->

		if (pub)
		{
			FBox *box = &_area_box;

			if ((box->x != x) || (box->y != y) || (box->w != w) || (box->h != h))
			{
				box->x = x;
				box->y = y;
				box->w = w;
				box->h = h;
				
				_area_flags |= FF_Area_Damaged;
			}

			IFEELIN F_Do(Obj, FM_Area_Layout);
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV, "F_Layout() Unable to get FWidgetPublic of %s{%08lx}", _object_classname(Obj), Obj);
		}
	}
}
//+

