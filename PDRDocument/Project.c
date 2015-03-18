/*
 
*************************************************************************************************
 
$VER: 01.00 (2006/03/06)

	Persistant Data Resouce Document


	FA_Persist

		This attribute is used to define  which  values  persists  from  one
		session  to  another.  For  example, one may want to have persistent
		window's  coordinates,  which  can  be  simply  done   by   defining
		FA_Persist as following:

			FA_Persist, "left right width height"

		or

			FA_Persist, "box"

		Only objects with an ID defined with the FA_ID  attribute  may  have
		persistent datas.


	OBJECT

		>> Il faut le faire au retour du renvoie vers FC_Object. Lorsque  la
		m�thode  redescent,  elle  donne  un  handler.  Pas  de  handler  si
		(FA_ID==NULL) ou (FA_Persist==NULL). Le  handler  contient  l'id  de
		l'objet  (chaine  de  carat�res)  ainsi  que  la  chaine  FA_Persist
		d�cortiqu�e.

F_METHOD(uint32,SavePersistentAttributes)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FPersistentAttribute *pa = (FPersistentAttribute *) F_SUPERDO();

	for (pa = ph->Attributes ; pa ; pa = pa->Next)
	{
		if (pa->Atom == _atom(LEFT))
		{
			F_PERSISTENT_SET_TYPED(pa, FV_TYPE_INTEGER, LOD->Box.x);
		}
		else if ...
	}

	return handler;
};

F_METHOD(uint32,LoadPersistenAttributes)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj),

	FPersistentAttribute *pa = (FPersistentAttribute *) F_SUPERDO();

	for ( ; pa ; pa = pa->Next)
	{
		if (pa->Atom == _atom(LEFT)
		{
			F_PERSISTENT_GET_NUMERIC(pa, LOD->Box.y);
		}
	}
}


<?xml version="1.0" ?>

<feelin:persist>

	<space id="windows.main">
		<item id="left">12</item>
		<item id="right">60</item>
	</space>

</feelin:persist>


		>> FC_PersistantSpace ne doit pas rester ouvert tout le temps de  la
		session,  �a  sert  �  rien,  il  devrait  �tre  aussi vite cr�e que
		d�truit.


		Lorsque FM_Import / FM_Export atteint FC_Object,

		FC_PDRDocument

			Nouvelle classes pour sauver les donn�es persistentes.

			Sous-classe de FC_XMLDocument.

			Les donn�es sont sauv�es dans un fichier XML, le format  binaire
			est par cons�quent interdit.


			** FM_PersistantSpace_Import **

			On lui donne une chaine  de  caract�res  avec  les  attributs  �
			importer  s�par�s  par un espace. Les attributs sont s�par�s par
			des  espaces  et  sont  r�solus  �   l'aide   de   la   fonction
			F_FindAttribute().   Il   faut  que  l'on  sache  la  nature  de
			l'attribut pour savoir quel type de don�e il contient,  pour  le
			convertir par exemple lors de 'limport / export. Attention donc,
			le  type   FV_TYPE_POINTER   et   ses   sous   classes   except�
			FV_TYPE_STRING  sont  interdits. Il va surement falloir cr�er de
			nouveaux attributs.



			FM_PersistantSpace_Export

************************************************************************************************/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void,PDRDocument_FindHandle);
F_METHOD_PROTO(void,PDRDocument_CreateHandle);
F_METHOD_PROTO(void,PDRDocument_Parse);
F_METHOD_PROTO(void,PDRDocument_Clear);
F_METHOD_PROTO(void,PDRDocument_Write);
//+

F_QUERY()
{
	switch (Which)
	{
///Class
		case FV_Query_ClassTags:
		{
			STATIC F_METHODS_ARRAY =
			{
				F_METHODS_ADD(PDRDocument_FindHandle,   "FindHandle"),
				F_METHODS_ADD(PDRDocument_CreateHandle, "CreateHandle"),
				
				F_METHODS_OVERRIDE(PDRDocument_Parse, "Document", "Parse"),
				F_METHODS_OVERRIDE(PDRDocument_Clear, "Document", "Clear"),
				F_METHODS_OVERRIDE(PDRDocument_Write, "Document", "Write"),
				
				F_ARRAY_END
			};

			STATIC F_RESOLVEDS_ARRAY =
			{
				F_RESOLVEDS_ADD("FM_Document_CreatePool"),
				F_RESOLVEDS_ADD("FM_Document_Stringify"),
				F_RESOLVEDS_ADD("FM_Document_Numerify"),

				F_RESOLVEDS_ADD("FM_XMLDocument_Push"),
				F_RESOLVEDS_ADD("FM_XMLDocument_Pop"),
				F_RESOLVEDS_ADD("FM_XMLDocument_Add"),

				F_ARRAY_END
			};

			STATIC F_ATOMS_ARRAY =
			{
				F_ATOMS_ADD(F_PDRDOCUMENT_ROOT),
				F_ATOMS_ADD(F_PDRDOCUMENT_SPACE),
				F_ATOMS_ADD(F_PDRDOCUMENT_ITEM),
				F_ATOMS_ADD(F_PDRDOCUMENT_ID),

				F_ARRAY_END
			};
			
			STATIC F_TAGS_ARRAY =
			{
				F_TAGS_ADD_SUPER(XMLDocument),
				F_TAGS_ADD_LOD,
				F_TAGS_ADD_METHODS,
				F_TAGS_ADD_RESOLVEDS,
				F_TAGS_ADD_ATOMS,

				F_ARRAY_END
			};

			return F_TAGS_PTR;
		}
//+
	}
	return NULL;
}
