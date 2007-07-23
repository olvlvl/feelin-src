#include "Private.h"

/* !! declaration can only be deleted from their owner. Because  styles  can
use  any  declarations  of  the  tree,  it's  might  be  dangerous  deleting
declarations from styles */

//#define F_ENABLE_DELETE_UNUSED_STYLES

//#define DB_COUNT

//#define DB_DECLARATION_CREATE
//#define DB_DECLARATION_DELETE
//#define DB_DECLARATION_FLUSH

//#define DB_STYLE_CREATE
//#define DB_STYLE_RELEASE
//#define DB_STYLE_DELETE
//#define DB_STYLE_OBTAIN
//#define DB_STYLE_FLUSH

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

/*** declarations ******************************************************************************/

///typedefs

/*

	une fois l'objet CSSDocument prêt, on  clone  toutes  les  déclarations.
	elles  sont  liées  dans 'declarations'. les propriétés sont crées en un
	seul bloc (un tableau). On utilise pas de liste pour aller le plus  vite
	possible.  De  plus,  l'utilisation  d'une liste est superflue vu qu'une
	fois le style crée il ne peut pas être modifié.

	*attention* au membre 'atom_pseudo'  !  il  différencie  la  délcaration
	principale et les différents pseudo qu'elle peut posséder.


*/

struct in_Declaration
{
	struct in_Declaration          *next;
	struct in_Declaration          *prev;

	FAtom                          *atom;
	FAtom                          *atom_pseudo;

	uint32                          refs_count;

	FPreferenceProperty           **properties;
	uint32                          properties_count;

	/* pointers array following */
	/* properties following */
};

//+

///declaration_create

/*

	It's  a  bit   tricky,   but   if   CSSMasterDeclaration   is   defined,
	CSSDeclaration  is  in  fact  a  pseudo  for  the declaration defined by
	CSSMasterDeclaration.

*/

bool32 declaration_create(FClass *Class, FObject Obj, FCSSDeclaration *CSSDeclaration, FCSSDeclaration *CSSMasterDeclaration)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FCSSProperty *cssproperty;

	uint32 n = 0;

	for (cssproperty = (FCSSProperty *) CSSDeclaration->PropertiesList.Head ; cssproperty ; cssproperty = (FCSSProperty *) cssproperty->Node.Next)
	{
		n++;
	}

	if (n != 0)
	{
		struct in_Declaration *declaration = IFEELIN F_NewP
		(
			LOD->pool,

			sizeof (struct in_Declaration) +
			sizeof (FPreferenceProperty *) * (n + 1) +
			sizeof (FPreferenceProperty) * n
		);

		if (declaration != NULL)
		{
			declaration->atom = IFEELIN F_AtomObtain(CSSDeclaration->Atom->Key, CSSDeclaration->Atom->KeyLength);

			if (CSSMasterDeclaration != NULL)
			{
				declaration->atom_pseudo = declaration->atom;
				declaration->atom = IFEELIN F_AtomObtain(CSSMasterDeclaration->Atom->Key, CSSMasterDeclaration->Atom->KeyLength);

				if (declaration->atom == NULL)
				{
					IFEELIN F_AtomRelease(declaration->atom_pseudo);

					declaration->atom_pseudo = NULL;
				}
			}

			if (declaration->atom != NULL)
			{
				FPreferenceProperty **pointers = (FPreferenceProperty **) ((uint32)(declaration) + sizeof (struct in_Declaration));
				FPreferenceProperty *array = (FPreferenceProperty *) ((uint32)(pointers) + sizeof (FPreferenceProperty *) * (n + 1));

				declaration->properties = pointers;
				declaration->properties_count = n;

				for (cssproperty = (FCSSProperty *) CSSDeclaration->PropertiesList.Head ; cssproperty ; cssproperty = (FCSSProperty *) cssproperty->Node.Next)
				{
					FPreferenceProperty *property = array++;

					*pointers++ = property;

					property->Atom = IFEELIN F_AtomObtain(cssproperty->Atom->Key, cssproperty->Atom->KeyLength);
					property->Value = IFEELIN F_StrNewP(LOD->pool, NULL, "%s", cssproperty->Value);
					property->NumericValue = cssproperty->NumericValue;
					property->NumericType = cssproperty->NumericType;

					if ((property->Atom == NULL) || (property->Value == NULL))
					{
						break;
					}

					#if 0//def DB_DECLARATION_CREATE

					IFEELIN F_Log
					(
						0, "property (%32.32s) @ (0x%08lx) [%-16.16s] numeric (%ld) type (0x%08lx)",

						property->Atom->Key,
						property,
						property->Value,
						property->NumericValue, property->NumericType
					);

					#endif
				}

				if (cssproperty == NULL)
				{

					/*  we've  been  through  the  CSSDeclaration's  properties,
					everything if fine then. We just have to link the preference
					declaration and return */

					#ifdef DB_DECLARATION_CREATE

					if (declaration->atom_pseudo != NULL)
					{
						IFEELIN F_Log
						(
							0, "declaration (%s) pseudo (%s) has (%ld) properties @ (0x%08lx)",

							declaration->atom->Key,
							declaration->atom_pseudo->Key,
							declaration->properties_count,
							declaration->properties
						);
					}
					else
					{
						IFEELIN F_Log
						(
							0, "declaration (%s) has (%ld) properties @ (0x%08lx)",

							declaration->atom->Key,
							declaration->properties_count,
							declaration->properties
						);
					}

					#endif

					IFEELIN F_LinkHead(&LOD->declaration_list, (FNode *) declaration);

					return TRUE;
				}

				/* 'cssproperty' is not NULL. We've had difficulties  allocation
				a value or obtaining an atom. */

				for (pointers = declaration->properties ; *pointers ; pointers++)
				{
					FPreferenceProperty *property = *pointers;

					if (property->Atom != NULL)
					{
						IFEELIN F_AtomRelease(property->Atom);
					}

					if (property->Value != NULL)
					{
						IFEELIN F_Dispose(property->Value);
					}
				}

				IFEELIN F_AtomRelease(declaration->atom);
			}

			IFEELIN F_Dispose(declaration);
		}
	}
	#ifdef DB_DECLARATION_CREATE
	else
	{
		IFEELIN F_Log(0,"declaration (%s) is empty", CSSDeclaration->Atom->Key);
	}
	#endif

	return FALSE;
}
//+
///declaration_delete
struct in_Declaration * declaration_delete(FClass *Class, FObject Obj, struct in_Declaration *Declaration)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	#if 0

	struct in_Declaration *node;

	for (node = (struct in_Declaration *) LOD->declaration_list.Head ; node ; node = node->next)
	{
		if (node == Declaration)
		{
			break;
		}
	}

	if (node)
	#endif
	{
		FPreferenceProperty **array;

		IFEELIN F_LinkRemove(&LOD->declaration_list, (FNode *) Declaration);

		for (array = Declaration->properties ; *array ; array++)
		{
			IFEELIN F_AtomRelease((*array)->Atom);
			IFEELIN F_Dispose((*array)->Value);
		}

		#ifdef DB_DECLARATION_DELETE

		IFEELIN F_Log(0,"delete declaration (0x%08lx) atom (0x%08lx) (%s) index (0x%08lx)",

			Declaration,
			Declaration->atom,
			Declaration->atom->Key ? Declaration->atom->Key : "deprecated",
			LOD->declaration_list.Head
			);

		#endif

		IFEELIN F_AtomRelease(Declaration->atom_pseudo);

		IFEELIN F_AtomRelease(Declaration->atom);
		IFEELIN F_Dispose(Declaration);
	}
	#if 0
	else
	{
		IFEELIN F_Log(0,"unknown declaration (0x%08lx)", Declaration);
	}
	#endif

	return (struct in_Declaration *) LOD->declaration_list.Head;
}
//+

