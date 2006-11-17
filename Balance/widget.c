#include "Private.h"

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

#define _parent_init                            FAreaPublic *parent_sub_public = F_GET_AREA_PUBLIC(parent)
#define _parent_x                               (parent_sub_public->Box.x)
#define _parent_y                               (parent_sub_public->Box.y)
#define _parent_w                               (parent_sub_public->Box.w)
#define _parent_h                               (parent_sub_public->Box.h)
#define _parent_set_damaged                     (parent_sub_public->Flags |= FF_Area_Damaged)

///balance_begin
void balance_begin(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *node,*self,*head,*tail;

	bool32 side;
	uint32 spc;

	IFEELIN F_Do
	(
		#ifdef F_NEW_GLOBALCONNECT
		_element_parent, FM_Get,
		#else
		_area_parent, FM_Get,
		#endif
		
		FA_Family_Head, &head,
		FA_Family_Tail, &tail,
		_balance_is_vertical ? FA_Group_HSpacing : FA_Group_VSpacing, &spc,
		
		TAG_DONE
	);

	for (_each)
	{
		if (node->Widget == Obj)
		{
			break;
		}
	}

	self = node;

	if (_balance_is_moving)
	{
		FObject active;

		if (!(FF_Render_Complex & _area_render->Flags))
		{
			_balance_set_complexset;

			_area_render->Flags |= FF_Render_Complex;
		}
		else
		{
			_balance_clear_complexset;
		}

		if (_balance_is_quickdraw)
		{
			IFEELIN F_Set(_area_render,FA_Render_Forbid,TRUE);
		}

		LOD->ActiveObj = NULL;

		if ((active = (FObject) IFEELIN F_Get(_area_win,FA_Window_ActiveObject)) != Obj)
		{
			LOD->ActiveObj = active;

			IFEELIN F_Set(_area_win, FA_Window_ActiveObject, NULL);
		}
	}

	LOD->BNum = 0; LOD->BMin = 0; LOD->BMax = 0;
	LOD->ANum = 0; LOD->AMin = 0; LOD->AMax = 0;

	side = 0;

	for (_each)
	{
		if (node->Widget == Obj)
		{
			side = 1;
		}
		else if (side)
		{
			LOD->ANum++;
			LOD->AMin += _balance_is_vertical ? _sub_minw : _sub_minh;
			LOD->AMax += _balance_is_vertical ? _sub_maxw : _sub_maxh;
		}
		else
		{
			LOD->BNum++;
			LOD->BMin += _balance_is_vertical ? _sub_minw : _sub_minh;
			LOD->BMax += _balance_is_vertical ? _sub_maxw : _sub_maxh;
		}
	}

	if (self->Prev)
	{
		FWidgetNode *prev = self->Prev;

		LOD->BSize = _balance_is_vertical
			? (_prev_x2 - _head_x + 1) - ((LOD->BNum - 1) * spc)
			: (_prev_y2 - _head_y + 1) - ((LOD->BNum - 1) * spc);
	}
	else
	{
		LOD->BSize = 0;
	}

	if (self->Next)
	{
		FWidgetNode *next = self->Next;

		LOD->ASize = _balance_is_vertical
			? (_tail_x2 - _next_x + 1) - ((LOD->ANum - 1) * spc)
			: (_tail_y2 - _next_y + 1) - ((LOD->ANum - 1) * spc);
	}
	else
	{
		LOD->BSize = 0;
	}

	#ifdef DB_BEGIN

	IFEELIN F_Log(0, "\n\n"

		"before: num (%ld) min (%ld) max (%ld) size (%ld)\n"
		"after: num (%ld) min (%ld) max (%ld) size (%ld)",

		LOD->BNum, LOD->BMin, LOD->BMax, LOD->BSize,
		LOD->ANum, LOD->AMin, LOD->AMax, LOD->ASize);

	#endif
}
//+
///balance_done
void balance_done(FClass *Class,FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FWidgetNode *node;

	_balance_clear_moving;

//   balance_set_widget_weight(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT
	for (node = (FWidgetNode *) IFEELIN F_Get(_element_parent, FA_Family_Head) ; node ; node = node->Next)
	#else
	for (node = (FWidgetNode *) IFEELIN F_Get(_area_parent, FA_Family_Head) ; node ; node = node->Next)
	#endif
	{
		if (node->Widget != Obj)
		{
			_sub_weight = _balance_is_vertical ? _sub_w : _sub_h;
		}
	}

	if (_balance_is_complexset)
	{
		_area_render->Flags &= ~FF_Render_Complex;

		_balance_clear_complexset;
	}

	if (_balance_is_quickdraw)
	{
		IFEELIN F_Set(_area_render, FA_Render_Forbid, FALSE);
		#ifdef F_NEW_GLOBALCONNECT
		IFEELIN F_Do(_element_parent, FM_Area_Layout);
		IFEELIN F_Draw(_element_parent, FF_Draw_Object);
		#else
		IFEELIN F_Do(_area_parent, FM_Area_Layout);
		IFEELIN F_Draw(_area_parent, FF_Draw_Object);
		#endif
	}

	if (LOD->ActiveObj && LOD->ActiveObj != Obj)
	{
		IFEELIN F_Set(_area_win, FA_Window_ActiveObject, (uint32) LOD->ActiveObj);
	}
}
//+
///balance_modify

