/*
**    Heart of Feelin's Object-Oriented-System
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************

$VER: 09.00 (2005/09/30)

	Added AmigaOS 4 support.

************************************************************************************************/

#include "Private.h"

/************************************************************************************************
*** Class management ****************************************************************************
************************************************************************************************/

///f_find_class
F_LIB_FIND_CLASS
{
	FAtom *atom = IFEELIN F_AtomFind(Name, ALL);

	if (atom)
	{
		FClass *node;

		F_CLASSES_LOCK;

		for (F_CLASSES_EACH(node))
		{
			if (atom == node->Atom)
			{
				break;
			}
		}

		F_CLASSES_UNLOCK;

		return node;
	}

	return NULL;
}
//+
///f_open_class
F_LIB_OPEN_CLASS
{
	FClass *cl; 

	F_CLASSES_LOCK;

	/* search with Name *as is* (needed for private classes "::FC_") */

	cl = IFEELIN F_FindClass(Name);

	if (!cl)
	{
		STRPTR name = IFEELIN F_StrNew(NULL,"Feelin/%s.fc",Name);

		/* Try to load library "<Name>.fc".*/

		if (name)
		{
			struct Library *FeelinClassBase = IEXEC OpenLibrary(name,0);
			
#ifdef __amigaos4__
			struct FeelinClassIFace *IFeelinClass = NULL;
			
			if (FeelinClassBase)
			{
				if (!(IFeelinClass = (struct FeelinClassIFace *) IEXEC GetInterface(FeelinClassBase, "main", 1, NULL)))
				{
					/* Can't get the interface, so we close the library */
					
					IEXEC CloseLibrary(FeelinClassBase); FeelinClassBase = NULL;
				}
			}
#endif
 
			/* open library */

			if (FeelinClassBase)
			{
				struct TagItem *tags = IFEELINCLASS F_Query(FV_Query_MetaClassTags,(struct FeelinBase *) FeelinBase);

				/* Before creating the class, I request  FV_Query_MetaTags.
				These tags are used to create the class of the class object
				(the meta class). */
				
				if (tags)
				{
					STRPTR meta_name = IFEELIN F_StrNew(NULL,":meta:%s",Name);
	  
					struct in_FeelinClass *meta = IFEELIN F_NewObj(FC_Class,
						  
						  FA_Class_Super,   FC_Class,
						  FA_Class_Pool,    FALSE,
						  FA_Class_Name,    meta_name,
						  
						  TAG_MORE, tags);
						  
					IFEELIN F_Dispose(meta_name);

					if (meta)
					{
						#ifdef DB_OPENCLASS
						IFEELIN F_Log(0,"OPEN(%s) META 0x%08lx %ld",Name,meta,meta->Public.UserCount);
						#endif
				  
						if ((tags = IFEELINCLASS F_Query(FV_Query_ClassTags,(struct FeelinBase *) FeelinBase)))
						{
							if ((cl = IFEELIN F_NewObj(meta->public.Name,
											  
									FA_Class_Name,    Name,
									FA_Class_Module,  FeelinClassBase,
							   
								TAG_MORE, tags)))
							{
								// the module is saved into meta class
#ifdef __amigaos4__
							 
/* HOMBRE: Gofro, pourquoi demander une 2ème ouverture de la bibliothèque si
on  ne récupère même pas le résultat ? Je suppose que c'est pour incrémenter
le compteur d'ouverture... ?

MAIS A QUEL MOMENT EST-IL DECREMENTE ????

GOFROMIEL: Je me rapelle plus trop, mais je crois  qu'il  y  a  problème  au
niveau  des  fermetures  de classes. Le pointeur du module est sauvé dans la
structure de la meta classes, et comme la meta classes est  supprimée  avant
la  classe  (ce  qui est stupide maintenant que j'y repense) le module n'est
plus disponible lorsque vient le tour de la classe... BREF à revoir tout  ça
:-)

En fait, quand j'ai mis en place les méta-classes c'était juste un test pour
voir  comment  je pourrais faire. Mais en fait, vu le design de Feelin, ça a
marché de suite... alors je me suis pas plus préoccupé du truc :-)
								
Mieux vaut conserver l'incrémentation jusqu'à ce que l'on  voit  ce  que  ça
donne avec les classes externes.
 
*/

								FeelinClassBase->lib_OpenCnt++;
								IFeelinClass->Data.RefCount++;    /* Incrémenter celui-ci peut peut-être suffir */
#else
								IEXEC OpenLibrary(name,0);
#endif

								#ifdef DB_OPENCLASS
								IFEELIN F_Log(0,"META 0x%08lx %ld - CLASSMETA 0x%08lx - FC_CLASS 0x%08lx",meta,meta->Public.UserCount,_object_class(cl),FeelinBase->ClassClass);
								#endif

								meta->module = FeelinClassBase;
								
								#ifdef __amigaos4__
								meta->module_iface = IFeelinClass;
								#endif
							}
						}
					 
						if (!cl)
						{
							IFEELIN F_DisposeObj(meta);
						}
					}
				}
				else
				{
					tags = IFEELINCLASS F_Query(FV_Query_ClassTags,(struct FeelinBase *) FeelinBase);

					if (tags)
					{
						cl = IFEELIN F_NewObj(FC_Class,
						
							FA_Class_Name,    Name,
							FA_Class_Module,  FeelinClassBase,
							
							TAG_MORE, tags);
					}
				}

				if (!cl)
				{
					#ifdef __amigaos4__
					IEXEC DropInterface((struct Interface *) IFeelinClass);
					#endif
					IEXEC CloseLibrary(FeelinClassBase);
				}
			}
			else
			{
				/* report opening failure to the debug console */

				IFEELIN F_Log(FV_LOG_USER,"F_OpenClass() Unable to open class \"%s\"",Name);
			}
			
			IFEELIN F_Dispose(name);
		}
	}

	if (cl)
	{
		cl->UserCount++;
	}

	F_CLASSES_UNLOCK;

	return cl;
}
//+
///f_close_class
F_LIB_CLOSE_CLASS
{
	if (Class)
	{
		F_CLASSES_LOCK;
   
		if ((--Class->public.UserCount) == 0)
		{
			if (Class->module)
			{
				IFEELIN F_DeleteClass((FClass *) Class);
			}
		}

		F_CLASSES_UNLOCK;
	}
	return 0;
}
//+
///f_create_classA
F_LIB_CREATE_CLASS
{
	struct TagItem *next = Tags,*item;
	FClass *cl=NULL;

	F_CLASSES_LOCK;

	while  ((item = IUTILITY NextTagItem(&next)))
	switch (item->ti_Tag)
	{
		case FA_Class_Name: Name = (STRPTR)(item->ti_Data); item->ti_Tag = TAG_IGNORE; break;
	}

	if (Name)
	{
		cl = IFEELIN F_FindClass(Name);
	}

	if (cl)
	{
		IFEELIN F_Log(FV_LOG_DEV,"F_CreateClassA() Class '%s' already exists",Name);
	}
	else
	{
		cl = IFEELIN F_NewObj(FC_Class, FA_Class_Name,Name,TAG_MORE,Tags);
	}

	F_CLASSES_UNLOCK;

	return cl;
}
//+

