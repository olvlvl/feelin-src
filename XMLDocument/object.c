#include "Private.h"

//#define DB_COPY
//#define DB_ENTITY
//#define DB_PUSH
//#define DB_POP
//#define DB_ADD
//#define DB_WRITE
//#define DB_PARSE
//#define DB_PARSE_PI

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///xmldocument_check_atomspool
STATIC APTR xmldocument_check_atomspool(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (LOD->atomspool == NULL)
	{
		LOD->atomspool = IFEELIN F_AtomCreatePool();

		if (LOD->atomspool == NULL)
		{
			IFEELIN F_Log(FV_LOG_USER, "Unable to create Atoms Pool");
		}
	}

	return LOD->atomspool;
}
//+
///xmldocument_preparse
void xmldocument_preparse(STRPTR Source)
{
	STRPTR s;
	STRPTR d;

	uint8 c;

	/* white spaces */

	s = Source;
	d = Source;

	while ((c = *s++) != '\0')
	{
		if ((c == '\r') && (*s == '\n'))
		{
			s++;

			*d++ = '\n';
		}
		else if (c == '\f')
		{
			*d++ = '\n';
		}
		else if (c == '\t')
		{
			*d++ = ' ';
		}
		else
		{
			*d++ = c;
		}
	}

	*d = '\0';

	/* comments */

	s = Source;
	d = Source;

	while ((c = *s++) != '\0')
	{
		if ((c == '<') && (s[0] == '!') && (s[1] == '-') && (s[2] == '-'))
		{
			s += 3;

			//IFEELIN F_Log(0, "preparse: comment [%-16.16s]", s);

			while ((c = *s++) != '\0')
			{
				if ((c == '-') && (s[0] == '-') && (s[1] == '>'))
				{
					s += 2;

					//IFEELIN F_Log(0, "preparse: end ? [%-16.16s]", s);

					break;
				}
				else if (c == '\n')
				{
					/* keep linefeeds to keep error report acurracy */

					*d++ = c;
				}
			}
		}
		else
		{
			*d++ = c;
		}
	}

	*d = '\0';
}
//+

#if 0
///get_type
STATIC STRPTR get_type(bits32 Type)
{
	switch (Type)
	{
		case FV_TYPE_BOOLEAN:       return "FV_TYPE_BOOLEAN";
		case FV_TYPE_INTEGER:       return "FV_TYPE_INTEGER";
		case FV_TYPE_DECIMAL:       return "FV_TYPE_DECIMAL";
		case FV_TYPE_HEXADECIMAL:   return "FV_TYPE_HEXADECIMAL";
		case FV_TYPE_BINARY:        return "FV_TYPE_BINARY";
		case FV_TYPE_4CC:           return "FV_TYPE_4CC";
		case FV_TYPE_PIXEL:         return "FV_TYPE_PIXEL";
		case FV_TYPE_PERCENTAGE:    return "FV_TYPE_PERCENTAGE";
		case FV_TYPE_POINTER:       return "FV_TYPE_POINTER";
		case FV_TYPE_STRING:        return "FV_TYPE_STRING";
		case FV_TYPE_OBJECT:        return "FV_TYPE_OBJECT";

		default:
		{
			if (FF_TYPE_BOOLEAN & Type)
			{
				return "FF_TYPE_BOOLEAN";
			}
			else if (FF_TYPE_INTEGER & Type)
			{
				return "FF_TYPE_INTEGER";
			}
			else if (FF_TYPE_POINTER & Type)
			{
				return "FF_TYPE_POINTER";
			}
			else if (FF_TYPE_USER & Type)
			{
				return "FF_TYPE_USER";
			}
		}
	}

	return "FV_TYPE_COMPOSITE";
}
//+
#endif

///xmldocument_print_markups
static void xmldocument_print_markups(FList *list, BPTR fh, uint32 level)
{
	FXMLMarkup *markup;
	
	for (markup = (FXMLMarkup *)(list->Head) ; markup ; markup = markup->Next)
	{
		FXMLAttribute *attribute;
 
		uint32 i;
 
		for (i = 0 ; i < level ; i++)
		{
			IDOS_ FPrintf(fh, "\t");
		}

		IDOS_ FPrintf(fh, "<%s", (int32) markup->Atom->Key);

		for (attribute = (FXMLAttribute *)(markup->AttributesList.Head) ; attribute ; attribute = attribute->Next)
		{
			IDOS_ FPrintf(fh, " %s=\"%s\"", (int32) attribute->Atom->Key, (int32) attribute->Value);
		}

		if (markup->ChildrenList.Head)
		{
			IDOS_ FPrintf(fh, ">\n");
			
			xmldocument_print_markups(&markup->ChildrenList, fh, level + 1);
		
			for (i = 0 ; i < level ; i++)
			{
				IDOS_ FPrintf(fh, "\t");
			}

			IDOS_ FPrintf(fh, "</%s>\n", (int32) markup->Atom->Key);
		}
		else if (markup->Body)
		{
			IDOS_ FPrintf(fh, ">%s</%s>\n", (int32) markup->Body, (int32) markup->Atom->Key);
		}
		else
		{
			IDOS_ FPrintf(fh, " />\n");
		}
	}
}
//+
///xmldocument_find_entity_data
STRPTR xmldocument_find_entity_data(FClass *Class, FObject Obj, STRPTR Name, uint32 NameLength)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (LOD->entities)
	{
		uint32 hash=0;

		FHashLink *entity = (FHashLink *) IFEELIN F_HashFind(LOD->entities, Name, NameLength, &hash);

		if (entity)
		{
			#ifdef DB_FINDENTITY
			IFEELIN F_Log(0,"entity (%s) definition (%s)(%ld)",entity->Key,entity->Data,IFEELIN F_StrLen(entity->Data));
			#endif

			return entity->Data;
		}
	}

	return NULL;
}
//+
												
///xmldocument_obtain_storage
STATIC APTR xmldocument_obtain_storage(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	if (!LOD->pool)
	{
		LOD->pool = (APTR) IFEELIN F_Do(Obj, F_RESOLVED_ID(CREATEPOOL));
	}
 
	return LOD->pool;
}
//+
///xmldocument_string_copy

