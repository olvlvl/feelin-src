#include "Private.h"

//#define DB_PREPARSE
//#define DB_PARSE
//#define DB_PARSE_DIRTY
//#define DB_ADD
//#define DB_FIND
//#define DB_WRITE
//#define DB_ASSOCIATED_OBTAIN
//#define DB_ASSOCIATED_RELEASE
//#define DB_DEPRECATED

//#define F_ENABLE_CTRLC
//#define DB_ADDPROPERTY

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///cssdocument_check_atomspool
STATIC APTR cssdocument_check_atomspool(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (LOD->atoms_pool == NULL)
	{
		LOD->atoms_pool = IFEELIN F_AtomCreatePool();

		if (LOD->atoms_pool == NULL)
		{
			IFEELIN F_Log(FV_LOG_USER, "Unable to create Atoms Pool");
		}
	}

	return LOD->atoms_pool;
}
//+

///string_skip_whites_and_comments
STATIC STRPTR string_skip_whites_and_comments(STRPTR str)
{
	uint8 c;
	
	while (F_CHAR(str))
	{
///F_ENABLE_CTRLC
		#ifdef F_ENABLE_CTRLC
		
		if (IEXEC SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
		{
			IFEELIN F_Log(0,"***break: string_skip_whites_and_comments(%16.16s)",str);
			
			return str;
		}
		
		#endif
//+
 
		switch (c)
		{
			case ' ':
			case '\n':
			case '\r':
			case '\t':
			{
				str++; c = ' ';
			}
			break;

			case '/':
			{
				if (str[1] == '*')
				{
					/* skip comment */

					str += 2;

					while (F_CHAR(str))
					{
///F_ENABLE_CTRLC
						#ifdef F_ENABLE_CTRLC

						if (IEXEC SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
						{
							IFEELIN F_Log(0,"***break: string_skip_whites_and_comments(%16.16s)",str);

							return str;
						}

						#endif
//+
 
						if ((c == '*') && (str[1] == '/'))
						{
							str += 2; c = ' '; break;
						}
						
						str++; 
					}
				}
			}
		}

		if (c != ' ') break;
	}

	return str;
}
//+
///cssdocument_obtain_declaration
STATIC FCSSDeclaration * cssdocument_obtain_declaration(FClass *Class, FObject Obj, STRPTR Key, uint32 KeyLength)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FAtom *atom = IFEELIN F_AtomObtainP(LOD->atoms_pool, Key, KeyLength);

	if (atom != NULL)
	{
		FCSSDeclaration *declaration;

		for (declaration = LOD->declarations ; declaration ; declaration = declaration->Next)
		{
			if (declaration->Atom == atom)
			{
				break;
			}
		}

		if (declaration != NULL)
		{
			IFEELIN F_AtomReleaseP(LOD->atoms_pool, atom);
		}
		else
		{
			declaration = IFEELIN F_NewP(LOD->pool, sizeof (FCSSDeclaration));

			if (declaration != NULL)
			{
	            declaration->Next = LOD->declarations;
		        declaration->Atom = atom;
			
			    LOD->declarations = declaration;
			}
		}

		return declaration;
	}
	else
	{
		IFEELIN F_Log(FV_LOG_USER, "Unable to obtain FAtom for Key [%-16.16s] of Length (%ld)", Key, KeyLength);
	}

	return NULL;
}
//+
///cssdocument_obtain_declaration_pseudo
STATIC FCSSDeclaration * cssdocument_obtain_declaration_pseudo(FClass *Class, FObject Obj, FCSSDeclaration *Declaration, STRPTR Key, uint32 KeyLength)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FAtom *atom = IFEELIN F_AtomObtainP(LOD->atoms_pool, Key, KeyLength);

	if (atom != NULL)
	{
		FCSSDeclaration *pseudo;

		for (pseudo = Declaration->Pseudos ; pseudo ; pseudo = pseudo->Next)
		{
			if (pseudo->Atom == atom)
			{
				break;
			}
		}

		if (pseudo != NULL)
		{
			IFEELIN F_AtomReleaseP(LOD->atoms_pool, atom);
		}
		else
		{
			pseudo = IFEELIN F_NewP(LOD->pool, sizeof (FCSSDeclaration));
			pseudo->Next = Declaration->Pseudos;
			pseudo->Atom = atom;

			#ifdef DB_PARSE
			IFEELIN F_Log(0,"add pseudo (%s) to declaration (%s)", atom->Key, Declaration->Atom->Key);
			#endif

			Declaration->Pseudos = pseudo;
		}

		return pseudo;
	}

	return NULL;
}
//+

