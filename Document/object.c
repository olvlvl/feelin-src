#include "Private.h"

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Document_New
F_METHOD(uint32,Document_New)
{
	struct TagItem *Tags = Msg, *real, item;

	APTR source = NULL;
	uint32 type = 0;
			
	while ((real = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_SOURCE:
		{
			source = (APTR)(item.ti_Data);

			real->ti_Tag = TAG_IGNORE;
		}
		break;
		
		case FV_ATTRIBUTE_SOURCETYPE:
		{
			type = item.ti_Data;

			real->ti_Tag = TAG_IGNORE;
		}
		break;
	}

	if ((source != NULL) && (type != 0))
	{
		if (IFEELIN F_Do(Obj, F_METHOD_ID(MERGE), F_MSG_DOCUMENT_MERGE(source, type)) == FALSE)
		{
			return 0;
		}
	}

	return F_SUPERDO();
}
//+
///Document_Dispose
F_METHOD(void,Document_Dispose)
{
	IFEELIN F_Do(Obj,F_METHOD_ID(CLEAR));
							
	F_SUPERDO();
}
//+
///Document_Get
F_METHOD(void,Document_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_VERSION:
		{
			F_STORE(LOD->version);
		}
		break;
		
		case FV_ATTRIBUTE_REVISION:
		{
			F_STORE(LOD->revision);
		}
		break;
		
		case FV_ATTRIBUTE_POOL:
		{
			F_STORE(LOD->pool);
		}
		break;

		case FV_ATTRIBUTE_SOURCE:
		{
			F_STORE(LOD->sourcename);
		}
		break;
	}

	F_SUPERDO();
}
//+
///Document_Set
F_METHOD(void,Document_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;
	
	uint32 type=0;
	APTR source=NULL;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_VERSION:     LOD->version = item.ti_Data; break;
		case FV_ATTRIBUTE_REVISION:    LOD->revision = item.ti_Data; break;
		case FV_ATTRIBUTE_SOURCE:      source = (APTR)(item.ti_Data); break;
		case FV_ATTRIBUTE_SOURCETYPE:  type = item.ti_Data; break;
	}

	F_SUPERDO();
	
	if (source && type)
	{
		IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);
		
		IFEELIN F_Do(Obj,F_METHOD_ID(CLEAR));
			
		if (!IFEELIN F_Do(Obj, F_METHOD_ID(MERGE), F_MSG_DOCUMENT_MERGE(source, type)))
		{
			IFEELIN F_Do(Obj,F_METHOD_ID(CLEAR));
		}

		IFEELIN F_Do(Obj, FM_Unlock);
	}
}
//+

///Document_Read
F_METHODM(STRPTR,Document_Read,FS_Document_Read)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	if (LOD->pool)
	{
		STRPTR buf=NULL;
	 
		if (Msg->Type == FV_Document_SourceType_File)
		{
			BPTR lock;

			LOD->sourcename = IFEELIN F_StrNewP(LOD->pool,NULL,"%s",Msg->Source);
			
			if ((lock = IDOS_ Lock(Msg->Source, ACCESS_READ)) != NULL)
			{
				BPTR file;
	 
				if ((file = IDOS_ Open(Msg->Source, MODE_OLDFILE)) != NULL)
				{
					struct FileInfoBlock *fib;

					if ((fib = IDOS_ AllocDosObject(DOS_FIB, TAG_DONE)) != NULL)
					{
						if (IDOS_ Examine(lock,fib))
						{
							if ((buf = IFEELIN F_NewP(LOD->pool,fib->fib_Size + 1)) != NULL)
							{
								if (!IDOS_ Read(file,buf,fib->fib_Size))
								{
									buf = NULL;
								}
							}
						}
						IDOS_ FreeDosObject(DOS_FIB,fib);
					}
					else
					{
						IFEELIN F_Log(FV_LOG_CORE,"AllocDosObject() failed");
					}
					IDOS_ Close(file);
				}
				else
				{
					IFEELIN F_Log(FV_LOG_USER,"Unable to open file \"%s\"",Msg->Source);
				}
				IDOS_ UnLock(lock);
			}
			#if 0
			else
			{
				IFEELIN F_Log(FV_LOG_USER,"Unable to lock file \"%s\"",Msg->Source);
			}
			#endif
		}
		else if (Msg->Type == FV_Document_SourceType_Memory)
		{
			buf = IFEELIN F_StrNewP(LOD->pool, NULL, Msg->Source);
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV,"Unknown source type (%ld,0x%08lx)",Msg->Type,Msg->Source);
		}

		return buf;
	}
	return NULL;
}
//+
///Document_Merge
F_METHODM(uint32,Document_Merge,FS_Document_Merge)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
 
	uint32 rc = FALSE;

	if (Msg->Source && Msg->Type)
	{
		IFEELIN F_Do(Obj, FM_Lock, FF_Lock_Exclusive);

		if (IFEELIN F_Do(Obj, F_METHOD_ID(CREATEPOOL)))
		{
			STRPTR source = (STRPTR) IFEELIN F_Do(Obj,F_METHOD_ID(READ), F_MSG_DOCUMENT_READ(Msg->Source, Msg->Type, LOD->pool));
			
			if (source != NULL)
			{
				rc = IFEELIN F_Do(Obj, F_METHOD_ID(PARSE), source, LOD->pool);

				IFEELIN F_Dispose(source);
			}
		}

		IFEELIN F_Do(Obj, FM_Unlock);
	}
 
	return rc;
}
//+