/* Cette fonction clone une chaîne  de  caractères.  La  fonction  prend  en
compte  les  caractères  spéciaux  définis  entre (&) et (;) e.g. &amp; sera
transformé en (&). ou &lt  et  &gt  en  (<)  et  (>),  qu'il  est  conseillé
d'utiliser. Si la fonction tombe sur une définition inconnue e.g. &value; ou
&true; elle est laisée telle quelle. */

typedef struct FeelinXMLReplace
{
	STRPTR                           Name;
	uint8                            Length;
	uint8                            Char;
}                                                           FXMLReplace;

static FXMLReplace sign_replace_table[]=
{
	{ "amp",   3, '&'          },
	{ "apos",  4, QUOTE_SINGLE },
	{ "gt",    2, '>'          },
	{ "lt",    2, '<'          },
	{ "quote", 5, QUOTE_DOUBLE },
	{ "tab",   3, '\t'         },
	{ "nl",    2, '\n'         },

	F_ARRAY_END
};

static STRPTR xmldocument_string_copy(FClass *Class, FObject Obj,STRPTR Source,STRPTR Destination,STRPTR *ResultPtr)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	/* general data type */
	
	if (Source)
	{
		STRPTR copy;
		
		/* first of all I check if  the  whole  string  is  an  entity  e.g.
		"&window-title;" in which case I return its definition */
		
		if (*Source == '&' && Source[1] != ' ')
		{
			STRPTR e = ++Source;
			
			while (*e != ';' && *e != ' ' && *e != '\t' && *e != '\n')
			{
				e++;
			}
		
			if (*e == ';')
			{
				if (e + 1 == Destination)
				{
					APTR data = xmldocument_find_entity_data(Class, Obj, Source, e - Source);

					#ifdef DB_COPY
					IFEELIN F_Log(0,"BIGTOKEN: (%16.16s) END (%16.16s) LEN (%ld, %ld) (%lc)",Source,e,e - Source, Destination - e,*Destination);
					#endif

					if (!data)
					{
						/* if we can't resolve the entity, the string is copied "as is" */

						data = IFEELIN F_NewP(LOD->pool, e - Source + 3);

						if (data)
						{
							IEXEC CopyMem(Source - 1, data, e - Source + 2);
						}
					}

					if (ResultPtr)
					{
						*ResultPtr = data;
					}

					return data ? Destination : NULL;
				}
			}

			--Source;
		}
		
		/* the string is not an entity but might content some */
		
		copy = IFEELIN F_NewP(LOD->pool,Destination - Source + 1);

		if (copy)
		{
			STRPTR c = copy;
 
			while (Source < Destination)
			{
				if (*Source == '&' && Source[1] != ' ')
				{
					int8 replaced = FALSE;
					FXMLReplace *replace;
					STRPTR e = ++Source;

					while (*e != ';' && *e != ' ' && *e != '\t' && *e != '\n')
					{
						e++;
					}
					
					if (*e == ';')
					{

						/* check builtin tokens */
					
						for (replace = sign_replace_table ; replace->Name ; replace++)
						{
							if (replace->Length == e - Source)
							{
								if (IFEELIN F_StrCmp(replace->Name,Source,replace->Length) == 0) break;
							}
						}

						if (replace->Name)
						{
							*c++ = replace->Char;
						
							replaced = TRUE;
						}

						/* check user tokens */

						if (!replaced)
						{
							STRPTR data = xmldocument_find_entity_data(Class, Obj, Source, e - Source);
							
							if (data)
							{
								STRPTR cpy2 = IFEELIN F_NewP(LOD->pool, (c - copy) + (Destination - e) + IFEELIN F_StrLen(data) + 1);
	 
								if (cpy2)
								{
									STRPTR old = copy;
										
									c = IFEELIN F_StrFmt(cpy2, "%s%s",copy,data);
									copy = cpy2;
										
									IFEELIN F_Dispose(old);
										
									replaced = TRUE;
								}
							}
						}

						if (replaced)
						{
							Source = e + 1;
						}
						else
						{
							*c++ = Source[-1];
						}
					}
					else
					{
						*c++ = *Source++;
					}
				}
				else if (*Source == '\n')
				{
					LOD->line++; Source++;
				}
				else
				{
					*c++ = *Source++;
				}
			}
			
			if (ResultPtr)
			{
				*ResultPtr = copy;
			}

			return Source;
		}
	}
	return NULL;
}
//+
///xmldocument_plain_to_escaped

/*

	This function copies a string, replacing forbidden characters.

		'<' is replaced by '&lt;'
		'>' is replaced by '&gt;'

*/

STRPTR xmldocument_plain_to_escaped(STRPTR Source, APTR Pool)
{
	STRPTR r = NULL;
	STRPTR k = Source;

	if (k)
	{
		uint8 c;

		uint32 plain_len = 0;
		uint32 escaped_len = 0;

		while ((c = *k++) != '\0')
		{
			if ((c == '<') || (c == '>'))
			{
				escaped_len += 4;
			}
			else
			{
				plain_len++;
			}
		}

		#ifdef DB_STRING
		IFEELIN F_Log(0,"plain_len (%ld) escaped_len (%ld)", plain_len, escaped_len);
		#endif

		if ((plain_len + escaped_len) > 0)
		{
			r = IFEELIN F_NewP(Pool, plain_len + escaped_len + 1);
			k = Source;

			while ((c = *k++) != '\0')
			{
				switch (c)
				{
					case '<':
					{
						*r++ = '&'; *r++ = 'l'; *r++ = 't' ; *r++ = ';';
					}
					break;

					case '>':
					{
						*r++ = '&'; *r++ = 'g'; *r++ = 't' ; *r++ = ';';
					}
					break;

					default:
					{
						*r++ = c;
					}
					break;
				}
			}

			r -= plain_len + escaped_len;

			#ifdef DB_STRING
			IFEELIN F_Log(0,"result: (%s)", r);
			#endif
		}
	}

	return r;
}
//+

