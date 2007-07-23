/*
**    Memory management
**
**    © 2001-2005 Olivier LAVIALE (gofromiel@gofromiel.com)
**
**************************************************************************************************

$VER: 04.02 (2006/05/20)

	Removed a really stupid bug that was creating a lot of fragmentation: if
	the  first puddle couldn't contain a new drop, a new puddle was created,
	a 'break' was misplaced.

	Forgot to clear 'puddles' when  creating  the  memory  pool,  which  was
	sometimes  resulting in random crashes when the pool was created without
	MEMF_CLEAR.

$VER: 04.00 (2005/08/05)

	Memory system  rewritten  and  greatly  improved  (particuliarly  memory
	disposal). F_DisposeP() is now deprecated.

	Memory is now cleared and filled with a turbo method.
	
	Added AmigaOS 4 support.

$VER: 03.02 (2005/04/06)
 
	Removed  an  error   introduced   while   porting   ASM   code   to   C.
	f_chunk_create()  failed allocating the last free memory chunk, making a
	new puddle to be created. I was testing a '<' instead of '<='...
 
$VER: 03.00 (2005/01/13)

	The whole memory system has finaly been ported to C, even  the  almighty
	F_NewP(), F_Dispose() and F_DisposeP() functions. The end of one era :-)

************************************************************************************************/

#include "Private.h"

/*

	? chaque puddle doit maintenir une variable 'largest' qui est la  taille
	de son plus gros morceau de mémoire

	>> le principe de la piscine est que  les  allocations  sont  égales  en
	taille,  on  ne devrait donc pas avoir à se soucier de la taille la plus
	large, si il y a de la  mémoire  libre,  sa  taille  est  supposée  être
	adéquate.

	? si la piscine connait la plus grosse taille libre ainsi que le  puddle
	qui la possède, on peut créer un puddle immédiatement si la taille de la
	nouvelle allocation est supérieure à celle disponible

	? remanier la liste des puddles pour que le premier soit toujours  celui
	qui dispose de la plus grande quatité de mémoire libre.

	>> ça c'est pas une mauvaise idée par contre. en  plus  c'est  facile  à
	faire, il suffit de comparer lorsqu'on alloue et lorsque l'on dispose et
	arranger tout ça s'il y a besoin. En plus, on peut arrêter de parcourrir
	la  liste  dès que la taille disponible d'un puddle est trop petite, car
	les puddle suivants seront encore moins bien lotis.

*/

//#define DB_NEWP

/*************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///f_memory_hash_rem
STATIC void f_memory_hash_rem(FPuddle *Puddle, struct in_FeelinBase *FeelinBase)
{
	uint32 h = F_MEMORY_HASH(Puddle->lower);

	FPuddle *prev=NULL;
	FPuddle *node;

	for (node = FeelinBase->hash_puddles[h] ; node ; node = node->hash_next)
	{
		if (node == Puddle)
		{
			if (prev)
			{
				prev->hash_next = Puddle->hash_next;
			}
			else
			{
				FeelinBase->hash_puddles[h] = Puddle->hash_next;
			}

///DB_MEMORY_STATS
			#ifdef DB_MEMORY_STATS
			FeelinBase->memory_stats_numpuddles--;
			#endif
//+

			return;
		}
		prev = node;
	}

	if (!node)
	{
		IFEELIN F_Log(0,"Puddle (0x%08lx) not found",Puddle);
	}
}
//+

/************************************************************************************************
*** API *****************************************************************************************
************************************************************************************************/

