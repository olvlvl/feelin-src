/*

    TextDisplay/context.h

*************************************************************************************************

    NOUVELLE GESTION DES CONTEXTES

    On va commencer avec la couleur (<color>). Cela sera valable  pour  tout
    les markup de contextes comme <font> ou <pens>.

    Lorsqu'on trouve un markup <color>, on crée une struc in_Color  qui  est
    chainée  à  'colors'.  On  crée  aussi  une  struct  in_ContextNode  qui
    référence notre couleur, cette structure est poussée  dans  le  contexte
    (en  gros  elle  se  retrouve  en  première  position  dans la liste des
    couleurs poussées). Seule la structure in_Color subsistera après la mise
    en place du texte.

*/

#ifndef FEELIN_TEXTDISPLAY_CONTEXT_H
#define FEELIN_TEXTDISPLAY_CONTEXT_H

#include "parse.h"

#ifndef F_NEW_CONTEXT
///FTDContextContainer

struct in_FeelinTD_Context_Container
{
    struct in_FeelinTD_Context_Container  *Next;
    struct in_FeelinTD_Context_Container  *Prev;

    UWORD                           Type;
    BYTE                            Stolen;
    BYTE                            _pad0;
    APTR                            Data;
};

typedef struct in_FeelinTD_Context_Container       FTDContextContainer;

/*  (FTDContextContainer.Types)

    All contexts are linked together in (FTDContext.ContextList),  avoiding
    list  management  for  each  kind of context (also saving some memory).
    This member  is  used  by  td_context_pop()  to  restaure  the  correct
    context.

*/

enum    {

        FV_TD_CONTEXT_FONT = 1,
        FV_TD_CONTEXT_PENS

        };

//+
///FTDContext
typedef struct in_FeelinTD_Context
{
    FTDParseResult                  result;                 // used by parse function to get attributes

    FTDLine                        *line;                   // current line
    UBYTE                           flags;
    UBYTE                           align;
    UBYTE                           spacing;
    UBYTE                           style;
    UBYTE                           mode;
    UBYTE                           shortcut;
    BYTE                            stop_shortcut;          // if TRUE, stop search for shortcuts
    BYTE                            stop_engine;            // if true engine is stoped, text is then considered as plain until </stop>

    /* contexts */

    FTDContextContainer            *container_font;
    FTDContextContainer            *container_pens;

    FList                           ContainerList;

    /* The ColorsList *must* be stolen  before  the  context  is  disposed,
    otherwise all FTDColor will be disposed with it. */

    FList                           ColorsList;
}
FTDContext;

 
#define FF_TD_CONTEXT_EXTRA_TOPLEFT             (1 << 0)
#define FF_TD_CONTEXT_EXTRA_BOTTOMRIGHT         (1 << 1)
//+
#else

struct in_ContextNode
{
    struct in_ContextNode          *next;
    APTR                            data;
};

typedef struct in_FeelinTD_Context
{
    FTDParseResult                  result;                 // used by parse function to get attributes

    FTDLine                        *line;                   // current line
    bits8                           flags;
    uint8                           align;
    uint8                           spacing;
    uint8                           style;
    uint8                           mode;
    uint8                           shortcut;

//FIXME: replace stop_shortcut and stop_engine by two flags

    int8                            stop_shortcut;          // if TRUE, stop search for shortcuts
    int8                            stop_engine;            // if true engine is stoped, text is then considered as plain until </stop>

    /* contexts */

    #ifdef F_ENABLE_COLOR
    struct in_ContextNode          *pushed_color_nodes;
    #endif
    #ifdef F_ENABLE_FONT
    struct in_ContextNode          *pushed_font_nodes;
    #endif
    #ifdef F_ENABLE_PENS
    struct in_ContextNode          *pushed_pens_nodes;
    #endif

    /* the following lists must be stolen  before  the  context  is  delete,
    otherwise their data will be deleted too */

    #ifdef F_ENABLE_COLOR
    struct in_Color                *colors;
    #endif
    #ifdef F_ENABLE_FONT
    struct in_Font                 *fonts;
    #endif
    #ifdef F_ENABLE_PENS
    struct in_Pens                 *pens;
    #endif
}
FTDContext;

#define FF_TD_CONTEXT_EXTRA_TOPLEFT             (1 << 0)
#define FF_TD_CONTEXT_EXTRA_BOTTOMRIGHT         (1 << 1)

#endif

/***********************************************************************************************/

/*

    The following functions are used to create, delete and  manipulate  the
    FTDContext structure

*/

FTDContext *        td_context_new(FClass *Class,FObject Obj);
void                td_context_dispose(FTDContext *Context);
APTR                td_context_push(FTDContext *Context,uint32 Type);
APTR                td_context_pop(FTDContext *Context,uint32 Type);

#ifdef F_ENABLE_PENS
struct in_Color * td_context_add_color(FTDContext *Context,STRPTR Spec);
#endif

enum    {

        FV_TD_CONTEXT_COLOR,
        FV_TD_CONTEXT_FONT,
        FV_TD_CONTEXT_PENS

        };

#endif
