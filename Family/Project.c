/*

$VER: 06.00 (2006/10/19)

	Objects are no longer connected and disconnected  during  the  AddMember
	and RemMember methods.

$VER: 05.00 (2005/12/10)

   	Metaclass support.
   
	The FM_New method is now passed to the super class  *after*  the  family
	has  been  created. Thus, if the family failed to be created, the FM_Set
	method won't be invoked by FC_Object.
 
$VER: 04.00 (2004/08/31)

	Uses the system variable DBG_FAMILY to toggle verbose of objects  adding
	and removing.

	Added the possibility to call hooks to create and delete family's nodes.

*/

#include "Project.h"

struct ClassUserData               *CUD;

///METHODS
F_METHOD_PROTO(void,Family_New);
F_METHOD_PROTO(void,Family_Dispose);
F_METHOD_PROTO(void,Family_Get);
F_METHOD_PROTO(void,Family_Add);
F_METHOD_PROTO(void,Family_Rem);
//+

///Class_New
F_METHOD(FObject,Class_New)
{
	CUD = F_LOD(Class,Obj);

	if (F_SUPERDO())
	{
		#ifdef F_ENABLE_VERBOSE

		CUD->db_Notify = DOSNotifyObject,"FA_DOSNotify_Name",DEBUGVARNAME,End;

		IFEELIN F_Do(Obj, F_IDM(FM_Meta_Debug));
	  
		IFEELIN F_Do(CUD->db_Notify,FM_Notify,"FA_DOSNotify_Update",FV_Notify_Always,
			Obj,F_IDM(FM_Meta_Debug),0);

		#endif

		CUD->NodePool = IFEELIN F_CreatePool(sizeof (FFamilyNode),

			FA_Pool_Items, 32,
			FA_Pool_Name,  "family-node",

		TAG_DONE);
	  
		if (CUD->NodePool != NULL)
		{
			return Obj;
		}
	}

	return NULL;
}
//+
///Class_Dispose
F_METHOD(uint32,Class_Dispose)
{
   CUD = F_LOD(Class,Obj);
 
   IFEELIN F_DeletePool(CUD->NodePool); CUD->NodePool = NULL;

   #ifdef F_ENABLE_VERBOSE

   IFEELIN F_DisposeObj(CUD->db_Notify); CUD->db_Notify = NULL;

   #endif
   
   return F_SUPERDO();
}
//+

#ifdef F_ENABLE_VERBOSE

///Class_Debug
F_METHOD(void,Class_Debug)
{
   STRPTR buf = IFEELIN F_New(1024);
   
   CUD = F_LOD(Class,Obj);

   CUD->db_AddMember = FALSE;
   CUD->db_RemMember = FALSE;

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
			#ifdef F_ENABLE_VERBOSE
			F_METHODS_ADD        (Class_Debug,   "Private1"),
			#endif
			F_METHODS_ADD_STATIC (Class_New,      FM_New),
			F_METHODS_ADD_STATIC (Class_Dispose,  FM_Dispose),

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
		 STATIC F_METHODS_ARRAY =
		 {
			F_METHODS_ADD_STATIC(Family_New,      FM_New),
			F_METHODS_ADD_STATIC(Family_Dispose,  FM_Dispose),
			F_METHODS_ADD_STATIC(Family_Get,      FM_Get),
			F_METHODS_ADD_STATIC(Family_Add,      FM_AddMember),
			F_METHODS_ADD_STATIC(Family_Rem,      FM_RemMember),

			F_ARRAY_END
		 };

		 STATIC F_TAGS_ARRAY =
		 {
			F_TAGS_ADD_SUPER(Object),
			F_TAGS_ADD_LOD,
			F_TAGS_ADD_METHODS,

			F_ARRAY_END
		 };

		 return F_TAGS_PTR;
	  }
//+
   }
   return NULL;
}