///cssdocument_new_property
STATIC FCSSProperty * cssdocument_new_property(FClass *Class, FObject Obj, FCSSDeclaration *Declaration, STRPTR PropertyKey, uint32 PropertyKeyLength, STRPTR ValueKey, uint32 ValueKeyLength)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FAtom *atom = IFEELIN F_AtomObtainP(LOD->atoms_pool, PropertyKey, PropertyKeyLength);

	if (atom != NULL)
	{
		STRPTR value = IFEELIN F_NewP(LOD->pool, ValueKeyLength + 1);

		if (value != NULL)
		{
			FCSSProperty *property;

			for (property = (FCSSProperty *) Declaration->PropertiesList.Head ; property ; property = (FCSSProperty *) property->Node.Next)
			{
				if (property->Atom == atom)
				{
					break;
				}
			}

			IEXEC CopyMem(ValueKey, value, ValueKeyLength);

			if (property != NULL)
			{
//                IFEELIN F_Log(0,"found same property, replace its value");

				IFEELIN F_Dispose(property->Value);
					
				property->Value = value;
				property->NumericValue = IFEELIN F_Do(Obj, F_RESOLVED_ID(NUMERIFY), property->Value, &property->NumericType);

				IFEELIN F_AtomReleaseP(LOD->atoms_pool, atom);

				return property;
			}
			else
			{
				property = IFEELIN F_NewP(LOD->pool, sizeof (FCSSProperty));

				if (property)
				{
					property->Atom = atom;
					property->Value = value;
					property->NumericValue = IFEELIN F_Do(Obj, F_RESOLVED_ID(NUMERIFY), property->Value, &property->NumericType);

					IFEELIN F_LinkTail(&Declaration->PropertiesList, (FNode *) property);

					#ifdef DB_PARSE
					IFEELIN F_Log(0,"new property: (%s)(%s)", property->Atom->Key, property->Value);
					#endif

					return property;
				}
			}

			IFEELIN F_Dispose(value);
		}

		IFEELIN F_AtomReleaseP(LOD->atoms_pool, atom);
	}

	return NULL;
}
//+
///cssdocument_rem_property
STATIC void cssdocument_rem_property(FClass *Class, FObject Obj, FCSSDeclaration *Declaration, FCSSProperty *Property)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	#if 1

	IFEELIN F_LinkRemove(&Declaration->PropertiesList, (FNode *) Property);

	IFEELIN F_AtomReleaseP(LOD->atoms_pool, Property->Atom);
	IFEELIN F_Dispose(Property->Value);
	IFEELIN F_Dispose(Property);

	#else

	FCSSProperty *prev = NULL;
	FCSSProperty *node;

	for (node = (FCSSProperty *) Declaration->PropertiesList.Head ; node ; node = (FCSSProperty *) node->Node.Next)
	{
		if (node == Property)
		{
			if (prev)
			{
				prev->Node.Next = node->Node.Next;
			}
			else
			{
				Declaration->Properties = node->Node.Next;
			}

			IFEELIN F_AtomReleaseP(LOD->atoms_pool, node->Atom);
			IFEELIN F_Dispose(node->Value);
			IFEELIN F_Dispose(node);

			break;
		}
		else
		{
			prev = node;
		}
	}

	#endif
}
//+

