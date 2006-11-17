/*
**    lib_Atoms.c
**
**    Global string management
**
**    © 2001-2006 Olivier LAVIALE (gofromiel@gofromiel.com)
**
*************************************************************************************************

$VER: 01.00 (2006/05/05)

************************************************************************************************/

#include "Private.h"
#include "lib_Atoms.h"

/*

GOFROMIEL @ HOMBRE: Je préfére que "lib_Atom.h" soit inclus ici pour  garder
une  indépendance  totale,  surtout  que  ce  qui est défini dans le fichier
include n'est utilisé nulle par ailleurs.

*/

//#define DB_HASH
//#define DB_RELEASE_GARBAGE

/************************************************************************************************
*** public **************************************************************************************
************************************************************************************************/

///f_atom_hash
STATIC struct in_Atom * f_atom_hash(STRPTR Key, uint32 KeyLength, uint32 *HashPtr, struct in_FeelinBase *FeelinBase)
{
	if (Key && KeyLength)
	{
		uint32 len = KeyLength;
		uint8 *k = Key;
		struct in_Atom *atom;
		uint32 h;

		#ifdef F_ENABLE_SAFE_STRING

		if (((int32)(Key) < 0xFFFF) || (IEXEC TypeOfMem(Key) == 0))
		{
			IFEELIN F_Log(0,"(db)F_HashFind() invalid string address (0x%08lx)",Key);

			return NULL;
		}

		#endif

		for (h = len ; len ; len--)
		{
			h = ((h << 5) ^ (h >> 27)) ^ *k++;
		}

		#ifdef DB_HASH

		IDOS_ VFPrintf(FeelinBase->Public.Console, "key (", NULL);
		IDOS_ Flush(FeelinBase->Public.Console);
		IDOS_ Write(FeelinBase->Public.Console,Key,KeyLength);
		IDOS_ FPrintf(FeelinBase->Public.Console, ") hash (0x%08lx, 0x%08lx)\n", h, h % F_ATOMS_TABLE->Size);

		#endif

		h = h % F_ATOMS_TABLE->Size;

		if (HashPtr)
		{
			*HashPtr = h;
		}

		for (atom = (struct in_Atom *) F_ATOMS_TABLE-> Entries[h] ; atom ; atom = atom->next)
		{
			if (atom->public.KeyLength == KeyLength)
			{
				uint8 *c = atom->public.Key;
				k = Key;

				for (len = KeyLength ; len ; len--)
				{
					if (*k++ != *c++) break;
				}

				if (!len) return atom;
			}
		}
	}
	return NULL;

}
//+

/************************************************************************************************
*** public **************************************************************************************
************************************************************************************************/

///f_atom_find
F_LIB_ATOM_FIND
{
	if (Length == ALL)
	{
		Length = IFEELIN F_StrLen(Str);
	}

	if (Str && Length)
	{
		uint32 hash = 0;
		struct in_Atom *atom = f_atom_hash(Str, Length, &hash, FeelinBase);

		if (atom)
		{
			#ifdef F_ENABLE_ATOM_GARBAGE

			if (atom->refs != 0)

			#endif
			{
				return F_ATOM_PUBLICIZE(atom);
			}
		}
	}

	return NULL;
}
//+
///f_atom_obtain
F_LIB_ATOM_OBTAIN
{
	if (Length == ALL)
	{
		Length = IFEELIN F_StrLen(Str);
	}

	if (Str && Length)
	{
		uint32 hash = 0;
		struct in_Atom *atom;

		F_ATOMS_LOCK;

		atom = f_atom_hash(Str, Length, &hash, FeelinBase);

		if (atom)
		{
			atom->refs++;
		}
		else
		{
			atom = IFEELIN F_NewP(F_ATOMS_POOL, sizeof (struct in_Atom) + Length + 1);

			if (atom)
			{
				atom->next = (struct in_Atom *) F_ATOMS_TABLE->Entries[hash];
				atom->public.Key = (APTR)((uint32)(atom) + sizeof (struct in_Atom));
				atom->public.KeyLength = Length;
				atom->refs = 1;

				IEXEC CopyMem(Str, atom->public.Key, atom->public.KeyLength);

				F_ATOMS_TABLE->Entries[hash] = (FHashLink *) atom;
			}
		}

		F_ATOMS_UNLOCK;

		if (atom)
		{
			return F_ATOM_PUBLICIZE(atom);
		}
	}

	return NULL;
}
//+
///f_atom_release
F_LIB_ATOM_RELEASE
{
	if (Atom != NULL)
	{
		uint32 hash = 0;

		F_ATOMS_LOCK;

		if (f_atom_hash(Atom->Key, Atom->KeyLength, &hash, FeelinBase))
		{
			FHashLink *node;

			#ifndef F_ENABLE_ATOM_GARBAGE
			FHashLink *prev = NULL;
			#endif

			struct in_Atom *real = F_ATOM_PRIVATIZE(Atom);

			for (node = F_ATOMS_TABLE->Entries[hash] ; node ; node = node->Next)
			{
				if (node == (FHashLink *) real)
				{
					real->refs--;

					if (real->refs == 0)
					{
						#ifdef F_ENABLE_ATOM_GARBAGE

						FeelinBase->atoms_garbage_count++;

						#else

						if (prev)
						{
							prev->Next = node->Next;
						}
						else
						{
							F_ATOMS_TABLE->Entries[hash] = node->Next;
						}

						IFEELIN F_Dispose(real);

						#endif
					}

					break;
				}
				#ifndef F_ENABLE_ATOM_GARBAGE
				else
				{
					prev = node;
				}
				#endif
			}

			if (node == NULL)
			{
				IFEELIN F_Log(FV_LOG_DEV,"(db)F_AtomRelease() Unknown FAtom (0x%08lx) with key (%s)",Atom,Atom->Key);
			}
		}
		else
		{
			IFEELIN F_Log(FV_LOG_DEV,"(db)F_AtomRelease() Unknown atom key (%s)",Atom->Key);
		}

		#ifdef F_ENABLE_ATOM_GARBAGE

		if (FeelinBase->atoms_garbage_count == FV_ATOM_GARBAGE_LIMIT)
		{
			uint32 i;

			#ifdef DB_RELEASE_GARBAGE
			IFEELIN F_Log(0,"garbage limit has been reached !!");
			#endif

			for (i = 0 ; i < F_ATOMS_TABLE->Size ; i++)
			{
				struct in_Atom *node;
				struct in_Atom *next;
				struct in_Atom *prev = NULL;

				for (node = (struct in_Atom *) F_ATOMS_TABLE->Entries[i] ; node ; node = next)
				{
					next = node->next;

					if (node->refs == 0)
					{
						if (prev)
						{
							prev->next = next;
						}
						else
						{
							F_ATOMS_TABLE->Entries[i] = (FHashLink *) next;
						}

						#ifdef DB_RELEASE_GARBAGE
						IFEELIN F_Log(0,"delete garbage (%s) prev (0x%08lx) next (0x%08lx)", node->public.Key, prev, next);
						#endif

						IFEELIN F_Dispose(node);
					}
					else
					{
						prev = node;
					}
				}
			}

			FeelinBase->atoms_garbage_count = 0;
		}

		#endif

		F_ATOMS_UNLOCK;
	}
}
//+

