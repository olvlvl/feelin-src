#include "Private.h"

struct FS_Page_Tree                             { int32 MouseX, MouseY; };

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Page_Setup
F_METHOD(bool32,Page_Setup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	LOD->style = F_LPD(Class, IFEELIN F_Get(Obj, FA_Element_DecodedStyle));

/*** tabs labels *******************************************************************************/

	if (LOD->style->tab_font == NULL)
	{
		struct TextAttr ta;

		ta.ta_Name  = _area_font->tf_Message.mn_Node.ln_Name;
		ta.ta_YSize = _area_font->tf_YSize;
		ta.ta_Style = _area_font->tf_Style;
		ta.ta_Flags = _area_font->tf_Flags;

		LOD->font = IGRAPHICS OpenFont(&ta);
	}
	else
	{
		LOD->font = (struct TextFont *) IFEELIN F_Do(_area_app, FM_Application_OpenFont, Obj, LOD->style->tab_font);
	}

	LOD->TD = TextDisplayObject,

		FA_TextDisplay_Font,      LOD->font,
		FA_TextDisplay_Shortcut,  FALSE,

		End;

	IFEELIN F_Do(LOD->TD, FM_TextDisplay_Setup, _area_render);

/*** backgrounds *******************************************************************************/

	LOD->back_active = ImageDisplayObject,

		FA_ImageDisplay_Spec, LOD->style->active_tab_back,

	End;

	if (LOD->back_active != NULL)
	{
		if ((IFEELIN F_Do(LOD->back_active, FM_ImageDisplay_Setup, _area_render)) == FALSE)
		{
			IFEELIN F_DisposeObj(LOD->back_active);

			LOD->back_active = NULL;
		}
	}

	/* inactive tab */

	LOD->back_inactive = ImageDisplayObject,

		FA_ImageDisplay_Spec, LOD->style->inactive_tab_back,

	End;

	if (LOD->back_inactive != NULL)
	{
		if ((IFEELIN F_Do(LOD->back_inactive, FM_ImageDisplay_Setup, _area_render)) == FALSE)
		{
			IFEELIN F_DisposeObj(LOD->back_inactive);

			LOD->back_inactive = NULL;
		}
	}

	#warning background

		/*

		HOMBRE @ GOFROMIEL : GCC n'aime pas trop les commentaires imbriqués ; il
				faudrait penser au moins à ne pas mettre le code de fermeture de commentaire

		* if we have our own background, we set its origin ourselves istead
		of  leaving  this to FC_Area, and set them below the tabs. This way,
		the user will be able to configure really nice gradients. *

		{
			FFramePublic *self_frame_public = (FFramePublic *) IFEELIN F_Get(Obj, FA_Frame_PublicData);
			FFramePublic *parent_frame_public = (FFramePublic *) IFEELIN F_Get(_area_parent, FA_Frame_PublicData);

			if (self_frame_public && parent_frame_public && (self_frame_public->Back != parent_frame_public->Back))
			{
				IFEELIN F_Set(self_frame_public->Back, FA_ImageDisplay_Origin, (uint32) &LOD->back_origin);
			}
		}

		*/

/*** tabs height *******************************************************************************/

	LOD->tabs_height = MAX
	(
		FV_TAB_BORDER_TOP + LOD->style->active_padding_top + LOD->style->active_padding_bottom,
		LOD->style->inactive_margin_top + FV_ITAB_BORDER_TOP + LOD->style->inactive_padding_top + LOD->style->inactive_padding_bottom
	)
	+ FV_FONT_SIZE;

/*** events ************************************************************************************/

	IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_BUTTON, 0);

	LOD->node_update = NULL;

	if (LOD->node_active == NULL)
	{
		LOD->node_active = (FWidgetNode *) IFEELIN F_Get(Obj, FA_Family_Head);
	}

	return TRUE;
}
//+
///Page_Cleanup
F_METHOD(uint32,Page_Cleanup)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
															
	if (LOD->back_active)
	{
		IFEELIN F_Do(LOD->back_active,FM_ImageDisplay_Cleanup);
		IFEELIN F_DisposeObj(LOD->back_active);
		
		LOD->back_active = NULL;
	}

	if (LOD->back_inactive)
	{
		IFEELIN F_Do(LOD->back_inactive,FM_ImageDisplay_Cleanup);
		IFEELIN F_DisposeObj(LOD->back_inactive);

		LOD->back_inactive = NULL;
	}

	IFEELIN F_Do(Obj,FM_Widget_ModifyEvents,0,FF_EVENT_BUTTON);

	IFEELIN F_Do(LOD->TD,FM_TextDisplay_Cleanup);
	IFEELIN F_DisposeObj(LOD->TD); LOD->TD = NULL;

	if (LOD->font)
	{
		IGRAPHICS CloseFont(LOD->font); LOD->font = NULL;
	}

	return F_SUPERDO();
}
//+
///Page_CreateDecodedStyle
F_METHODM(bool32, Page_CreateDecodedStyle, FS_Element_CreateDecodedStyle)
{
	struct LocalPropertiesData *space = F_LPD(Class, Msg->DecodedSpace);

	FPreferenceStyle *style;

	space->padding_left              = DEF_PAGE_PADDING_LEFT;
	space->padding_right             = DEF_PAGE_PADDING_RIGHT;
	space->padding_top               = DEF_PAGE_PADDING_TOP;
	space->padding_bottom            = DEF_PAGE_PADDING_BOTTOM;
	
	space->active_padding_top        = DEF_TAB_PADDING_TOP;
	space->active_padding_bottom     = DEF_TAB_PADDING_BOTTOM;
	
	space->inactive_padding_top      = DEF_ITAB_PADDING_TOP;
	space->inactive_padding_bottom   = DEF_ITAB_PADDING_BOTTOM;
	space->inactive_margin_top       = DEF_ITAB_MARGIN_TOP;

	for (style = Msg->Style ; style ; style = style->Next)
	{
		FPreferenceProperty **properties_a;

		for (properties_a = style->Properties ; *properties_a ; properties_a++)
		{
			FPreferenceProperty *prop = *properties_a;

			if (prop->Atom == F_PROPERTY_ATOM(PADDING_LEFT))
			{
				space->padding_left = prop->NumericValue;
			}
			else if (prop->Atom == F_PROPERTY_ATOM(PADDING_RIGHT))
			{
				space->padding_right = prop->NumericValue;
			}
			else if (prop->Atom == F_PROPERTY_ATOM(PADDING_TOP))
			{
				space->padding_top = prop->NumericValue;
			}
			else if (prop->Atom == F_PROPERTY_ATOM(PADDING_BOTTOM))
			{
				space->padding_bottom = prop->NumericValue;
			}

			else if (prop->Atom == F_PROPERTY_ATOM(ACTIVE_PADDING_TOP))
			{
				space->active_padding_top = prop->NumericValue;
			}
			else if (prop->Atom == F_PROPERTY_ATOM(ACTIVE_PADDING_BOTTOM))
			{
				space->active_padding_bottom = prop->NumericValue;
			}

			else if (prop->Atom == F_PROPERTY_ATOM(INACTIVE_PADDING_TOP))
			{
				space->inactive_padding_top = prop->NumericValue;
			}
			else if (prop->Atom == F_PROPERTY_ATOM(INACTIVE_PADDING_BOTTOM))
			{
				space->inactive_padding_bottom = prop->NumericValue;
			}
			else if (prop->Atom == F_PROPERTY_ATOM(INACTIVE_MARGIN_TOP))
			{
				space->inactive_margin_top = prop->NumericValue;
			}

			else if (prop->Atom == F_PROPERTY_ATOM(TAB_FONT))
			{
				space->tab_font = prop->Value;
			}

			else if (prop->Atom == F_PROPERTY_ATOM(PREPARSE_ACTIVE))
			{
				space->active_tab_prep = prop->Value;
			}
			else if (prop->Atom == F_PROPERTY_ATOM(PREPARSE_INACTIVE))
			{
				space->inactive_tab_prep = prop->Value;
			}

			else if (prop->Atom == F_PROPERTY_ATOM(TAB_BACK_ACTIVE))
			{
				space->active_tab_back = prop->Value;
			}
			else if (prop->Atom == F_PROPERTY_ATOM(TAB_BACK_INACTIVE))
			{
				space->inactive_tab_back = prop->Value;
			}

		}
	}

	return TRUE;
}
//+