void balance_modify(FClass *Class,FObject Obj,LONG Diff)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	#ifdef F_NEW_GLOBALCONNECT
	FObject parent = _element_parent;
	#else
	FObject parent = _area_parent;
	#endif
	FWidgetNode *node,*head,*tail;

	uint32 hspace,vspace;

	bool32 side;
	uint32 BSize,ASize,BNum,ANum,pos;
	uint16 loss,size;

	IFEELIN F_Do
	(
		parent, FM_Get,
		
		FA_Family_Head,     &head,
		FA_Family_Tail,     &tail,
		FA_Group_HSpacing,  &hspace,
		FA_Group_VSpacing,  &vspace,
		
		TAG_DONE
		
	);

	BNum = LOD->BNum; BSize = LOD->BSize + Diff;
	ANum = LOD->ANum; ASize = LOD->ASize - Diff;

	pos = _balance_is_vertical ? _area_x : _area_y;

	if (BSize < LOD->BMin) { ASize += (BSize - LOD->BMin); BSize = LOD->BMin; }
	if (ASize < LOD->AMin) { BSize += (ASize - LOD->AMin); ASize = LOD->AMin; }
	if (BSize > LOD->BMax)   return;
	if (ASize > LOD->AMax)   return;

/*** On s'occupe d'abord des Objets fixes **********************************/

	side = 0;

	for (_each)
	{
		if (node->Widget == Obj)
		{
			side = 1;
		}
		else
		{
			if (_balance_is_vertical)
			{
				if (_sub_minw == _sub_maxw)
				{
					if (side) { ASize -= _sub_w; ANum--; }
					else      { BSize -= _sub_w; BNum--; }
				}
				else
				{
					_sub_set_compute_w;
				}
			}
			else
			{
				if (_sub_minh == _sub_maxh)
				{
					if (side) { ASize -= _sub_h; ANum--; }
					else      { BSize -= _sub_h; BNum--; }
				}
				else
				{
					_sub_set_compute_h;
				}
			}
		}
	}