///declaration_flush
void declaration_flush(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_Declaration *declaration;
	struct in_Declaration *next;

	for (declaration = (struct in_Declaration *) LOD->declaration_list.Head ; declaration ; declaration = next)
	{
		next = declaration->next;

		if (declaration->refs_count == 0)
		{
			#ifdef DB_DECLARATION_FLUSH

			if (declaration->atom)
			{
				IFEELIN F_Log(0,"delete unreferenced declaration (%s)", declaration->atom->Key);
			}
			else
			{
				IFEELIN F_Log(0,"delete deprecated declaration (0x%08lx)", declaration);
			}

			#endif

			declaration_delete(Class, Obj, declaration);
		}
		else if (declaration->atom != NULL)
		{
			#ifdef DB_DECLARATION_FLUSH

			IFEELIN F_Log(0,"ANONYMATE declaration (%s) has (%ld) refs", declaration->atom->Key, declaration->refs_count);

			#endif

			IFEELIN F_AtomRelease(declaration->atom);

			declaration->atom = NULL;
		}
	}
}
//+

/*** styles ************************************************************************************/

///typedef

/*
	pas besoin de compteur d'utilisation, celui de la 'declaration' suffit.


	lorsque le developpeur demande de composer un style, il  ne  reste  plus
	qu'à  compter  le  nombre  de  propriétés (au pire il y en aura moins si
	certaines se surchargent) pour créer un tableau  de  pointeur  vers  les
	propriétés.  D'ailleurs,  si ont crée le tableau dans le bon ordre, on a
	pas à se soucier du surchargeage, si  l'item  qui  surcharge  se  trouve
	avant l'item à surcharger.

	Donc: on compte le nombre de 'property', on crée un tableau de pointeurs
	(terminé par un NULL, comme d'hab), ensuite on copie les pointeurs.

*/

struct in_Style
{
	struct in_Style                *next;

	/* public */

	FPreferenceStyle                public;

	/* private */

	FAtom                          *atom;
	FAtom                          *element_class_atom;
	FAtom                          *element_style_atom;

	struct in_Declaration         **declarations;