///xmldocument_parse_entity
STRPTR xmldocument_parse_entity(FClass *Class, FObject Obj, STRPTR Source)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	STRPTR name_s;
	STRPTR name_e = NULL;

	uint8 c;

	#ifdef DB_ENTITY
	IFEELIN F_Log(0,"parse entity [%16.16s]", Source);
	#endif

	F_SKIPWHITE(Source);

	name_s = Source;

	/* search end of entity name */

	while ((c = *Source) != '\0')
	{
		if (c == ' ' || c == '\t' || c == '\n')
		{
			name_e = Source++;

			break;
		}
		else if (c == '>' || c == '<')
		{

			break;
		}

		Source++;
	}

	if (name_e)
	{
		uint8 quote;

		#ifdef DB_ENTITY
		IFEELIN F_Log(0,"name end [%16.16s] (%ld)", name_e, name_e - name_s);
		#endif

		F_SKIPWHITE(Source);

		quote = *Source;

		if (quote == '\"' || quote == '\'')
		{
			STRPTR def_s = ++Source;
			STRPTR def_e = NULL;

			while ((c = *Source) != '\0')
			{
				if (c == quote)
				{
					def_e = Source++;

					break;
				}
				else if ((c == '<' || c == '>'))
				{
					break;
				}

				Source++;
			}

			if (def_e)
			{
				#ifdef DB_ENTITY
				IFEELIN F_Log(0,"def length (%ld) [%16.16s][%16.16s]", def_e - def_s, def_s, def_e);
				#endif

				F_SKIPWHITE(Source);

				if (*Source == '>')
				{
					*name_e = '\0';
					*def_e = '\0';

					IFEELIN F_Do(Obj, F_METHOD_ID(ADDENTITY), name_s, def_s);

					return Source + 1;
				}
			}
		}
	}

__error:

	IFEELIN F_Log(0,"ENTITY END [%16.16s]",Source);

	while (((c = *Source) != '\0') && (c != '>'))
	{
		Source++;
	}

	IFEELIN F_Log(0,"end parse entity [%16.16s]", Source);

	if (*Source == '>')
	{
		return Source + 1;
	}

	return NULL;
}
//+
///xmldocument_parse_attributes

/* Cette fonction recherche la fin du marqueur. Elle  prend  une  chaîne  de
caractères  en argument, le début de la liste des attributs. e.g. "Size='12'
Pouic='true'>". Les caractères '/' et '?'  sont  pris  en  compte  avant  la
balise  de  fermeture  '>'.  La  fonction  s'assure  de  la  bonne forme des
attributs et des arguments. Elle prend en compte les guillement simples  (')
et doubles (").

Si tout c'est bien passé, la fonction retourne l'endroit, dans la chaine  de
caractère, où elle s'est arrétée e.g. ...> ou ...?> ou .../> */

STATIC STRPTR xmldocument_parse_attributes(FClass *Class, FObject Obj, STRPTR Source, FList *AttributesList)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 er=0;

	for (;;)
	{
		FXMLAttribute *attr;
		STRPTR item_s;
		uint8 quote;

		F_SKIPWHITE(Source);

		if (!*Source)
		{
			F_ERROR(NULL);
		}

		#ifdef DB_PARSE 
		IFEELIN F_Log(0,"xml_parse_attributes() [%16.16s](%02lx)",Source,*Source);
		#endif

		/* should be attribute name or end */

		if (*Source == '?' || *Source == '/' )
		{
			if (Source[1] != '>')
			{
				F_ERROR(CLOSE);
			}

			#ifdef DB_PARSE
			IFEELIN F_Log(0,"ATTR_END [%16.16s]",Source);
			#endif

			return Source;
		}
		else if (*Source == '>')
		{
			#ifdef DB_PARSE
			IFEELIN F_Log(0,"ATTR_END [%16.16s]",Source);
			#endif

			return Source;
		}

/*** allocate attribute structure **************************************************************/

		attr = IFEELIN F_NewP(LOD->pool,sizeof (FXMLAttribute));

		if (!attr)
		{
			F_ERROR(MEMORY);
		}

		IFEELIN F_LinkTail(AttributesList,(FNode *) attr);

/*** obtain name *******************************************************************************/

		item_s = Source; // start of attribute's name

		while (*Source && (*Source != ' ' && *Source != '\t' && *Source != '\n' && *Source != '='))
		{
			Source++;
		}

		if (!*Source)
		{
			F_ERROR(NULL);
		}

		attr->Line = LOD->line;
		//attr->Atom = (FAtom *) IFEELIN F_Do(Obj,F_RESOLVED_ID(OBTAINNAME),item_s,Source - item_s);
		attr->Atom = (FAtom *) IFEELIN F_AtomObtainP(LOD->atomspool, item_s, Source - item_s);
		
		if (!attr->Atom)
		{
			F_ERROR(MEMORY);
		}

/*** check equal sign **************************************************************************/

		F_SKIPWHITE(Source);

		if (*Source != '=')
		{
			F_ERROR(EQUAL);
		}

		Source++;

		F_SKIPWHITE(Source);

/*** check quotes, find end of attribute's data ************************************************/

		quote = *Source;

		if (quote != QUOTE_SINGLE && quote != QUOTE_DOUBLE)
		{
			F_ERROR(QUOTE)
		}

		item_s = ++Source;

		while (*Source && *Source != quote) Source++;

		if (!*Source)
		{
			F_ERROR(NULL);
		}

/*** copy attribute's data *********************************************************************/

		Source = xmldocument_string_copy(Class, Obj, item_s, Source, &attr->Value);

		if (!Source)
		{
			F_ERROR(MEMORY);
		}

		if (*Source != quote)
		{
			F_ERROR(QUOTE);
		}

		Source++;
		
/*** create a numeric value of attribute's data ************************************************/
											
		{
			uint32 value;
			bits32 type;
			
			value = IFEELIN F_Do(Obj, F_RESOLVED_ID(NUMERIFY), attr->Value, &type);
			
			if (type)
			{
				attr->NumericValue = value;
				attr->NumericType = type;
			}
			
			#ifdef DB_PARSE
			IFEELIN F_Log(0,"attr (%s) value (%s) numeric (0x%08lx) type (0x%08lx)", attr->Atom->Key, attr->Value, attr->NumericValue, attr->NumericType);
			#endif
		}
	}

/*** error handling ****************************************************************************/