///Document_Log
F_METHODM(void,Document_Log,FS_Document_Log)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	STRPTR fst=NULL;
	STRPTR str;

	if (LOD->sourcename)
	{
		fst = IFEELIN F_StrNew(NULL,"Error in '%s' ",LOD->sourcename);
	}

	str = IFEELIN F_StrNewA(NULL,Msg->Fmt,(APTR)((uint32)(Msg) + sizeof (struct FS_Document_Log)));

	if (Msg->Source)
	{
		IFEELIN F_Log(0,"\n\n%s%04ld: %s\n\n%32.32s\n",(fst) ? (fst) : (STRPTR)(""),Msg->Line,str,Msg->Source);
	}
	else
	{
		IFEELIN F_Log(0,"\n\n%s%04ld: %s\n",(fst) ? (fst) : (STRPTR)(""),Msg->Line,str);
	}
	
	IFEELIN F_Dispose(fst);
	IFEELIN F_Dispose(str);
}
//+
///Document_Resolve
F_METHODM(uint32,Document_Resolve,FS_Document_Resolve)
{
	uint32 value=0;
	bits32 done=0;

	#ifdef DB_RESOLVE
	IFEELIN F_Log(0,"attribute (%s)(0x%08lx)(%02ld) - values (0x%08lx) - data (%s)(0x%08lx)",Attribute->Name,Attribute,Attribute->Type,Attribute->Values,Data,Data);
	#endif

	if (Msg->Data)
	{

/*** check values ******************************************************************************/

		if (Msg->Values)
		{
			FDOCValue *val;

			for (val = Msg->Values ; val->Name ; val++)
			{
				if (IFEELIN F_StrCmp(Msg->Data,val->Name,ALL) == 0) break;
			}

			if (val->Name)
			{
				value = val->Value; done = 0xFFFFFFFF;
			}
		}

/*** check boolean *****************************************************************************/

		if (!done && (FV_TYPE_BOOLEAN & Msg->Type))
		{
			done = FV_TYPE_BOOLEAN;

			if (IFEELIN F_StrCmp("true",Msg->Data,ALL) == 0)         value = TRUE;
			else if (IFEELIN F_StrCmp("yes",Msg->Data,ALL) == 0)     value = TRUE;
			else if (IFEELIN F_StrCmp("false",Msg->Data,ALL) == 0)   value = FALSE;
			else if (IFEELIN F_StrCmp("no",Msg->Data,ALL) == 0)      value = FALSE;
			else
			{
				done = FV_TYPE_BOOLEAN;
			}
		}

/*** check integer *****************************************************************************/

		if (!done)
		{
			if (*Msg->Data == '#')
			{
				if (f_stch(Msg->Data + 1, (int32 *) &value))
				{
					done = FV_TYPE_HEXADECIMAL;
				}
			}

			#if F_CODE_DEPRECATED

			else if (*Msg->Data == '!')
			{
				if (IFEELIN F_StrLen(Msg->Data + 1) == 4)
				{
					value = *((uint32 *)(Msg->Data + 1)); done = FV_TYPE_4CC;
				}
			}

			#endif

			else if (*Msg->Data == '%')
			{
				if (IFEELIN F_StrLen(Msg->Data + 1) < 32)
				{
					STRPTR bit;

					for (bit = Msg->Data + 1; *bit ; bit++)
					{
						if (*bit == '1')
						{
							value = value << 1;
							value |= 1;
						}
						else if (*bit == '0')
						{
							value = value << 1;
						}
						else
						{
							IFEELIN F_Do(Obj,F_METHOD_ID(LOG),0,NULL,"Binary error (%s)",Msg->Data);
						}
					}

					if (!*bit)
					{
						done = FV_TYPE_BINARY;
					}
				}
				else
				{
					IFEELIN F_Do(Obj,F_METHOD_ID(LOG),0,NULL,"Binary too long (%s)",Msg->Data);
				}
			}
			else
			{
				uint32 len = f_stcd(Msg->Data, (int32 *) &value);
 
				if (len)
				{
					if (Msg->Data[len] == '%')
					{
						done = FV_TYPE_PERCENTAGE;
					}
					else if (Msg->Data[len] == 'p' && Msg->Data[len+1] == 'x')
					{
						done = FV_TYPE_PIXEL;
					}
					else
					{
						done = FV_TYPE_INTEGER;
					}
				}
			}
		}

/*** check string ******************************************************************************/

		if (!done && (FV_TYPE_STRING & Msg->Type))
		{
			done = FV_TYPE_STRING; value = (uint32)(Msg->Data);
		}
	}

	if (Msg->Done)
	{
		*Msg->Done = done;
	}

	return value;
}
//+
#if F_CODE_DEPRECATED
///Document_FindName
F_METHODM(FAtom *,Document_FindName,FS_Document_FindName)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->names)
	{
		FAtom *atom = IFEELIN F_AtomFind(Msg->Key, Msg->KeyLength);

		if (atom)
		{
			struct in_Name *name;

			for (name = LOD->names ; name ; name = name->next)
			{
				if (name->atom == atom)
				{
					return name->atom;
				}
			}
		}
	}

	return NULL;
}
//+
///Document_ObtainName
F_METHODM(FAtom *,Document_ObtainName,FS_Document_ObtainName)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FAtom *atom = IFEELIN F_AtomObtain(Msg->Key, Msg->KeyLength);

	IFEELIN F_Log(0,"disabled [%-16.16s]", Msg->Key);

	if (atom)
	{
		struct in_Name *name;

		for (name = LOD->names ; name ; name = name->next)
		{
			if (name->atom == atom)
			{
				break;
			}
		}

		if (name)
		{
			IFEELIN F_AtomRelease(atom);
		}
		else
		{
			name = IFEELIN F_NewP(LOD->pool, sizeof (struct in_Name));

			if (name)
			{
				name->atom = atom;
				name->next = LOD->names;

				LOD->names = name;
			}
			else
			{
				IFEELIN F_AtomRelease(atom);
			}
		}

		if (name)
		{
			return name->atom;
		}
	}

	return NULL;
}
//+
#endif
///Document_Clear
F_METHOD(void,Document_Clear)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_Name *name;

	for (name = LOD->names ; name ; name = name->next)
	{
		IFEELIN F_AtomRelease(name->atom);
	}

	LOD->names = NULL;
	LOD->sourcename = NULL;
	
	IFEELIN F_DeletePool(LOD->pool); LOD->pool = NULL;
}
//+

