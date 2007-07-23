#include "Private.h"

//#define DB_ATTRIBUTE
//#define DB_WRITE
//#define DB_CREATE
//#define DB_PARSE

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///pdr_create_attribute
STATIC FPDRAttribute * pdr_create_attribute(APTR Pool, STRPTR Key, uint32 KeyLength)
{
	FAtom *atom = IFEELIN F_AtomObtain(Key, KeyLength);

	if (atom)
	{
		FPDRAttribute *node = IFEELIN F_NewP(Pool, sizeof (FPDRAttribute));

		if (node)
		{
			node->Atom = atom;

			#ifdef DB_ATTRIBUTE
			IFEELIN F_Log(0,"attribute (0x%08lx) key (0x%08lx)(%s)", node, node->Atom, node->Atom->Key);
			#endif

			return node;
		}

		IFEELIN F_AtomRelease(atom);
	}

	return NULL;
}
//+
///pdr_delete_attribute
STATIC void pdr_delete_attribute(FPDRAttribute *Attribute)
{
	if (Attribute)
	{
		IFEELIN F_AtomRelease(Attribute->Atom);
	}

	IFEELIN F_Dispose(Attribute);
}
//+

///pdr_explode
STATIC FPDRAttribute * pdr_explode(APTR Pool, STRPTR String)
{
	FPDRAttribute *index = NULL;

	STRPTR copy = IFEELIN F_StrNewP(Pool, NULL, "%s", String);

	#ifdef DB_EXPLODE
	IFEELIN F_Log(0,"explode: copy (%s)", copy);
	#endif

	if (copy)
	{
		STRPTR s = copy;
		uint8 c;

		while ((c = *s) != '\0')
		{
			if (c == ' ')
			{
				s++;
			}
			else
			{
				FPDRAttribute *node;

				STRPTR start = s++;

				while ((c = *s) != '\0')
				{
					if (c != ' ')
					{
						s++;
					}
					else break;
				}

				node = pdr_create_attribute(Pool, start, s - start);

				if (node)
				{
					node->Next = index;
					index = node;
				}
				else // error
				{
					while (index)
					{
						index = node->Next;

						pdr_delete_attribute(node);
					}

					IFEELIN F_Dispose(copy);

					return NULL;
				}
			}
		}

		IFEELIN F_Dispose(copy);
	}

	#ifdef DB_EXPLODE
	IFEELIN F_Log(0,"explode: INDEX (0x%08lx)", index);
	#endif

	return index;
}
//+
 