	FPreferenceProperty            *element_properties_array;
	uint32                          element_properties_count;

	uint32                          refs_count;
};

#define F_PUBLICIZE_STYLE(style)    &style->public
#define F_PRIVATIZE_STYLE(style)    (struct in_Style *) ((uint32)(style) - sizeof (struct in_Style *))

//+

struct in_ComposePseudo
{
	struct in_ComposePseudo        *next;
	FAtom                          *atom;

	uint32                          properties_count;
	FPreferenceProperty           **properties_pointers;
};

struct in_Compose
{
	APTR                            pool;

	uint32                          declarations_count;
	struct in_Declaration         **declarations_pointers;

	uint32                          properties_count_class;
	uint32                          properties_count_element;
	FPreferenceProperty            *properties_array;
	FPreferenceProperty           **properties_pointers;

	uint32                          pseudos_count;
	struct in_ComposePseudo        *pseudos;
};

///style_compose_count_pseudo
STATIC void style_compose_count_pseudo(struct in_Declaration *Declaration, struct in_Compose *Compo)
{
	struct in_ComposePseudo *pseudo;

	for (pseudo = Compo->pseudos ; pseudo ; pseudo = pseudo->next)
	{
		if (pseudo->atom == Declaration->atom_pseudo)
		{
			break;
		}
	}

	if (pseudo == NULL)
	{
		pseudo = IFEELIN F_NewP(Compo->pool, sizeof (struct in_ComposePseudo));

		if (pseudo != NULL)
		{
			pseudo->next = Compo->pseudos;
			pseudo->atom = Declaration->atom_pseudo;

			Compo->pseudos_count++;
			Compo->pseudos = pseudo;
		}
	}

	if (pseudo != NULL)
	{
		pseudo->properties_count += Declaration->properties_count;
	}
}
//+
///style_compose_count_by_reference
STATIC void style_compose_count_by_reference(FClass *Class, FObject Obj, FClass *ReferenceClass, struct in_Compose *Compo)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 len = 0;
	STRPTR name = IFEELIN F_StrNewP(LOD->pool, &len, ReferenceClass->Name);

	if (name)
	{
		FAtom *atom;
		STRPTR k = name;

		while (*k)
		{
			*k = IUTILITY ToLower(*k); k++;
		}

		atom = IFEELIN F_AtomFind(name, len);

		if (atom)
		{
			struct in_Declaration *declaration;

			for (declaration = (struct in_Declaration *) LOD->declaration_list.Head ; declaration ; declaration = declaration->next)
			{
				if (declaration->atom == atom)
				{
					Compo->declarations_count++;
					Compo->properties_count_class += declaration->properties_count;

					if (declaration->atom_pseudo != NULL)
					{
						style_compose_count_pseudo(declaration, Compo);
					}
				}
			}
		}

		IFEELIN F_Dispose(name);
	}

	if (ReferenceClass->Super)
	{
		style_compose_count_by_reference(Class, Obj, ReferenceClass->Super, Compo);
	}
}
//+
///style_compose_count_by_atom
STATIC void style_compose_count_by_atom(FClass *Class, FObject Obj, FAtom *Atom, struct in_Compose *Compo)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_Declaration *declaration;

	for (declaration = (struct in_Declaration *) LOD->declaration_list.Head ; declaration ; declaration = declaration->next)
	{
		if (declaration->atom == Atom)
		{
			Compo->declarations_count++;
			Compo->properties_count_class += declaration->properties_count;

			if (declaration->atom_pseudo != NULL)
			{
				style_compose_count_pseudo(declaration, Compo);
			}
		}
	}
}
//+
///style_compose_count
STATIC void style_compose_count(FClass *Class, FObject Obj, FClass *ReferenceClass, FAtom *ElementClassAtom, struct in_Compose *Msg)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (LOD->reference != NULL)
	{
		style_compose_count(Class, LOD->reference, ReferenceClass, ElementClassAtom, Msg);
	}

	if (ReferenceClass != NULL)
	{
		style_compose_count_by_reference(Class, Obj, ReferenceClass, Msg);
	}

	if (ElementClassAtom != NULL)
	{
		STRPTR s = ElementClassAtom->Key;
		STRPTR k = s;

		uint8 c;

		FAtom *atom;

//		  IFEELIN F_Log(0, "ElementClassAtom (%s)", ElementClassAtom->Key);

		for (;;)
		{
			c = *k++;

			if ((c != ' ') && (c != '\n') && (c != '\0'))
			{
				continue;
			}

//            IFEELIN F_Log(0, "multiple class '%s' (%ld)", s, k - s - 1);

			atom = IFEELIN F_AtomObtain(s, k - s - 1);

			if (atom != NULL)
			{
//				  IFEELIN F_Log(0,"count: atom (%s)", atom->Key);

				style_compose_count_by_atom(Class, Obj, atom, Msg);

				IFEELIN F_AtomRelease(atom);
			}

			s = k;

			while ((c == ' ') && (c == '\n'))
			{
				c = *k++;
			}

			if (c == '\0')
			{
				break;
			}
		}

		//style_compose_count_by_atom(Class, Obj, ElementClassAtom, Msg);
	}
}
//+