__error:

	{
		STRPTR str = NULL;

		switch (er)
		{
			case FV_XML_ERROR_NULL:    str = "Unexpected end of data"; break;
			case FV_XML_ERROR_CLOSE:   str = "Expected closed braket"; break;
			case FV_XML_ERROR_EQUAL:   str = "Expected equal sign"; break;
			case FV_XML_ERROR_QUOTE:   str = "Missing quote"; break;
			case FV_XML_ERROR_MEMORY:  str = "Not enough memory"; break;
		}

		if (str)
		{
			IFEELIN F_Do(Obj,F_RESOLVED_ID(LOG),LOD->line,Source,str);
		}
	}

	return NULL;
}
//+
///xmldocument_parse_pi
STATIC STRPTR xmldocument_parse_pi(FClass *Class, FObject Obj, STRPTR PIName, uint32 PINameLength, STRPTR Source)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FXMLPI *pi = IFEELIN F_NewP(LOD->pool, sizeof (FXMLPI));

	if (pi == NULL)
	{
		return NULL;
	}

	pi->Atom = IFEELIN F_AtomObtainP(LOD->atomspool, PIName, PINameLength);

	if (pi->Atom == NULL)
	{
		return NULL;
	}

	pi->Line = LOD->line;

	Source = xmldocument_parse_attributes(Class, Obj, Source, &pi->AttributesList);

	if (Source != NULL)
	{
		IFEELIN F_LinkTail(&LOD->pi_list, (FNode *) pi);
	}

	return Source;
}
//+
///xmldocument_parse_markup

/* Markups created are added to MarkupsList, this way children  are  easily
added */

STATIC STRPTR xmldocument_parse_markup(FClass *Class,FObject Obj,STRPTR Source,FList *MarkupsList)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FXMLMarkup *markup=NULL;
	uint32 er;
	STRPTR item_s=NULL;

	uint8 c;

/*** skip possible leading spaces **************************************************************/

	F_SKIPWHITE(Source);

	#ifdef DB_PARSE
	IFEELIN F_Log(0,"xmldocument_parse() (%16.16s)",Source);
	#endif

	if (*Source != '<')
	{
		F_ERROR(OPEN);
	}

	if (!*++Source)
	{
		F_ERROR(NULL);
	}

/*** processing instructions ********************************************************************

	Definition: Processing instructions (PIs)  allow  documents  to  contain
	instructions for applications.

	Processing Instructions
	[16]   	PI	   ::=   	'<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
	[17]   	PITarget	   ::=   	Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))

	PIs are not part of the document's character data, but  MUST  be  passed
	through  to the application. The PI begins with a target (PITarget) used
	to identify the application to which the instruction  is  directed.  The
	target names "XML", "xml", and so on are reserved for standardization in
	this  or  future  versions  of  this  specification.  The  XML  Notation
	mechanism  MAY  be  used for formal declaration of PI targets. Parameter
	entity references MUST NOT be recognized within processing instructions.

************************************************************************************************/

	if (*Source == '?')
	{
		STRPTR pi_s = ++Source;

		#ifdef DB_PARSE_PI
		IFEELIN F_Log(0,"found '?' [%-32.32s]", Source);
		#endif

		/* search processing instruction's name end */

		while ((c = *Source++) != 0)
		{
			if (c == '<')
			{
				F_ERROR(SYNTAX);
			}
			else if ((c == ' ') || (c == '?'))
			{
				break;
			}
		}

		if (c == ' ')
		{
			uint32 pi_name_length = Source - pi_s - 1;

			#ifdef DB_PARSE_PI
			IFEELIN F_Log(0, "cursor [%-32.32s], pi name length (%ld)", Source, pi_name_length);
			#endif
		
			Source = xmldocument_parse_pi(Class, Obj, pi_s, pi_name_length, Source);

			if (Source == NULL)
			{
				F_ERROR(NONE);
			}
		}

		if (*Source != '?')
		{
			F_ERROR(INTEROGATION);
		}

		Source++;

		if (*Source != '>')
		{
			F_ERROR(CLOSE);
		}

		Source++;

		return Source;
	}


/** system markup ******************************************************************************/

	if (*Source == '!')
	{
		Source++;

		#if 0
		if ((Source[0] == '-') && (Source[1] == '-'))
		{
			Source += 2;

			#ifdef DB_PARSE
			IFEELIN F_Log(0,"COMMENT_BEGIN [%16.16s]",Source);
			#endif

			while (*Source)
			{
				if (*Source == '-' && Source[1] == '-' && Source[2] == '>') break;

				Source++;
			}

			if (!*Source)
			{
				F_ERROR(NULL);
			}

			Source += 3;

			#ifdef DB_PARSE
			IFEELIN F_Log(0,"COMMENT_END [%16.16s]",Source);
			#endif

			return Source;
		}
		else
		#endif

		if (IFEELIN F_StrCmp(Source, "ENTITY ", 7) == 0)
		{
			Source = xmldocument_parse_entity(Class, Obj, Source + 7);

			#ifdef DB_ENTITY
			IFEELIN F_Log(0,"FROM PARSE ENTITY [%16.16s]", Source);
			#endif

			return Source;
		}
	}

/*** allocate Markup ***************************************************************************/

	#ifdef DB_PARSE
	IFEELIN F_Log(0,"allocate markup (%16.16s)",Source);
	#endif
 
	markup = IFEELIN F_NewP(LOD->pool,sizeof (FXMLMarkup));

	if (!markup)
	{
		F_ERROR(MEMORY);
	}

	IFEELIN F_LinkTail(MarkupsList,(FNode *) markup);

/*** obtaining Markup name *********************************************************************/

	item_s = Source;

	while (*Source && (*Source != ' ' && *Source != '\t' && *Source != '\n' && *Source != '>' && *Source != '/')) Source++;

	if (!*Source) F_ERROR(NULL);

	#ifdef DB_PARSE
	IFEELIN F_Log(0,"markup name (%16.16s)(%16.16s)",item_s,Source);
	#endif
 
	markup->Line = LOD->line;
	
	if (*item_s == '&' && Source[-1] == ';')
	{
		STRPTR data = xmldocument_find_entity_data(Class, Obj, item_s + 1, Source - item_s - 2);
		
		#ifdef DB_PARSE
		IFEELIN F_Log(0,"markup entity: (%16.16s) (%lc)(%lc) >> (0x%08lx)",item_s,*item_s,Source[-1],data);
		#endif
					
		if (!data)
		{
			F_ERROR(ENTITY);
		}
  
		#if F_CODE_DEPRECATED

		if (entity->Type != FV_TYPE_STRING)
		{
			F_ERROR(ENTITY_TYPE)
		}

		#endif

		#ifdef DB_PARSE
		IFEELIN F_Log(0,"entity def (%s)",data);
		#endif
			
		markup->Atom = (FAtom *) IFEELIN F_AtomObtainP(LOD->atomspool, data, ALL);
	}
	else
	{
		markup->Atom = (FAtom *) IFEELIN F_AtomObtainP(LOD->atomspool, item_s, Source - item_s);
	}
	
	if (!markup->Atom)
	{
		F_ERROR(MEMORY);
	}

	#ifdef DB_PARSE
	IFEELIN F_Log(0,"Markup (%s) NEXT (%16.16s)", markup->Atom->Key, Source);
	#endif

