/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <stdlib.h>

#include <libraries/iffparse.h>
#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/iffparse.h>
#include <proto/feelin.h>

#ifdef __amigaos4__
#define IIFFPARSE                               IIFFParse ->
#else
#define IIFFPARSE
#endif

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_ACCEPT,
		FV_ATTRIBUTE_ADVANCEONCR,
		FV_ATTRIBUTE_CHANGED,
		FV_ATTRIBUTE_CONTENTS,
		FV_ATTRIBUTE_JUSTIFY,
		FV_ATTRIBUTE_INTEGER,
		FV_ATTRIBUTE_MAX,
		FV_ATTRIBUTE_REJECT,
		FV_ATTRIBUTE_SECRET,
		FV_ATTRIBUTE_ATTACHEDLIST,
		FV_ATTRIBUTE_DISPLAYABLE

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalPropertiesData
{
	STRPTR							cursor_spec;

	STRPTR							text_active;
	STRPTR							text_inactive;
	STRPTR							text_block;

	STRPTR							blink_spec;
	uint32							blink_speed;
};

enum    {

		FV_PROPERTY_CURSOR,

		FV_PROPERTY_TEXT_ACTIVE,
		FV_PROPERTY_TEXT_INACTIVE,
		FV_PROPERTY_TEXT_BLOCK,

		FV_PROPERTY_BLINK_SPEED,
		FV_PROPERTY_BLINK_IMAGE

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	F_MEMBER_AREA_PUBLIC;
	F_MEMBER_WIDGET_PUBLIC;

	STRPTR                          String;
	STRPTR                          Undo;
	STRPTR                          Accept;
	STRPTR                          Reject;

	uint8                           Justify;
	uint8                           displayable;
	uint16                          Flags;
	uint16                          Allocated;

	int16                           TextX;
	uint16                          TextW;
	int16                           CursorX;
	uint16                          CursorW;

	uint16                          Pos;
	uint16                          Cur;
	uint16                          Len;
	uint16                          Max;
	uint16                          Old;
	uint16                          Sel;
	uint16                          Dsp;
	uint8                           BlinkSpeed;
	uint8                           BlinkElapsed;

	FColor                         *APen;       // Active Pen
	FColor                         *BPen;       // Block Pen
	FColor                         *IPen;       // Inactive Pen

	FObject                         Cursor;     // Cursor ImageDisplayObject
	FObject                         Blink;      // Blink ImageDisplayObject
	FObject                         AttachedList;

	//APTR                            attached_list_notify_handler;
};

struct FeelinString_TextFit
{
	uint32                          Position;       // Read / Written
	STRPTR                          String;         // Read / Written
	uint16                          TextWidth;      // Read / Written
	uint16                          Displayable;
	BOOL                            Clip;
};

#define FF_String_Secret                        (1 << 0)
#define FF_String_AdvanceOnCR                   (1 << 1)
#define FF_String_Blink                         (1 << 2)

#define FF_Draw_Move                            FF_Draw_Custom_1
#define FF_Draw_Insert                          FF_Draw_Custom_2
#define FF_Draw_Delete                          FF_Draw_Custom_3
#define FF_Draw_Backspace                       FF_Draw_Custom_4
#define FF_Draw_Blink                           FF_Draw_Custom_5

enum    {

		REPAIR_NOTHING,
		REPAIR_ALL,
		REPAIR_OLDC,
		REPAIR_FROM_OLDC,
		REPAIR_WITHCUR,
		REPAIR_FROMCUR,
		REPAIR_TOCUR
		
		};

/************************************************************************************************
*** Prototpyes **********************************************************************************
************************************************************************************************/

int32 String_Insert(FClass *Class,FObject Obj,uint8 Char);

/************************************************************************************************
*** Preferences *********************************************************************************
************************************************************************************************/

#define DEF_STRING_CURSOR                       "highlight"
#define DEF_STRING_BLINK_IMAGE                  NULL
#define DEF_STRING_BLINK_SPEED                  0
#define DEF_STRING_TEXT_ACTIVE                  "dark"
#define DEF_STRING_TEXT_BLOCK                   "shadow"
#define DEF_STRING_TEXT_INACTIVE                "shine"
#define DEF_STRING_DISPLAYABLE                  4
