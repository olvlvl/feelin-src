/*
 
*************************************************************************************************
 
$VER: vs.rv (yyyy/mm/dd)

************************************************************************************************/

/* The "Project.h" file should be generated with the F_Make command. */

#include "Project.h"

struct ClassUserData               *CUD;

///METHODS
F_METHOD_PROTO(void,);
//+

///Class_New
F_METHOD(uint32, Class_New)
{
	CUD = F_LOD(Class,Obj);
   
	return F_SUPERDO();
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
	CUD = F_LOD(Class,Obj);

	return F_SUPERDO();
}
//+
 
F_QUERY()
{
	switch (Which)
	{
///Meta
		case FV_Query_MetaClassTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Class_New,      FM_New),
				F_METHODS_ADD_STATIC(Class_Dispose,  FM_Dispose),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Class),
				F_TAGS_ADD(LODSize, sizeof (struct ClassUserData)),
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_VALUES_ARRAY(<values_name>) =
			{
				F_VALUES_ADD("<name>",<value>)
		   
				F_ARRAY_END
			};
   
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD("<name>",<type>),
				F_ATTRIBUTES_ADD_VALUES("<name>",<type>,<values_name>),
				
				/* the two following macros are only used for creating
				low-level classe whom IDs are defined in
				<libraries/feelin.h> */
				
				F_ATTRIBUTES_ADD_STATIC(<id>,<type>),
				F_ATTRIBUTES_ADD_STATIC_VALUES(<id>,<type>,<values_name>),
				
				F_ARRAY_END
			};
				
			STATIC F_METHODS_ARRAY =
			{

				/* to implement one of your superclass method, set <name> to
				the   full   name  of  the  method  e.g.  FM_Object_Dispose.
				Otherwise, to define your  own  method  just  name  it  e.g.
				"Multiply" */
	 
				F_METHODS_ADD(<func>,"<name>"),
				
				F_METHODS_ADD_STATIC(<func>,<id>),
				F_METHODS_ADD_BOTH(<func>,"<name>",<id>),
				
				F_ARRAY_END
			};

			/* style properties */

			STATIC F_PROPERTIES_ARRAY =
			{
				F_PROPERTIES_ADD("<propername-name>"),

				F_ARRAY_END
			};
			
			/* Use the following macros to import methods or attributes from
			one of your superclasses. */
	
			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("<name>"),

				F_ARRAY_END
			};

			/* Use the following macros to automatically import  methods  or
			attributes from any class when they are created */
			
			STATIC F_AUTOS_ARRAY =
			{
				F_AUTOS_ADD("<name>"),
			   
				F_ARRAY_END
			};
						
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(<superclass-name>),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_PROPERTIES,
				F_TAGS_ADD_RESOLVEDS,
				F_TAGS_ADD_AUTOS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
///Prefs
		case FV_Query_PrefsTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD_STATIC(Prefs_New, FM_New),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(PreferenceGroup),
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