/*** Vérification des tailles minimales et maximales ***********************/

	side = 0;

	for (_each)
	{
		if (node->Widget == Obj)
		{
			side = 1;
		}
		else
		{
			if (_balance_is_vertical)
			{
				if (_sub_is_compute_w)
				{
					size = (side) ? ASize / ANum : BSize / BNum;

					if (_sub_minw > size)
					{
						if (_sub_w != _sub_minw)
						{
							_sub_w = _sub_minw;
							_sub_set_damaged;
						}

						_sub_clear_compute_w;
					}
					else if (_sub_maxw < size)
					{
						if (_sub_w != _sub_maxw)
						{
							_sub_w = _sub_maxw;
							_sub_set_damaged;
						}

						_sub_clear_compute_w;
					}
					else continue;//goto __compute_again;

					if (side) { ASize -= _sub_w; ANum--;}
					else      { BSize -= _sub_w; BNum--;}

					node = head ; side = 0;
				}
			}
			else
			{
				if (_sub_is_compute_h)
				{
					size = (side) ? ASize / ANum : BSize / BNum;

					if (_sub_minh > size)
					{
						if (_sub_h != _sub_minh)
						{
							_sub_h = _sub_minh;
							_sub_set_damaged;
						}

						_sub_clear_compute_h;
					}
					else if (_sub_maxh < size)
					{
						if (_sub_h != _sub_maxh)
						{
							_sub_h = _sub_maxh;
							_sub_set_damaged;
						}

						_sub_clear_compute_h;
					}
					else continue;//goto __compute_again;

					if (side) { ASize -= _sub_h; ANum--;}
					else      { BSize -= _sub_h; BNum--;}

					node = head ; side = 0;
				}
			}
		}
//__compute_again:;
	}

	loss = ASize - ((ASize / ANum) * ANum);

	if (ASize < 0) { BSize += ASize; ASize = 0; }
	if (BSize < 0) { ASize += BSize; BSize = 0; }

	side = 0;

	for (_each)
	{
		if (node->Widget == Obj)
		{
			side = 1;
		}
		else
		{
			if (_balance_is_vertical)
			{
				if (_sub_is_compute_w)
				{
					uint16 w = (side) ? ASize / ANum : BSize / BNum;
					
					if (loss) { w += loss; loss = 0; }
 
					if (_sub_w != w)
					{
						_sub_w = w;
						_sub_set_damaged;
					}
					
					_sub_clear_compute_w;
				}
			}
			else
			{
				if (_sub_is_compute_h)
				{
					uint16 h = (side) ? ASize / ANum : BSize / BNum;

					if (loss) { h += loss; loss = 0; }
				
					if (_sub_h != h)
					{
						_sub_h = h;
						_sub_set_damaged;
					}
					
					_sub_clear_compute_h;
				}
			}
		}
	}

/*** Mise en place des Objets **********************************************/

	for (_each)
	{
		if (_sub_prev)
		{
			FWidgetNode *prev = _sub_prev;

			if (_balance_is_vertical)
			{
				int16 x = _prev_x2 + hspace + 1;
 
				if (_sub_x != x)
				{
					_sub_x = x;
					_sub_set_damaged;
				}
			}
			else
			{
				int16 y = _prev_y2 + vspace + 1;
				
				if (_sub_y != y)
				{
					_sub_y = y;
					_sub_set_damaged;
				}
			}
		}
	}

	if (tail && loss)
	{
		if (_balance_is_vertical)
		{
			_tail_x += loss;
			_tail_set_damaged;
		}
		else
		{
			_tail_y += loss;
			_tail_set_damaged;
		}
	}