/************************************************************************************************
**** Invokation *********************************************************************************
************************************************************************************************/

///f_doa
F_LIB_DO
{
	if (Obj && Method)
	{
		#ifdef F_ENABLE_SAFE_INVOKATION
		{
			FClass *node;

			for (F_CLASSES_EACH(node))
			{
				if (node == _object_class(Obj)) break;
			}

			if (node == NULL)
			{
				STRPTR method_name = f_find_method_name(Method,FeelinBase);
				
				if (method_name)
				{
					IFEELIN F_Log(FV_LOG_USER,"(db)F_DoA() 0x%08lx is not an Object !! Class 0x%08lx - Method 0x%08lx (%s)",Obj,_object_class(Obj),Method,method_name);
				}
				else
				{
					IFEELIN F_Log(FV_LOG_USER,"(db)F_DoA() 0x%08lx is not an Object !! Class 0x%08lx - Method 0x%08lx",Obj,_object_class(Obj),Method);
				}

				return 0;
			}
		}
		#endif

		/* The following code allows Dynamic IDs (strings)  to  be  used  as
		well as static or Resolved IDs (numerical representatio of a Dynamic
		ID). This is maybe not a good idea since, IDs are  numeric  most  of
		the  time...  and the developer can use F_DynamicFindID() himself...
		maybe it's more convenient since Dynamic IDs is the future's key :-)
		*/

		if (((0xFF000000 & Method) != MTHD_BASE) &&
			((0xFF000000 & Method) != DYNA_MTHD) &&
			((0xFF000000 & Method) != FCCM_BASE))
		{
			FClassMethod *m = IFEELIN F_DynamicFindMethod((STRPTR)(Method), _object_class(Obj), NULL);
			
			if (m)
			{
				Method = m->ID;
			}
		}

		return IFEELIN F_ClassDoA(_object_class(Obj),Obj,Method,Msg);
	}
	return 0;
}
//+
///f_classdoa
F_LIB_CLASS_DO
/*

   This function is the heart of the invokation mechanism.

*/
{
	while (Class)
	{
		FMethod func=NULL;

		/* The methods array is prefered over the dispatcher. If a dispatcher
		is  defined,  it  is  called *only* if the method is not handled by a
		function in the array. */

		if (Class->Methods)
		{
			FClassMethod *me;

			for (me = Class->Methods ; me->Function ; me++)
			{
				if (me->ID == Method)
				{
				   func = me->Function; break;
				}
			}
		}

		/* Method has not been found in the methods table.  If  class  has  a
		dispatcher  it  is called instead. Otherwise the class is replaced by
		the super class and we try again. */

		if (!func)
		{
		   func = ((struct in_FeelinClass *)(Class))->dispatcher;
		}

		if (func)
		{
			uint32 prev_method = FeelinBase->debug_classdo_method;
			FObject prev_object = FeelinBase->debug_classdo_object;
			FClass *prev_class = FeelinBase->debug_classdo_class;
			uint32 rc;

			/* Previous Method/Object/Class are  saved  before  calling  the
			method  function, and restored after. These information are used
			by F_LogA() to locate message loggin.
			
			This is absolutely not thread safe, but who cares :-) */

			#if 0
			if (FeelinBase->Public.Console)
			{
			   IFEELIN F_Log(0,"CLASSDO >> %s{%08lx}.0x%08lx",Class->Name,Obj,Method);
			}
			#endif

			FeelinBase->debug_classdo_class = Class;
			FeelinBase->debug_classdo_object = Obj;
			FeelinBase->debug_classdo_method = Method;

			rc = F_CALL_EXTERN_METHOD(func,Class,Obj,Method,Msg,(struct FeelinBase *) FeelinBase);
 
			FeelinBase->debug_classdo_class = prev_class;
			FeelinBase->debug_classdo_object = prev_object;
			FeelinBase->debug_classdo_method = prev_method;

			return rc;
		}
		Class = Class->Super;
	}

	/* The method is not implemented by the  class,  or  any  of  its  super
	class, 0 is returned. */

	return 0;
}
//+
///f_superdoa
F_LIB_SUPER_DO
{
	if (Obj && Class && Class->Super)
	{
		return IFEELIN F_ClassDoA(Class->Super,Obj,Method,Msg);
	}
	return 0;
}
//+

