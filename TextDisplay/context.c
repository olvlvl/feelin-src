#include "Private.h"
#include "parse.h"
#include "context.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///td_context_new
FTDContext * td_context_new(FClass *Class,FObject Obj)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    FTDContext *context = IFEELIN F_New(sizeof (FTDContext));

    if (context)
    {
        context->line          = NULL;
        context->align         = FV_TD_LINE_ALIGN_LEFT;
        context->spacing       = 1;
        context->style         = FS_NORMAL;
        context->mode          = JAM1;
        context->shortcut      = 0;
        context->stop_shortcut = (0 == (FF_TD_SHORTCUT & LOD->Flags));
        context->stop_engine   = FALSE;
    }
    else
    {
        IFEELIN F_Log(0,"Unable to create FTDContext. Can't parse lines");
    }

    return context;
}
//+
///td_context_dispose
void td_context_dispose(FTDContext *Context)
{
    if (Context)
    {
        #ifdef F_ENABLE_COLOR
        
        while (Context->pushed_color_nodes)
        {
            struct in_ContextNode *node = Context->pushed_color_nodes;
            
            Context->pushed_color_nodes = node->next;
            
            IFEELIN F_Dispose(node); 
        }
    
        while (Context->colors)
        {
            struct in_Color *node = Context->colors;
            
            Context->colors = node->next;
            
            IFEELIN F_Dispose(node->spec);
            IFEELIN F_Dispose(node);
        }
    
        #endif
        
        #ifdef F_ENABLE_FONT
        
        while (Context->pushed_font_nodes)
        {
            struct in_ContextNode *node = Context->pushed_font_nodes;
            
            Context->pushed_font_nodes = node->next;
            
            IFEELIN F_Dispose(node);
        }
        
        while (Context->fonts)
        {
            struct in_Font *node = Context->fonts;

            Context->fonts = node->next;

            if ((FF_TD_FONT_REFERENCE_FACE & node->flags) == 0)
            {
                IFEELIN F_Dispose(node->face);
            }
             
            IFEELIN F_Dispose(node);
        }
 
        #endif 
        
        #ifdef F_ENABLE_PENS

        while (Context->pushed_pens_nodes)
        {
            struct in_ContextNode *node = Context->pushed_pens_nodes;

            Context->pushed_pens_nodes = node->next;

            IFEELIN F_Dispose(node);
        }

        #endif
 
        IFEELIN F_Dispose(Context);
    }
}
//+

#if defined(F_ENABLE_COLOR) || defined(F_ENABLE_FONT) || defined(F_ENABLE_PENS)
///td_context_push
APTR td_context_push(FTDContext *Context,uint32 Type)
{
    struct in_ContextNode *node = IFEELIN F_NewP(CUD->Pool, sizeof (struct in_ContextNode));
    
    if (node)
    {
        switch (Type)
        {
            #ifdef F_ENABLE_COLOR
            
            case FV_TD_CONTEXT_COLOR:
            {
                node->data = IFEELIN F_NewP(CUD->Pool, sizeof (struct in_Color));
                
                if (node->data)
                {
                    node->next = Context->pushed_color_nodes;
                    Context->pushed_color_nodes = node;
                    
                    ((struct in_Color *) node->data)->next = Context->colors;
                    Context->colors = (struct in_Color *) node->data;
                }
            }
            break;
             
            #endif
            
            #ifdef F_ENABLE_FONT
            
            case FV_TD_CONTEXT_FONT:
            {
                node->data = IFEELIN F_NewP(CUD->Pool, sizeof (struct in_Font));
                
                if (node->data)
                {
                    node->next = Context->pushed_font_nodes;
                    Context->pushed_font_nodes = node;
                    
                    ((struct in_Font *) node->data)->next = Context->fonts;
                    Context->fonts = (struct in_Font *) node->data;
                    
                    if (node->next)
                    {
                        ((struct in_Font *) node->data)->parent = (struct in_Font *) node->next->data;
                    }
                }
            }
            break;
        
            #endif
            
            #ifdef F_ENABLE_PENS
            
            case FV_TD_CONTEXT_PENS:
            {
                node->data = IFEELIN F_NewP(CUD->Pool, sizeof (struct in_Pens));

                if (node->data)
                {
                    node->next = Context->pushed_pens_nodes;
                    Context->pushed_pens_nodes = node;
                    
                    ((struct in_Pens *) node->data)->next = Context->pens;
                    Context->pens = (struct in_Pens *) node->data;
                    
                    if (node->next)
                    {
                        ((struct in_Pens *) node->data)->parent = (struct in_Pens *) node->next->data;
                    }
                }
            }
            break;
 
            #endif
        }
        
        if (node->data)
        {
            return node->data;
        }
    }
    
    return NULL;
}
//+
///td_context_pop
APTR td_context_pop(FTDContext *Context,uint32 Type)
{
    struct in_ContextNode *node = NULL;
    
    APTR rc = NULL;
 
    switch (Type)
    {
        #ifdef F_ENABLE_COLOR

        case FV_TD_CONTEXT_COLOR:
        {
            node = Context->pushed_color_nodes;
            
            if (node)
            {
                Context->pushed_color_nodes = node->next;
            }
        }
        break;
        
        #endif
        
        #ifdef F_ENABLE_FONT
        
        case FV_TD_CONTEXT_FONT:
        {
            node = Context->pushed_font_nodes;
            
            if (node)
            {

                /* The first font node cannot be poped. The node was  pushed
                by  td_create()  and  not  by  the  parser, thus if a pop is
                requested it's an error because the developer  has  used  to
                many </font> */
 
                if (node->next)
                {
                    Context->pushed_font_nodes = node->next;
                }
                else
                {
                    node = NULL;
                }
            }
        }
        break;
        
        #endif
        
        #ifdef F_ENABLE_PENS

        case FV_TD_CONTEXT_PENS:
        {
            node = Context->pushed_pens_nodes;

            if (node)
            {
                Context->pushed_pens_nodes = node->next;
            }
        }
        break;

        #endif
    }

    if (node)
    {
        rc = node->data;
 
        IFEELIN F_Dispose(node);
    }

    return rc;
}
//+
#endif

#ifdef F_ENABLE_PENS
///td_context_add_color
struct in_Color * td_context_add_color(FTDContext *Context,STRPTR Spec)
{
    struct in_Color *color = IFEELIN F_NewP(CUD->Pool,sizeof (struct in_Color));

    if (color)
    {
        color->next = Context->colors;
        color->spec = (STRPTR) Spec;

        Context->colors = color;
    }
    
    return color;
}
//+
#endif

