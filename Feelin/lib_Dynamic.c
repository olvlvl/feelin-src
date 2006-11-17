/*
**    lib_Dynamic.c
**
**    Heart of the Dynamic ID system
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************
						
$VER: 08.03 (2005/09/30)

	Added AmigaOS 4 support.
 
************************************************************************************************/

#include "Private.h"

#define FV_ID_TODO                              -1
#define FV_ID_DONE                              0

/* FV_ID_TODO & FV_ID_DONE are used by Dynamic table  resolvers.  FV_ID_TODO
is  used  to  mark  entries  to  resolve. FV_ID_DONE is used to mark invalid
entries. This may happen is a Dynamic ID has not been found. */

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///f_dynamic_log_defined
STATIC void f_dynamic_log_defined
(
	STRPTR Name,
	uint8 Type,
	FClass *Class,
	struct in_FeelinBase *FeelinBase
)
{
	IFEELIN F_Log(FV_LOG_DEV,"F_Dynamic() - '%s' (0x%08.lx) does not exists in FC_%s.",Name,Name,Class->Name);
	IDOS_ FPrintf(FeelinBase->Public.Console,"Defined %s: ",(Type == 'A') ? "Attributes" : "Methods");

	if (Type == 'A')
	{
		FClassAttribute *en;

		for (F_ATTRIBUTES_EACH(Class->Attributes, en))
		{
			IDOS_ FPrintf(FeelinBase->Public.Console,"%s ", _attribute_name(en));
		}
	}
	else
	{
		FClassMethod *en;

		for (F_METHODS_EACH(Class->Methods, en))
		{
			if (_method_name(en))
			{
				IDOS_ FPrintf(FeelinBase->Public.Console,"%s ", _method_name(en));
			}
		}
	}
	
	IDOS_ FPrintf(FeelinBase->Public.Console,"\n");
	
#ifdef __amigaos4__
	IDOS_ FFlush(FeelinBase->Public.Console);
#else
	IDOS_ Flush(FeelinBase->Public.Console);
#endif
}
//+

/************************************************************************************************
*** Shared **************************************************************************************
************************************************************************************************/

///f_dynamic_find_name
FClass * f_dynamic_find_name
(
	STRPTR String,
	struct in_FeelinBase *FeelinBase
)
{
	FAtom *atom;

	STRPTR str = String;

	while (*str && *str != '_') str++;

	atom = IFEELIN F_AtomFind(String, str - String);

	if (atom)
	{
		FClass *node;

		for (F_CLASSES_EACH(node))
		{
			if (atom == node->Atom)
			{
				break;
			}
		}

		return node;
	}

	return NULL;
}
//+

/************************************************************************************************
*** API *****************************************************************************************
************************************************************************************************/