///f_atom_create_pool
F_LIB_ATOM_CREATE_POOL
{
	APTR pool = IFEELIN F_CreatePool(sizeof (struct in_LocalAtom), FA_Pool_Items, 20, TAG_DONE);

	if (pool != NULL)
	{
		struct in_LocalAtomPool *ap = IFEELIN F_NewP(pool, sizeof (struct in_LocalAtomPool));

		if (ap != NULL)
		{
			ap->pool = pool;

			return ap;
		}

		IFEELIN F_DeletePool(pool);
	}

	return NULL;
}
//+
///f_atom_delete_pool
F_LIB_ATOM_DELETE_POOL
{
	F_ATOMS_LOCK;

	if (Pool != NULL)
	{
		struct in_LocalAtom *node;

		for (node = Pool->local_atoms ; node ; node = node->next)
		{
			IFEELIN F_AtomRelease(node->atom);
		}

		IFEELIN F_DeletePool(Pool->pool);
	}

	F_ATOMS_UNLOCK;
}
//+
///f_atom_findp
F_LIB_ATOM_FINDP
{
	struct in_LocalAtom *latom = NULL;

	F_ATOMS_LOCK;

	if (Pool != NULL)
	{
		FAtom *atom = IFEELIN F_AtomFind(Key, KeyLength);

		if (atom != NULL)
		{

			for (latom = Pool->local_atoms ; latom ; latom = latom->next)
			{
				if (latom->atom == atom)
				{
					break;
				}
			}
		}
	}

	F_ATOMS_UNLOCK;

	if (latom != NULL)
	{
		return F_LATOM_PUBLICIZE(latom);
	}

	return NULL;
}
//+
///f_atom_obtainp
F_LIB_ATOM_OBTAINP
{
	struct in_LocalAtom *latom = NULL;

	F_ATOMS_LOCK;

	if (Pool != NULL)
	{
		FAtom *atom = IFEELIN F_AtomObtain(Key, KeyLength);

		if (atom != NULL)
		{
			for (latom = Pool->local_atoms ; latom ; latom = latom->next)
			{
				if (latom->atom == atom)
				{
					latom->refs++;

					#ifdef DB_OBTAINP
					IFEELIN F_Log(0,"re-use (%s) refs (%ld)", atom->Key, latom->refs);
					#endif

					break;
				}
			}

			if (latom == NULL)
			{
				latom = IFEELIN F_NewP(Pool->pool, sizeof (struct in_LocalAtom));

				if (latom != NULL)
				{
					latom->next = Pool->local_atoms;
					latom->atom = atom;
					latom->refs = 1;

					Pool->local_atoms = latom;

					#ifdef DB_OBTAINP
					IFEELIN F_Log(0,"local atom created for (%s)", latom->atom->Key);
					#endif
				}
				else
				{
					IFEELIN F_AtomRelease(atom);
				}
			}
			else
			{
				IFEELIN F_AtomRelease(atom);
			}
		}
	}

	F_ATOMS_UNLOCK;

	return latom ? F_LATOM_PUBLICIZE(latom) : NULL;
}
//+
///f_atom_releasep
F_LIB_ATOM_RELEASEP
{
	if ((Pool != NULL) && (Atom != NULL))
	{
		struct in_LocalAtom *latom = F_LATOM_PRIVATIZE(Atom);
		struct in_LocalAtom *prev = NULL;
		struct in_LocalAtom *node;

		F_ATOMS_LOCK;

		for (node = Pool->local_atoms ; node ; node = node->next)
		{
			if (node == latom)
			{
				node->refs--;

				if (node->refs == 0)
				{
					if (prev)
					{
						prev->next = node->next;
					}
					else
					{
						Pool->local_atoms = node->next;
					}

					IFEELIN F_AtomRelease(node->atom);
					IFEELIN F_Dispose(node);
				}

				break;
			}
			else
			{
				prev = node;
			}
		}

		F_ATOMS_UNLOCK;
	}
}
//+