/*** Dessin des Objets si modification *************************************/

	if (pos != (_balance_is_vertical ? _area_x : _area_y))
	{
		_parent_init;
		_parent_set_damaged;
		_area_flags |= FF_Area_Damaged;

/*** Mise à jour des sous groupes **************************************************************/
		
		if (_balance_is_quickdraw)
		{
			if (_balance_is_vertical)
			{
				IFEELIN F_Erase(parent,_parent_x,_parent_y,_area_x - 1,_parent_y + _parent_h - 1,0);
				IFEELIN F_Erase(parent,_area_x2 + 1,_parent_y,_parent_x + _parent_w - 1,_parent_y + _parent_h - 1,0);
			}
			else
			{
				IFEELIN F_Erase(parent,_parent_x,_parent_y,_parent_x + _parent_w - 1,_area_y - 1,0);
				IFEELIN F_Erase(parent,_parent_x,_area_y2 + 1,_parent_x + _parent_w - 1,_parent_y + _parent_h - 1,0);
			}

			for (_each)
			{
				if (node->Widget == Obj)
				{
					IFEELIN F_Do(Obj, FM_Area_Layout);
					IFEELIN F_Do(Obj, FM_Area_Draw, FF_Draw_Object);
				}
				else
				{
					struct RastPort *rp = _area_rp;

					int16 x1 = _sub_x,x2 = x1 + _sub_w - 1;
					int16 y1 = _sub_y,y2 = y1 + _sub_h - 1;

					_FPen(FV_Pen_Shine);
					_Move(x1,y1); _Draw(x2,y2);
					_Draw(x1,y2); _Draw(x2,y1); _Draw(x1,y1);
				}
			}
		}
		else
		{
			for (_each)
			{
				if (_sub_is_damaged)
				{
					IFEELIN F_Do(node->Widget, FM_Area_Layout);
				}
			}

			IFEELIN F_Draw(parent, FF_Draw_Damaged | FF_Draw_Object);
		}
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Balance_HandleEvent
F_METHODM(uint32,Balance_HandleEvent,FS_Widget_HandleEvent)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	FEvent *fev = Msg->Event;

	switch (fev->Class)
	{
///FF_EVENT_KEY
		case FF_EVENT_KEY:
		{
			if (FF_EVENT_KEY_UP & fev->Flags)
			{
				if (_balance_is_moving)
				{
					balance_done(Class,Obj);
				}
			}
			else if (fev->Key)
			{
				if (fev->Key == FV_KEY_PRESS)
				{
					return FF_HandleEvent_Eat;
				}
				else
				{
					int16 v;

					if (_balance_is_vertical)
					{
						switch (fev->Key)
						{
							case FV_KEY_LEFT:       v = -5     ; break;
							case FV_KEY_RIGHT:      v =  5     ; break;
							case FV_KEY_STEPLEFT:   v = -20    ; break;
							case FV_KEY_STEPRIGHT:  v =  20    ; break;
							case FV_KEY_FIRST:      v = -10000 ; break;
							case FV_KEY_LAST:       v =  10000 ; break;
							default:                             return 0;
						}
					}
					else
					{
						switch (fev->Key)
						{
							case FV_KEY_UP:         v = -5    ; break;
							case FV_KEY_DOWN:       v =  5    ; break;
							case FV_KEY_STEPUP:     v = -20   ; break;
							case FV_KEY_STEPDOWN:   v =  20   ; break;
							case FV_KEY_TOP:        v = -10000; break;
							case FV_KEY_BOTTOM:     v =  10000; break;
							default:                            return 0;
						}
					}

					if (!_balance_is_moving)
					{
						_balance_set_moving;
						LOD->Mouse  = 0;

						balance_begin(Class,Obj);
					}

					LOD->Mouse += v;

					if (LOD->Mouse >  10000) LOD->Mouse =  10000;
					if (LOD->Mouse < -10000) LOD->Mouse = -10000;

					balance_modify(Class,Obj,LOD->Mouse);

					return FF_HandleEvent_Eat;
				}
			}
		}
		break;
//+
///FF_EVENT_BUTTON
		case FF_EVENT_BUTTON:
		{
			if (fev->Code == FV_EVENT_BUTTON_SELECT)
			{
				if (FF_EVENT_BUTTON_DOWN & fev->Flags)
				{
					if (fev->MouseX >= _area_x && fev->MouseX <= _area_x2 &&
						fev->MouseY >= _area_y && fev->MouseY <= _area_y2)
					{
						LOD->Offset  = _balance_is_vertical ? fev->MouseX - _area_x + 1 : fev->MouseY - _area_y + 1;
						LOD->Mouse   = _balance_is_vertical ? fev->MouseX : fev->MouseY;
						_balance_set_moving;

						if ((IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT) & fev->Qualifier)
						{
							IFEELIN F_Log(FV_LOG_DEV,"'restricted' not implemented yet");
						}

						IFEELIN F_Do
						(
							Obj, FM_Set,

							FA_Widget_Selected, TRUE,
							FA_Widget_Pressed, TRUE,

							TAG_DONE
						);
						
						balance_begin(Class,Obj);
						
						IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, FF_EVENT_MOTION, 0);
						
						return FF_HandleEvent_Eat;
					}
				}
				else
				{
					if (_balance_is_moving)
					{
						IFEELIN F_Do
						(
							Obj, FM_Set,

							FA_Widget_Selected, FALSE,
							FA_Widget_Pressed, FALSE,

							TAG_DONE
						);
						
						IFEELIN F_Do(Obj, FM_Widget_ModifyEvents, 0, FF_EVENT_MOTION);
						
						balance_done(Class,Obj);

						return FF_HandleEvent_Eat;
					}
				}
			}
		}
		break;
//+
///FF_EVENT_MOTION
		case FF_EVENT_MOTION:
		{
			if (_balance_is_moving)
			{
				if (_balance_is_vertical)
				{
					if (fev->MouseX - LOD->Offset + 1 == _area_x)
					{
					   return 0;
					}
				}
				else
				{
					if (fev->MouseY - LOD->Offset + 1 == _area_y)
					{
						return 0;
					}
				}

				balance_modify(Class, Obj, _balance_is_vertical ? fev->MouseX - LOD->Mouse : fev->MouseY - LOD->Mouse);

				return FF_HandleEvent_Eat;
			}
		}
		break;
//+
	}
	return 0;
}
//+
