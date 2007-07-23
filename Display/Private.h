/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <string.h>

#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/notify.h>
#include <libraries/feelin.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/feelin.h>

#include <feelin/string.h>
#include <feelin/rtg.h>

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_METHOD_FIND,
		FV_METHOD_CREATE,
		FV_METHOD_DELETE

		};

enum    {

		FV_ATTRIBUTE_BACKGROUND,
		FV_ATTRIBUTE_COLORMAP,
		FV_ATTRIBUTE_DEPTH,
		FV_ATTRIBUTE_BITMAP,
		FV_ATTRIBUTE_DRAWINFO,
		FV_ATTRIBUTE_HEIGHT,
		FV_ATTRIBUTE_NAME,
		FV_ATTRIBUTE_SCREEN,
		FV_ATTRIBUTE_SCREENMODE,
		FV_ATTRIBUTE_SPEC,
		FV_ATTRIBUTE_TITLE,
		FV_ATTRIBUTE_WIDTH

		};

enum    {

		FV_ATOM_CONTRAST,
		FV_ATOM_DARK,
		FV_ATOM_FILL,
		FV_ATOM_HIGHLIGHT,
		FV_ATOM_SATURATION,
		FV_ATOM_SCHEME,
		FV_ATOM_SHINE,
		FV_ATOM_TEXT

		};

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

/*** types *************************************************************************************/

struct FeelinObjectList
{
	FObject                         Head;
	FObject                         Tail;
};

struct ClassUserData
{
	FObject                         Semaphore;

	APTR                            colors_pool;
	APTR                            palettes_pool;

	struct FeelinObjectList         DisplayList;

	FObject                         document;
	uint32                          document_source_id;
	uint32                          document_sourcetype_id;
	uint32                          document_markups_id;

	FList                           decoded_list;
};

#define F_LOCK_ARBITER                          IFEELIN F_Do(CUD->Semaphore,FM_Lock,FF_Lock_Exclusive)
#define F_UNLOCK_ARBITER                        IFEELIN F_Do(CUD->Semaphore,FM_Unlock)

#define F_LOCK_COLORS                           IFEELIN F_OPool(CUD->colors_pool)
#define F_UNLOCK_COLORS                         IFEELIN F_RPool(CUD->colors_pool)

#define FF_Display_Created          (1L << 0)

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	bits32                          Flags;
	uint32                          UserCount;

	FObject                         Next;
	FObject                         Prev;

	struct Screen                  *Screen;
	struct ColorMap                *ColorMap;
	struct DrawInfo                *DrawInfo;
	FList                           ColorList;
	
	struct in_Palette			   *palettes;

	uint16                          Width;      // SA_Width
	uint16                          Height;     // SA_Height
	uint16                          Depth;      // SA_Depth
	uint16                          Type;       // FA_Display_Type

	STRPTR                          PublicName; // SA_PubName
	STRPTR                          ScreenName; // SA_Title

//    ULONG                           DisplayID;  // SA_DisplayID
//    struct TextFont                *Font;
};

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

struct in_Color
{
	struct in_Color                *Next;
	struct in_Color                *Prev;
// end of FeelinNode header
	uint32                          Pen;
	uint32                          ARGB;
	uint32                          UserCount;
	bits32                          Flags;
};

#define FF_Color_Shared             (1L << 0)


struct in_Palette
{
	struct in_Palette			   *next;
	uint32							references;
	FPalette						public;
};

#define F_PALETTE_PUBLICIZE(ptr)				(&ptr->public)
#define F_PALETTE_PRIVATIZE(ptr)				((FPalette *)((uint32)(ptr) + sizeof (FPalette) - sizeof (struct in_Palette)))
