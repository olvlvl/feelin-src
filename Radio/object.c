#include "Private.h"
	
//#define DB_NEW
//#define DB_DISPOSE
//#define DB_ADDMEMBER
//#define DB_REMMEMBER
//#define DB_LAYOUT
 
/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///radio_update_positions
STATIC void radio_update_positions(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	struct in_Item *item;
	
	uint32 i = 0;
	
	for (item = (struct in_Item *) LOD->items_list.Head ; item ; item = item->next)
	{
		item->position = i++;
	}
}
//+
///radio_layout
STATIC void radio_layout(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_Item *item;
	
	IFEELIN F_Set(Obj, FA_Group_Quiet, TRUE);
	
	#ifdef DB_LAYOUT
	IFEELIN F_Log(0,"remove previously added images and labels");
	#endif
	
	for (item = (struct in_Item *) LOD->items_list.Head ; item ; item = item->next)
	{
		if (IFEELIN F_Get(item->image, FA_Element_Parent))
		{
			IFEELIN F_SuperDo(Class,Obj, FM_RemMember, item->image);
		}

		if (IFEELIN F_Get(item->label, FA_Element_Parent))
		{
			IFEELIN F_SuperDo(Class,Obj, FM_RemMember, item->label);
		}
	}
 
	if ((LOD->count * 2 / LOD->rows) < 2)
	{
		#ifdef DB_LAYOUT
		IFEELIN F_Log(0,"not enough object for special layout");
		#endif

		for (item = (struct in_Item *) LOD->items_list.Head ; item ; item = item->next)
		{
			IFEELIN F_SuperDo(Class, Obj, FM_AddMember, (LOD->layout == FV_Radio_Layout_Above) ? item->image : item->label, FV_AddMember_Tail);
			IFEELIN F_SuperDo(Class, Obj, FM_AddMember, (LOD->layout == FV_Radio_Layout_Above) ? item->label : item->image, FV_AddMember_Tail);
		}
	}
	else
	{
		#ifdef DB_LAYOUT
		IFEELIN F_Log(0,"use special layout (%ld) rows (%ld) >> %ld pass",LOD->layout,LOD->rows,LOD->count * 2 / LOD->rows);
		#endif

		for (item = (struct in_Item *) LOD->items_list.Head ; item ; /*item = item->next*/)
		{
			struct in_Item *start = item;
			uint32 i;
				
			/* place the 'above' object (image or label) */

			for (i = 0 ; i < (LOD->count * 2 / LOD->rows) ; i++)
			{
				#ifdef DB_LAYOUT
				IFEELIN F_Log
				(
					0, "add item (0x%08lx) %s 0x%08lx",
										
					item,
					(LOD->layout == FV_Radio_Layout_Above) ? "image" : "label",
					(LOD->layout == FV_Radio_Layout_Above) ? item->image : item->label
				);
				#endif

				IFEELIN F_SuperDo(Class,Obj, FM_AddMember, (LOD->layout == FV_Radio_Layout_Above) ? item->image : item->label, FV_AddMember_Tail);
				
				item = item->next;
				
				if (item == NULL)
				{
					break;
				}
			}
							
			/* reset 'radio_item' */
			
			item = start;
		
			/* place the 'below' object (label or image */
									
			for (i = 0 ; i < (LOD->count * 2 / LOD->rows) ; i++)
			{
				#ifdef DB_LAYOUT
				IFEELIN F_Log
				(
					0, "add item (0x%08lx) %s 0x%08lx",
										
					item,
					(LOD->layout == FV_Radio_Layout_Above) ? "label" : "image",
					(LOD->layout == FV_Radio_Layout_Above) ? item->label : item->image
				);
				#endif
				
				IFEELIN F_SuperDo(Class,Obj, FM_AddMember, (LOD->layout == FV_Radio_Layout_Above) ? item->label : item->image, FV_AddMember_Tail);

				item = item->next;

				if (item == NULL)
				{
					break;
				}
			}
		}
	}

	IFEELIN F_Set(Obj, FA_Group_Quiet, FALSE);
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Radio_New
F_METHOD(uint32,Radio_New)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg,*real,item;

	uint32 active = 0;
 
	LOD->rows = 0;
	LOD->layout = FV_Radio_Layout_None;

	#ifdef F_NEW_GLOBALCONNECT
	F_SAVE_ELEMENT_PUBLIC;
	#endif

	while ((real = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ACTIVE:
		{
			active = item.ti_Data;
			
			real->ti_Tag = TAG_IGNORE;
			real->ti_Data = 0;
		}
		break;
 
		case FV_ATTRIBUTE_PREPARSE:
		{
			LOD->preparse_render = (STRPTR) item.ti_Data;
		}
		break;
		
		case FV_ATTRIBUTE_ALTPREPARSE:
		{
			LOD->preparse_select = (STRPTR) item.ti_Data;
		}
		break;
		
		case FA_Group_Rows:
		{
			LOD->rows = item.ti_Data;
		}
		break;

		case FV_ATTRIBUTE_LAYOUT:
		{
			LOD->layout = item.ti_Data;
		}
		break;
	}

	if (IFEELIN F_SuperDo(Class,Obj,Method,

		FA_Widget_Chainable, TRUE,

		TAG_MORE, Msg))
	{
		IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(ACTIVE), active);
 
		return (uint32) Obj;
	}

	return 0;
}
//+
///Radio_Dispose
F_METHOD(uint32,Radio_Dispose)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_Item *item;
	
	while ((item = (struct in_Item *) LOD->items_list.Tail) != NULL)
	{
		FObject radioitem = item->radioitem;
			
		#ifdef DB_DISPOSE
		IFEELIN F_Log(0,"remove item (0x%08lx) radioitem (0x%08lx)",item,radioitem);
		#endif
 
		IFEELIN F_Do(Obj, FM_RemMember, radioitem);
			
		#ifdef DB_DISPOSE
		IFEELIN F_Log(0,"dispose radioitem (0x%08lx)",radioitem);
		#endif
 
		IFEELIN F_DisposeObj(radioitem);
	}
 
	return F_SUPERDO();
}
//+
///Radio_Set
F_METHOD(void,Radio_Set)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	struct TagItem *Tags = Msg, *real, item;

	while ((real = IFEELIN F_DynamicNTI(&Tags,&item,Class)) != NULL)
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ACTIVE:
		{
			struct in_Item *active;

			switch (item.ti_Data)
			{
				case FV_Radio_Active_Prev:
				{
					if ((LOD->active == NULL) || (LOD->active == (struct in_Item *) LOD->items_list.Head))
					{
						active = (struct in_Item *) LOD->items_list.Tail;
					}
					else
					{
						active = LOD->active->prev;
					}
				}
				break;
				
				case FV_Radio_Active_Next:
				{
					if ((LOD->active == NULL) || (LOD->active == (struct in_Item *) LOD->items_list.Tail))
					{
						active = (struct in_Item *) LOD->items_list.Head;
					}
					else
					{
						active = LOD->active->next;
					}
				}
				break;
				
				case FV_Radio_Active_Last:
				{
					active = (struct in_Item *) LOD->items_list.Tail;
				}
				break;
				
				case FV_Radio_Active_First:
				{
					active = (struct in_Item *) LOD->items_list.Head;
				}
				break;
				
				default:
				{
					for (active = (struct in_Item *) LOD->items_list.Head ; active ; active = active->next)
					{
						if (active->position == item.ti_Data)
						{
							break;
						}
					}
				}
				break;
			};

			if (active != LOD->active)
			{
				if (LOD->active)
				{
					IFEELIN F_Do
					(
						LOD->active->image, FM_Set,

						FA_NoNotify, TRUE,
						FA_Widget_Selected, FALSE,

						TAG_DONE
					);

					if (LOD->active->radioitem_child == NULL)
					{
						STRPTR preparse = (STRPTR) IFEELIN F_Get(active->radioitem, F_AUTO_ID(PREPARSE));

						if (preparse == NULL)
						{
							preparse = LOD->preparse_render;
						}

						IFEELIN F_Do
						(
							LOD->active->label, FM_Set,

							FA_NoNotify, TRUE,
							FA_Widget_Selected, FALSE,
							FA_Text_PreParse, preparse,

							TAG_DONE
						);
					}
				}

				LOD->active = active;

				if (LOD->active)
				{
					IFEELIN F_Do
					(
						LOD->active->image, FM_Set,

						FA_NoNotify, TRUE,
						FA_Widget_Selected, TRUE,

						TAG_DONE
					);

					if (LOD->active->radioitem_child == NULL)
					{
						STRPTR preparse = (STRPTR) IFEELIN F_Get(active->radioitem, F_AUTO_ID(ALTPREPARSE));

						if (preparse == NULL)
						{
							preparse = LOD->preparse_select;
						}

						IFEELIN F_Do
						(
							LOD->active->label, FM_Set,

							FA_NoNotify, TRUE,
							FA_Widget_Selected, TRUE,
							(preparse != NULL) ? FA_Text_PreParse : TAG_IGNORE, preparse,

							TAG_DONE
						);
					}
				}
			}
		
			real->ti_Data = LOD->active ? LOD->active->position : 0;
		}
		break;

		/* */

		case FA_Widget_Active:
		{
//			  IFEELIN F_Log(0, "active (%ld)", item.ti_Data);

			if (LOD->active != NULL)
			{
				IFEELIN F_Set(LOD->active->image, FA_Widget_Active, item.ti_Data);
			}
		}
		break;
	}
 
	F_SUPERDO();
}
//+
///Radio_Get
F_METHOD(void,Radio_Get)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);
	struct TagItem *Tags = Msg,item;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_ACTIVE:
		{
			F_STORE(LOD->active ? LOD->active->position : 0);
		}
		break;
	}

	F_SUPERDO();
}
//+

