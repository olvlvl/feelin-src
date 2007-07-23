/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <libraries/feelin.h>
#include <feelin/preference.h>

#include "Public.h"

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/feelin.h>

/************************************************************************************************
*** IDs *****************************************************************************************
************************************************************************************************/

enum    {

		FV_ATTRIBUTE_ACTIVE,                         // [ISG] LONG - GET/SET THE NUMBER OF THE ACTIVE ENTRY
		FV_ATTRIBUTE_ACTIVATION,                     // [..G]
		FV_ATTRIBUTE_COMPAREHOOK,                    // [IS.] STRUCT HOOK *
		FV_ATTRIBUTE_CONSTRUCTHOOK,                  // [IS.] STRUCT HOOK *
		FV_ATTRIBUTE_DESTRUCTHOOK,                   // [IS.] STRUCT HOOK *
		FV_ATTRIBUTE_DISPLAYHOOK,                    // [IS.] STRUCT HOOK *
		FV_ATTRIBUTE_ENTRIES,                        // [..G] LONG - NUMBER OF ENTRIES IN THE LIST
		FV_ATTRIBUTE_VISIBLE,
		FV_ATTRIBUTE_FIRST,                          // [..G] LONG - GET THE NUMBER OF THE FIRST VISIBLE ENTRY
		FV_ATTRIBUTE_LAST,                           // [..G] LONG - GET THE NUMBER OF THE LAST VISIBLE ENTRY
		FV_ATTRIBUTE_FORMAT,                         // [ISG] STRPTR
		FV_ATTRIBUTE_POOL,                           // [I..] APTR
		FV_ATTRIBUTE_POOLITEMNUMBER,                 // [I..] UINT32
		FV_ATTRIBUTE_POOLITEMSIZE,                   // [I..] UINT32
		FV_ATTRIBUTE_QUIET,                          // [.S.] BOOL - TOGGLE THE FF_LIST_QUIET FLAG
		FV_ATTRIBUTE_SORTMODE,                       // [IS.] LONG - SET AUTOMATIC SORT OF INSERTED LINES
		FV_ATTRIBUTE_SOURCEARRAY,                    // [I..] APTR *
		FV_ATTRIBUTE_TITLE,                          // [ISG] STRPTR
		FV_ATTRIBUTE_READONLY,
		
		/* PREFERENCES */
 
		FV_ATTRIBUTE_STEPS,
		FV_ATTRIBUTE_SPACING,                        // [ISG] BYTE - NUMBER OF PIXELS BETWEEN LINES
		FV_ATTRIBUTE_CURSORACTIVE

		};

enum    {

		FV_PROPERTY_STEPS,
		FV_PROPERTY_SPACING,
		FV_PROPERTY_CURSOR_ACTIVE,
		FV_PROPERTY_TITLE_COLOR_SCHEME

		};

enum    {

		FV_METHOD_COMPARE,
		FV_METHOD_CONSTRUCT,
		FV_METHOD_DESTRUCT,
		FV_METHOD_DISPLAY,
		FV_METHOD_GETENTRY,
		FV_METHOD_INSERT,
		FV_METHOD_INSERTSINGLE,
		FV_METHOD_REMOVE,
		FV_METHOD_FINDSTRING

		};

enum    {

		FV_ATOM_BAR,
		FV_ATOM_COL,
		FV_ATOM_FIXED,
		FV_ATOM_MAX,
		FV_ATOM_MIN,
		FV_ATOM_PADDING,
		FV_ATOM_SPAN,
		FV_ATOM_WEIGHT,
		FV_ATOM_WIDTH

		};

/************************************************************************************************
*** Structures **********************************************************************************
************************************************************************************************/

/*** FColumn ***********************************************************************************/
 
typedef struct FeelinList_Column
{
	bits8                           Flags;
	
	uint16                          Width;
	uint16                          AdjustedWidth;

	uint8                           Padding;
	uint8                           Weight;

//    uint8                           Position;   // Column number
}
FColumn;

#define FF_COLUMN_BAR                           (1 << 0)
#define FF_COLUMN_FIXED                         (1 << 1)
#define FF_COLUMN_PREVBAR                       (1 << 2) // previous columns has a bar
#define FF_COLUMN_PADLEFT                       (1 << 3)
#define FF_COLUMN_PADRIGHT                      (1 << 4)

/* AdjustedWidth is the Width adjusted to the available space. This width is
not  modified  if  FF_COLUMN_FIXED is set. Note that the width of padding is
also added to AdjustedWidth */

/*** FLine *************************************************************************************/