/*** read Markup attributes ********************************************************************/

	/* if Markup is not closed, there are attributes to read */

	F_SKIPWHITE(Source);

	if (*Source == '<')
	{
		F_ERROR(ATTRIBUTE);
	}
 
	if (*Source != '>' && *Source != '/')
	{
		#ifdef DB_PARSE
		IFEELIN F_Log(0,"should parse arguments (%16.16s)",Source);
		#endif

		Source = xmldocument_parse_attributes(Class,Obj,Source,&markup->AttributesList);

		if (!Source) F_ERROR(NONE); // an error message have already been logged

		#ifdef DB_PARSE
		IFEELIN F_Log(0,"ATTR_NEXT [%16.16s] - Markup (%s)", Source, markup->Atom->Key);
		#endif
	}

/*** closing Markup ****************************************************************************/

	#ifdef DB_PARSE
	IFEELIN F_Log(0,"CLOSE: [%16.16s] - Markup '%s'", Source, markup->Atom->Key);
	#endif

	/* Si il y avait des attributs ils  ont  été  lus.  On  vérifie  que  le
	marqueur  se  termine  de  façon correcte. D'abord, on va vérifier si le
	marqueur ne termine tout seul '/>' */

	if (*Source == '/')
	{
		if (*++Source != '>') F_ERROR(CLOSE);

		/* si le marqueur se termine tout seul, il n'y a plus rien à lire. On
		retourne donc la position actuelle de la source */

		return Source + 1;
	}
	else if (*Source == '>')
	{

		/* Le marqueur est fermé. Il y a peut être  des  données  à  lire  ou
		encore  des  enfants  à  ajouter,  ou  bien  un  marqueur de fin e.g.
		<Button></Button>.  D'abord  on  va  s'occuper  des  données  et  des
		enfants,  ensuite,  on  aura plus qu'a regarder si le terminateur est
		bien là. */

		Source++;

		F_SKIPWHITE(Source);

		if (*Source != '<')
		{

			/* Des données sont définies. Sinon il y aurait  un  '<'  pour  un
			enfant, ou le marqueur de fin */

			item_s = Source;

			while (*Source && *Source != '<') Source++;

			if (!*Source) F_ERROR(NULL);

			Source = xmldocument_string_copy(Class,Obj,item_s,Source,&markup->Body);

			if (!Source)
			{
				F_ERROR(MEMORY);
			}
		}

		if (*Source != '<') F_ERROR(OPEN);

		/* Vérification du type de marqueur. Soit des enfants <Name...>, soit
		le marqueur de fin </...> */

		if (Source[1] != '/')
		{

			/* Définition d'enfants */

			while (*Source && (*Source == '<' && Source[1] != '/'))
			{
				#ifdef DB_PARSE
				IFEELIN F_Log(0,"ADDING [%16.16s] Markup (%s)", Source,  markup->Atom->Key);
				#endif

				Source = xmldocument_parse_markup(Class,Obj,Source,&markup->ChildrenList);

				if (!Source) F_ERROR(NONE); // an error has already been logged

				F_SKIPWHITE(Source);

				#ifdef DB_PARSE
				IFEELIN F_Log(0,"RETURN [%16.16s] Markup (%s)",Source, markup->Atom->Key);
				#endif
			}

			if (!*Source) F_ERROR(NULL);
		}

		/* Si il y avait des enfants ils ont tous été  ajouté.  Mainteant  la
		Source doit être sur le marqueur de fin </...> */

		if (Source[1] == '/')
		{
			/* Marqueur de fin. On vérifie quand même qu'il s'agit du bon. */

			Source += 2;

			if (IFEELIN F_StrCmp(markup->Atom->Key,Source,markup->Atom->KeyLength) != 0)
			{
				F_ERROR(NAME);
			}

			#ifdef DB_PARSE
			IFEELIN F_Log(0,"END OF (%s) NEXT [%16.16s]", markup->Atom->Key,Source);
			#endif

			if (Source[markup->Atom->KeyLength] != '>')
			{
				F_ERROR(CLOSE);
			}

			return Source + markup->Atom->KeyLength + 1;
		}
		else F_ERROR(SYNTAX);
	}
	else F_ERROR(CLOSE);

/*** error logging *****************************************************************************/

__error:

	{
		STRPTR str = NULL;

		switch (er)
		{
			case FV_XML_ERROR_NULL:          str = "Unexpected end of data"; break;
			case FV_XML_ERROR_MEMORY:        str = "Unable to allocate Markup"; break;
			case FV_XML_ERROR_SYNTAX:        str = "Syntax Error"; break;
			case FV_XML_ERROR_OPEN:          str = "Expected opened braket"; break;
			case FV_XML_ERROR_CLOSE:         str = "Expected closed braket"; break;
			case FV_XML_ERROR_ATTRIBUTE:     str = "Expected attribute"; break;
			case FV_XML_ERROR_INTEROGATION:  str = "Expected '?'"; break;
			case FV_XML_ERROR_NAME:
			{
				IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG),LOD->line,Source,"Expected (%s)",markup->Atom->Key);
			}
			break;
		
			case FV_XML_ERROR_ENTITY:
			{
				IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG), LOD->line,Source,"Unresolvable entity (%16.16s)",item_s);
			}
			break;
		
			case FV_XML_ERROR_ENTITY_TYPE:   str = "FV_TYPE_STRING entity type required"; break;
		}

		if (str)
		{
			IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG), LOD->line,Source,str);
		}
	}

	return NULL;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///XMLDocument_New
