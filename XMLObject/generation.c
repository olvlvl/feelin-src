#include "Private.h"
	
//#define DB_CREATE
//#define DB_REFERENCE
#define DB_CHECK_REFERENCE

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///get_type
STATIC STRPTR get_type(bits32 Type)
{
	if (FF_TYPE_LITERAL & Type)
	{
		if (FV_TYPE_STRING & Type)
		{
			return "FV_TYPE_STRING";
		}
		else
		{
			return "FF_TYPE_LITERAL";
		}
	}
	else if (FF_TYPE_BIT & Type)
	{
		if (FV_TYPE_BOOLEAN & Type)
		{
			return "FV_TYPE_BOOLEAN";
		}
		else if (FV_TYPE_FLAGS & Type)
		{
			return "FV_TYPE_FLAGS";
		}
		else
		{
			return "FF_TYPE_BIT";
		}
	}
	else if (FF_TYPE_NUMERIC & Type)
	{
		if (FV_TYPE_INTEGER & Type)
		{
			return "FV_TYPE_INTEGER";
		}
		else if (FV_TYPE_HEXADECIMAL & Type)
		{
			return "FV_TYPE_HEXADECIMAL";
		}
		else if (FV_TYPE_BINARY & Type)
		{
			return "FV_TYPE_BINARY";
		}
		else if (FV_TYPE_PERCENTAGE & Type)
		{
			return "FV_TYPE_PERCENTAGE";
		}
		else if (FV_TYPE_PIXEL & Type)
		{
			return "FV_TYPE_PIXEL";
		}
		else
		{
			return "FF_TYPE_NUMERIC";
		}
	}
	else if (FF_TYPE_REFERENCE & Type)
	{
		if (FV_TYPE_POINTER & Type)
		{
			return "FV_TYPE_POINTER";
		}
		else if (FV_TYPE_OBJECT & Type)
		{
			return "FV_TYPE_OBJECT";
		}
		else
		{
			return "FF_TYPE_REFERENCE";
		}
	}

	return "FV_TYPE_UNKOWN";
}
//+

/*** builtin object collection *****************************************************************/

typedef void (*FMakeFunc)(FObject Obj,FXMLMarkup *Markup,uint32 *Params);

///make_bartitle
STATIC void make_bartitle(FObject Obj,FXMLMarkup *Markup,uint32 *Params)
{
   *Params = (uint32) Markup->Body;
}
//+
///make_checkbox
STATIC void make_checkbox(FObject Obj,FXMLMarkup *Markup,uint32 *Params)
{
   *Params = IFEELIN F_Do(Obj,(uint32) "FM_Document_Resolve",Markup->Body,FV_TYPE_BOOLEAN,NULL,NULL);
}
//+
///make_hbox
STATIC void make_hbox(FObject Obj,FXMLMarkup *Markup,uint32 *Params)
{
   *Params++ = (uint32) FA_Area_Orientation;
   *Params   = (uint32) FV_Area_Orientation_Horizontal;
}
//+
///make_vbox
STATIC void make_vbox(FObject Obj,FXMLMarkup *Markup,uint32 *Params)
{
   *Params++ = (uint32) FA_Area_Orientation;
   *Params   = (uint32) FV_Area_Orientation_Vertical;
}
//+
///make_hlabel
STATIC void make_hlabel(FObject Obj,FXMLMarkup *Markup,uint32 *Params)
{
   *Params++ = (uint32) TRUE;
   *Params   = (uint32) Markup->Body;
}
//+
///make_button
STATIC void make_button(FObject Obj,FXMLMarkup *Markup,uint32 *Params)
{
   *Params   = (uint32) Markup->Body;
}
//+
///make_vlabel
STATIC void make_vlabel(FObject Obj,FXMLMarkup *Markup,uint32 *Params)
{
   *Params++ = (uint32) FALSE;
   *Params   = (uint32) Markup->Body;
}
//+

typedef struct FeelinXMLMake
{
	STRPTR                          Name;
	STRPTR                          ClassName;
	uint32                          ID;
	uint32                          Params;
	FMakeFunc                       write;
}
FXMLMake;

/* this array must be in alphabetical order */

STATIC FXMLMake xml_object_make[] =
{
	{ "BarTitle",       "Bar",   FV_MakeObj_BarTitle,    1, make_bartitle },
	{ "Button",   		"Text",  FV_MakeObj_Button,      1, make_button },
	{ "Checkbox",       "Image", FV_MakeObj_Checkbox,    1, make_checkbox },
	{ "HLabel",         "Text",  FV_MakeObj_Label,       2, make_hlabel },
	{ "VLabel",         "Text",  FV_MakeObj_Label,       2, make_vlabel },
	{ "hbox",           "Group", FV_MakeObj_None,        2, make_hbox },
	{ "vbox",           "Group", FV_MakeObj_None,        2, make_vbox },

	F_ARRAY_END
};

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///XMLObject_Create
F_METHODM(FObject,XMLObject_Create,FS_XMLObject_Create)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	STRPTR cl_name = Msg->Markup->Atom->Key;

	FXMLMake *make;
	FClass *cl;
	
	#ifdef DB_CREATE
	IFEELIN F_Log(0,"markup (%s)",Msg->Markup->Atom->Key);
	#endif 
 