///style_compose_array_pseudo
STATIC void style_compose_array_pseudo(FClass *Class, FObject Obj, struct in_Declaration *Declaration, struct in_Compose *Compose)
{
	if (Declaration->atom_pseudo != NULL)
	{
		struct in_ComposePseudo *pseudo;

		for (pseudo = Compose->pseudos ; pseudo ; pseudo = pseudo->next)
		{
			if (pseudo->atom == Declaration->atom_pseudo)
			{
				break;
			}
		}

		if (pseudo != NULL)
		{
			IEXEC CopyMem(Declaration->properties, pseudo->properties_pointers, sizeof (FPreferenceProperty *) * Declaration->properties_count);

			pseudo->properties_pointers += Declaration->properties_count;
		}
	}
}
//+
///style_compose_array_by_reference
STATIC void style_compose_array_by_reference(FClass *Class, FObject Obj, FClass *ReferenceClass, struct in_Compose *Compo)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 len = 0;
	STRPTR name = IFEELIN F_StrNewP(LOD->pool, &len, ReferenceClass->Name);

	if (name)
	{
		FAtom *atom;
		STRPTR k = name;

		while (*k)
		{
			*k = IUTILITY ToLower(*k); k++;
		}

		atom = IFEELIN F_AtomFind(name, len);

		if (atom)
		{
			struct in_Declaration *declaration;

			for (declaration = (struct in_Declaration *) LOD->declaration_list.Head ; declaration ; declaration = declaration->next)
			{
				if (declaration->atom == atom)
				{
					declaration->refs_count++;
					*Compo->declarations_pointers++ = declaration;

					if (declaration->atom_pseudo == NULL)
					{
						IEXEC CopyMem(declaration->properties, Compo->properties_pointers, sizeof (FPreferenceProperty *) * declaration->properties_count);

						#ifdef DB_ARRAY

						IFEELIN F_Log
						(
							0, "add declaration (%s) @ (0x%08lx), copy (%ld) properties pointer @ (0x%08lx)",

							declaration->atom->Key,
							Compo->declarations_array,
							declaration->properties_count,
							Compo->properties_array
						);

						#endif

						Compo->properties_pointers += declaration->properties_count;
					}
					else
					{
						style_compose_array_pseudo(Class, Obj, declaration, Compo);
					}
				}
			}
		}

		IFEELIN F_Dispose(name);
	}

	if (ReferenceClass->Super)
	{
		style_compose_array_by_reference(Class, Obj, ReferenceClass->Super, Compo);
	}
}
//+
///style_compose_array_by_atom
STATIC void style_compose_array_by_atom(FClass *Class, FObject Obj, FAtom *Atom, struct in_Compose *Compo)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_Declaration *declaration;

	for (declaration = (struct in_Declaration *) LOD->declaration_list.Head ; declaration ; declaration = declaration->next)
	{
		if (declaration->atom == Atom)
		{
			declaration->refs_count++;
			*Compo->declarations_pointers++ = declaration;

			if (declaration->atom_pseudo == NULL)
			{
				IEXEC CopyMem(declaration->properties, Compo->properties_pointers, sizeof (FPreferenceProperty *) * declaration->properties_count);

				#ifdef DB_ARRAY

				IFEELIN F_Log
				(
					0, "add declaration (%s) @ (0x%08lx), copy (%ld) properties pointer @ (0x%08lx)",

					declaration->atom->Key,
					Compo->declarations_array,
					declaration->properties_count,
					Compo->properties_array
				);

				#endif

				Compo->properties_pointers += declaration->properties_count;
			}
			else
			{
				style_compose_array_pseudo(Class, Obj, declaration, Compo);
			}
		}
	}
}
//+
///style_compose_array
STATIC void style_compose_array(FClass *Class, FObject Obj, FClass *ReferenceClass, FAtom *ElementClassAtom, struct in_Compose *Compo)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	if (LOD->reference)
	{
		style_compose_array(Class, LOD->reference, ReferenceClass, ElementClassAtom, Compo);
	}

	if (ReferenceClass != NULL)
	{
		style_compose_array_by_reference(Class, Obj, ReferenceClass, Compo);
	}

	if (ElementClassAtom != NULL)
	{
		STRPTR s = ElementClassAtom->Key;
		STRPTR k = s;

		uint8 c;

		FAtom *atom;

		//IFEELIN F_Log(0, "ElementClassAtom (%s)", ElementClassAtom->Key);

		for (;;)
		{
			c = *k++;

			if ((c != ' ') && (c != '\n') && (c != '\0'))
			{
				continue;
			}

//            IFEELIN F_Log(0, "multiple class '%s' (%ld)", s, k - s - 1);

			atom = IFEELIN F_AtomObtain(s, k - s - 1);

			if (atom != NULL)
			{
//				  IFEELIN F_Log(0,"array: atom (%s)", atom->Key);

				style_compose_array_by_atom(Class, Obj, atom, Compo);

				IFEELIN F_AtomRelease(atom);
			}

			s = k;

			while ((c == ' ') && (c == '\n'))
			{
				c = *k++;
			}

			if (c == '\0')
			{
				break;
			}
		}
	}
}
//+

