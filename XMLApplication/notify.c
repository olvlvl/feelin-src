#include "Private.h"

/*

	How to build notification message ?

	example: "message='FA_

	Names begining with upper case are taken as attributes relative  to  the
	target  object.  If  the attribute doesn't exists, I try it as a method.
	The best way to avoid confusion is to provide full length attributes and
	methods e.g. FA_Numeric_Value or FM_Numeric_Increase.


*/

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///f_separe_words
STRPTR * f_separe_words(STRPTR Source,APTR Pool,uint32 *Words)
{
	if (Words)
	{
		*Words = 0;
	}

	if (Source)
	{
		STRPTR buf = Source;
		uint32 n=0;
		uint32 w=0;

		while (*buf++)
		{
			if (*buf == ',') n++;
		}

		if (n)
		{
			STRPTR *array = IFEELIN F_NewP(Pool,sizeof (STRPTR *) * (n + 3));

			if (array)
			{
				STRPTR *pos = array;

				buf = Source;

				while (*buf)
				{
					STRPTR item_s = buf;

					if (*buf == ',')
					{
						IFEELIN F_Log(0,"ERROR MISSING PARAMETER: (%s)",buf); break;
					}

					while (*buf && *buf != ',') buf++;

					if ((*pos = IFEELIN F_NewP(Pool,buf - item_s + 1)) != NULL)
					{
						IEXEC CopyMem(item_s,*pos,buf - item_s);

						pos++; w++;
					}
					else break;

					if (!*buf)
					{
						break;
					}
					else if (*buf == ',')
					{
						buf++;
					}
					else
					{
						IFEELIN F_Log(0,"MISSING COMA: (%s)",buf); break;
					}
				}

				if (!*buf)
				{
					if (Words)
					{
						*Words = w;
					}

					return array;
				}
			}
		}
	}
	return NULL;
}
//+
///f_built_message
int32 f_built_message(FClass *Class,FObject Obj,uint32 *Msg,STRPTR *Strings,FObject Target)
{
	FClassAttribute *ca=NULL;

	for ( ; *Strings ; Strings++, Msg++)
	{
		STRPTR str = *Strings;

		if (*str >= 'a' && *str <= 'z')
		{
			if (IFEELIN F_StrCmp(str,"value",ALL) == 0)
			{
				*Msg = FV_Notify_Value;
			}
			else if (IFEELIN F_StrCmp(str,"toggle",ALL) == 0)
			{
				*Msg = FV_Notify_Toggle;
			}
			else
			{
				int32 ok=FALSE;
				uint32 rc;
				
				rc = IFEELIN F_Do(Obj, F_RESOLVED_ID(RESOLVE), str,ca->Type,ca->Values,&ok);
				
				if (ok)
				{
					*Msg = rc;
				}
			}
		}
		else if (*str >= 'A' && *str <= 'Z')
		{
			FClassMethod *cm;

			if ((ca = IFEELIN F_DynamicFindAttribute(str,_object_class(Target),NULL)) != NULL)
			{
				*Msg = ca->ID;
			}
			else if ((cm = IFEELIN F_DynamicFindMethod(str,_object_class(Target),NULL)) != NULL)
			{
				*Msg = cm->ID;
			}
			else
			{
				IFEELIN F_Log(0,"Unable to resolve method or attribute (%s)",str);

				return FALSE;
			}
		}
		else
		{
			IFEELIN F_Log(0,"check ERROR (%s)",str); return FALSE;
		}

		if (!*Msg)
		{
			IFEELIN F_Log(0,"Unable to resolve message item (%s)",str);

			return FALSE;
		}
	}
	return TRUE;
}
//+

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///XMLApplication_CreateNotify
F_METHODM(LONG,XMLApplication_CreateNotify,FS_XMLApplication_CreateNotify)
{
	FXMLMarkup *markup;

	for (markup = (FXMLMarkup *)(Msg->Markup->ChildrenList.Head) ; markup ; markup = markup->Next)
	{
		FXMLAttribute *attribute;

		STRPTR data_obj=markup->Atom->Key;
		STRPTR data_attribute=NULL;
		STRPTR data_value=NULL;
		STRPTR data_target=NULL;
		STRPTR data_method=NULL;
		STRPTR data_message=NULL;

		FObject notify_obj=NULL;

		for (attribute = (FXMLAttribute *)(markup->AttributesList.Head) ; attribute ; attribute = attribute->Next)
		{
			if (attribute->Atom == F_ATOM(ATTRIBUTE))
			{
				data_attribute = attribute->Value;
			}
			else if (attribute->Atom == F_ATOM(VALUE))
			{
				data_value = attribute->Value;
			}
			else if (attribute->Atom == F_ATOM(TARGET))
			{
				data_target = attribute->Value;
			}
			else if (attribute->Atom == F_ATOM(METHOD))
			{
				data_method = attribute->Value;
			}
			else if (attribute->Atom == F_ATOM(MESSAGE))
			{
				data_message = attribute->Value;
			}
		}

		if (!data_attribute) { IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG),attribute->Line,NULL,"Attribute not defined for object '%s'",data_obj); continue; }
		if (!data_value)     { IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG),attribute->Line,NULL,"Value not defined for attribute '%s'",data_attribute); continue; }
		if (!data_target)    { IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG),attribute->Line,NULL,"Target not defined"); continue; }
		if (!data_method)    { IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG),attribute->Line,NULL,"Method not defined for target '%s'",data_target); continue; }

		#ifdef F_NEW_GETELEMENTBYID

		notify_obj = (FObject) IFEELIN F_Do(Obj, FM_GetElementById, data_obj);

		#else

		IFEELIN F_Do
		(
			Obj, F_RESOLVED_ID(GETOBJECTS),

			data_obj, &notify_obj,

			NULL
		);

		#endif

		if (notify_obj)
		{
			FObject notify_target;

			if (IFEELIN F_StrCmp("this", data_target, ALL) == 0)
			{
				notify_target = (FObject) notify_obj;
			}
			else if (IFEELIN F_StrCmp("parent", data_target, ALL) == 0)
			{
				notify_target = (FObject) IFEELIN F_Get(notify_obj, FA_Element_Parent);
			}
			else if (IFEELIN F_StrCmp("window", data_target, ALL) == 0)
			{
				#ifdef F_NEW_GLOBALCONNECT
				notify_target = (FObject) IFEELIN F_Get(notify_obj, FA_Element_Window);
				#else
				notify_target = (FObject) IFEELIN F_Get(notify_obj, FA_Window);
				#endif
			}
			else if (IFEELIN F_StrCmp("application", data_target, ALL) == 0)
			{
				#ifdef F_NEW_GLOBALCONNECT
				notify_target = (FObject) IFEELIN F_Get(notify_obj, FA_Element_Application);
				#else
				notify_target = (FObject) IFEELIN F_Get(notify_obj, FA_Application);
				#endif
			}
			else
			{
				#ifdef F_NEW_GETELEMENTBYID

				notify_target = (FObject) IFEELIN F_Do(Obj, FM_GetElementById, data_target);

				#else

				IFEELIN F_Do
				(
					Obj, F_RESOLVED_ID(GETOBJECTS),

					data_target, &notify_target,

					NULL
				);

				#endif
			}

			if (notify_target)
			{
				struct FS_Notify *notify;
				uint32 notify_nargs=0;

				if (data_message)
				{
					STRPTR *strings = f_separe_words(data_message,Msg->Pool,&notify_nargs);

					if (strings)
					{
						if ((notify = IFEELIN F_NewP(Msg->Pool,sizeof (struct FS_Notify) + (sizeof (uint32) * (notify_nargs + 1)))) != NULL)
						{
							if (!f_built_message(Class,Obj,(uint32 *)((uint32)(notify) + sizeof (struct FS_Notify)),strings,notify_target))
							{
								IFEELIN F_Do(Obj,F_RESOLVED_ID(LOG),markup->Line,NULL,"Unable to built message (%s)",data_message);

								return FALSE;
							}
						}
					}
					else return FALSE;
				}
				else
				{
					if (!(notify = IFEELIN F_NewP(Msg->Pool,sizeof (struct FS_Notify) + sizeof (uint32))))
					{
						IFEELIN F_Do(Obj,F_RESOLVED_ID(LOG),markup->Line,NULL,"Out of memory"); return FALSE;
					}
				}

				if (notify)
				{
					FClassAttribute *notify_attribute = IFEELIN F_DynamicFindAttribute(data_attribute,_object_class(notify_obj),NULL);

					if (notify_attribute)
					{
						int32 notify_value_ok = FALSE;

						if (IFEELIN F_StrCmp("always",data_value,ALL) == 0)
						{
							notify->Value = FV_Notify_Always; notify_value_ok = TRUE;
						}
						else
						{
							notify->Value = IFEELIN F_Do(Obj,F_RESOLVED_ID(RESOLVE),data_value,notify_attribute->Type,notify_attribute->Values,&notify_value_ok);
						}

						if (notify_value_ok)
						{
							FClassMethod *notify_method = IFEELIN F_DynamicFindMethod(data_method,_object_class(notify_target),NULL);

							if (notify_method)
							{
								notify->Attribute  = notify_attribute->ID;
								notify->Target     = notify_target;
								notify->Method     = notify_method->ID;
								notify->Count      = notify_nargs;

								IFEELIN F_DoA(notify_obj,FM_Notify,notify);
							}
						}
						else return FALSE;
					}
					else
					{
						IFEELIN F_Do(Obj,F_RESOLVED_ID(LOG),markup->Line,NULL,"Unable to resolve attribute (%s) for object (%s)",data_attribute,data_obj);
					}
				}
			}
			else
			{
				IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG), markup->Line, NULL, "Unable to resolve target (%s)", data_target);

				return FALSE;
			}
		}
		else return FALSE;
	}
	return TRUE;
}
//+