///cssdocument_parse_declaration
STRPTR cssdocument_parse_declaration(FClass *Class, FObject Obj, FCSSDeclaration *Declaration, STRPTR Source)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	uint8 c;

	while (F_CHAR(Source))
	{
		STRPTR p_s;
///F_ENABLE_CTRLC
		#ifdef F_ENABLE_CTRLC

		if (IEXEC SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
		{
			IFEELIN F_Log(0,"***break: (%16.16s)",Source);

			return FALSE;
		}

		#endif
//+
		#ifdef DB_PARSE_DIRTY
		IFEELIN F_Log(0,"skip whites and comments 2: [%16.16s]", Source);
		#endif

		Source = string_skip_whites_and_comments(Source);

		if (*Source == '}')
		{
			#ifdef DB_PARSE_DIRTY
			IFEELIN F_Log(0,"end of declaration block: (%s) at [%16.16s]", Declaration->Atom->Key, Source);
			#endif

			Source++;

			break;
		}
		else if (*Source == '\0')
		{
			IFEELIN F_Log(0,"unexpected end of data [%16.16s]", Source - 16);

			return NULL;
		}

		/* 'Source' should be on a _property_ */

		p_s = Source;

		#ifdef DB_PARSE_DIRTY
		IFEELIN F_Log(0,"property: (%16.16s)",p_s);
		#endif

		/* search _property_'s name end */

		while (F_CHAR(Source))
		{
///F_ENABLE_CTRLC
			#ifdef F_ENABLE_CTRLC

			if (IEXEC SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
			{
				IFEELIN F_Log(0,"***break: (%16.16s)",Source);

				return FALSE;
			}

			#endif
//+

			if ((c == ' ') || (c == ':'))
			{
				break;
			}
			else if (c == '\0')
			{
				IFEELIN F_Log(0,"unexpected end of data [%16.16s]", Source - 16);

				return NULL;
			}

			Source++;
		}

		if (c == ':')
		{
			STRPTR p_e = Source;
			STRPTR v_s;

			#ifdef DB_PARSE_DIRTY
			IFEELIN F_Log(0,"end of property name (%16.16s) (%16.16s) (%ld)",p_s,Source,Source - p_s);
			#endif

			Source++;

			F_SKIP_WHITE(Source);

			v_s = Source;

			#ifdef DB_PARSE_DIRTY
			IFEELIN F_Log(0,"value (%16.16s)",v_s);
			#endif

			while (F_CHAR(Source))
			{
				if ((c == ';') || (c == '}'))
				{
					FCSSProperty *property;

					#ifdef DB_PARSE_DIRTY
					IFEELIN F_Log(0,"end of value [%16.16s] [%16.16s] (%ld)",v_s,Source,Source - v_s);
					#endif

					property = cssdocument_new_property(Class,Obj, Declaration, p_s, p_e - p_s, v_s, Source - v_s);

					if (property == NULL)
					{
						IFEELIN F_Log(0, "Unable to create Property");

						return NULL;
					}

					Source++;

					break;
				}

				Source++;
			}

			if (c == '}')
			{
				#ifdef DB_PARSE_DIRTY
				IFEELIN F_Log(0,"end of declaration block (%16.16s)",Source);
				#endif

				break;
			}
		}
		else if (c == '\0')
		{
			IFEELIN F_Log(0,"unexpected end of data [%16.16s]", Source - 16);

			return NULL;
		}
		else
		{
			IFEELIN F_Log(0,"semi-column expected [%16.16s]", Source);

			/* property definition is maformed, we skip
			the rest of the declaration */

			while (F_CHAR(Source))
			{
				Source++;

				if (c == '}')
				{
					return Source;
				}
				else if (c == ';')
				{
					break;
				}
			}
		}
	}

	return Source;
}
//+

///cssdocument_preparse
void cssdocument_preparse(STRPTR Source)
{
	STRPTR s = Source;
	STRPTR d = Source;
	uint8 c;

	/* replace all white spaces by the same */

	while (F_CHAR(s))
	{
		if (c == '@')
		{
			s++;

			IFEELIN F_Log(0, "skipping at rule");

			while (F_CHAR(s))
			{
				s++;

				if ((c == ';') || (c == '\n') || (c == '\r') || (c == '\f'))
				{
					break;
				}
			}
		}
		else if ((c == '"') || (c == '\'') || (c == '('))
		{
			STRPTR e = s;
			uint8 q = c == '(' ? ')' : c;

			while (F_CHAR(e))
			{
				e++;

				if ((c == q) || (c == ';') || (c == '\n') || (c == '\r') || (c == '\f'))
				{
					break;
				}
			}

			if ((c == q) && (e != s))
			{
				e++;

				IEXEC CopyMem(s, d, e - s);

				d += (e - s);
			}

			s = e;
		}
		else if ((c == '\n') || (c == '\r') || (c == '\f') || (c == '\t'))
		{
			s++;
		}
		else if (c == ' ')
		{
			if (d[-1] != ' ')
			{
				*d++ = ' ';
			}

			while (F_CHAR(s))
			{
				if (c != ' ')
				{
					break;
				}

				s++;
			}
		}
		else if ((c == '/') && (s[1] == '*'))
		{
			s += 2;

			while (F_CHAR(s))
			{
				s++;

				if ((c == '*') && (*s == '/'))
				{
					s++;

					break;
				}
			}
		}
		else
		{
			*d++ = *s++;
		}
	}

	*d = '\0';

	#ifdef DB_PREPARSE
	IFEELIN F_Log(0, "PREPASE: [%s]", Source);
	#endif
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///CSSDocument_Get
F_METHOD(uint32,CSSDocument_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;
									
	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_DECLARATIONS:
		{
			F_STORE(LOD->declarations);
		}
		break;
	}

	return F_SUPERDO();
}
//+
///CSSDocument_Parse

F_METHODM(bool32,CSSDocument_Parse,FS_Document_Parse)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	LOD->line = 1;
	LOD->pool = Msg->Pool;

	if (cssdocument_check_atomspool(Class, Obj) == NULL)
	{
		return FALSE;
	}

	if (Msg->Pool && Msg->Source)
	{
		FCSSDeclaration *declaration;
		
		STRPTR source = Msg->Source;
		uint8 c;

		cssdocument_preparse(source);
		
		while (F_CHAR(source))
		{
			STRPTR declaration_s = source;
			STRPTR declaration_e;

			/* search the end of the _declaration_ name */
					
			while (F_CHAR(source))
			{
///F_ENABLE_CTRLC
						#ifdef F_ENABLE_CTRLC

						if (IEXEC SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
						{
							IFEELIN F_Log(0,"***break: (%16.16s)",source);

							return FALSE;
						}

						#endif
//+
				if ((c == ' ') || (c == ':') || (c == '{'))
				{
					break;
				}
				else if (c == '\0')
				{
					IFEELIN F_Log(0, "unexpected end of data [%-16.16s]", source - 16);

					return FALSE;
				}
					
				source++;
			}

			declaration_e = source;

			#ifdef DB_PARSE_DIRTY
			IFEELIN F_Log(0,"declaration: start [%16.16s] end [%16.16s] length (%ld)",declaration_s,source,source - declaration_s);
			#endif
					
			declaration = cssdocument_obtain_declaration(Class, Obj, declaration_s, declaration_e - declaration_s);

			if (declaration == NULL)
			{
				IFEELIN F_Log(0,"unable to obtain declaration");

				return FALSE;
			}

/*** pseudo ************************************************************************************/

			if (c == ':')
			{
				STRPTR pseudo_s = ++source;

				while (F_CHAR(source))
				{
///F_ENABLE_CTRLC
							#ifdef F_ENABLE_CTRLC

							if (IEXEC SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
							{
								IFEELIN F_Log(0,"***break: (%16.16s)",source);

								return FALSE;
							}

							#endif
//+
					if ((c == ' ') || (c == '{'))
					{
						break;
					}

					source++;
				}

				#ifdef DB_PARSE
				IFEELIN F_Log(0,"pseudo [%-16.16s] [%-16.16s]", pseudo_s, source);
				#endif

				if (pseudo_s == source)
				{
					IFEELIN F_Log(0,"pseudo name expected [%-16.16s]", source);
				}
				else
				{
					while (F_CHAR(source))
					{
///F_ENABLE_CTRLC
							#ifdef F_ENABLE_CTRLC

							if (IEXEC SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
							{
								IFEELIN F_Log(0,"***break: (%16.16s)",source);

								return FALSE;
							}

							#endif
//+
						if ((c == ' ') || (c == '{'))
						{
							break;
						}

						source++;
					}

					#ifdef DB_PARSE
					IFEELIN F_Log(0,"declaration (%s) pseudo [%-16.16s] [%-16.16s] len (%ld)",

						declaration->Atom->Key,
						pseudo_s,
						source,
						source - pseudo_s);
					#endif
							
					declaration = cssdocument_obtain_declaration_pseudo(Class, Obj, declaration, pseudo_s, source - pseudo_s);
				}
			}

					
			if ((c != '\0') && (c != '{'))
			{
				/* search declaration block markup '{' */
							
				source = string_skip_whites_and_comments(source);
						
				c = *source;
			}
				
			if (c == '{')
			{
				source++;

				source = cssdocument_parse_declaration(Class, Obj, declaration, source);

				if (source == NULL)
				{
					return FALSE;
				}
			}
			else if (*source == '\0')
			{
				//#ifdef DB_PARSE_DIRTY
				IFEELIN F_Log(0,"unexpected end of data");
				//#endif

				return FALSE;
			}
			else
			{
				IFEELIN F_Log(0,"declaration block not found (%16.16s) (%16.16s)",declaration_s,source);
						
				return FALSE;
			}
		}
	
		return TRUE;
	}

	return FALSE;
}
//+
///CSSDocument_Clear
F_METHOD(uint32,CSSDocument_Clear)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	LOD->pool = NULL;
	LOD->declarations = NULL;

	if (LOD->atoms_pool != NULL)
	{
		IFEELIN F_AtomDeletePool(LOD->atoms_pool);

		LOD->atoms_pool = NULL;
	}
	
	return F_SUPERDO();
}
//+
///CSSDocument_Write
F_METHODM(bool32, CSSDocument_Write, FS_Document_Write)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	bool32 rc = FALSE;

	IFEELIN F_Log(0,"WRITE TO (%s)", Msg->File);

	if (Msg->File)
	{
		BPTR file = IDOS_ Open(Msg->File, MODE_NEWFILE);

		if (file)
		{
			FCSSDeclaration *declaration;

			IDOS_ FPrintf(file, "/* %s generated by Feelin:CSSDocument */\n\n", (int32) Msg->File);

			for (declaration = LOD->declarations ; declaration ; declaration = declaration->Next)
			{
				FCSSProperty *property = (FCSSProperty *) declaration->PropertiesList.Head;

				if (property != NULL)
				{
					IDOS_ FPrintf(file, "%s\n{\n", (int32) declaration->Atom->Key);

					for ( ; property ; property = (FCSSProperty *) property->Node.Next)
					{
						IDOS_ FPrintf(file, "\t%s: %s;\n", (int32) property->Atom->Key, (int32) property->Value);
					}

					IDOS_ FPrintf(file, "}\n\n");
				}
			}

			IDOS_ Close(file);
		}
	}

	return rc;
}
//+

///CSSDocument_AddProperty
F_METHODM(FCSSProperty *, CSSDocument_AddProperty, FS_CSSDocument_AddProperty)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 declaration_length = IFEELIN F_StrLen(Msg->DeclarationName);
	uint32 property_length = IFEELIN F_StrLen(Msg->PropertyName);

	if (declaration_length && property_length && Msg->DataType)
	{
		FCSSDeclaration *declaration = NULL;
		FAtom *atom = IFEELIN F_AtomFind(Msg->DeclarationName, declaration_length);

		if (atom)
		{
			for (declaration = LOD->declarations ; declaration ; declaration = declaration->Next)
			{
				if (declaration->Atom == atom)
				{
					break;
				}
			}
		}

		if (declaration == NULL)
		{
			declaration = cssdocument_obtain_declaration(Class, Obj, Msg->DeclarationName, declaration_length);
		}

		if (declaration != NULL)
		{
			FCSSProperty *property = NULL;
			atom = IFEELIN F_AtomFind(Msg->PropertyName, property_length);

			if (atom)
			{
				for (property = (FCSSProperty *) declaration->PropertiesList.Head ; property ; property = (FCSSProperty *) property->Node.Next)
				{
					if (property->Atom == atom)
					{
						break;
					}
				}
			}

			if (property != NULL)
			{
				STRPTR value;

				/* Check if the value has really changed. If the  new  value
				is empty (e.g. NULL), the property is deleted */

				if (Msg->Data == NULL)
				{
					cssdocument_rem_property(Class, Obj, declaration, property);

					return NULL;
				}
				else if (FF_TYPE_LITERAL & Msg->DataType)
				{
					if (IFEELIN F_StrCmp(Msg->Data, property->Value, ALL) == 0)
					{
						//IFEELIN F_Log(0,"same string");

						return property;
					}
				}
				else if ((uint32) Msg->Data == property->NumericValue)
				{
						//IFEELIN F_Log(0,"same value");

					return property;
				}

				/* if the property already exists, we update its contents */

				if (FF_TYPE_LITERAL & Msg->DataType)
				{
					value = IFEELIN F_StrNewP(LOD->pool, NULL, "%s", Msg->Data);
				}
				else
				{
					value = (STRPTR) IFEELIN F_Do(Obj, F_RESOLVED_ID(STRINGIFY), Msg->Data, Msg->DataType);
				}

				IFEELIN F_Log
				(
					0,"property (%s) already exists, replace its content [%-16.16s] by [%-16.16s]",
					property->Atom->Key,
					property->Value,
					value);

				if (value != NULL)
				{
					IFEELIN F_Dispose(property->Value);

					property->Value = value;
					property->NumericValue = IFEELIN F_Do(Obj, F_RESOLVED_ID(NUMERIFY), property->Value, &property->NumericType);
				}
			}
			else if (Msg->Data)
			{
				if (FF_TYPE_LITERAL & Msg->DataType)
				{
					property = cssdocument_new_property(Class, Obj, declaration, Msg->PropertyName, property_length, Msg->Data, IFEELIN F_StrLen(Msg->Data));
				}
				else
				{
					STRPTR data = (STRPTR) IFEELIN F_Do(Obj, F_RESOLVED_ID(STRINGIFY), Msg->Data, Msg->DataType);

					if (data)
					{
						property = cssdocument_new_property(Class, Obj, declaration, Msg->PropertyName, property_length, data, IFEELIN F_StrLen(data));

						IFEELIN F_Dispose(data);
					}
				}
			}
			else
			{
				#ifdef DB_ADDPROPERTY

				else
				{
					IFEELIN F_Log(0,"property (%s:%s) is empty", Msg->DeclarationName, Msg->PropertyName);
				}

				#endif
			}

			#ifdef DB_ADDPROPERTY

			if (property)
			{
				IFEELIN F_Log
				(
					0, "declaration (%s) property (%s) value (%s)(%ld)",

					declaration->Atom->Key,
					property->Atom->Key,
					property->Value,
					property->NumericValue
				);
			}

			#endif

			return property;
		}
	}

	return NULL;
}
//+
///CSSDocument_GetProperty
F_METHODM(FCSSProperty *, CSSDocument_GetProperty, FS_CSSDocument_GetProperty)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 declaration_length = IFEELIN F_StrLen(Msg->DeclarationName);
	uint32 property_length = IFEELIN F_StrLen(Msg->PropertyName);

	if (declaration_length && property_length)
	{
		FCSSDeclaration *declaration = NULL;
		FAtom *atom = IFEELIN F_AtomFind(Msg->DeclarationName, declaration_length);

		if (atom)
		{
			for (declaration = LOD->declarations ; declaration ; declaration = declaration->Next)
			{
				if (declaration->Atom == atom)
				{
					break;
				}
			}
		}

		if (declaration != NULL)
		{
			FCSSProperty *property = NULL;
			atom = IFEELIN F_AtomFind(Msg->PropertyName, property_length);

			if (atom)
			{
				for (property = (FCSSProperty *) declaration->PropertiesList.Head ; property ; property = (FCSSProperty *) property->Node.Next)
				{
					if (property->Atom == atom)
					{
						break;
					}
				}
			}

			return property;
		}
		#ifdef DB_GETPROPERTY
		else
		{
			IFEELIN F_Log(0,"declaration (%s) not found for property (%s) - declarations (0x%08lx)", Msg->DeclarationName, Msg->PropertyName, LOD->declarations);
		}
		#endif
	}

	return NULL;
}
//+