typedef bool32 (*feelin_func_parse)               (FClass *Class, FObject Obj, STRPTR PropertyName, uint32 PropertyNameLength, STRPTR Value, uint32 ValueLength, APTR UserData);

///style_properties_parse
STATIC void style_properties_parse(FClass *Class, FObject Obj, STRPTR Style, feelin_func_parse Callback, APTR UserData)
{
	if (Style)
	{
		STRPTR k = Style;

		uint8 c;

		while (*k)
		{
			STRPTR name_s;

			/* skip leading spaces */

			while (((c = *k) != '\0') && (c == ' ')) k++;

			if (c == '\0')
			{
				return;
			}

			name_s = k;

			/* search name's ending */

			while (((c = *k) != '\0') && (c != ' ') && (c != ':')) k++;

			if (c == ':')
			{
				STRPTR name_e = k++;

				//IFEELIN F_Log(0,"PROPERTY NAME (%-16.16s)(%ld)(%lc)(%lc)", name_s, name_e - name_s, *name_s, *name_e);

				/* skip leading spaces */

				while (((c = *k) != '\0') && (c == ' ')) k++;

				if (c != '\0')
				{
					STRPTR value_s = k;

					while (((c = *k) != '\0') && (c != ';')) k++;

					//IFEELIN F_Log(0,"PROPERTY VALUE (%-16.16s)(%ld)", value_s, k - value_s);

					//element_style_add(Class, Obj, name_s, name_e - name_s, value_s, k - value_s);

					if (Callback(Class, Obj, name_s, name_e - name_s, value_s, k - value_s, UserData) == FALSE)
					{
						return;
					}

					if (c == ';')
					{
						k++;
					}
				}
				else return;
			}
			else return;
		}
	}
}
//+
///style_properties_count_code

STATIC bool32 style_properties_count_code(FClass *Class, FObject Obj, STRPTR PropertyName, uint32 PropertyNameLength, STRPTR Value, uint32 ValueLength, struct in_Compose *Compo)
{
	Compo->properties_count_element++;

	return TRUE;
}
//+
///style_properties_array_code
STATIC bool32 style_properties_array_code(FClass *Class, FObject Obj, STRPTR PropertyName, uint32 PropertyNameLength, STRPTR Value, uint32 ValueLength, struct in_Compose *Compo)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FPreferenceProperty *property = Compo->properties_array++;

	STRPTR property_value = NULL;

	//if (IFEELIN F_StrCmp("none", Value, ValueLength) != 0)
	{
		property_value = IFEELIN F_NewP(LOD->pool, ValueLength + 1);

		if (property_value)
		{
			IEXEC CopyMem(Value, property_value, ValueLength);
		}
		else
		{
			return NULL;
		}
	}

	property->Atom = IFEELIN F_AtomObtain(PropertyName, PropertyNameLength);
	property->Value = property_value;

	if (property->Atom != NULL)
	{
		if (property_value)
		{
			property->NumericValue = IFEELIN F_Do(Obj, F_RESOLVED_ID(NUMERIFY), property->Value, &property->NumericType);
		}

		#ifdef DB_STYLE_CREATE
		IFEELIN F_Log(0, "element property (%s) value (%s)(%ld)", property->Atom->Key, property->Value, property->NumericValue);
		#endif

		*Compo->properties_pointers++ = property;
	}

	return TRUE;
}
//+

///style_create
STATIC struct in_Style * style_create(FClass *Class, FObject Obj, FObject *Reference, STRPTR ElementClass, STRPTR ElementStyle)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_Style *style;

	FAtom *atom;
	FAtom *element_class_atom = NULL;
	FAtom *element_style_atom = NULL;

	FClass *reference_class = _object_class(Reference);

	struct in_Compose compo;

	compo.pool = LOD->pool;
	compo.declarations_count = 0;
	compo.declarations_pointers = NULL;
	compo.properties_count_class = 0;
	compo.properties_count_element = 0;
	compo.properties_array = NULL;
	compo.properties_pointers = NULL;
	compo.pseudos_count = 0;
	compo.pseudos = NULL;

//	  IFEELIN F_Log(0, "style for Reference (%s) ElementClass (0x%08lx) ElementStyle (0x%08lx)", _object_classname(Reference), ElementClass, ElementStyle);

/** atoms **************************************************************************************/

	atom = IFEELIN F_AtomObtain(reference_class->Atom->Key, reference_class->Atom->KeyLength);

	if (atom == NULL)
	{
		goto __error;
	}

	if (ElementClass != NULL)
	{
		element_class_atom = IFEELIN F_AtomObtain(ElementClass, ALL);

		if (element_class_atom == NULL)
		{
			goto __error;
		}
	}

	if (ElementStyle != NULL)
	{
		element_style_atom = IFEELIN F_AtomObtain(ElementStyle, ALL);

		if (element_style_atom == NULL)
		{
			goto __error;
		}
	}