///f_pool_create_a
F_LIB_POOL_CREATE
{
	struct TagItem *tags = Tagl,*item;

#ifdef __amigaos4__

	/* On OS4, we could put public  memory  as  MEMF_SHARED  (not  completly
	public : not suitable for IPC) | MEMF_VIRTUAL */

	uint32 public_pool_attributes = MEMF_CLEAR | MEMF_PUBLIC;
	
	/* MEMF_PRIVATE = Memory protection, by default...? */

	uint32 private_pool_attributes = MEMF_CLEAR | MEMF_PRIVATE | MEMF_VIRTUAL;  

#else
	
	uint32 public_pool_attributes = MEMF_CLEAR;
	uint32 private_pool_attributes = MEMF_CLEAR;

#endif
	
	uint32 pool_attributes = 0;                     /* 0 = automatic */
	uint32 pool_itemnum = 10;
	uint32 pool_public = FALSE;
	STRPTR pool_name = "Private";
	#ifdef F_ENABLE_MEMORY_WALL
	uint32 pool_wall_size = FeelinBase->memory_wall_size;
	#endif

	FPool *LOD;

	while ((item = IUTILITY NextTagItem(&tags)))
	switch (item->ti_Tag)
	{
		case FA_Pool_Attributes:   pool_attributes = item->ti_Data; break;
		case FA_Pool_ItemSize:     ItemSize = item->ti_Data; break;
		case FA_Pool_Items:        pool_itemnum = item->ti_Data; break;
		case FA_Pool_Name:         pool_name = (STRPTR) item->ti_Data; break;
		case FA_Pool_Public:       pool_public = item->ti_Data; break;

		default:
		{
			IFEELIN F_Log(FV_LOG_DEV,"F_CreatePoolA() Unknown attribute 0x%08lx (0x%08lx)",item->ti_Tag);
		}
	}

#ifdef DEBUG_CODE

	if (FF_DEBUG_PUBLICSEMAPHORES & FeelinBase->debug_flags)
	{
		pool_public = TRUE;
	}

#endif
	
	if (!pool_attributes)       /* pool_attributes = 0 = automatic */
	{
		if (pool_public)
		{
			pool_attributes = public_pool_attributes;
		}
		else
		{
			pool_attributes = private_pool_attributes;
		}
	}
#ifdef __amigaos4__
	else
	{
		/* pool_attributes has been set, we check that we are not
		in the MEMF_PUBLIC | MEMF_VIRTUAL case, forbidden on AOS4 */
		if (pool_public && (pool_attributes && MEMF_VIRTUAL))
		{
			/* In this special case, we set off the MEMF_VIRTUAL flags */
			pool_attributes &= ~MEMF_VIRTUAL;
		}
	}
#endif
 
	LOD = IEXEC AllocMem(sizeof (FPool), pool_attributes);

	if (LOD)
	{
		LOD->puddles = NULL;

		IEXEC InitSemaphore(&LOD->Semaphore);

		LOD->Semaphore.ss_Link.ln_Name = pool_name;

		LOD->flags = pool_attributes;
		
		#ifdef F_ENABLE_MEMORY_WALL

		LOD->thresh_size = sizeof (FMemChunk) + (((pool_wall_size * 2 + ItemSize + 3) >> 2) << 2);
		LOD->wall_size = pool_wall_size;

		#else

		LOD->thresh_size = sizeof (FMemChunk) + (((ItemSize + 3) >> 2) << 2);

		#endif

		LOD->puddle_size = LOD->thresh_size * pool_itemnum;

		IEXEC Forbid();

		if (pool_public)
		{
			IEXEC Enqueue(&FeelinBase->Public.SYS->SemaphoreList,(struct Node *) &LOD->Semaphore);
		}

		LOD->next = FeelinBase->pools;
		FeelinBase->pools = LOD;

		IEXEC Permit();
	}
	return LOD;
}
//+
///f_pool_delete
F_LIB_POOL_DELETE
{
	IEXEC Forbid();

	if (Pool)
	{

/*** search and remove memory pool *************************************************************/

		FPool *prev=NULL;
		FPool *node;

		for (node = FeelinBase->pools ; node ; node = node->next)
		{
			if (node == Pool)
			{
				if (prev)
				{
					prev->next = node->next;
				}
				else
				{
					FeelinBase->pools = node->next;
				}
				break;
			}
			prev = node;
		}

/*** delete pool and all of its puddles ********************************************************/

		if (node)
		{
			FPuddle *next = Pool->puddles;
			FPuddle *puddle;

			#ifdef DB_POOLDELETE
			IDOS_ Printf("*** DELETE POOL (0x%08lx) FIRST 0x%08lx\n",Pool,Pool->puddles);
			#endif

			while ((puddle = next) != NULL)
			{
				next = puddle->next;

				#ifdef DB_POOLDELETE
				IDOS_ Printf("delete puddle (0x%08lx) next (0x%08lx)\n",puddle,next);
				#endif

				f_memory_hash_rem(puddle, FeelinBase);

				IEXEC FreeMem(puddle,puddle->size);
			}

			/* remove semaphore */

			if (Pool->Semaphore.ss_Link.ln_Pred ||
				Pool->Semaphore.ss_Link.ln_Succ)
			{
				IEXEC RemSemaphore(&Pool->Semaphore);
			}

			IEXEC FreeMem(Pool,sizeof (FPool));
		}
		else
		{
			IFEELIN F_Log(0,"F_DeletePool() Unknown Pool (0x%08lx)",Pool);
		}
	}

	IEXEC Permit();

	return 0;
}
//+
///f_new
F_LIB_NEW
{
	return IFEELIN F_NewP(FeelinBase->DefaultPool,Size);
}
//+
///f_newp
F_LIB_NEWP
{
	if (Pool && Size)
	{
		FMemChunk *chunk = NULL;
		FPuddle *puddle;

		IEXEC Forbid();

		/* round size to multiple of (uint32) */

		#ifdef F_ENABLE_MEMORY_WALL
		Size = sizeof (FMemChunk) + (((Size + Pool->wall_size * 2 + 3) >> 2) << 2);
		#else
		Size = sizeof (FMemChunk) + (((Size + 3) >> 2) << 2);
		#endif

		if (Size <= Pool->thresh_size)
		{
			for (puddle = Pool->puddles ; puddle ; puddle = puddle->next)
			{
				#ifdef DB_NEWP
				IFEELIN F_Log(0,"try to create a memory chunk - Pool (0x%08lx) Puddle (0x%08lx)",Pool,puddle);
				#endif
	 
				if (puddle->free >= Size)
				{

					/* this puddle may have enough space for  our  drop.  *may*,
					because  'free' is the total memory available in the puddle,
					we don't know if a *big  enough*  space  is  available,  but
					since  pools are designed to hold elements of the same size,
					it should be. */

					FMemChunk *next = puddle->chunks;
					FMemChunk *prev = NULL;

					chunk = puddle->lower;

					for (;;)
					{
						if (next)
						{
							if ((uint32)(chunk) + Size <= (uint32)(next))
							{

								/* houray ! we have found enough free space  for
								our new drop. */

								chunk = (FMemChunk *)((uint32)(next) - Size);

								break;
							}
							else
							{
								/* darn, we need to search further */

								chunk = (FMemChunk *)((uint32)(next) + next->size);
								
								prev = next;
								next = next->next;
							}
						}
						else
						{

							/* there are no blocks left, we need to  compare
							from our position to the end of the puddle */

							if ((uint32)((uint32)(chunk) + Size) > (uint32)(puddle->upper))
							{

								/* damned, there is no room left in this puddle,
								we need to check another */

								chunk = NULL;
							}
							else
							{

								/*  nice  !  there  is  enough  space  from  our
								position  to the end of the puddle. Note that we
								allocate the new chunk *from  the  end*  of  the
								puddle to keep free space first. */

								chunk = (FMemChunk *)((uint32)(puddle->upper) - Size);
							}
							
							break;
						}
					}

					if (chunk != NULL)
					{

						/* enough space was found for  a  new  Drop  (or  memory
						chunk),  the  Drop is created from 'from' and the Puddle
						is updated */

						chunk->next = next;
						chunk->size = Size;

///F_ENABLE_MEMORY_SHORTCUT
						#ifdef F_ENABLE_MEMORY_SHORTCUT
						
						/* The 'middle' chunk is updated if the  new  chunk  is
						located  in  the second part of the puddle. If there is
						no 'middle' chunk  defined,  the  new  chunk  is  used,
						otherwise  the  'middle'  chunk  is  adjusted using the
						lowest chunk. */
						
						if ((uint32)(chunk) >= ((uint32)(puddle->upper) - (uint32)(puddle->lower)) / 2 + (uint32)(puddle->lower))
						{
							if (puddle->middle)
							{
								if (chunk < puddle->middle)
								{
									puddle->middle = chunk;
								}
							}
							else
							{
								puddle->middle = chunk;
							}
						}
						#endif
//+

						if (prev)
						{
							prev->next = chunk;
						}

						if (puddle->chunks == next)
						{
							puddle->chunks = chunk;
						}

						puddle->free -= Size;

///F_ENABLE_MEMORY_REARANGE
						#ifdef F_ENABLE_MEMORY_REARANGE

						if ((puddle->next != NULL) && (puddle->next->free > puddle->free))
						{
							FPuddle *prev;

							for (prev = puddle->next ; prev->next ; prev = prev->next)
							{
								if (prev->free < puddle->free)
								{
									break;
								}
							}

							IFEELIN F_Log
							(
								0, "new(%s): rearange puddle (0x%08lx)(%ld) prev (0x%08lx)(%ld) prev.next (0x%08lx)(%ld)",

								Pool->Semaphore.ss_Link.ln_Name,
								puddle,
								puddle->free,
								prev,
								prev->free,
								prev->next ? prev->next : NULL,
								prev->next ? prev->next->free : 0
							);

							// pour commencer on enlève le puddle de la chaine

							if (puddle->next)
							{
								puddle->next->prev = puddle->prev;
							}

							if (puddle->prev)
							{
								puddle->prev->next = puddle->next;
							}
							else
							{
								Pool->puddles = puddle->next;
							}

							// maintenant on l'insère

							if (prev->next)
							{
								prev->next->prev = puddle;
							}

							puddle->next = prev->next;
							puddle->prev = prev;

							prev->next = puddle;
						}

						#endif
//+

						break;
					}
				}
			}
		}

		if (chunk == NULL)
		{
			uint32 p_size = (Size > Pool->thresh_size) ? Size : Pool->puddle_size;
			uint32 h;

/*** create empty puddle & its first chunk *****************************************************/

			/* We don't need to clear the memory  allocated,  because  each
			drop  (memory  chunk) clear its very own memory space if needed
			e.i. when the flag MEMF_CLEAR is set */

			puddle = IEXEC AllocMem(sizeof (FPuddle) + p_size, Pool->flags & ~MEMF_CLEAR);

			if (puddle == NULL)
			{
				IEXEC Permit(); return NULL;
			}

///F_ENABLE_MEMORY_NEW_FILL
			#ifdef F_ENABLE_MEMORY_NEW_FILL
			{
				uint32 *mem = (uint32 *) puddle;
				uint32 len = sizeof (FPuddle) + p_size;
				uint32 len8;

				len >>= 2;
				len8 = (len >> 3) + 1;

				switch (len & 7)
				{
					do
					{
						*mem++ = FV_MEMORY_NEW_FILL_PATTERN;

						case 7:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 6:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 5:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 4:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 3:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 2:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 1:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 0:  len8--;
					}
					while (len8);
				}
			}
			#endif
//+

			/* Because puddle's memory was not cleared when  allocated,  we
			need to set *all* puddle's members. */
			
			puddle->next = Pool->puddles;
			puddle->prev = NULL;
			puddle->lower = (APTR)((uint32)(puddle) + sizeof (FPuddle));
			puddle->upper = (APTR)((uint32)(puddle) + sizeof (FPuddle) + p_size);
			puddle->chunks = puddle->lower;
			puddle->middle = NULL;
			puddle->size = p_size + sizeof (FPuddle);
			puddle->free = p_size - Size;
			puddle->pool = Pool;

/*** add puddle to the pool's chain ************************************************************/

			/* In order for new allocations to find free space as  fast  as
			possible,  new  puddles  are  always  linked at the head of the
			chain. */
							
			if (Pool->puddles != NULL)
			{
				Pool->puddles->prev = puddle;
			}

			Pool->puddles = puddle;

/*** add puddle to the memory hash table *******************************************************/

			/* For accurate Chunks searches, 'lower' is used  as  hash  key
			instead  of the address of the puddle, because 'lower' is often
			the first chunk of the puddle. */
			
			h = F_MEMORY_HASH(puddle->lower);

			puddle->hash_next = FeelinBase->hash_puddles[h];
			FeelinBase->hash_puddles[h] = puddle;
			
///DB_MEMORY_STATS
			#ifdef DB_MEMORY_STATS
			FeelinBase->memory_stats_numpuddles++; // for performance monitoring
			#endif
//+

/*** create first chunk, it's really simple ****************************************************/

			chunk = puddle->lower;
			chunk->size = Size;
			chunk->next = NULL;
		}

		if (chunk != NULL)
		{
///F_ENABLE_MEMORY_MARK
			#ifdef F_ENABLE_MEMORY_MARK
			{
				STRPTR class_name = "unknown";
				STRPTR method_name = NULL;

				if (FeelinBase->debug_classdo_class)
				{
					FClass *node;
		
					class_name = FeelinBase->debug_classdo_class->Name;
					
					for (node = (FClass *) FeelinBase->Classes.Head ; node ; node = node->Next)
					{
						if (node->Methods)
						{
							FClassMethod *me;

							for (me = node->Methods ; me->Function ; me++)
							{
								if (me->Name)
								{
									if (FeelinBase->debug_classdo_method == me->ID)
									{
										method_name = me->Name; break;
									}
								}
							}
						}
						if (method_name) break;
					}
				
				}

				if (!method_name)
				{
					method_name = "unknown";
				}

				IFEELIN F_StrFmt(&chunk->mark,"[%15.15s.%-14.14s",class_name,method_name);
				
				chunk->mark[31] = ']';
			}
			#endif
//+
			
			chunk = (APTR)((uint32)(chunk) + sizeof (FMemChunk));
 
///F_ENABLE_MEMORY_WALL
			#ifdef F_ENABLE_MEMORY_WALL

			if (Pool->wall_size)
			{
				uint32 *mem = (uint32 *) chunk;
				uint32 len = Pool->wall_size;
				uint32 len8;
				
				len >>= 2;
				len8 = (len >> 3) + 1;

				switch (len & 7)
				{
					do
					{
						*mem++ = FV_MEMORY_WALL_PATTERN;

						case 7:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 6:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 5:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 4:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 3:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 2:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 1:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 0:  len8--;
					}
					while (len8);
				}

				chunk = (FMemChunk *) mem;
			}
		
			#endif
//+
 
			/* if MEMF_CLEAR is set the chunk is filled with zeros */

			if (MEMF_CLEAR & Pool->flags)
			{
				uint32 *mem = (uint32 *) chunk;
				#ifdef F_ENABLE_MEMORY_WALL
				uint32 len = Size - sizeof (FMemChunk) - Pool->wall_size * 2;
				#else
				uint32 len = Size - sizeof (FMemChunk);
				#endif
				uint32 len8;

				len >>= 2;
				len8 = (len >> 3) + 1;

				switch (len & 7)
				{
					do
					{
						*mem++ = 0;

						case 7:  *mem++ = 0;
						case 6:  *mem++ = 0;
						case 5:  *mem++ = 0;
						case 4:  *mem++ = 0;
						case 3:  *mem++ = 0;
						case 2:  *mem++ = 0;
						case 1:  *mem++ = 0;
						case 0:  len8--;
					}
					while (len8);
				}
			}
///F_ENABLE_MEMORY_NEW_FILL
			#ifdef F_ENABLE_MEMORY_NEW_FILL
			else
			{
				uint32 *mem = (uint32 *) chunk;
				#ifdef F_ENABLE_MEMORY_WALL
				uint32 len = Size - sizeof (FMemChunk) - Pool->wall_size * 2;
				#else
				uint32 len = Size - sizeof (FMemChunk);
				#endif
				uint32 len8;

				len >>= 2;
				len8 = (len >> 3) + 1;

				switch (len & 7)
				{
					do
					{
						*mem++ = FV_MEMORY_NEW_FILL_PATTERN;

						case 7:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 6:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 5:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 4:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 3:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 2:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 1:  *mem++ = FV_MEMORY_NEW_FILL_PATTERN;
						case 0:  len8--;
					}
					while (len8);
				}
			}
			#endif
//+

///F_ENABLE_MEMORY_WALL
			#ifdef F_ENABLE_MEMORY_WALL
			if (Pool->wall_size)
			{
				uint32 *mem = (uint32 *)((uint32)(chunk) + Size - sizeof (FMemChunk) - Pool->wall_size * 2);
				uint32 len = Pool->wall_size;
				uint32 len8;

				len >>= 2;
				len8 = (len >> 3) + 1;

				switch (len & 7)
				{
					do
					{
						*mem++ = FV_MEMORY_WALL_PATTERN;

						case 7:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 6:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 5:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 4:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 3:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 2:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 1:  *mem++ = FV_MEMORY_WALL_PATTERN;
						case 0:  len8--;
					}
					while (len8);
				}
			}
			#endif
//+

			IEXEC Permit();

///DEBUG_CODE / FF_VERBOSE_NEW
			#ifdef DEBUG_CODE
			if (FF_VERBOSE_NEW & FeelinBase->verbose_flags)
			{
				IFEELIN F_Log(0,"(vb)F_New() Chunk 0x%08lx - Size %8ld - Pool 0x%08lx (%s)",chunk,Size,Pool,Pool->Semaphore.ss_Link.ln_Name);
			}
			#endif
//+

			return chunk;
		}
		else
		{
			IFEELIN F_Log(FV_LOG_CORE, "F_NewP() Unable to allocate %ld bytes", Size);
		}
	}
	return NULL;
}
//+
///f_dispose
F_LIB_DISPOSE
{
	if (Mem)
	{
///F_ENABLE_MEMORY_STATS
		#ifdef F_ENABLE_MEMORY_STATS

		uint32 stats_n = 0;
		uint32 stats_n_prev = 0;
		
		#endif
//+
		#ifdef F_ENABLE_MEMORY_WALL
		uint32 h = F_MEMORY_HASH(Mem);
		#else
		uint32 h = F_MEMORY_HASH(((uint32)(Mem) - sizeof (FMemChunk)));
		#endif
		
		FPuddle *puddle = NULL;

		FPuddle **stp = &FeelinBase->hash_puddles[-1];
		FPuddle **pos = &FeelinBase->hash_puddles[h];

		IEXEC Forbid();
		
		/* We search the puddle which may contains  the  Chunk  between  its
		'lower'  and  'upper'  addresses.  Puddles are checked from the hash
		point to the first (zero), because if the Chunk is not  in  the  'h'
		puddle (or in the hash link chain) its always before it, very really
		after (not to say never). */
		
		while (pos != stp)
		{
#if 0
///F_ENABLE_MEMORY_STATS
			#ifdef F_ENABLE_MEMORY_STATS
			uint32 stats_link_n = 0;
			#endif
//+
#endif

			puddle = *pos--;
	
			while (puddle)
			{
///F_ENABLE_MEMORY_STATS
#ifdef F_ENABLE_MEMORY_STATS

				stats_n++;

#endif
//+

				if ((uint32)(Mem) >= (uint32)(puddle->lower) &&
					(uint32)(Mem) <  (uint32)(puddle->upper))
				{
///F_ENABLE_MEMORY_STATS
#ifdef F_ENABLE_MEMORY_STATS

					if (stats_n == 1)
					{
						FeelinBase->memory_stats_immediate++;
					}
					else if (stats_n_prev)
					{
						FeelinBase->memory_stats_prev++;
						FeelinBase->memory_stats_prev_distance_max = MAX(FeelinBase->memory_stats_prev_distance_max, stats_n_prev);
					}
					else
					{
						FeelinBase->memory_stats_link++;
					}

#endif
//+
					pos = stp;
					
					break;
				}
				else
				{
#if 0
///F_ENABLE_MEMORY_STATS
					#ifdef F_ENABLE_MEMORY_STATS
					stats_link_n++;
					#endif
//+
#endif
					puddle = puddle->hash_next;
				}
			}
///F_ENABLE_MEMORY_STATS
			#ifdef F_ENABLE_MEMORY_STATS
			stats_n_prev++;
			#endif
//+
		}

/*** search in following puddles (very rare) ***************************************************/
 
		/* We didn't find the Chunk. Maybe it's after the 'h' puddle,  let's
		check it out. */
		
		if (puddle == NULL)
		{
			stp = &FeelinBase->hash_puddles[FV_MEMORY_HASH_SIZE];
			pos = &FeelinBase->hash_puddles[h + 1];
							
			while (pos != stp)
			{
				puddle = *pos++;

				while (puddle)
				{
					if ((uint32)(Mem) >= (uint32)(puddle->lower) &&
						(uint32)(Mem) <  (uint32)(puddle->upper))
					{
///F_ENABLE_MEMORY_STATS
						#ifdef F_ENABLE_MEMORY_STATS
						FeelinBase->memory_stats_next++;
						#endif
//+
						pos = stp;

						break;
					}
					else
					{
						puddle = puddle->hash_next;
					}
				}
			}
		}

/*** remove memory chunk ***********************************************************************/
 
		if (puddle)
		{
			#ifdef F_ENABLE_MEMORY_WALL
			FMemChunk *real = (FMemChunk *)((uint32)(Mem) - puddle->pool->wall_size - sizeof (FMemChunk));
			#else
			FMemChunk *real = (FMemChunk *)((uint32)(Mem) - sizeof (FMemChunk));
			#endif
			FMemChunk *prev;
			FMemChunk *node;
			
			#ifdef F_ENABLE_MEMORY_SHORTCUT
			
			/* If we are lucky, the Chunk to dispose is after the one in the
			middle.  This  will save us a lot of checkings e.g. half of them
			:-). The middle chunk could be our Chunk, but we cannot  use  it
			because we need the previous chunk in order to unlink it. */
 
			if (puddle->middle != NULL && real > puddle->middle)
			{
				prev = puddle->middle;
				node = puddle->middle->next;
			}
			else
			#endif /* F_ENABLE_MEMORY_SHORTCUT */
			{
				prev = NULL;
				node = puddle->chunks;
			}
 
			while (node)
			{
				if (node == real)
				{
					puddle->free += node->size;
			
					#ifdef F_ENABLE_MEMORY_WALL
///check memory wall integrity
					if (puddle->pool->wall_size)
					{
						uint32 *wall = (uint32 *)((uint32)(Mem) - puddle->pool->wall_size);
						uint32 i;
						
						for (i = 0 ; i < puddle->pool->wall_size / 4 ; i++)
						{
							if (*wall != FV_MEMORY_WALL_PATTERN)
							{
								IFEELIN F_Log(0,"F_Dispose() Memory wall of Chunk (0x%08lx)(%s) altered at (%ld): 0x%08lx",Mem,puddle->pool->Semaphore.ss_Link.ln_Name,(uint32)(wall) - (uint32)(Mem),*wall);
							}
							wall++;
						}
					
						wall = (uint32 *)((uint32)(Mem) - puddle->pool->wall_size - sizeof (FMemChunk) + node->size - puddle->pool->wall_size);
						
						for (i = 0 ; i < puddle->pool->wall_size / 4 ; i++)
						{
							if (*wall != FV_MEMORY_WALL_PATTERN)
							{
								IFEELIN F_Log(0,"F_Dispose() Memory wall of Chunk (0x%08lx)(%s) altered at (%ld): 0x%08lx",Mem,puddle->pool->Semaphore.ss_Link.ln_Name,(uint32)(wall) - (uint32)(Mem),*wall);
							}
							wall++;
						}
					}
///+
					#endif
					
					#ifdef F_ENABLE_MEMORY_SHORTCUT
					
					/* If our Chunk is the middle one,  we  set  the  middle
					chunk  to  the  next  chunk, which may be NULL. We don't
					need to do anything else,  since  the  middle  chunk  is
					adjusted to its best when allocating new item. */
 
					if (puddle->middle == node)
					{
						puddle->middle = node->next;
					}
					
					#endif /* F_ENABLE_MEMORY_SHORTCUT */

///unlink chunk
					if (puddle->chunks == node)
					{
						puddle->chunks = node->next;
					}
					else if (prev)
					{
						prev->next = node->next;
					}
//+
///fill memory chunk
				//stegerg FIXME/CHECKME broken?
				#ifdef F_ENABLE_MEMORY_FILL
				{
					uint32 *mem = (uint32 *) node;
					uint32 len = node->size;
					uint32 len8;

					len >>= 2;
					len8 = (len >> 3) + 1;

					switch (len & 7)
					{
						do
						{
							*mem++ = FV_MEMORY_FILL_PATTERN;

							case 7:  *mem++ = FV_MEMORY_FILL_PATTERN;
							case 6:  *mem++ = FV_MEMORY_FILL_PATTERN;
							case 5:  *mem++ = FV_MEMORY_FILL_PATTERN;
							case 4:  *mem++ = FV_MEMORY_FILL_PATTERN;
							case 3:  *mem++ = FV_MEMORY_FILL_PATTERN;
							case 2:  *mem++ = FV_MEMORY_FILL_PATTERN;
							case 1:  *mem++ = FV_MEMORY_FILL_PATTERN;
							case 0:  len8--;
						}
						while (len8);
					}
				}

				#endif
//+
///free empty puddle
					if (puddle->chunks == NULL)
					{
						f_memory_hash_rem(puddle, FeelinBase);
							
						if (puddle->next)
						{
							puddle->next->prev = puddle->prev;
						}

						if (puddle->pool->puddles == puddle)
						{
							puddle->pool->puddles = puddle->next;
						}
						else if (puddle->prev)
						{
							puddle->prev->next = puddle->next;
						}

						IEXEC FreeMem(puddle,puddle->size);
					}
//+
					#ifdef F_ENABLE_MEMORY_REARANGE

					else
					{
						if ((puddle->prev != NULL) && (puddle->prev->free > puddle->free))
						{
							FPuddle *next;

							for (next = puddle->prev ; next->prev ; next = next->prev)
							{
								if (next->free < puddle->free)
								{
									break;
								}
							}

							IFEELIN F_Log
							(
								0, "dispose(%s): rearange puddle (0x%08lx)(%ld) next (0x%08lx)(%ld) next.prev (0x%08lx)(%ld)",

								puddle->pool->Semaphore.ss_Link.ln_Name,
								puddle,
								puddle->free,
								next,
								next->free,
								next->prev ? next->prev : NULL,
								next->prev ? next->prev->free : 0
							);

							// pour commencer on enlève le puddle de la chaine

							if (puddle->next)
							{
								puddle->next->prev = puddle->prev;
							}

							if (puddle->prev)
							{
								puddle->prev->next = puddle->next;
							}
							else
							{
								puddle->pool->puddles = puddle->next;
							}

							// maintenant on l'insère

							if (next->prev)
							{
								next->prev->next = puddle;
							}

							puddle->next = next;
							puddle->prev = next->prev;

							next->prev = puddle;
						}
					}
					#endif

					break;
				}       
				else
				{
					prev = node;
					node = node->next;
				}
			}
		
			if (node == NULL)
			{
				IFEELIN F_Log(0,"F_Dispose() Memory chunk (0x%08lx) is not allocated", Mem);
			}
		}
		else
		{
///F_ENABLE_MEMORY_STATS
			#ifdef F_ENABLE_MEMORY_STATS
			FeelinBase->memory_stats_fucked++;
			#endif
//+
			IFEELIN F_Log(0,"F_Dispose() Unknown memory chunk (0x%08lx)", Mem);
		}
	
		IEXEC Permit();
	}
	#if 0
///F_ENABLE_MEMORY_STATS
	#ifdef F_ENABLE_MEMORY_STATS
 
	if (FeelinBase->memory_stats_immediate > FeelinBase->memory_stats_step ||
		FeelinBase->memory_stats_link > FeelinBase->memory_stats_step ||
		FeelinBase->memory_stats_prev > FeelinBase->memory_stats_step ||
		FeelinBase->memory_stats_next > FeelinBase->memory_stats_step ||
		FeelinBase->memory_stats_fucked > FeelinBase->memory_stats_step)
	{
		IFEELIN F_Log(0,"STATS: i (%ld) l (%ld) p (%ld, dis %ld) n (%ld) f (%ld)",FeelinBase->memory_stats_immediate,FeelinBase->memory_stats_link,FeelinBase->memory_stats_prev,FeelinBase->memory_stats_prev_total_distance,FeelinBase->memory_stats_next,FeelinBase->memory_stats_fucked);
		
		FeelinBase->memory_stats_step += 20;
	}
	#endif
//+
	#endif

	return 0;
}
//+
///f_opool
F_LIB_OPOOL
{
	if (Pool)
	{
		IEXEC ObtainSemaphore(&Pool->Semaphore);
	}
}
//+
///f_spool
F_LIB_SPOOL
{
	if (Pool)
	{
		IEXEC ObtainSemaphoreShared(&Pool->Semaphore);
	}
}
//+
///f_rpool
F_LIB_RPOOL
{
	if (Pool)
	{
		IEXEC ReleaseSemaphore(&Pool->Semaphore);
	}
}
//+

