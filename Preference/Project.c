/*

$VER: 05.00 (2005/12/30)
 
	Metaclass support.
   
	The class is now a subclass of  CSSDocument.  Preference  files  are  no
	longer  painful-to-edit IFF files, but user-friendly CSS ones. The class
	has   been    totaly    rewritten    the    FM_Preference_Resolve    and
	FM_Preference_ResolveInt  are  deprecated,  and  were  replaced  by  the
	FM_Preference_Find method.
	
	The FM_Prefrence_Find method does no longer return the  data  associated
	with  a preference key, but returns TRUE if the resolve process succeed.
	Two members have been added: "APTR *Data"  and  "uint32  *Value".  These
	fields  must  be used to obtain the data associated to a preference key,
	and its possible numeric representation.
	
	This version introduce  the  concept  of  *associated  data*.  With  the
	FM_Preference_ObtainAssociated    and    FM_Preference_ReleaseAssociated
	methods.
 
	A notification statement set upon  the  reference  object  is  now  used
	instead of a global list to keep objects up to date.
	
$VER: 04.00 (2005/05/03)
 
	Preference items have been totaly changed. They are no longer defined as
	"FP_Button_Frame"  but  more  CSS  style like e.g. "$frame-button". Note
	well the dollar sign, which is used instead  of  "FP_"  to  recognize  a
	preference item from a developer override.
 
$VER: 03.00 (2004/12/18)

	The class does no longer manage a global preference object.  Instead,  a
	reference  object  can  be defined for each object. The AppServer shared
	object holds a FC_Preference  object  used  by  each  application  as  a
	reference for their own FC_Preference object.

	The class does no longer update applications, which  was  a  very  dirty
	job.  Instead,  the  FA_Preference_Update  attribute is set to TRUE when
	preferences have been  modified.  By  setting  a  notification  on  this
	attribute, applications can react on preferences modifications like they
	want to. The attribute is NOT set by the FM_Preference_Read method,  nor
	by any FM_Preference_AddXxx method. The attribute is set to TRUE only on
	notification of file modification or by FC_PreferenceEditor objects.

	The FA_Preference_Application attribute as been removed  because  it  is
	now useless.

	When writting preference file, items with the  same  contents  as  their
	reference are removed.

*/

#include "Project.h"
				
struct ClassUserData               *CUD;

///METHODS
F_METHOD_PROTO(void,Prefs_New);
F_METHOD_PROTO(void,Prefs_Dispose);
F_METHOD_PROTO(void,Prefs_Get);
								
F_METHOD_PROTO(void,Prefs_Clear);
F_METHOD_PROTO(void,Prefs_Read);
F_METHOD_PROTO(void,Prefs_Write);

F_METHOD_PROTO(void,Prefs_FileChange);
F_METHOD_PROTO(void,Prefs_Merge);
F_METHOD_PROTO(void,Prefs_GetProperty);
F_METHOD_PROTO(void,Prefs_AddProperty);
F_METHOD_PROTO(void,Prefs_ObtainStyle);
F_METHOD_PROTO(void,Prefs_ReleaseStyle);
//+

///Class_New
F_METHOD(uint32,Class_New)
{
	CUD = F_LOD(Class,Obj);
	
	CUD->arbiter = IFEELIN F_NewObj(FC_Object, TAG_DONE);

	if (CUD->arbiter)
	{
		return F_SUPERDO();
	}
	
	return 0;
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
	CUD = F_LOD(Class,Obj);

	if (CUD->arbiter)
	{
		IFEELIN F_DisposeObj(CUD->arbiter);

		CUD->arbiter = NULL;
	}
	
	return F_SUPERDO();
}
//+

#ifdef F_ENABLE_DEBUG
///Class_Debug
F_METHOD(void,Class_Debug)
{
	STRPTR buf = IFEELIN F_New(1024);

	CUD = F_LOD(Class,Obj);

	CUD->db_Read    = FALSE;
	CUD->db_Write   = FALSE;
	CUD->db_Resolve = FALSE;

	if (buf)
	{
		BPTR file = IDOS_ Open(DEBUGVARNAME,MODE_OLDFILE);

		if (file)
		{
			if (IDOS_ FGets(file,buf,1024))
			{
				struct RDArgs *rda = (struct RDArgs *) IDOS_ AllocDosObject(DOS_RDARGS,NULL);

				if (rda)
				{
					rda->RDA_Source.CS_Buffer = buf;
					rda->RDA_Source.CS_Length = IFEELIN F_StrLen(buf);

					if (IDOS_ ReadArgs(TEMPLATE,&CUD->db_Array,rda))
					{
						IDOS_ FreeArgs(rda);
					}
					IDOS_ FreeDosObject(DOS_RDARGS,rda);
				}
			}
			IDOS_ Close(file);
		}
		IFEELIN F_Dispose(buf);
	}
}
//+
#endif

F_QUERY()
{
	switch (Which)
	{
///Meta
		case FV_Query_MetaClassTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_OVERRIDE_STATIC(Class_New,      FM_New),
				F_METHODS_OVERRIDE_STATIC(Class_Dispose,  FM_Dispose),

				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(Class),
				F_TAGS_ADD(LocalSize, sizeof (struct ClassUserData)),
				F_TAGS_ADD_METHODS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_ATTRIBUTES_ARRAY =
			{
				F_ATTRIBUTES_ADD("Name", FV_TYPE_STRING),
				F_ATTRIBUTES_ADD("Reference", FV_TYPE_OBJECT),
				F_ATTRIBUTES_ADD("Update", FV_TYPE_BOOLEAN),

				F_ARRAY_END
			};

			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(Prefs_Clear, "Clear"),
				F_METHODS_ADD(Prefs_Read, "Read"),
				F_METHODS_ADD(Prefs_Write, "Write"),
				F_METHODS_ADD(Prefs_FileChange, "Private1"),
				F_METHODS_ADD(Prefs_Merge, "FM_Document_Merge"),

				F_METHODS_OVERRIDE(Prefs_GetProperty, "CSSDocument", "GetProperty"),
				F_METHODS_OVERRIDE(Prefs_AddProperty, "CSSDocument", "AddProperty"),
				
				F_METHODS_ADD(Prefs_ObtainStyle, "ObtainStyle"),
				F_METHODS_ADD(Prefs_ReleaseStyle, "ReleaseStyle"),

				F_METHODS_OVERRIDE_STATIC(Prefs_New,     FM_New),
				F_METHODS_OVERRIDE_STATIC(Prefs_Dispose, FM_Dispose),
				F_METHODS_OVERRIDE_STATIC(Prefs_Get,	 FM_Get),
				
				F_ARRAY_END
			};
			
			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FA_Document_Pool"),
				F_RESOLVEDS_ADD("FA_Document_Source"),
				F_RESOLVEDS_ADD("FA_Document_SourceType"),
				F_RESOLVEDS_ADD("FM_Document_Clear"),
				F_RESOLVEDS_ADD("FM_Document_Numerify"),
				F_RESOLVEDS_ADD("FM_Document_Write"),
				
				F_RESOLVEDS_ADD("FA_CSSDocument_Declarations"),
				
				F_ARRAY_END
			};

			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(CSSDocument),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_ATTRIBUTES,
				F_TAGS_ADD_RESOLVEDS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