///Radio_AddMember
F_METHODM(uint32,Radio_AddMember,FS_AddMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
																	
	if (Msg->Orphan)
	{
		struct in_Item *item = IFEELIN F_New(sizeof (struct in_Item));
		
		if (item)
		{
			STRPTR label = NULL;
			STRPTR image_render = NULL;
			STRPTR image_select = NULL;
			STRPTR preparse_render = NULL;
//			  STRPTR preparse_select = NULL;
			
			item->radioitem = Msg->Orphan;
			
			IFEELIN F_Do
			(
				item->radioitem, FM_Get,

				F_AUTO_ID(LABEL),       &label,
				F_AUTO_ID(IMAGE),       &image_render,
				F_AUTO_ID(ALTIMAGE),    &image_select,
				F_AUTO_ID(PREPARSE),    &preparse_render,
//				  F_AUTO_ID(ALTPREPARSE), &preparse_select,
				
				FA_Child, &item->radioitem_child,

				TAG_DONE
			);

			#ifdef DB_NEW

			IFEELIN F_Log
			(
				0, "item[%02ld] label (%s) image (%s) preparse (%s) altpreparse (%s)",

				LOD->count,
				label ? label : (STRPTR) "",
				image ? image : (STRPTR) "",
				preparse_render ? preparse_render : (STRPTR) "",
				preparse_select ? preparse_select : (STRPTR) ""
			);

			#endif

/*** create image button ***********************************************************************/

			item->image = ImageObject,

				FA_Element_Class,  		"radio-button",

				FA_Widget_SetMax,     	FV_Widget_SetBoth,

				FA_Widget_Chainable,	FALSE,
				#ifdef F_NEW_WIDGET_MODE
				FA_Widget_Mode,     	FV_Widget_Mode_Touch,
				#else
				FA_Widget_Mode,     	FV_Widget_Mode_Immediate,
				#endif

				image_render ? (uint32) "FA_Image_Spec" : TAG_IGNORE, image_render,
				image_select ? (uint32) "FA_Image_AltSpec" : TAG_IGNORE, image_select,

				End;

			if (item->image != NULL)
			{
				IFEELIN F_Do
				(
					item->image, FM_Notify,
					FA_Widget_Selected, FV_Notify_Always,
					Obj, F_METHOD_ID(ACTIVATE), 1, item
				);

/*** create labels (which may be objects *******************************************************/

				if (item->radioitem_child)
				{
					item->label = item->radioitem_child;
				}
				else
				{
					item->label = TextObject,

						FA_Widget_SetMax,         FV_Widget_SetHeight,
						
						FA_Widget_Chainable,    FALSE,
						#ifdef F_NEW_WIDGET_MODE
						FA_Widget_Mode,         FV_Widget_Mode_Touch,
						#else
						FA_Widget_Mode,         FV_Widget_Mode_Immediate,
						#endif
						
						FA_Text_Contents,       label,
						FA_Text_PreParse,       preparse_render ? preparse_render : LOD->preparse_render,
//						  FA_Text_AltPreParse,    preparse_select ? preparse_select : LOD->preparse_select,

						End;
				}
			
				if (item->label)
				{
					IFEELIN F_Do
					(
						item->label, FM_Notify,
						FA_Widget_Selected, FV_Notify_Always,
						Obj, F_METHOD_ID(ACTIVATE), 1, item
					);
					
//FIXME: FV_AddMember_Insert not supported yet !
									
					if (Msg->Position == FV_AddMember_Head)
					{
						IFEELIN F_LinkHead(&LOD->items_list, (FNode *) item);
					}
					else
					{
						IFEELIN F_LinkTail(&LOD->items_list, (FNode *) item);
					}
				
					radio_update_positions(Class,Obj);
 
					LOD->count++;
				
/*** add image and label ***********************************************************************/
 
					if (LOD->rows && LOD->layout != FV_Radio_Layout_None)
					{
						radio_layout(Class,Obj);
					}
					else
					{
						#ifdef DB_ADDMEMBER
						IFEELIN F_Log(0,"addmember image (0x%08lx)",item->image);
						#endif
						
						IFEELIN F_SuperDo(Class,Obj, FM_AddMember, item->image, FV_AddMember_Tail);
						
						#ifdef DB_ADDMEMBER
						IFEELIN F_Log(0,"addmember label (0x%08lx)",item->label);
						#endif

						IFEELIN F_SuperDo(Class,Obj, FM_AddMember, item->label, FV_AddMember_Tail);
					}
 
					return TRUE;
				}
				
				IFEELIN F_DisposeObj(item->image);
			}
			IFEELIN F_Dispose(item);
		}
	}
	
	return FALSE; 
}
//+
///Radio_RemMember
F_METHODM(uint32,Radio_RemMember,FS_RemMember)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	
	struct in_Item *item;
	
	for (item = (struct in_Item *) LOD->items_list.Head ; item ; item = (struct in_Item *) item->next)
	{
		if (item->radioitem == Msg->Member)
		{
			#ifdef DB_REMMEMBER
			IFEELIN F_Log(0,"item (0x%08lx) unlink",item);
			#endif

			IFEELIN F_LinkRemove(&LOD->items_list, (FNode *) item);
			
/*** remove and dispose 'image' ****************************************************************/
 
			if (IFEELIN F_Get(item->image, FA_Element_Parent))
			{
				#ifdef DB_REMMEMBER
				IFEELIN F_Log(0,"item (0x%08lx) remove image (0x%08lx)",item,item->image);
				#endif
 
				IFEELIN F_SuperDo(Class,Obj, FM_RemMember, item->image);
			}
		
			IFEELIN F_DisposeObj(item->image);

/*** remove and dispose 'label' ****************************************************************/

			if (IFEELIN F_Get(item->label, FA_Element_Parent))
			{
				#ifdef DB_REMMEMBER
				IFEELIN F_Log(0,"item (0x%08lx) remove label (0x%08lx)",item,item->label);
				#endif

				IFEELIN F_SuperDo(Class,Obj, FM_RemMember, item->label);
			}
		
			if (item->radioitem_child == NULL)
			{
				IFEELIN F_DisposeObj(item->label);
			}
		
/*** dispose item structure ********************************************************************/
				
			#ifdef DB_REMMEMBER
			IFEELIN F_Log(0,"item (0x%08lx) dispose",item);
			#endif
 
			IFEELIN F_Dispose(item);

/*** update positions **************************************************************************/

			radio_update_positions(Class,Obj);

			LOD->count--;

			if (LOD->rows && LOD->layout != FV_Radio_Layout_None)
			{
				radio_layout(Class,Obj);
			}
 
			return TRUE;
		}
	}

	#ifdef DB_REMMEMBER
	IFEELIN F_Log(0,"item %s{%08lx} not found",_object_classname(Msg->Member), Msg->Member);
	#endif
	
	return FALSE; 
}
//+

///Radio_Activate

/* The only purpose of this method  is  to  avoid  the  activation  of  item
already active, triggering the 'Active' attribute uselessly */

struct FS_Radio_Activate						{ struct in_Item *Item; };

F_METHODM(uint32, Radio_Activate, FS_Radio_Activate)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	struct in_Item *item = Msg->Item;

	if ((item == NULL) || (item == LOD->active))
	{
		return 0;
	}

	IFEELIN F_Set(Obj, F_ATTRIBUTE_ID(ACTIVE), item->position);

	return 0;
}
//+