///f_dynamic_create
F_LIB_DYNAMIC_CREATE
/*

	If a class defines either attributes or methods, a Dynamic ID  is  given
	to  the  class  as  a basis for its attributes and methods IDs. Since v8
	this is the case of almost all  classes,  because  dynamic  methods  are
	defined  in  the  same  structure as method handlers. Thus, only classes
	using a dispatcher but defining no attributes nor methods are considered
	static.

*/
{
	int32 rc = TRUE;
	int32 dynamic_id_used = FALSE;
	uint32 class_id = 0;

	if (Class == NULL)
	{
		return FALSE;
	}

	F_DYNAMIC_LOCK;
	
	if (Class->Attributes || Class->Methods)
	{
		FClass *node;

		for (F_CLASSES_EACH(node))
		{
			if ((node != Class) && (((struct in_FeelinClass *) node)->id == class_id))
			{
				class_id += FV_Class_DynamicSpace;
			}
		}
	}

/*** resolve attributes ************************************************************************/

	if (Class->Attributes)
	{
		FClassAttribute *en;
		uint32 id = DYNA_ATTR | class_id;

		for (F_ATTRIBUTES_EACH(Class->Attributes, en))
		{
			/* Attributes that use  a  static  ID  e.g.  FA_Right  are  left
			untouched.  Thus,  an  attribute  can have a Dynamic name, but a
			static numerical representation */
			
			if (_attribute_name(en)[0] >= 'A' && _attribute_name(en)[0] <= 'Z')
			{
				if ((0xFF000000 & en->ID) != ATTR_BASE)
				{
					en->ID = id++;

					dynamic_id_used = TRUE;
				}

				#ifdef F_NEW_ATOMS_AMV

				_attribute_atom(en) = IFEELIN F_AtomObtain(_attribute_name(en), ALL);

				if (_attribute_atom(en) == NULL)
				{
					goto __error;
				}

				if (en->Values)
				{
					FClassAttributeValue *val;

					for (F_VALUES_EACH(en->Values, val))
					{
						_value_atom(val) = IFEELIN F_AtomObtain(_value_name(val), ALL);

						if (_value_atom(val) == NULL)
						{
							goto __error;
						}
					}
				}

				#endif
			}
			else
			{
				IFEELIN F_Log(FV_LOG_CORE,"Invalid attribute name (0x%08lx)(%s)", _attribute_name(en), _attribute_name(en));
				
				goto __error;
			}
		}
	}

/*** resolve methods ***************************************************************************/

	if (Class->Methods)
	{
		FClassMethod *en;
		uint32 id = DYNA_MTHD | class_id;

		for (F_METHODS_EACH(Class->Methods, en))
		{
			/* Methods that use a static ID e.g. FM_New are left  untouched.
			Thus,  a  method can have a Dynamic name, but a static numerical
			representation */

			if (_method_name(en)) 
			{
				#ifdef F_NEW_ATOMS_AMV

				_method_atom(en) = IFEELIN F_AtomObtain(_method_name(en), ALL);

				if (_method_atom(en) == NULL)
				{
					goto __error;
				}

				#endif

				if ((en->ID & 0xFF000000) != MTHD_BASE)
				{
					/* A method can be inherited by simply suppling its whole  e.g.
					"FM_Object_New".  Otherwise the string supplied here is used as
					the name of the method. */
					
					if (_method_name(en)[0] >= 'A' && _method_name(en)[0] <= 'Z')
					{
						if (_method_name(en)[2] == '_' && _method_name(en)[0] == 'F' && _method_name(en)[1] == 'M')
						{
							FClassMethod *m = IFEELIN F_DynamicFindMethod(_method_name(en), Class->Super, NULL);

							if (m)
							{
								en->ID = m->ID;
							}
						}
						else
						{
							en->ID = id++;

							dynamic_id_used = TRUE;

							#if 0//def F_NEW_ATOMS_AMV

							_method_atom(en) = IFEELIN F_AtomObtain(_method_name(en), ALL);

							if (_method_atom(en) == NULL)
							{
								goto __error;
							}

							#endif
						}
					}
					else
					{
						IFEELIN F_Log(FV_LOG_CORE,"Invalid method name (0x%08lx)(%s)",_method_name(en), _method_name(en));

						goto __error;
					}
				}
			}
		}
	}

	/* if the class dynamic id has not be used, we reset it to  0  to  avoid
	wasting dynamic space */
 
	if (dynamic_id_used)
	{
		((struct in_FeelinClass *) Class)->id = class_id;
	}

	IFEELIN F_DynamicResolveTable(Class->Resolveds);
	
	f_dynamic_auto_add_class(Class, FeelinBase);

	if (Class->Autos)
	{
		rc = IFEELIN F_DynamicAddAutoTable(Class->Autos);
	}

	F_DYNAMIC_UNLOCK;

	return rc;

__error:

	F_DYNAMIC_UNLOCK;

	return FALSE;
}
//+
///f_dynamic_delete
F_LIB_DYNAMIC_DELETE
{
	if (Class)
	{
		F_CLASSES_LOCK;
		F_DYNAMIC_LOCK;

		if (Class->Autos)
		{
			IFEELIN F_DynamicRemAutoTable(Class->Autos);
		}

		#ifdef F_NEW_ATOMS_AMV

		if (Class->Attributes)
		{
			FClassAttribute *node;

			for (F_ATTRIBUTES_EACH(Class->Attributes, node))
			{
				if (_attribute_atom(node))
				{
					IFEELIN F_AtomRelease(_attribute_atom(node));

					_attribute_atom(node) = NULL;

					if (node->Values)
					{
						FClassAttributeValue *value;

						for (F_VALUES_EACH(node->Values, value))
						{
							if (_value_atom(value))
							{
								IFEELIN F_AtomRelease(_value_atom(value));

								_value_atom(value) = NULL;
							}
						}
					}
				}
			}
		}

		if (Class->Methods)
		{
			FClassMethod *node;

			for (F_METHODS_EACH(Class->Methods, node))
			{
				if (_method_atom(node))
				{
					IFEELIN F_AtomRelease(_method_atom(node));

					_method_atom(node) = NULL;
				}
			}
		}

		#endif

		F_DYNAMIC_UNLOCK;
		F_CLASSES_UNLOCK;
	}
}
//+
///f_dynamic_find_attribute
F_LIB_DYNAMIC_FIND_ATTRIBUTE
{
	if (Name)
	{
		STRPTR back = Name;

		#ifdef F_ENABLE_SAFE_STRING

		if (((int32)(Name) < 0xFFFF) || (IEXEC TypeOfMem(Name) == 0))
		{
			IFEELIN F_Log(0,"(db)F_FindAttribute() invalid string address (0x%08lx)", Name);

			return NULL;
		}

		#endif
 
		if (Name[2] == '_' && Name[1] == 'A' && Name[0] == 'F')
		{

			/* if a complete name is supplied, 'FromClass'  is  replaced  with
			the  correct  class e.g. for "FA_Area_Left", 'FromClass' will be a
			pointer to the class "Area". Then 'Name' adjusted to the last part
			e.g. "Left". */

			Name += 3;

			FromClass = f_dynamic_find_name(Name, FeelinBase);

			if (FromClass == NULL)
			{
				IFEELIN F_Log(0,"F_DynamicFindAttribute() (0x%08lx)(%s) not found", back, back);

				return NULL;
			}

			while (*Name != '_') Name++; Name++;
				
			if (FromClass->Attributes)
			{
				#ifdef F_NEW_ATOMS_AMV

				FAtom *atom = IFEELIN F_AtomFind(Name, ALL);

				if (atom)
				{
					FClassAttribute *en;

					for (F_ATTRIBUTES_EACH(FromClass->Attributes, en))
					{
						if (atom == _attribute_atom(en))
						{
							if (RealClass)
							{
								*RealClass = FromClass;
							}

							return en;
						}
					}
				}

				#else

				FClassAttribute *en;

				for (F_ATTRIBUTES_EACH(FromClass->Attributes, en))
				{
					if (IFEELIN F_StrCmp(Name, _attribute_name(en), ALL) == 0)
					{
						if (RealClass)
						{
							*RealClass = FromClass;
						}

						return en;
					}
				}

				#endif
			}
		}
		else
		{
			while (FromClass)
			{
				if (FromClass->Attributes)
				{
					#ifdef F_NEW_ATOMS_AMV

					FAtom *atom = IFEELIN F_AtomFind(Name, ALL);

					if (atom)
					{
						FClassAttribute *en;

						for (F_ATTRIBUTES_EACH(FromClass->Attributes, en))
						{
							//IFEELIN F_Log(0,"atom (0x%08lx)(%s) -- attribute (0x%08lx)(%s)", atom, atom->Key, _attribute_atom(en), _attribute_name(en));

							//if (IFEELIN F_StrCmp(Name, _attribute_name(en), ALL) == 0)
							if (atom == _attribute_atom(en))
							{
								if (RealClass)
								{
									*RealClass = FromClass;
								}

								return en;
							}
						}
					}

					#else

					FClassAttribute *en;

					for (F_ATTRIBUTES_EACH(FromClass->Attributes, en))
					{
						if (IFEELIN F_StrCmp(Name, _attribute_name(en), ALL) == 0)
						{
							if (RealClass)
							{
								*RealClass = FromClass;
							}

							return en;
						}
					}

					#endif
				}

				FromClass = FromClass->Super;
			}
		}
		
		IFEELIN F_Log(FV_LOG_DEV,"F_DynamicFindAttribute() (0x%08lx)(%s) not found",back,back);
	}
	return NULL;
}
//+
///f_dynamic_find_method
F_LIB_DYNAMIC_FIND_METHOD
{
	if (Name)
	{
		STRPTR back = Name;

		#ifdef F_ENABLE_SAFE_STRING

		if (((int32)(Name) < 0xFFFF) || (IEXEC TypeOfMem(Name) == 0))
		{
			IFEELIN F_Log(0,"(db)F_FindMethod() invalid string address (0x%08lx)",Name);

			return NULL;
		}

		#endif
 
		if (Name[2] == '_' && Name[1] == 'M' && Name[0] == 'F')
		{

			/* if a complete name is supplied, 'FromClass'  is  replaced  with
			the  correct class e.g. for "FM_Area_Setup", 'FromClass' will be a
			pointer to the class "Area". Then 'Name' adjusted to the last part
			e.g. "Setup". */

			Name += 3;

			FromClass = f_dynamic_find_name(Name,FeelinBase);

			if (FromClass == NULL)
			{
				IFEELIN F_Log(0,"F_DynamicFindMethod() (0x%08lx)(%s) not found", back, back);

				return NULL;
			}

			while (*Name != '_') Name++; Name++;
				
			if (FromClass->Methods)
			{
				#ifdef F_NEW_ATOMS_AMV

				FAtom *atom = IFEELIN F_AtomFind(Name, ALL);

				if (atom)
				{
					FClassMethod *en;

					for (F_METHODS_EACH(FromClass->Methods, en))
					{
						if (atom == _method_atom(en))
						{
							if (RealClass)
							{
								*RealClass = FromClass;
							}

							return en;
						}
					}
				}

				#else

				FClassMethod *en;

				for (F_METHODS_EACH(FromClass->Methods, en))
				{
					if (IFEELIN F_StrCmp(Name, _method_name(en), ALL) == 0)
					{
						if (RealClass)
						{
							*RealClass = FromClass;
						}

						return en;
					}
				}

				#endif
			}
		}
		else
		{
			while (FromClass)
			{
				if (FromClass->Methods)
				{
					#ifdef F_NEW_ATOMS_AMV

					FAtom *atom = IFEELIN F_AtomFind(Name, ALL);

					if (atom)
					{
						FClassMethod *en;

						for (F_METHODS_EACH(FromClass->Methods, en))
						{
							if (atom == _method_atom(en))
							{
								if (RealClass)
								{
									*RealClass = FromClass;
								}

								return en;
							}
						}
					}

					#else

					FClassMethod *en;

					for (F_METHODS_EACH(FromClass->Methods, en))
					{
						if (IFEELIN F_StrCmp(Name, _method_name(en), ALL) == 0)
						{
							if (RealClass)
							{
								*RealClass = FromClass;
							}

							return en;
						}
					}

					#endif
				}

				FromClass = FromClass->Super;
			}
		}
		
		IFEELIN F_Log(FV_LOG_DEV,"F_DynamicFindMethod() '%s' not found", back);
	}
	return NULL;
}
//+
///f_dynamic_find_id
F_LIB_DYNAMIC_FIND_ID
{
	struct FeelinClass *cl;
	uint8 type;
	uint8 *name;

	#ifdef DB_FINDID
	IFEELIN F_Log(0,"F_DynamicFindID() - Name '%s' (0x%lx)",Name,Name);
	#endif

	if (!Name) return 0;

	switch ((uint32)(Name) & 0xFF000000)
	{
		case TAG_USER:
		case MTHD_BASE:
		case ATTR_BASE:
		case DYNA_MTHD:
		case DYNA_ATTR: return (uint32)(Name);
	}

	#ifdef F_ENABLE_SAFE_STRING

	if (((int32)(Name) < 0xFFFF) || (IEXEC TypeOfMem(Name) == 0))
	{
		IFEELIN F_Log(0,"(db)F_DynamicFindID() invalid string address (0x%08lx)",Name);

		return 0;
	}

	#endif

	type = Name[1];

	if (Name[2] != '_' || Name[0] != 'F' || (type != 'A' && type != 'M'))
	{
		IFEELIN F_Log(FV_LOG_DEV,"F_DynamicFindID() - Incorrect name '%s' (0x%08.lx)", Name, Name);

		return -1;
	}

	cl = f_dynamic_find_name(name = Name + 3, FeelinBase);

	if (cl)
	{
		while (*name != '_') name++; name++;
		
		#ifdef DB_FINDID
		F_Log(0,"Entries 0x%lx (0x%lx - 0x%lx)",entry, cl->Methods, cl->Attributes);
		#endif

		if (type == 'A')
		{
			#ifdef F_NEW_ATOMS_AMV

			FAtom *atom = IFEELIN F_AtomFind(name, ALL);

			if (atom)
			{
				FClassAttribute *en;

				for (F_ATTRIBUTES_EACH(cl->Attributes, en))
				{
					if (atom == _attribute_atom(en))
					{
						return en->ID;
					}
				}
			}

			#else

			FClassAttribute *en;

			for (F_ATTRIBUTES_EACH(cl->Attributes, en))
			{
				if (IFEELIN F_StrCmp(name, _attribute_name(en), ALL) == 0)
				{
					return en->ID;
				}
			}

			#endif
		}
		else
		{
			#ifdef F_NEW_ATOMS_AMV

			FAtom *atom = IFEELIN F_AtomFind(name, ALL);

			if (atom)
			{
				FClassMethod *en;

				for (F_METHODS_EACH(cl->Methods, en))
				{
					if (atom == _method_atom(en))
					{
						return en->ID;
					}
				}
			}

			#else

			FClassMethod *en;

			for (F_METHODS_EACH(cl->Methods, en))
			{
				if (IFEELIN F_StrCmp(name, _method_name(en), ALL) == 0)
				{
					return en->ID;
				}
			}

			#endif
		}

		f_dynamic_log_defined(Name,type,cl,FeelinBase);
	}
	else
	{
		IFEELIN F_Log(FV_LOG_DEV,"F_DynamicFindID() - Unable to find '%s' (0x%08.lx)",Name,Name);
	}
	return 0;
}
//+