/** counting things ****************************************************************************/

	if (element_style_atom != NULL)
	{
		style_properties_parse(Class, Obj, element_style_atom->Key, (feelin_func_parse) style_properties_count_code, &compo);
	}

//	  IFEELIN F_Log(0, "style_compose_count >> BEGIN");
	style_compose_count(Class, Obj, reference_class, element_class_atom, &compo);
/*
	IFEELIN F_Log(0, "style_compose_count >> DONE - pseudos (%ld) decl (%ld) class prop (%ld) style prop (%d)",
			compo.pseudos_count,
			compo.declarations_count,
			compo.properties_count_class,
			compo.properties_count_element);
*/

	#ifdef DB_STYLE_CREATE

	if (compo.pseudos_count != 0)
	{
		IFEELIN F_Log
		(
			0, "(%ld) pseudos for (%s) of class (%s) - (%ld) declarations - (%ld) class properties (%ld) element properties",

			compo.pseudos_count,
			reference_class->Name,
			ElementClass ? ElementClass : (STRPTR) "none",
			compo.declarations_count,
			compo.properties_count_class,
			compo.properties_count_element
		);

	}

	#endif

	#if 0

	style = NULL;

	#else

	/* we allocate a big peace of memory for the style. it's a bit complex,
	but at least we won't have to chase after small blocks when we'll need
	to delete the style.

	note that we create the style even if there is no properties, thus we
	won't have to check the style again and again */

	style = IFEELIN F_NewP
	(
		LOD->pool,

		/* the structure for the main style */

		(sizeof (struct in_Style)) +

		/* NULL-terminated array  of  pointers  to  'struct  in_Declaration'
		items.  These  pointers  are  used to reference declarations used to
		build the style. Their reference counter  is  incremented  when  the
		style is created and decremented when the style is deleted */

		(sizeof (struct in_Declaration *) * (compo.declarations_count + 1)) +

		/* NULL-terminated arrays of pointers to  FPreferenceProperty  items
		defined  by  the  class,  the  element  and  the pseudo classes. The
		properties defined  by  pseudo  classes  are  used  like  standalone
		NULL-terminated arrays. */

		(sizeof (FPreferenceProperty *) * (compo.properties_count_class + compo.properties_count_element + 1 + compo.pseudos_count)) +

		/* if the reference element  define  some  properties,  we  need  to
		create  an  array  of  FPreferenceProperty  and  create  them, their
		pointers are then added to the properties pointers,  with  those  of
		the class */

		(sizeof (FPreferenceProperty) * compo.properties_count_element) +

		/* array of FPreferenceStyle items, one for each pseudo */

		(sizeof (FPreferenceStyle) * compo.pseudos_count) +

		#if 0

		/* if pseudos are defined, we  create  a  NULL-terminated  array  of
		pointers  to  FPreferenceStylePseudo  items  as  well as an array of
		FPreferenceStylePseudo items */

		(compo.pseudos_count ?
			
			sizeof (FPreferenceStylePseudo *) * (compo.pseudos_count + 1) +
			sizeof (FPreferenceStylePseudo) * compo.pseudos_count

			: 0) +

		#endif

		/* some memory space to decode properties, might be 0 ! */

		(reference_class->PropertiesOffset + reference_class->PropertiesLocalSize)
	);

	if (style != NULL)
	{
		uint32 mem = (uint32)(style) + sizeof (struct in_Style);
		uint32 mem_properties_end;

		style->atom = atom;
		style->element_class_atom = element_class_atom;
		style->element_style_atom = element_style_atom;

		/* declarations pointers array */

		style->declarations = compo.declarations_pointers = (struct in_Declaration **) mem;
		mem += (sizeof (struct in_Declaration *) * (compo.declarations_count + 1));

		/* class and element properties pointers */

		style->public.Properties = compo.properties_pointers = (FPreferenceProperty **) mem;
		mem += (sizeof (FPreferenceProperty *) * (compo.properties_count_class + compo.properties_count_element + 1 + compo.pseudos_count));
		mem_properties_end = mem;

		/* pseudos */

		if (compo.pseudos_count != 0)
		{
			FPreferenceStyle *node = &style->public;
			struct in_ComposePseudo *pseudo;

			for (pseudo = compo.pseudos ; pseudo ; pseudo = pseudo->next)
			{
				node->Next = (FPreferenceStyle *) mem;

				/* initiate new node */

				node = (FPreferenceStyle *) mem;
				mem += sizeof (FPreferenceStyle);

				pseudo->properties_pointers = (FPreferenceProperty **) (mem_properties_end - (sizeof (FPreferenceProperty *) * (pseudo->properties_count + 1)));
				mem_properties_end -= (sizeof (FPreferenceProperty *) * (pseudo->properties_count + 1));

				node->Next = NULL;
				node->Properties = pseudo->properties_pointers;
				node->Atom = pseudo->atom;
				node->DecodedPropertiesSpace = NULL;
			}
		}

		/* decoded space */

		if (reference_class->PropertiesOffset + reference_class->PropertiesLocalSize)
		{
			style->public.DecodedPropertiesSpace = (APTR) mem;
			mem += (reference_class->PropertiesOffset + reference_class->PropertiesLocalSize);
		}

		/* compose */

		style_compose_array(Class, Obj, reference_class, element_class_atom, &compo);

		/* element properties (if any) */

		if (compo.properties_count_element != 0)
		{
			style->element_properties_count = compo.properties_count_element;
			style->element_properties_array = compo.properties_array = (FPreferenceProperty *) mem;

			mem += (sizeof (FPreferenceProperty) * compo.properties_count_element);

			style_properties_parse(Class, Obj, element_style_atom->Key, (feelin_func_parse) style_properties_array_code, &compo);
		}

/** publicize **********************************************************************************/

		#ifdef DB_STYLE_CREATE

		{
			FPreferenceProperty **property_a;

			IDOS_ Printf("\ndeclaration (%s) class (%s)\n", style->atom->Key, style->element_class_atom ? style->element_class_atom->Key : (STRPTR) "none");

			for (property_a = style->public.Properties ; *property_a ; property_a++)
			{
				IDOS_ Printf("   property %32.32s: %s\n", (*property_a)->Atom->Key, (*property_a)->Value);
			}

			if (style->public.Next != NULL)
			{
				FPreferenceStyle *pseudo;

				for (pseudo = style->public.Next ; pseudo ; pseudo = pseudo->Next)
				{
					IDOS_ Printf("\n   pseudo (%s)\n", pseudo->Atom->Key);

					for (property_a = pseudo->Properties ; *property_a ; property_a++)
					{
						IDOS_ Printf("      property %29.29s: %s\n", (*property_a)->Atom->Key, (*property_a)->Value);
					}
				}
			}
		}

		#endif

		style->next = LOD->styles;
		LOD->styles = style;

		return style;
	}

	#endif