/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///PDRDocument_Parse
F_METHODM(bool32, PDRDocument_Parse, FS_Document_Parse)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO())
	{
		FXMLMarkup *root = (FXMLMarkup *) IFEELIN F_Get(Obj, (uint32) "FA_XMLDocument_Markups");

		LOD->pool = Msg->Pool;

		for ( ; root ; root = root->Next)
		{
			#ifdef DB_PARSE
			IFEELIN F_Log(0,"root (%s)", root->Atom->Key);
			#endif

			if (root->Atom == F_ATOM(ROOT))
			{
				break;
			}
		}

		if (root)
		{
			FXMLMarkup *space;

			for (space = (FXMLMarkup *) root->ChildrenList.Head ; space ; space = space->Next)
			{
				FXMLAttribute *attribute;

				STRPTR id = NULL;

				#ifdef DB_PARSE
				IFEELIN F_Log(0,"*** NEW SPACE");
				#endif

				for (attribute = (FXMLAttribute *) space->AttributesList.Head ; attribute ; attribute = attribute->Next)
				{
					if (attribute->Atom == F_ATOM(ID))
					{
						id = attribute->Value;
					}
				}

				#ifdef DB_PARSE
				IFEELIN F_Log(0,"space (%s)", id);
				#endif

				if (id)
				{
					struct in_Handle *handle = IFEELIN F_NewP(LOD->pool, sizeof (struct in_Handle));

					if (handle)
					{
						handle->atom = IFEELIN F_AtomObtain(id, ALL);

						if (handle->atom)
						{
							handle->public.Pool = LOD->pool;

							handle->next = LOD->handles;
							LOD->handles = handle;

							#ifdef DB_PARSE
							IFEELIN F_Log(0,"handle (0x%08lx) next (0x%08lx) atom (0x%08lx)(%s)", handle, handle->next, handle->atom, handle->atom->Key);
							#endif

							if (space->ChildrenList.Head)
							{
								FXMLMarkup *item;

								for (item = (FXMLMarkup *) space->ChildrenList.Head ; item ; item = item->Next)
								{
									STRPTR id = NULL;
									STRPTR value = item->Body;

									FXMLAttribute *attribute;

									for (attribute = (FXMLAttribute *) item->AttributesList.Head ; attribute ; attribute = attribute->Next)
									{
										if (attribute->Atom == F_ATOM(ID))
										{
											id = attribute->Value;
										}
									}

									#ifdef DB_PARSE
									IFEELIN F_Log(0,"id (%s) body (%s)", id, item->Body);
									#endif

									if (id && item->Body)
									{
										FPDRAttribute *node = pdr_create_attribute(LOD->pool, id, ALL);

										if (node)
										{
											node->Value = value;
											node->NumericValue = IFEELIN F_Do(Obj, F_RESOLVED_ID(NUMERIFY), node->Value, &node->NumericType);

											#ifdef DB_PARSE
											IFEELIN F_Log(0,"attribute (%s) value (%s) (%ld)(0x%08lx)", node->Atom->Key, node->Value, node->NumericValue, node->NumericType);
											#endif

											node->Next = handle->public.Attributes;
											handle->public.Attributes = node;
										}
									}
								}
							}
						}
						else
						{
							IFEELIN F_Dispose(handle);
						}
					}
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}
//+
///PDRDocument_Clear
F_METHOD(uint32, PDRDocument_Clear)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_Handle *handle;

	for (handle = LOD->handles ; handle ; handle = handle->next)
	{
		FPDRAttribute *attribute;

		for (attribute = handle->public.Attributes ; attribute ; attribute = attribute->Next)
		{
			IFEELIN F_AtomRelease(attribute->Atom);
		}

		IFEELIN F_AtomRelease(handle->atom);
	}

	LOD->pool = NULL;

	return F_SUPERDO();
}
//+
///PDRDocument_Write
F_METHOD(bool32, PDRDocument_Write)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if ((LOD->exported == FALSE) && (LOD->handles != NULL))
	{
		struct in_Handle *handle;

		IFEELIN F_Do(Obj, F_RESOLVED_ID(PUSH), F_PDRDOCUMENT_ROOT, NULL);

		for (handle = LOD->handles ; handle ; handle = handle->next)
		{
			FPDRAttribute *attribute;

			#ifdef DB_WRITE
			IFEELIN F_Log(0,"handle (%s)", handle->atom->Key);
			#endif

			IFEELIN F_Do(Obj, F_RESOLVED_ID(PUSH), F_PDRDOCUMENT_SPACE, NULL);
			IFEELIN F_Do(Obj, F_RESOLVED_ID(ADD), F_PDRDOCUMENT_ID, handle->atom->Key);

			for (attribute = handle->public.Attributes ; attribute ; attribute = attribute->Next)
			{
				if (attribute->Value == NULL)
				{
					if (attribute->NumericType != 0)
					{
						attribute->Value = (STRPTR) IFEELIN F_Do(Obj, F_RESOLVED_ID(STRINGIFY), attribute->NumericValue, attribute->NumericType);
					}
				}

				#ifdef DB_WRITE
				IFEELIN F_Log
				(
					0,"   attribute (%s) value (%s) numeric (%ld)(0x%08lx)",

					attribute->Atom->Key,
					attribute->Value,
					attribute->NumericValue,
					attribute->NumericType

				);
				#endif

				if (attribute->Value)
				{
					IFEELIN F_Do(Obj, F_RESOLVED_ID(PUSH), F_PDRDOCUMENT_ITEM, attribute->Value);
					IFEELIN F_Do(Obj, F_RESOLVED_ID(ADD), F_PDRDOCUMENT_ID, attribute->Atom->Key);
					IFEELIN F_Do(Obj, F_RESOLVED_ID(POP));
				}
			}

			IFEELIN F_Do(Obj, F_RESOLVED_ID(POP));
		}

		IFEELIN F_Do(Obj, F_RESOLVED_ID(POP));

		LOD->exported = TRUE;
	}

	return F_SUPERDO();
}
//+

///PDRDocument_FindHandle
F_METHODM(FPDRHandle *, PDRDocument_FindHandle, FS_PDRDocument_FindHandle)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->ID && Msg->Persist)
	{
		FAtom *atom = IFEELIN F_AtomFind(Msg->ID, ALL);

		if (atom)
		{
			struct in_Handle *node;

			for (node = LOD->handles ; node ; node = node->next)
			{
				if (node->atom == atom)
				{
					#ifdef DB_FIND
					IFEELIN F_Log(0,"find handle (0x%08lx) for (%s)",F_PUBLICIZE_HANDLE(node), Msg->ID);
					#endif

					return F_PUBLICIZE_HANDLE(node);
				}
			}
		}
	}

	return NULL;
}
//+
///PDRDocument_CreateHandle
F_METHODM(FPDRHandle *, PDRDocument_CreateHandle, FS_PDRDocument_CreateHandle)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef DB_CREATE
	IFEELIN F_Log(0,"id (%s) persist (%s)", Msg->ID, Msg->Persist);
	#endif

	if (LOD->pool == NULL)
	{
		LOD->pool = (APTR) IFEELIN F_Do(Obj, F_RESOLVED_ID(CREATEPOOL));

		if (LOD->pool == NULL)
		{
			return NULL;
		}
	}
				
	if (Msg->ID && Msg->Persist)
	{
		FAtom *atom = IFEELIN F_AtomObtain(Msg->ID, ALL);

		#ifdef DB_CREATE
		IFEELIN F_Log(0,"atom (0x%08lx) for id (%s)",atom,Msg->ID);
		#endif

		if (atom)
		{
			struct in_Handle *node;

			for (node = LOD->handles ; node ; node = node->next)
			{
				if (node->atom == atom)
				{
					IFEELIN F_AtomRelease(atom);

					IFEELIN F_Log(FV_LOG_DEV, "id (%s) used twice !!", Msg->ID);

					return NULL;
				}
			}

			/* we need to create a new handler */

			node = IFEELIN F_NewP(LOD->pool, sizeof (struct in_Handle));

			#ifdef DB_CREATE
			IFEELIN F_Log(0,"node (0x%08lx) for id (%s)",node,Msg->ID);
			#endif

			if (node)
			{
				node->atom = atom;
				node->public.Pool = LOD->pool;
				node->public.Attributes = pdr_explode(LOD->pool, Msg->Persist);

				#ifdef DB_CREATE
				IFEELIN F_Log(0,"handle (0x%08lx)(%s) pool (0x%08lx)", node, node->atom->Key, LOD->pool);
				#endif

				if (node->public.Attributes)
				{
					node->next = LOD->handles;
					LOD->handles = node;

					return F_PUBLICIZE_HANDLE(node);
				}

				IFEELIN F_Dispose(node);
			}

			IFEELIN F_AtomRelease(atom);
		}
	}
	
	return NULL; 
}
//+