///Document_Numerify
F_METHODM(uint32,Document_Numerify,FS_Document_Numerify)
{
	uint32 value = 0;
	bits32 type = 0;

	uint32 len = IFEELIN F_StrLen(Msg->Data);

	if (len)
	{
		switch (*Msg->Data)
		{
			/* hexadecimal */

			case '#':
			{
				if (len < 9)
				{
					if (f_stch(Msg->Data + 1, (int32 *) &value))
					{
						type = FV_TYPE_HEXADECIMAL;
					}
				}
			}
			break;

			#if F_CODE_DEPRECATED

			/* 4CC */

			case '!':
			{
				if (len == 5)
				{
					value = *((uint32 *)(Msg->Data + 1));
					type = FV_TYPE_4CC;
				}
			}
			break;

			#endif

			/* binary */

			case '%':
			{
				if (len < 33)
				{
					STRPTR bit;

					for (bit = Msg->Data + 1; *bit ; bit++)
					{
						if (*bit == '1')
						{
							value = value << 1;
							value |= 1;
						}
						else if (*bit == '0')
						{
							value = value << 1;
						}
						#if 0
						else
						{
							IFEELIN F_Do(Obj,F_METHOD_ID(LOG),LOD->line,NULL,"Binary error (%s)",Msg->Data);
						}
						#endif
					}

					if (!*bit)
					{
						value = value;
						type = FV_TYPE_BINARY;
					}
				}
				#if 0
				else
				{
					IFEELIN F_Do(Obj,F_METHOD_ID(LOG),LOD->line,NULL,"Binary too long (%s)",Msg->Data);
				}
				#endif
			}
			break;

			/* boolean: TRUE */

			case 't':
			{
				if ((len == 4) && (IFEELIN F_StrCmp("true",Msg->Data,ALL) == 0))
				{
					value = TRUE;
					type = FV_TYPE_BOOLEAN;
				}
			}
			break;

			case 'y':
			{
				if ((len == 3) && (IFEELIN F_StrCmp("yes",Msg->Data,ALL) == 0))
				{
					value = TRUE;
					type = FV_TYPE_BOOLEAN;
				}
			}
			break;

			/* boolean: FALSE */

			case 'f':
			{
				if ((len == 5) && (IFEELIN F_StrCmp("false",Msg->Data,ALL) == 0))
				{
					value = FALSE;
					type = FV_TYPE_BOOLEAN;
				}
			}
			break;

			case 'n':
			{
				if ((len == 2) && (IFEELIN F_StrCmp("no",Msg->Data,ALL) == 0))
				{
					value = FALSE;
					type = FV_TYPE_BOOLEAN;
				}
			}
			break;

			/* decimal */

			default:
			{
				if (len < 24)
				{
					if ((Msg->Data[0] == '0') && (Msg->Data[1] == 'x'))
					{
						uint32 len = f_stch(Msg->Data + 2, (int32 *) &value);

						if (len == 8)
						{
							type = FV_TYPE_POINTER;
						}
					}
					else
					{
						uint32 len = f_stcd(Msg->Data, (int32 *) &value);

						if (len)
						{
							if (Msg->Data[len] == '%')
							{
								type = FV_TYPE_PERCENTAGE;
							}
							else if ((Msg->Data[len] == 'p') && (Msg->Data[len+1] == 'x'))
							{
								type = FV_TYPE_PIXEL;
							}
							else
							{
								type = FV_TYPE_INTEGER;
							}
						}
					}
				}
			}
			break;
		}
	}

	#ifdef DB_NUMERIFY
	IFEELIN F_Log(0, "date (%s) value (0x%08lx)", Msg->Data, value);
	#endif

	if (Msg->TypePtr)
	{
		*Msg->TypePtr = type;
	}

	return type ? value : 0;
}
//+
///Document_Stringify
F_METHODM(STRPTR, Document_Stringify, FS_Document_Stringify)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	STRPTR value = NULL;

	if (LOD->pool == NULL)
	{
		IFEELIN F_Do(Obj, F_METHOD_ID(CREATEPOOL));
	}

	switch (Msg->Type)
	{
		case FV_TYPE_BOOLEAN:
		{
			IFEELIN F_Log(0,"boolean for (%ld)", Msg->Data);

			if (Msg->Data != 0)
			{
				value = IFEELIN F_StrNewP(LOD->pool, NULL, "true");
			}
			else
			{
				value = IFEELIN F_StrNewP(LOD->pool, NULL, "false");
			}
		}
		break;

		case FV_TYPE_INTEGER:
		{
			value = IFEELIN F_StrNewP(LOD->pool, NULL, "%ld", Msg->Data);
		}
		break;

		case FV_TYPE_HEXADECIMAL:
		{
			value = IFEELIN F_StrNewP(LOD->pool, NULL, "#%08lx", Msg->Data);
		}
		break;

		#if F_CODE_DEPRECATED

		case FV_TYPE_4CC:
		{
			value = IFEELIN F_NewP(LOD->pool, 5);

			if (value)
			{
				*((uint32 *)(value)) = Msg->Data;
			}
		}
		break;

		#endif

		case FV_TYPE_PERCENTAGE:
		{
			value = IFEELIN F_StrNewP(LOD->pool, NULL, "%ld%%", Msg->Data);
		}
		break;

		case FV_TYPE_PIXEL:
		{
			value = IFEELIN F_StrNewP(LOD->pool, NULL, "%ldpx", Msg->Data);
		}
		break;

		case FV_TYPE_DEGREES:
		{
			value = IFEELIN F_StrNewP(LOD->pool, NULL, "%ld°", Msg->Data);
		}
		break;

		case FV_TYPE_POINTER:
		{
			value = IFEELIN F_StrNewP(LOD->pool, NULL, "0x%08lx", Msg->Data);
		}
		break;
	}

	return value;
}
//+
///Document_CreatePool
F_METHOD(APTR,Document_CreatePool)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	if (LOD->pool == NULL)
	{
		LOD->pool = IFEELIN F_CreatePool(2048,FA_Pool_Items,1,FA_Pool_Name,"document-data",TAG_DONE);
	}

	return LOD->pool;
}
//+

///Document_Dummy
F_METHOD(uint32,Document_Dummy)
{
	return 0;
}
//+