__error:

	IFEELIN F_AtomRelease(atom);
	IFEELIN F_AtomRelease(element_class_atom);
	IFEELIN F_AtomRelease(element_style_atom);

	return NULL;
}
//+
///style_obtain
STATIC struct in_Style *style_obtain(FClass *Class, FObject Obj, FObject Reference, STRPTR ElementClass, STRPTR ElementStyle)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_Style *style = NULL;

	FClass *reference_class = _object_class(Reference);
	FAtom *reference_atom = reference_class->Atom;
	FAtom *element_class_atom = NULL;
	FAtom *element_style_atom = NULL;

	if (ElementClass != NULL)
	{
		element_class_atom = IFEELIN F_AtomFind(ElementClass, ALL);

		if (element_class_atom == NULL)
		{
			goto __create;
		}
	}

	if (ElementStyle != NULL)
	{
		element_style_atom = IFEELIN F_AtomFind(ElementStyle, ALL);

		if (element_style_atom == NULL)
		{
			goto __create;
		}
	}

/** searching same style ***********************************************************************/

	if (reference_atom != NULL)
	{
		for (style = LOD->styles ; style ; style = style->next)
		{
			if ((style->atom == reference_atom) &&
				(style->element_class_atom == element_class_atom) &&
				(style->element_style_atom == element_style_atom))
			{
				break;
			}
		}
	}

__create:

	if (style == NULL)
	{
		style = style_create(Class, Obj, Reference, ElementClass, ElementStyle);
	}

	if (style != NULL)
	{
		if ((style->refs_count == 0) && (style->public.DecodedPropertiesSpace != NULL))
		{
			IFEELIN F_Do(Reference, FM_Element_CreateDecodedStyle, style->public.DecodedPropertiesSpace, &style->public, LOD->pool);
		}

		style->refs_count++;

		#ifdef DB_STYLE_OBTAIN
		IFEELIN F_Log(0,"obtained style (%s)(%ld) index (0x%08lx)", style->atom->Key, style->refs_count, LOD->styles);
		#endif
	}

	return style;
}
//+