///f_dynamic_resolve_table

enum  {
	
		FV_RESOLVE_TYPE_INVALID,
		FV_RESOLVE_TYPE_ATTRIBUTE,
		FV_RESOLVE_TYPE_METHOD

		};
 
F_LIB_DYNAMIC_RESOLVE_TABLE
{
	uint32 n=0;
	
	if (Entries)
	{
		FClass *cl = NULL;
		FDynamicEntry *en;
		
		for (en = Entries ; en->Name ; en++)
		{
			uint32 type = FV_RESOLVE_TYPE_INVALID;
			
			if (en->Name[0] == 'F' && en->Name[2] == '_')
			{
				switch (en->Name[1])
				{
					case 'A': type = FV_RESOLVE_TYPE_ATTRIBUTE; break;
					case 'M': type = FV_RESOLVE_TYPE_METHOD; break;
				}
			}
		
			if (type)
			{
				if (cl)
				{
					if (IFEELIN F_StrCmp(en->Name + 3, cl->Name, ALL) != '_')
					{
						cl = NULL;
					}
				}
					 
				if (!cl)
				{
					cl = f_dynamic_find_name(en->Name + 3, FeelinBase);
				}
				
				if (cl)
				{
					STRPTR name = en->Name + 3;

					while (*name != '_') name++; name++;
					
					if (type == FV_RESOLVE_TYPE_ATTRIBUTE && cl->Attributes)
					{
						#ifdef F_NEW_ATOMS_AMV

						FAtom *atom = IFEELIN F_AtomFind(name, ALL);

						if (atom)
						{
							FClassAttribute *attribute;

							for (F_ATTRIBUTES_EACH(cl->Attributes, attribute))
							{
								if (atom == _attribute_atom(attribute))
								{
									en->ID = attribute->ID; n++; break;
								}
							}

							if (_attribute_name(attribute) == NULL)
							{
								IFEELIN F_Log(FV_LOG_CLASS,"Attribute (%s) not defined by Class (%s)",en->Name,cl->Name);

								en->ID = 0;
							}
						}

						#else

						FClassAttribute *attribute;

						for (F_ATTRIBUTES_EACH(cl->Attributes, attribute))
						{
							if (IFEELIN F_StrCmp(_attribute_name(attribute), name, ALL) == 0)
							{
								en->ID = attribute->ID; n++; break;
							}
						}

						if (_attribute_name(attribute) == NULL)
						{
							IFEELIN F_Log(FV_LOG_CLASS,"Attribute (%s) not defined by Class (%s)",en->Name,cl->Name);

							en->ID = 0;
						}

						#endif
					}
					else if (type == FV_RESOLVE_TYPE_METHOD && cl->Methods)
					{
						#ifdef F_NEW_ATOMS_AMV

						FAtom *atom = IFEELIN F_AtomFind(name, ALL);

						if (atom)
						{
							FClassMethod *method;

							for (F_METHODS_EACH(cl->Methods, method))
							{
								if (atom == _method_atom(method))
								{
									en->ID = method->ID; n++; break;
								}
							}

							if (_method_name(method) == NULL)
							{
								IFEELIN F_Log(FV_LOG_CLASS,"Method (%s) not defined by Class (%s)", en->Name, cl->Name);

								en->ID = 0;
							}
						}

						#else

						FClassMethod *method;
						
						for (F_METHODS_EACH(cl->Methods, method))
						{
							if (IFEELIN F_StrCmp(_method_name(method), name, ALL) == 0)
							{
								en->ID = method->ID; n++; break;
							}
						}

						if (_method_name(method) == NULL)
						{
							IFEELIN F_Log(FV_LOG_CLASS,"Method (%s) not defined by Class (%s)", en->Name, cl->Name);

							en->ID = 0;
						}

						#endif
					}
					else
					{
						IFEELIN F_Log(FV_LOG_CLASS,"Unable to resolve (%s), Class (%s) defines none", en->Name, cl->Name);
					}
				}
				else
				{
					IFEELIN F_Log(FV_LOG_CLASS,"Unable to find class to resolve (%s)", en->Name);
				}
			}
			else IFEELIN F_Log(FV_LOG_CLASS,"Invalid name (0x%08lx)(%s)", en->Name, en->Name);
		}
	}
	return n;
}
//+