/************************************************************************************************
*** Object management ***************************************************************************
************************************************************************************************/

///f_new_obja
F_LIB_NEW_OBJ
{
	struct in_FeelinClass *Class = (struct in_FeelinClass *) IFEELIN F_OpenClass(Name);

	if (Class)
	{
		FObject Obj = IFEELIN F_NewP(Class->pool, sizeof (FClass *) + Class->public.Offset + Class->public.LocalSize);

		if (Obj)
		{
			/* Class pointer is located *before* object data. The pointer need
			to be adjusted. */

			*((FClass **)(Obj)) = (FClass *)(Class);
			Obj = (FObject)((uint32)(Obj) + sizeof (FClass *));

			if (IFEELIN F_DoA(Obj,FM_New,Tags))
			{
				return Obj;
			}
	
			IFEELIN F_Do(Obj,FM_Dispose);
		}
		IFEELIN F_CloseClass((FClass *) Class);
	}
	return NULL;
}
//+
///f_make_obja
F_LIB_MAKE_OBJ
{
	switch (Type)
	{
		case FV_MakeObj_Label:
		{
			if (*Params++)
			{
				return TextObject,

					FA_Widget_SetMax,       FV_Widget_SetBoth,
					FA_Widget_Chainable,    FALSE,
					FA_Text_Contents,   	Params[0],
				
				TAG_MORE, Params + 1);
			}
			else
			{
				return TextObject,

					FA_Widget_SetMax,       FV_Widget_SetHeight,
					FA_Widget_Chainable,    FALSE,
					FA_Text_HCenter,        TRUE,
					FA_Text_Contents,       Params[0],

				TAG_MORE, Params + 1);
			}
		}

		case FV_MakeObj_Button:
		{
			return TextObject,

				FA_Element_Class,       "button",
				FA_Widget_SetMax,         FV_Widget_SetHeight,
				FA_Widget_Mode,         FV_Widget_Mode_Release,
				FA_Text_Contents,       Params[0],
				FA_Text_HCenter,       	TRUE,

			TAG_MORE, Params + 1);
		}

		case FV_MakeObj_BarTitle:
		{
			return BarObject,

				FA_Widget_SetMax,         FV_Widget_SetHeight,
			   "FA_Bar_Title",          Params[0],

			TAG_MORE, Params + 1);
		}

		case FV_MakeObj_Gauge:
		{
			return GaugeObject,

				FA_Area_Orientation,    Params[0] ? FV_Area_Orientation_Horizontal : FV_Area_Orientation_Vertical,
			   "FA_Numeric_Min",        Params[1],
			   "FA_Numeric_Max",        Params[2],
			   "FA_Numeric_Value",      Params[3],
			   
			TAG_MORE, Params + 4);
		}

		case FV_MakeObj_Slider:
		{
			return SliderObject,

				FA_Area_Orientation,    Params[0] ? FV_Area_Orientation_Horizontal : FV_Area_Orientation_Vertical,
			   "FA_Numeric_Min",        Params[1],
			   "FA_Numeric_Max",        Params[2],
			   "FA_Numeric_Value",      Params[3],

			TAG_MORE, Params + 4);
		}

		case FV_MakeObj_Prop:
		{
			return PropObject,
			   
				FA_Area_Orientation,    Params[0] ? FV_Area_Orientation_Horizontal : FV_Area_Orientation_Vertical,
			   "FA_Prop_Entries",       Params[1],
			   "FA_Prop_Visible",       Params[2],
			   "FA_Prop_First",         Params[3],
			   
			   TAG_MORE, Params + 4);
		}

		case FV_MakeObj_String:
		{
			return StringObject,

				FA_Widget_SetMax,         FV_Widget_SetHeight,
			   "FA_String_Contents",    Params[0],
			   "FA_String_MaxLen",      Params[1],
			   
			   TAG_MORE, Params + 2);
		}

		case FV_MakeObj_Checkbox:
		{
			return ImageObject,
			   
				FA_Element_Class,       "checkbox",
				FA_Widget_SetMin,         FV_Widget_SetBoth,
				FA_Widget_SetMax,         FV_Widget_SetBoth,
				FA_Widget_Mode,         FV_Widget_Mode_Toggle,
				FA_Widget_Selected,     Params[0],
			   
			   TAG_MORE, Params + 1);
		}
	}
	return NULL;
}
//+
///f_dispose_obj
F_LIB_DISPOSE_OBJ
{
	if (Obj)
	{
		struct in_FeelinClass *Class = (struct in_FeelinClass *) _object_class(Obj);

		#ifdef F_ENABLE_SAFE_INVOKATION
		{
			FClass *node;

			for (F_CLASSES_EACH(node))
			{
			   if (((struct in_FeelinClass *) node) == Class) break;
			}

			if (node == NULL)
			{
				IFEELIN F_Log(FV_LOG_DEV,"(db)F_DisposeObj() 0x%08lx is not an Object !! - Class 0x%08lx",Obj,Class);

				return 0;
			}
		}
		#endif

		IFEELIN F_Do(Obj,FM_Dispose);

		IFEELIN F_Dispose((APTR)((uint32)(Obj) - sizeof (FClass *)));

		IFEELIN F_CloseClass((FClass *) Class);
	}

	return 0;
}
//+

/************************************************************************************************
*** Extended ************************************************************************************
************************************************************************************************/

///f_get
F_LIB_GET
/*

   This function was  created  because  AmigE  didn't  support  va_arg  (it
   creates  A  LOT  of arrays instead). This function may seam useless now,
   but in fact its very nice. If you only need to get one attribute because
   the value used to receive the result is always cleared.

*/
{
	uint32 rc=0;

	IFEELIN F_Do(Obj, FM_Get, Tag, &rc, TAG_DONE);

	return rc;
}
//+
///f_set
F_LIB_SET
{
	IFEELIN F_Do(Obj, FM_Set, Tag, Data, TAG_DONE);
}
//+

