#include "Private.h"

//#define DB_OPENFONT
 
/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///app_create_ta
STATIC struct TextAttr *app_create_ta(STRPTR Define,struct TextAttr *TA)
{
	if (Define)
	{
		STRPTR name = IFEELIN F_New(256);

		if (name)
		{
			STRPTR back = name;

			while ((*Define != NULL) && (*Define != '/'))
			{
				*name++ = *Define++;
			}

			IEXEC CopyMem(".font",name,5);
			
			name = back;

			if (*Define++ == '/')
			{
				uint16 size = atol(Define);
				
				if (size)
				{
					TA->ta_Name  = name;
					TA->ta_YSize = size;
					TA->ta_Style = FS_NORMAL;
					TA->ta_Flags = NULL;

					return TA;
				}
			}
			IFEELIN F_Dispose(name);
		}
	}
	return NULL;
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Application_OpenFont
F_METHODM(struct TextFont *,Application_OpenFont,FS_Application_OpenFont)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TextFont *font = NULL;

	if (Msg->Spec != NULL)
	{
		struct TextAttr ta;

		if (app_create_ta(Msg->Spec, &ta) != NULL)
		{
			font = IDISKFONT OpenDiskFont(&ta);

			IFEELIN F_Dispose(ta.ta_Name);
		}
	}

/*** fall back : screen font *******************************************************************/
 
	if (!font)
	{
		struct Screen *scr = (struct Screen *) IFEELIN F_Get(LOD->display, (uint32) "FA_Display_Screen");

		if (scr)
		{
			font = IGRAPHICS OpenFont(scr->Font);
		}
	}

	#ifdef DB_OPENFONT
	IFEELIN F_Log(0,"FONT (%s)(0x%08lx) - YSize %ld",font->tf_Message.mn_Node.ln_Name,font,font->tf_YSize);
	#endif

	return font;
}
//+