///style_delete
bool32 style_delete(FClass *Class, FObject Obj, struct in_Style *Style)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_Style *prev = NULL;
	struct in_Style *node;

	for (node = LOD->styles ; node ; node = node->next)
	{
		if (node == Style)
		{
			struct in_Declaration **array;

			if (prev)
			{
				prev->next = node->next;
			}
			else
			{
				LOD->styles = node->next;
			}

			/* unreference declarations */

			//IFEELIN F_Log(0,"declarations (0x%08lx)", node->declarations);

			for (array = node->declarations ; *array ; array++)
			{
				//IFEELIN F_Log(0,"unref declaration (%s)", (*array)->atom->Key);

				(*array)->refs_count--;
			}

			#ifdef DB_STYLE_DELETE

			IFEELIN F_Log(0,"style.delete (%s) index (0x%08lx)", node->atom->Key, LOD->styles);

			#endif

			if (node->element_properties_count != 0)
			{
				uint32 i;

				//IFEELIN F_Log(0,"element properties array (0x%08lx)(%ld)", node->element_properties_array, node->element_properties_count);

				for (i = 0 ; i < node->element_properties_count ; i++)
				{
					IFEELIN F_AtomRelease(node->element_properties_array[i].Atom);
					IFEELIN F_Dispose(node->element_properties_array[i].Value);
				}
			}

			IFEELIN F_AtomRelease(node->atom);
			IFEELIN F_AtomRelease(node->element_class_atom);
			IFEELIN F_AtomRelease(node->element_style_atom);

			IFEELIN F_Dispose(node);

			break;
		}
		else
		{
			prev = node;
		}
	}

	return (bool32) (node != NULL);
}
//+

///style_flush
void style_flush(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_Style *style;
	struct in_Style *next;

	for (style = LOD->styles ; style ; style = next)
	{
		next = style->next;

		if (style->refs_count == 0)
		{
			#ifdef DB_STYLE_FLUSH

			if (style->atom)
			{
				IFEELIN F_Log(0,"delete unreferenced style (%s)", style->atom->Key);
			}
			else
			{
				IFEELIN F_Log(0,"delete deprecated style (0x%08lx)", style);
			}

			#endif

			style_delete(Class, Obj, style);
		}
		else if (style->atom != NULL)
		{
			#ifdef DB_STYLE_FLUSH

			IFEELIN F_Log(0,"ANONYMATE: style (%s) which has (%ld) refs", style->atom->Key, style->refs_count);

			#endif

			IFEELIN F_AtomRelease(style->atom);

			style->atom = NULL;
		}
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Prefs_GetProperty
F_METHOD(uint32, Prefs_GetProperty)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint32 rc;

	F_LOCK_ARBITER;

	rc = F_SUPERDO();

	if ((rc == 0) && (LOD->reference != NULL))
	{
		rc = F_OBJDO(LOD->reference);
	}

	F_UNLOCK_ARBITER;

	return rc;
}
//+
///Prefs_AddProperty
F_METHOD(uint32, Prefs_AddProperty)
{
	uint32 rc;

	F_LOCK_ARBITER;

	rc = F_SUPERDO();

	F_UNLOCK_ARBITER;

	return rc;
}
//+

///Prefs_ObtainStyle
F_METHODM(FPreferenceStyle *, Prefs_ObtainStyle, FS_Preference_ObtainStyle)
{
	if (Msg->Reference != NULL)
	{
		struct in_Style *style;

		STRPTR element_class = NULL;
		STRPTR element_style = NULL;

		IFEELIN F_Do(Msg->Reference, FM_Get,

			FA_Element_Class, &element_class,
			FA_Element_Style, &element_style,

			TAG_DONE);

		F_LOCK_ARBITER;

		style = style_obtain(Class, Obj, Msg->Reference, element_class, element_style);

		F_UNLOCK_ARBITER;

		if (style)
		{
			return F_PUBLICIZE_STYLE(style);
		}
	}

	return NULL;
}
//+
///Prefs_ReleaseStyle
F_METHODM(bool32, Prefs_ReleaseStyle, FS_Preference_ReleaseStyle)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	bool32 rc = FALSE;

	if (Msg->Style)
	{
		struct in_Style *style = F_PRIVATIZE_STYLE(Msg->Style);
		struct in_Style *node;

		F_LOCK_ARBITER;

		#ifdef DB_STYLE_RELEASE

		IFEELIN F_Log(0,"release style (%s)", style->atom->Key);

		#endif

		for (node = LOD->styles ; node ; node = node->next)
		{
			if (node == style)
			{
				node->refs_count--;

				if (node->refs_count == 0)
				{
					if (style->public.DecodedPropertiesSpace != NULL)
					{
						IFEELIN F_Do(Msg->Reference, FM_Element_DeleteDecodedStyle, style->public.DecodedPropertiesSpace);
					}

					#ifdef F_ENABLE_DELETE_UNUSED_STYLES

						#ifdef DB_STYLE_RELEASE

						IFEELIN F_Log(0,"delete unreferenced style (%s)", node->atom->Key);

						#endif

						style_delete(Class, Obj, node);

					#elif defined (DB_STYLE_RELEASE)

					IFEELIN F_Log(0,"style (%s) class (%s) (%ld element properties) is not referenced anymore",

						node->atom->Key,
						node->element_class_atom ? node->element_class_atom->Key : (STRPTR) "none",
						node->element_properties_count
					);

					#endif
				}

				break;
			}
		}

		if (node == NULL)
		{
			IFEELIN F_Log(FV_LOG_DEV, "Unknown Style handle (0x%08lx)", Msg->Style);
		}

		F_UNLOCK_ARBITER;
	}

	return rc;
}
//+