F_METHOD(uint32,XMLDocument_New)
{
	struct TagItem *Tags = Msg, item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ENTITIES:
		{
			IFEELIN F_Do(Obj, F_METHOD_ID(ADDENTITIES), item.ti_Data);
		}
		break;
	}

	return F_SUPERDO();
}
//+
///XMLDocument_Get
F_METHOD(void,XMLDocument_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ATTRIBUTES:  				
		{
			F_STORE(LOD->attributes_list.Head);
		}
		break;
		
		case FV_ATTRIBUTE_MARKUPS:     				
		{
			F_STORE(LOD->markups_list.Head);
		}
		break;
		
		case FV_ATTRIBUTE_INSTRUCTIONS:
		{
			F_STORE(LOD->pi_list.Head);
		}
		break;
	}

	F_SUPERDO();
}
//+
///XMLDocument_Parse
F_METHODM(bool32,XMLDocument_Parse,FS_Document_Parse)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	STRPTR Source = Msg->Source;

	LOD->line = 1;
	LOD->pool = Msg->Pool;

	if (xmldocument_check_atomspool(Class, Obj) == NULL)
	{
		return FALSE;
	}

	if (Source)
	{
		xmldocument_preparse(Source);

		for (;;)
		{
			Source = xmldocument_parse_markup(Class,Obj,Source,&LOD->markups_list);

			if (Source)
			{
				F_SKIPWHITE(Source); if (!*Source) break;
			}
			else break;
		}
	
		if (Source)
		{
			FXMLAttribute *attribute;

/*** store attributes **************************************************************************/

			for (attribute = (FXMLAttribute *)(LOD->attributes_list.Head) ; attribute ; attribute = attribute->Next)
			{
				if (attribute->Atom == F_ATOM(VERSION))
				{
					int32 val;
					uint32 len = f_stcd(attribute->Value, &val);

					if (len)
					{
						IFEELIN F_Set(Obj, F_RESOLVED_ID(VERSION), val);

						if (attribute->Value[len] == '.')
						{
							len = f_stcd(attribute->Value + len + 1, &val);

							if (len)
							{
								IFEELIN F_Set(Obj, F_RESOLVED_ID(REVISION), val);
							}
						}
					}
				}
			}
			
			#ifdef F_ENABLE_DISPLAY
			xmldocument_display_markups(&LOD->markups_list,0);
			#endif
			
			return TRUE;
		}
		#if 0
		else
		{
			IFEELIN F_Log(0,"XML source is not valid (0x%08lx) >> DUMPING !",Msg->Source);
			IDOS_ FPrintf(FeelinBase->Console,Msg->Source);
		}
		#endif
	}
	return FALSE;
}
//+
///XMLDocument_Clear
F_METHOD(void,XMLDocument_Clear)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	LOD->attributes_list.Head = NULL;
	LOD->attributes_list.Tail = NULL;
	
	LOD->markups_list.Head = NULL;
	LOD->markups_list.Tail = NULL;
	
	LOD->pool = NULL;
	LOD->line = 0;

	LOD->entities = NULL;

	if (LOD->atomspool != NULL)
	{
		IFEELIN F_AtomDeletePool(LOD->atomspool);

		LOD->atomspool = NULL;
	}

	F_SUPERDO();
}
//+

///XMLDocument_AddEntity
F_METHODM(uint32,XMLDocument_AddEntity,FS_XMLDocument_AddEntity)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	/* the FHashTable to store entities is create on the fly when needed */

	if (!LOD->entities)
	{
		if (xmldocument_obtain_storage(Class,Obj))
		{
			LOD->entities = IFEELIN F_NewP(LOD->pool,sizeof (FHashTable) + sizeof (APTR) * FV_HASH_NORMAL);

			if (LOD->entities)
			{
				LOD->entities->Size = FV_HASH_NORMAL;
				LOD->entities->Entries = (APTR)((uint32)(LOD->entities) + sizeof (FHashTable));
			}
		}
	}

	#ifdef DB_ENTITY
	IFEELIN F_Log(0,"<!ENTITY %s \"%s\" >",Msg->Name, Msg->Definition);
	#endif

	/* now that we have checked the FHashTable we can add the entry */

	if (LOD->entities)
	{
		volatile uint32 hash=0;

		FHashLink *entity = (FHashLink *) IFEELIN F_HashFind(LOD->entities, Msg->Name, IFEELIN F_StrLen(Msg->Name), &hash);

		if (entity)
		{
			STRPTR def = IFEELIN F_StrNewP(LOD->pool, NULL, "%s", Msg->Definition);

			if (def)
			{
				IFEELIN F_Dispose(entity->Data);
				entity->Data = def;

				return TRUE;
			}
		}
		else
		{
			FHashLink *entity = IFEELIN F_NewP(LOD->pool, sizeof(FHashLink));

			uint32 name_len = 0;

			STRPTR name = IFEELIN F_StrNewP(LOD->pool, &name_len, "%s", Msg->Name);
			STRPTR def = IFEELIN F_StrNewP(LOD->pool, NULL, "%s", Msg->Definition);

			if (entity && name && def)
			{
				entity->Key = name;
				entity->KeyLength = name_len;
				entity->Data = def;

				#ifdef DB_ENTITY
				IFEELIN F_Log(0,"new entity (%s) (%s)", entity->Key, entity->Data);
				#endif

				entity->Next = LOD->entities->Entries[hash];
				LOD->entities->Entries[hash] = entity;

				return TRUE;
			}

			IFEELIN F_Dispose(entity);
			IFEELIN F_Dispose(name);
			IFEELIN F_Dispose(def);
		}
	}

	return FALSE;
}
//+
///XMLDocument_AddEntities
F_METHODM(uint32,XMLDocument_AddEntities,FS_XMLDocument_AddEntities)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Entities)
	{
		if (xmldocument_obtain_storage(Class,Obj))
		{
			FXMLEntity *e;

			for (e = Msg->Entities ; e->Name ; e++)
			{
				STRPTR def = NULL;

				if (FF_TYPE_BIT & e->Type)
				{
					if (FV_TYPE_BOOLEAN & e->Type)
					{
						def = (uint32) e->Definition ? "true" : "false";
					}
					else
					{
						IFEELIN F_Log(FV_LOG_DEV, "unsupported FF_TYPE_BIT type (0x%08lx)", e->Type);
					}
				}
				else if (FF_TYPE_NUMERIC & e->Type)
				{
					if (FV_TYPE_INTEGER & e->Type)
					{
						def = IFEELIN F_StrNewP(LOD->pool, NULL, "%ld", e->Definition);
					}
					else
					{
						IFEELIN F_Log(FV_LOG_DEV, "unsupported FF_TYPE_NUMERIC type (0x%08lx)", e->Type);
					}
				}
				else if (FF_TYPE_LITERAL & e->Type)
				{
					if (FV_TYPE_STRING & e->Type)
					{
						def = e->Definition;
					}
					else if (FV_TYPE_HEXADECIMAL & e->Type)
					{
						def = IFEELIN F_StrNewP(LOD->pool, NULL, "#%lx", e->Definition);
					}
					else if (FV_TYPE_PERCENTAGE & e->Type)
					{
						def = IFEELIN F_StrNewP(LOD->pool, NULL, "%ld%%", e->Definition);
					}
					else if (FV_TYPE_PIXEL & e->Type)
					{
						def = IFEELIN F_StrNewP(LOD->pool, NULL, "%ldpx", e->Definition);
					}
					else
					{
						IFEELIN F_Log(FV_LOG_DEV, "unsupported FF_TYPE_LITERAL type (0x%08lx)", e->Type);
					}
				}
				else if (FF_TYPE_REFERENCE & e->Type)
				{
					if (FV_TYPE_POINTER & e->Type)
					{
						def = IFEELIN F_StrNewP(LOD->pool, NULL, "0x%08lx", e->Definition);
					}
					else
					{
						IFEELIN F_Log(FV_LOG_DEV, "unsupported FF_TYPE_REFERENCE type (0x%08lx)", e->Type);
					}
				}
				else
				{
					IFEELIN F_Log(FV_LOG_DEV, "type (0x%08lx) for entity (%s) is not yet supported", e->Type, e->Name);
				}

				if (def)
				{
					IFEELIN F_Do(Obj, F_METHOD_ID(ADDENTITY), e->Name, def);
				}
			}
		}
	}

	return 0; // FIXME: undefined yet
}
//+
///XMLDocument_FindEntity
F_METHODM(STRPTR,XMLDocument_FindEntity,FS_XMLDocument_FindEntity)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef DB_FINDENTITY
	IDOS_ FPrintf(FeelinBase->Console, "entity (");
	IDOS_ Flush(FeelinBase->Console);
	IDOS_ Write(FeelinBase->Console, Msg->Name, Msg->NameLength);
	IDOS_ FPrintf(FeelinBase->Console,")(%ld) list (0x%08lx)\n",Msg->NameLength,LOD->entities);
	#endif

	if (LOD->entities)
	{
		uint32 hash=0;

		FHashLink *entity = (FHashLink *) IFEELIN F_HashFind(LOD->entities, Msg->Name, IFEELIN F_StrLen(Msg->Name), &hash);

		#ifdef DB_FINDENTITY
		IFEELIN F_Log(0,"entity (%s)(%s)",entity->Key,entity->Data);
		#endif

		return entity->Data;
	}

	return NULL;
}
//+