///f_dynamic_nti
F_LIB_DYNAMIC_NTI
{
	if(!(*TLP)) return NULL;

	item->ti_Tag  = 0;
	item->ti_Data = 0;

	for (;;)
	{
		switch ((*TLP)->ti_Tag)
		{
			case TAG_MORE: if (!((*TLP) = (struct TagItem *)(*TLP)->ti_Data)) return NULL; continue;
			case TAG_IGNORE: break;
			case TAG_END: (*TLP) = NULL; return NULL;
			case TAG_SKIP: (*TLP) += (*TLP)->ti_Data + 1; continue;

			default:
			{
				/* Use post-increment (return will return the current value and
				then tagListPtr will be incremented) */

				item->ti_Tag  = (*TLP)->ti_Tag;
				item->ti_Data = (*TLP)->ti_Data;

				if (!(TAG_USER & item->ti_Tag))
				{
					FClass *real_class=NULL;
					FClassAttribute *attr = IFEELIN F_DynamicFindAttribute((STRPTR) item->ti_Tag, (FClass *) Class, &real_class);

					if (attr)
					{
						item->ti_Tag = attr->ID;

						if (attr->Values && IEXEC TypeOfMem((APTR) item->ti_Data))
						{
							#ifdef F_NEW_ATOMS_AMV

							FAtom *atom = IFEELIN F_AtomFind((STRPTR) item->ti_Data, ALL);

							if (atom)
							{
								FClassAttributeValue *val;

								for (F_VALUES_EACH(attr->Values, val))
								{
									if (atom == _value_atom(val))
									{
										item->ti_Data = val->Value;

										break;
									}
								}
							}

							#else

							FClassAttributeValue *val;

							for (F_VALUES_EACH(attr->Values, val))
							{
								if (IFEELIN F_StrCmp((STRPTR)(item->ti_Data), _value_name(val), ALL) == 0)
								{
									item->ti_Data = val->Value;

									break;
								}
							}

							#endif
						}
					}
					else
					{
						item->ti_Tag = TAG_IGNORE;
					}
				}
				
				/* save values in the real  tag,  so  that  we  don't  have  to
				resolve everything again */
 
				(*TLP)->ti_Tag  = item->ti_Tag;
				(*TLP)->ti_Data = item->ti_Data;

				if (item->ti_Tag != TAG_IGNORE)
				{
					if (Class)
					{
						if (item->ti_Tag >=  (DYNA_ATTR | Class->id) &&
							 item->ti_Tag <  ((DYNA_ATTR | Class->id) + FV_Class_DynamicSpace))
						{
							item->ti_Tag = (0x00FFFFFF & item->ti_Tag) - Class->id;
						}
					}
					return (struct TagItem *)(*TLP)++;
				}
			}
		}
		(*TLP)++;
	}
}
//+
///f_dynamic_fti
F_LIB_DYNAMIC_FTI
{
	if(!(Tags)) return NULL;

	Attribute = IFEELIN F_DynamicFindID((STRPTR)(Attribute));

	for (;;)
	{
		switch (Tags->ti_Tag)
		{
			case TAG_MORE:
			{
				if (!(Tags = (struct TagItem *)(Tags->ti_Data)))
				{
					return NULL;
				}
			}
			continue;

			case TAG_IGNORE: break;
			case TAG_END: return Tags = NULL;

			case TAG_SKIP:
			{
				Tags += Tags->ti_Data + 1;
			}
			continue;

			default:
			{
				uint32 id = IFEELIN F_DynamicFindID((STRPTR)(Tags->ti_Tag));

				if (!id)
				{
					id = Tags->ti_Tag;
				}

				if (id == Attribute) return Tags;
			}
		}
		Tags++;
	}
}
//+
///f_dynamic_gtd
F_LIB_DYNAMIC_GTD
{
	struct TagItem *item = IFEELIN F_DynamicFTI(Attribute,Tags);

	if (item)
	{
		return item->ti_Data;
	}
	else
	{
		return Default;
	}
}
//+