/*** check markup ******************************************************************************/
  
	if (!Msg->Markup)
	{
		IFEELIN F_Log(FV_LOG_DEV,"Markup is NULL");
		
		return NULL;
	}
	
/*** check from built in collection ************************************************************/

	for (make = xml_object_make ; make->Name ; make++)
	{
		int32 cmp = IFEELIN F_StrCmp(make->Name, Msg->Markup->Atom->Key, ALL);

		if (cmp == 0)
		{
			cl_name = make->ClassName;
		
			break;
		}
		else if (cmp > 0)
		{
			make = NULL;
		
			break;
		}
	}

	if (make && !make->Name) make = NULL;

/*** open class ********************************************************************************/

	cl = IFEELIN F_OpenClass(cl_name);

	if (cl != NULL)
	{
		FXMLMarkup *markup;
		FXMLAttribute *attribute;
		uint32 i= make ? 1 + make->Params : 1;
		struct TagItem *tags; 

		struct in_Reference *reference_node = NULL;
		
		/* now that we have opened the class,  we  can  create  and  resolve
		attributes   and   parameters.  Note  well  the  difference  between
		parameters (uint32) and tags (struct TagItem *). If 'make->ID'  is
		different then FV_MakeObj_None (0) then we use F_MakeObj() to create
		the object, thats why parameters are so important.
		
		Free space is reserved for parameters, right before additional tags,
		which  must  be  filled  with  parameters for F_MakeObj() or default
		attributes for F_NewObj(). */
	 
		/* attributes */
   
		for (attribute = (FXMLAttribute *) Msg->Markup->AttributesList.Head ; attribute ; attribute = attribute->Next)
		{
			i += 2;
		}
					
		/* children */
   
		for (markup = (FXMLMarkup *) Msg->Markup->ChildrenList.Head ; markup ; markup = markup->Next)
		{
			i += 2;
		}
	
		i += 2; // FA_ID, NULL
	 
		/* we allocate enough memory for parameters (or default tags),  user
		attributes and TAG_DONE */

		tags = IFEELIN F_NewP(Msg->Pool, i * sizeof (uint32));

		if (tags != NULL)
		{
			struct TagItem *item = tags;
			FObject obj=NULL;
			
			/* if we  are  going  to  create  an  object  from  our  builtin
			collection,  we  call  the  function  'make->write' to write the
			appropriate parameters or tags before we add  markup  attributes
			and children */
			
			if (make)
			{
			   make->write(Obj, Msg->Markup, (uint32 *) item);
			   item = (struct TagItem *) ((uint32)(item) + make->Params * sizeof (uint32));
			}

			/* Now we parse and  add  markup  attributes.  Don't  forget  to
			handle  the  speciale  attribute  'name'  used  to reference the
			object. */
			
			for (attribute = (FXMLAttribute *)(Msg->Markup->AttributesList.Head) ; attribute ; attribute = attribute->Next)
			{
				volatile FClass *ca_class;
				FClassAttribute *ca;

				ca = IFEELIN F_DynamicFindAttribute(attribute->Atom->Key, cl, &ca_class);

				if (ca != NULL)
				{
					uint32 value = 0;
					bool32 resolve_ok = FALSE;

					#if 0

					IFEELIN F_Log
					(
						0, "class attribute (%s) expecting type (0x%08lx) - value (%s) numeric (%ld)(0x%08lx)",

						ca->Name,
						ca->Type,
						attribute->Value,
						attribute->NumericValue,
						attribute->NumericType
					);

					#endif

					if (attribute->NumericType)
					{
						// attribute value was successfuly been numerized

						if (FF_TYPE_LITERAL & ca->Type)
						{
							value = (uint32) attribute->Value;
						}
						else
						{
							value = attribute->NumericValue;
						}

						resolve_ok = TRUE;
					}
					else
					{
						// search for a matching special value

						if (ca->Values)
						{
							FClassAttributeValue *av;

							for (F_VALUES_EACH(ca->Values, av))
							{
								if (IFEELIN F_StrCmp(_value_name(av), attribute->Value, ALL) == 0)
								{
									break;
								}
							}

							if (_value_name(av))
							{
								// special value found

								value = av->Value;
								resolve_ok = TRUE;
							}
						}

						if (resolve_ok == FALSE)
						{
							if (FF_TYPE_LITERAL & ca->Type)
						{
								value = (uint32) attribute->Value;
								resolve_ok = TRUE;
							}
						}
					}

					if (resolve_ok == FALSE)
					{
						IFEELIN F_Do
						(
							Obj, F_RESOLVED_ID(LOG),

							attribute->Line, NULL,

							"(%s) is not a valid value for the FA_%s_%s attribute, of type (%s)",

							attribute->Value, ca_class->Name, _attribute_name(ca), get_type(ca->Type)
						);

						goto __error;
					}

					item->ti_Tag = ca->ID;
					item->ti_Data = value;

					/* If the Id attribute is defined, we create a reference
					node,  which  will later be filled with a pointer to the
					object we are creating.

					We also check if the Id has not yet been used */

					if (ca->ID == FA_Element_Id)
					{
						FAtom *atom = IFEELIN F_AtomObtain(attribute->Value, ALL);

						if (atom)
						{
							struct in_Reference *node;

							for (node = LOD->references ; node ; node = node->next)
							{
								if (node->atom == atom)
								{
									break;
								}
							}

							if (node != NULL)
							{
								IFEELIN F_AtomRelease(atom);

								IFEELIN F_Do
								(
									Obj, F_RESOLVED_ID(LOG),

									attribute->Line, NULL,

									"Reference id (%s) used multiple time", attribute->Value
								);

								goto __error;
							}

							reference_node = IFEELIN F_NewP(Msg->Pool, sizeof (struct in_Reference));

							if (reference_node)
							{
								reference_node->atom = atom;
									
								reference_node->next = LOD->references;
								LOD->references = reference_node;
							}
							else
							{
								IFEELIN F_AtomRelease(atom);

								IFEELIN F_Do
								(
									Obj, F_RESOLVED_ID(LOG),

									attribute->Line, NULL,

									"Unable to create Reference for id (%s)", attribute->Value
								);

								goto __error;
							}
						}
						else
						{
							IFEELIN F_Do
							(
								Obj, F_RESOLVED_ID(LOG),

								attribute->Line, NULL,

								"Unable to create Atom for id (%s)", attribute->Value
							);

							goto __error;
						}
					}

					item++;
				}
				else
				{
					IFEELIN F_Do(Obj, F_RESOLVED_ID(LOG), attribute->Line, NULL, "Attribute '%s' not found in '%s' class", attribute->Atom->Key, cl->Name);

					goto __error;
				}
			}
			
			/* create and add children to the taglist */

			for (markup = (FXMLMarkup *) Msg->Markup->ChildrenList.Head ; markup ; markup = markup->Next)
			{
				FObject child = (FObject) IFEELIN F_Do(Obj, F_METHOD_ID(CREATE), markup, Msg->Pool);

				if (child)
				{
					item->ti_Tag  = FA_Child;
					item->ti_Data = (uint32) child;
					item++;
				}
				else goto __error;
			}
		 
			/* Eveything  is  ready,  we  can  create  the  object  now.  If
			'make->ID' we use F_MakeObj(), otherwise F_NewObj(). */

			if (make && make->ID)
			{
				obj = IFEELIN F_MakeObjA(make->ID, (uint32 *) tags);
			}
			else
			{
				obj = IFEELIN F_NewObjA(cl->Name, tags);
			}
		 
			if (obj)
			{
				if (reference_node)
				{
					reference_node->object = obj;
				}
			 
				IFEELIN F_Dispose(tags);

				IFEELIN F_CloseClass(cl);
				 
				return obj;
			}

/*** error handling ****************************************************************************/

__error:

			if (obj)
			{
				IFEELIN F_DisposeObj(obj);
			}
			else if (tags)
			{
				struct TagItem *item;
				struct TagItem *next = (make) ? (struct TagItem *)((uint32)(tags) + make->Params * sizeof (uint32)) : tags;

				/* if the object has not been created  we  need  to  dispose
				each child found in the taglist */

				while ((item = IUTILITY NextTagItem(&next)) != NULL)
				{
				   if (item->ti_Tag == FA_Child)
				   {
					  IFEELIN F_DisposeObj((FObject) item->ti_Data); item->ti_Tag = TAG_IGNORE;
				   }
				}
			}
	
			IFEELIN F_Dispose(tags);
		}
		
		IFEELIN F_CloseClass(cl);
	}
	else
	{
		IFEELIN F_Log(0,"unable to open class (0x%08lx)(%s)\nMarkup (0x%08lx) Key (0x%08lx)(%s)",cl_name,cl_name,Msg->Markup,Msg->Markup->Atom->Key,Msg->Markup->Atom->Key);
	}
	return NULL;
}
//+