///
/*
<feelin:persist>

-> PUSH "feelin:persist"

	<space id="windows.main">

   ->PUSH "space"
   ->ADD "id", "windows.main"

		<item id="left" value="#FF00FF">12</item>

	   ->PUSH "item" BODY "12"
	   ->ADD "id" STRING "left"
	   ->ADD "value" HEXADECIMAL #FF00FF
	   ->POP

		<item id="right" empty="true" value="13" />

	   ->PUSH "item"
	   ->ADD "id", "right"
	   ->ADD "empty" BOOLEAN TRUE
	   ->ADD "value" INTEGER 13
	   ->POP

	</space>

   ->POP

</feelin:persist>

-> POP
*/
//+

/* The objet should be locked before using editing methods */

///XMLDocument_Push
F_METHODM(uint32,XMLDocument_Push,FS_XMLDocument_Push)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
										
	#ifdef DB_PUSH
	IFEELIN F_Log(0,"name (%s)",Msg->Name);
	#endif
	
	if (LOD->markup_cursor && LOD->markup_cursor->Body)
	{
		IFEELIN F_Log(0,"pushing a child in a markup with a body is illegal");
		
		return FALSE;
	}
	
	if (Msg->Name)
	{
		APTR pool = xmldocument_obtain_storage(Class,Obj);

		#ifdef DB_PUSH
		IFEELIN F_Log(0,"pool (0x%08lx)",pool);
		#endif

		if (xmldocument_check_atomspool(Class, Obj) == NULL)
		{
			return FALSE;
		}
  
		if (pool)
		{
			FXMLMarkup *markup = IFEELIN F_NewP(pool, sizeof (FXMLMarkup));
		
			#ifdef DB_PUSH
			IFEELIN F_Log(0,"markup (0x%08lx)",markup);
			#endif
 
			if (markup)
			{
				markup->Parent = LOD->markup_cursor;
 
				if (Msg->Body)
				{
					markup->Body = xmldocument_plain_to_escaped(Msg->Body, pool);
					
					#ifdef DB_PUSH
					IFEELIN F_Log(0,"body (0x%08lx) [%-32.32s]",markup->Body,markup->Body);
					#endif
				}
			
				if ((Msg->Body && markup->Body) || (Msg->Body == NULL))
				{
					/*
					markup->Atom = (FAtom *) IFEELIN F_Do
					(
						Obj, F_RESOLVED_ID(OBTAINNAME),
						
						Msg->Name, IFEELIN F_StrLen(Msg->Name)
					);
					*/

					markup->Atom = (FAtom *) IFEELIN F_AtomObtainP(LOD->atomspool, Msg->Name, ALL);

					
					#ifdef DB_PUSH
					IFEELIN F_Log(0,"atom (0x%08lx) key (%s)", markup->Atom, markup->Atom->Key);
					#endif
					
					if (markup->Atom)
					{
						
						#ifdef DB_PUSH

						IFEELIN F_Log
						(
							0, "markup (0x%08lx) atom (0x%08lx) key (%s) parent (%s)",
							
							markup,
							markup->Atom,
							markup->Atom->Key,
							markup->Parent ? markup->Parent->Atom->Key : (uint8 *) "?xml"
						);

						#endif
	 
						IFEELIN F_LinkTail(LOD->markup_cursor ? &LOD->markup_cursor->ChildrenList : &LOD->markups_list, (FNode *) markup);

						LOD->markup_cursor = markup;
 
						return TRUE;
					}
				}
				
				IFEELIN F_Dispose(markup->Body);
				IFEELIN F_Dispose(markup);
			}
		}
	}
	return FALSE;
}
//+
///XMLDocument_Pop
F_METHOD(uint32,XMLDocument_Pop)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
					
	#ifdef DB_POP
	
	if (LOD->markup_cursor)
	{
		IFEELIN F_Log
		(
			0, "cursor (%s) parent (%s)",
			
			LOD->markup_cursor->Name->Key,
			LOD->markup_cursor->Parent ? LOD->markup_cursor->Parent->Name->Key : (uint8 *) "?xml"
		);
	}
	else
	{
		IFEELIN F_Log
		(
			0, "cursor (?xml) - pop is illegal !!"
		);
	}
	
	#endif
 
	if (LOD->markup_cursor)
	{
		LOD->markup_cursor = LOD->markup_cursor->Parent;
		
		return TRUE;
	}
	
	return FALSE;
}
//+
///XMLDocument_Add
F_METHODM(uint32,XMLDocument_Add,FS_XMLDocument_Add)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Name && Msg->Value)
	{
		APTR pool = xmldocument_obtain_storage(Class,Obj);

		if (xmldocument_check_atomspool(Class, Obj) == NULL)
		{
			return FALSE;
		}
		
		if (pool)
		{
			FXMLAttribute *attribute = IFEELIN F_NewP(pool, sizeof (FXMLAttribute));
			
			if (attribute)
			{
				attribute->Value = xmldocument_plain_to_escaped(Msg->Value, pool);
				
				if (attribute->Value)
				{
					uint32 value;
					bits32 type;

					value = IFEELIN F_Do(Obj, F_RESOLVED_ID(NUMERIFY), attribute->Value, &type);

					if (type)
					{
					   attribute->NumericValue = value;
					   attribute->NumericType = type;
					}
 
					/*
					attribute->Atom = (FAtom *) IFEELIN F_Do
					(
						Obj, F_RESOLVED_ID(OBTAINNAME),

						Msg->Name, IFEELIN F_StrLen(Msg->Name)
					);
					*/

					attribute->Atom = (FAtom *) IFEELIN F_AtomObtainP(LOD->atomspool, Msg->Name, ALL);
					
					if (attribute->Atom)
					{
						#ifdef DB_ADD
						
						IFEELIN F_Log
						(
							0, "attribute (0x%08lx)(%s) data (%s) value (%ld) type (0x%08lx)",
							
							attribute,
							attribute->Name->Key,
							attribute->Value,
							attribute->NumericValue,
							attribute->NumericType
						);
 
						#endif
						
						IFEELIN F_LinkTail(LOD->markup_cursor ? &LOD->markup_cursor->AttributesList : &LOD->attributes_list, (FNode *) attribute);
						
						return TRUE;
					}
				
					IFEELIN F_Dispose(attribute->Value);
				} 
			}
			
			IFEELIN F_Dispose(attribute);
		}
	}
	return FALSE;
}
//+
///XMLDocument_AddInt
F_METHODM(uint32,XMLDocument_AddInt,FS_XMLDocument_AddInt)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (Msg->Name && Msg->Value)
	{
		APTR pool = xmldocument_obtain_storage(Class,Obj);

		if (xmldocument_check_atomspool(Class, Obj) == NULL)
		{
			return FALSE;
		}

		if (pool)
		{
			FXMLAttribute *attribute = IFEELIN F_NewP(pool, sizeof (FXMLAttribute));

			if (attribute)
			{
				switch (Msg->Type)
				{
					case FV_TYPE_INTEGER:
					{
						attribute->Value = IFEELIN F_StrNewP(pool, NULL, "%ld", Msg->Value);
					}
					break;
					
					case FV_TYPE_HEXADECIMAL:
					{
						attribute->Value = IFEELIN F_StrNewP(pool, NULL, "#%08lx", Msg->Value);
					}
					break;
					 
					case FV_TYPE_BOOLEAN:
					{
						if (Msg->Value)
						{
							attribute->Value = IFEELIN F_StrNewP(pool, NULL, "true");
						}
						else
						{
							attribute->Value = IFEELIN F_StrNewP(pool, NULL, "false");
						}
					}
					break;

					#if F_CODE_DEPRECATED

					case FV_TYPE_4CC:
					{
						attribute->Value = IFEELIN F_NewP(pool, 5);
						
						if (attribute->Value)
						{
							*((uint32 *)(attribute->Value)) = Msg->Value;
						}
					}
					break;

					#endif
				}
			
				if (attribute->Value)
				{
					attribute->NumericValue = Msg->Value;
					attribute->NumericType = Msg->Type;
				
					/*
					attribute->Atom = (FAtom *) IFEELIN F_Do
					(
						Obj, F_RESOLVED_ID(OBTAINNAME),

						Msg->Name, IFEELIN F_StrLen(Msg->Name)
					);
					*/

					attribute->Atom = (FAtom *) IFEELIN F_AtomObtainP(LOD->atomspool, Msg->Name, ALL);

					if (attribute->Atom)
					{
						#ifdef DB_ADD

						IFEELIN F_Log
						(
							0, "attribute (0x%08lx)(%s) data (%s) value (%ld) type (0x%08lx)",

							attribute,
							attribute->Name->Key,
							attribute->Value,
							attribute->NumericValue,
							attribute->NumeircType
						);

						#endif

						IFEELIN F_LinkTail(LOD->markup_cursor ? &LOD->markup_cursor->AttributesList : &LOD->attributes_list, (FNode *) attribute);

						return TRUE;
					}

					IFEELIN F_Dispose(attribute->Value);
				}
			}

			IFEELIN F_Dispose(attribute);
		}
	}
	return FALSE;
}
//+

///XMLDocument_Write
F_METHODM(uint32, XMLDocument_Write, FS_XMLDocument_Write)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	uint32 rc = FALSE;
 
	if (Msg->Source)
	{
		if (Msg->SourceType == FV_Document_SourceType_File)
		{
			BPTR fh = IDOS_ Open(Msg->Source, MODE_NEWFILE);
				
			#ifdef DB_WRITE
			IFEELIN F_Log(0,"file (%s) handle (0x%08lx)",Msg->Source, fh);
			#endif
 
			if (fh)
			{
				IDOS_ FPrintf(fh, "<?xml version=\"1.0\" ?>\n\n");

				xmldocument_print_markups(&LOD->markups_list, fh, 0);
				
				IDOS_ Close(fh);
			}
		}
	}

	return rc;
}
//+
