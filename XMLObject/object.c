#include "Private.h"

//#define DB_READ

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///XMLObject_New
F_METHOD(FObject,XMLObject_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_TAGS:
		{
			LOD->tags_initial = (struct TagItem *) item.ti_Data;
		}
		break;
	}
		
	if (F_SUPERDO())
	{
		uint32 markup = 0;
		APTR pool = NULL;
		
		IFEELIN F_Do(Obj,FM_Get,
			
			"FA_Document_Pool", &pool,
			"FA_XMLDocument_Markups", &markup,

			TAG_DONE);

		if (markup && pool)
		{
			if (IFEELIN F_Do(Obj, F_METHOD_ID(BUILD)))
			{
				/* resolve references */

				Tags = Msg;

				while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
				{
					if (item.ti_Tag == FV_ATTRIBUTE_REFERENCES)
					{
						if (item.ti_Data)
						{
							FXMLReference *ref;

							for (ref = (FXMLReference *) item.ti_Data ; ref->Name ; ref++)
							{
								if (ref->ObjectPtr)
								{
									FAtom *atom = IFEELIN F_AtomFind(ref->Name, ALL);

									if (atom)
									{
										struct in_Reference *node;

										for (node = LOD->references ; node ; node = node->next)
										{
											if (node->atom == atom)
											{
												*ref->ObjectPtr = node->object;

												break;
											}
										}
									}
								}
							}
						}
					}
				}

				return Obj;
			}
			else
			{
				IFEELIN F_Log(FV_LOG_DEV,"Building failed");
			}
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV,"Document is empty");
		}
	}

	return NULL;
}
//+
///XMLObject_Dispose
F_METHOD(uint32,XMLObject_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_Reference *node;

	for (node = LOD->references ; node ; node = node->next)
	{
		IFEELIN F_AtomRelease(node->atom);
	}

	return F_SUPERDO();
}
//+
///XMLObject_Get
F_METHOD(void,XMLObject_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;
	
	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_TAGS:
		{
			F_STORE(LOD->tags_built);
		}
		break;
	}

	F_SUPERDO();
}
//+
///XMLObject_Read
F_METHODM(uint32,XMLObject_Read,FS_Document_Read)
{
	if (Msg->Type == FV_Document_SourceType_File)
	{
		uint32 rc=NULL;
		STRPTR buf;

		if ((buf = IFEELIN F_NewP(Msg->Pool, 1024)) != NULL)
		{
			BPTR lock;
			
/** get file "as is" relative to current directory *********************************************/

			IDOS_ GetCurrentDirName(buf, 1024);
			IDOS_ AddPart(buf, Msg->Source, 1024);
			lock = IDOS_ Lock(buf,ACCESS_READ);

			#ifdef DB_READ
			IFEELIN F_Log(0,"CD: lock (0x%08lx)(%s)",lock,buf);
			#endif

/*** get file "as is" relative to program directory ********************************************/
	 
			if (!lock)
			{
				if ((lock = IDOS_ GetProgramDir()) != NULL)
				{
					IDOS_ NameFromLock(lock, buf, 1024);
					IDOS_ AddPart(buf, Msg->Source, 1024);
					lock = IDOS_ Lock(buf,ACCESS_READ);
					
					#ifdef DB_READ
					IFEELIN F_Log(0,"AS: lock (0x%08lx)(%s)",lock,buf);
					#endif
				}
			}
		
/*** get file part relative to program directory ***********************************************/
											
			if (!lock)
			{
				if ((lock = IDOS_ GetProgramDir()) != NULL)
				{
					IDOS_ NameFromLock(lock, buf, 1024);
					IDOS_ AddPart(buf, IDOS_ FilePart(Msg->Source), 1024);
					lock = IDOS_ Lock(buf,ACCESS_READ);

					#ifdef DB_READ
					IFEELIN F_Log(0,"RE: lock (0x%08lx)(%s)",lock,buf);
					#endif
				}
			}

/*** get file absolute *************************************************************************/

			if (!lock)
			{
				IFEELIN F_StrFmt(buf,F_XMLSOURCES_PATH);
				IDOS_ AddPart(buf,Msg->Source,1024);

				lock = IDOS_ Lock(buf,ACCESS_READ);
				
				#ifdef DB_READ
				IFEELIN F_Log(0,"AB: lock (0x%08lx)(%s)",lock,buf);
				#endif
			}
		
/******/

			if (lock)
			{
				rc = IFEELIN F_SuperDo
				(
					Class, Obj, Method,
					
					F_MSG_DOCUMENT_READ(buf, Msg->Type, Msg->Pool)
				);
				
				IDOS_ UnLock(lock);
			}
			else
			{
				IFEELIN F_Log(FV_LOG_DEV,"Unable to lock (%s)",Msg->Source);
			}

 
			IFEELIN F_Dispose(buf);
					
			return rc;
		}
	}
	return F_SUPERDO();
}
//+
///XMLObject_GetObjects

