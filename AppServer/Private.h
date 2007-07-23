/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <devices/input.h>
#include <graphics/gels.h>
#include <libraries/feelin.h>
#include <feelin/preference.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/feelin.h>

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

        FV_ATTRIBUTE_PREFERENCE

        };

enum    {

        FV_METHOD_DNDHANDLE

        };

struct  FS_AppServer_DnDHandle                  { FObject Source; FBox *Box; FRender *Render; };

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
    bits32                          flags;

    FObject                         Server;
    FObject                         Preference;

    struct FeelinDnDInfo           *DnDInfo; 

    // events

    struct Screen                  *screen;

    uint16                          mouse_x;
    uint16                          mouse_y;
};

#define FF_APPSERVER_DRAGGING                   (1 << 0)

/************************************************************************************************
*** Structures **********************************************************************************
************************************************************************************************/

#if F_CODE_DEPRECATED

struct FeelinBob
{
    uint16                          Width;
    uint16                          Height;
    
    struct RastPort                 MainRast;
    struct ViewPort                *MainView;
    
    struct VSprite                  vsprite;        // VSprite used for bob
    struct Bob                      bob;            // Data for a single bob
    struct RastPort                 rport;          // Rastport for our BOB
    struct BitMap                   bitmap;         // Bitmap for our BOB

    uint32                          chipsize;       // Total size of allocated CHIP
    uint8                          *chipdata;       // Pointer to all CHIP RAM data

    struct GelsInfo                 gel_info;           // GelInfo for entire structure
    uint16                          gel_nextline[8];    // Nextline data
    int32                           gel_lastcolor[8];   // Last colour data, PTR TO PTR TO INT
    struct collTable                gel_colltable;      // Collision table
    struct VSprite                  gel_vshead;         // Head sprite anchor
    struct VSprite                  gel_vstail;         // Tail sprite anchor
};

#else

struct FeelinBob
{
    FBox                            box;        // current bob position and size

    struct RastPort                 rp;         // Rastport for our BOB
    struct RastPort                 brp;        // Rastport for our backup

    struct Region                  *region;

    struct RastPort                *srp;        // Source Rastport (bitmap to copy on screen)
    struct RastPort                *drp;        // Destination Rastport (screen)
    struct BitMap                  *backup;     // Bitmap for our backup
    struct BitMap                  *bitmap;     // Bitmap for our BOB
};

#endif

typedef struct FeelinDnDInfo
{
    FObject                         Source;
    FObject                         Target;
    FBox                            SourceBox;
    FBox                            TargetBox;
    struct Window                  *SourceWin;
    struct Window                  *TargetWin;

    struct Screen                  *Scr;
    FRender                        *Render;
    APTR                            handler;
    
    uint16                          msex,msey;
    uint16                          offx,offy;
    
    struct FeelinBob               *Bob;
    FBox                            BobBox;
}
FDnDInfo;

/************************************************************************************************
*** Prototypes **********************************************************************************
************************************************************************************************/

/* bob.c */

#if F_CODE_DEPRECATED
struct FeelinBob *  Bob_Create      (struct RastPort *RPort,struct ViewPort *VPort,uint32 Width,uint32 Height,int32 Transp);
#else
struct FeelinBob *  Bob_Create      (struct RastPort *RPort, FBox * SourceBox, int32 Transp);
#endif
void                Bob_Delete      (struct FeelinBob *Bob);
void                Bob_Update      (struct FeelinBob *Bob, int32 x, int32 y);
void                Bob_Add         (struct FeelinBob *Bob);
void                Bob_Rem         (struct FeelinBob *Bob);
void                Bob_MakeGhost   (struct FeelinBob *Bob);
void                Bob_MakeSolid   (struct FeelinBob *Bob);
void                xblit           (struct BitMap *sbm, int32 sx, int32 sy, struct BitMap *dbm, int32 x, int32 y, int32 w, int32 h);