typedef struct FeelinList_Line
{
	struct FeelinList_Line         *Next;
	struct FeelinList_Line         *Prev;
// end of FeelinNode header
	APTR                            Entry;
	STRPTR                         *Strings;    // The array of string follow the header of the structure
	STRPTR                         *PreParses;
	uint16                         *Widths;     // The widths of the column
	uint16                          Height;     // Line Height;
	bits16                          Flags;      // see below
//    uint16                          Parents;    // number of entries parent's, used for the list tree stuff

	/* 'Position' *MUST* be at the end of the structure because the array of
	line's  strings  follows.  The  array  is  sent to the display hook thus
	array[-1] gives the line number.

	'Position' is followed by the array of Strings, the array  of  PreParses
	and the array of Widths. */

	uint32                           Position;
}
FLine;

#define FF_LINE_COMPUTED                        (1 << 0)
#define FF_LINE_SELECTED                        (1 << 1)

/* FF_LINE_COMPUTED is set when the dimensions of a line have been computed.
The  flag  is  used  to avoid multiple computing of a same line. The flag is
cleared during FM_Cleanup. */

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	F_MEMBER_AREA_PUBLIC;
	F_MEMBER_WIDGET_PUBLIC;

	bits32                          Flags;

	APTR                            pool;               // This Pool is used by the user to allocate Entry data.
	APTR                            LinePool;           // This Pool is used to allocate entries

	struct Hook                    *Hook_Compare;
	struct Hook                    *Hook_Construct;
	struct Hook                    *Hook_Destruct;
	struct Hook                    *Hook_Display;

//  title bar and columns

	FLine                          *TitleBar;       // See list_create_format()
	FObject                         TitleFrame;
	FAreaPublic                    *TitleFramePublic;
	FPalette                       *TitleScheme;
	FColumn                        *Columns;
	uint32                          ColumnCount;

//  lines

	FList                           LineList;       // List of FeelinList_Line nodes
	uint32                          LineCount;
	FLine                          *First;          // First visible Line
	FLine                          *Last;           // Last visible Line
	FLine                          *Active;         // Active Line
	uint32                          Visible;        // # of visible entries
	uint8                           spacing;        // Number of pixel between each line
	uint16                          MaximumH;       // Maximun height of a line
	uint16                          TitleBarH;
	
	int16                           Quiet;          // nested, >= 0 update real time, < 0 nothing is displayed. == 0 complete refresh
	uint16                          SortMode;
	uint16                          Activation;
	uint16                          steps;

//  graphics

	FObject                         TDObj;
	FPalette                       *Scheme;
	FObject                         CursorActive;
	FObject                         CursorSelected;

//  draw

	FLine                          *Update;         // Previous active Line that should be redrawn along the new one
	FLine                          *LastFirst;

//  popstring

	FObject                         pop_window;
	FObject                         pop_string;
	uint32                          pop_delay;

	#ifndef F_NEW_STYLES

//  preferences

	STRPTR                          p_Spacing;
	STRPTR                          p_Steps;
	STRPTR                          p_CursorActive;

	STRPTR                          p_title_back;
	STRPTR                          p_title_frame;
	STRPTR                          p_title_scheme;

	#endif
};

#define FF_LIST_READONLY                        (1 << 0) // input
#define FF_LIST_OWNPOOL                         (1 << 1) // Set if Pool is allocated by the Class
#define FF_LIST_MODIFIED                        (1 << 2)
#define FF_LIST_ACTIVATION_INTERNAL             (1 << 3)
#define FF_LIST_TITLE_INHERITED_BACK            (1 << 4)
#define FF_LIST_ACTIVE                          (1 << 5)

#define _list_is_readonly                       ((FF_LIST_READONLY & LOD->Flags) != 0)

/* FF_LIST_MODIFIED is set when the list is  in  quiet  mode  and  has  been
modified. This flag is used to avoid unnecessary drawing */

/***********************************************************************************************/

#define FF_Draw_Line                            FF_Draw_Custom_1 // draw line LOD -> Update, request erase

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

/*** support.c ***/

void line_display(FClass *Class,FObject Obj,FLine *Line);
void line_compute_dimensions(FClass *Class,FObject Obj,FLine *line);
void list_update(FClass *Class,FObject Obj,bool32 Notify);
void list_adjust_first(FClass *Class,FObject Obj,uint32 pos);
				
/*** titlebar.c ***/

int32 titlebar_new(FClass *Class,FObject Obj,STRPTR Fmt);
void titlebar_dispose(FClass *Class,FObject Obj);
void titlebar_compute_dimensions(FClass *Class,FObject Obj);
void titlebar_adjust_dimensions(FClass *Class,FObject Obj);
bool32 titlebar_setup(FClass *Class,FObject Obj);
void titlebar_cleanup(FClass *Class,FObject Obj);
void titlebar_draw(FClass *Class,FObject Obj);

/*** area.c ***/

void list_popstring_delete(FClass *Class,FObject Obj);



#define DEF_LIST_POPSTRING_DELAY                20
#define DEF_LIST_STEPS                          3
#define DEF_LIST_SPACING                        1