struct FS_XMLObject_GetObjects                  { STRPTR Name; FObject *ObjectPtr; /* ... NULL */ };

F_METHODM(uint32,XMLObject_GetObjects,FS_XMLObject_GetObjects)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 n = 0;

	for ( ; Msg->Name ; Msg++)
	{
		struct in_Reference *node = NULL;

		FAtom *atom = IFEELIN F_AtomFind(Msg->Name, ALL);

		if (atom)
		{
			for (node = LOD->references ; node ; node = node->next)
			{
				if (node->atom == atom)
				{
					break;
				}
			}
		}

		if (node != NULL)
		{
			if (Msg->ObjectPtr)
			{
				*Msg->ObjectPtr = node->object;

				n++;
			}
		}
		else
		{
			if (Msg->ObjectPtr)
			{
				*Msg->ObjectPtr = NULL;
			}

			IFEELIN F_Log(FV_LOG_DEV,"Object '%s' is not referenced", Msg->Name);
		}
	}

	return n;
}
//+
///XMLObject_Build
F_METHOD(int32,XMLObject_Build)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	APTR pool=NULL;
	FXMLMarkup *markup=NULL;
	
	IFEELIN F_Do(Obj,FM_Get,
		
		"FA_Document_Pool",         &pool,
		"FA_XMLDocument_Markups",   &markup,
		
		TAG_DONE);

/*** searching 'objects' markup ****************************************************************/

	for ( ; markup ; markup = markup->Next)
	{
		if (markup->Atom == F_ATOM(OBJECTS))
		{
			break;
		}
	}

	if (markup)
	{
		struct TagItem *tags;
		FXMLMarkup *m;
		uint32 n=0;
		
		for (m = (FXMLMarkup *)(markup->ChildrenList.Head) ; m ; m = m->Next)
		{
			n++;
		}
	
		tags = IFEELIN F_NewP(pool,sizeof (struct TagItem) * (n + 1));

		if (tags)
		{
			struct TagItem *item = tags;
			
			for (m = (FXMLMarkup *)(markup->ChildrenList.Head) ; m ; m = m->Next)
			{
				item->ti_Tag = FA_Child;
				item->ti_Data = IFEELIN F_Do(Obj, F_METHOD_ID(CREATE), m, pool);

				if (!item->ti_Data)
				{
					for (item = tags ; item->ti_Tag ; item++)
					{
						IFEELIN F_DisposeObj((FObject)(item->ti_Data));
					}
				
					return FALSE;
				}
				item++;
			}
		
			if (LOD->tags_initial)
			{
				item->ti_Tag = TAG_MORE;
				item->ti_Data = (uint32)(LOD->tags_initial);
			}
			else
			{
				item->ti_Tag = TAG_DONE;
				item->ti_Data = 0;
			}
		
			LOD->tags_built = tags;

			return TRUE;
		}
	}
	else
	{
		IFEELIN F_Do(Obj,F_RESOLVED_ID(LOG),0,NULL,"Unable to locate markup 'feelin:objects'");
	}
	
	return FALSE;
}
//+
///XMLObject_Resolve
F_METHODM(uint32,XMLObject_Resolve,FS_Document_Resolve)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 value = 0;
	int32 done=FALSE;

	if (Msg->Data)
	{

/*** check objects *****************************************************************************/
 
		if (!done && (FV_TYPE_OBJECT == Msg->Type))
		{
			FAtom *atom = IFEELIN F_AtomFind(Msg->Data, ALL);

			if (atom)
			{
				struct in_Reference *node;

				for (node = LOD->references ; node ; node = node->next)
				{
					if (node->atom == atom)
					{
						value = (uint32) node->object;
					}
				}
			}

			if (value != 0)
			{
				done = FV_TYPE_OBJECT;
			}
			else
			{
				IFEELIN F_Log(FV_LOG_DEV, "object (%s) not found", Msg->Data);
			}
		}

/*** pass to XMLDocument ***********************************************************************/
  
		if (!done)
		{
			value = IFEELIN F_SuperDo
			(
				Class,Obj,Method,
				
				F_MSG_DOCUMENT_RESOLVE(Msg->Data,Msg->Type,Msg->Values,&done)
			);
		}
	}

	if (Msg->Done)
	{
		*Msg->Done = done;
	}

	return value;
}
//+
