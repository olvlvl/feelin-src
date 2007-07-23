#include "Private.h"

//#define DB_NOT_DAMAGED
//#define DB_DAMAGED

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

struct FeelinLayoutElement
{
	uint16 min;
	uint16 max;
	uint16 dim;
	uint16 weight;
};

/*

060618: To speed things a little, and avoid  checking  _sub_is_showable  too
much, I should create an array of FWidgetNode items and discard objects that
cannot be layouted...

*/

#define F_ENABLE_WIDGETS_ARRAY

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///code_layout_array
F_HOOK(void,code_layout_array)
{
	struct FeelinClass     *Class = Hook->h_Data;
	struct LocalObjectData *LOD   = F_LOD(Class,Obj);
	struct FeelinLayoutElement *rowinfo;
	struct FeelinLayoutElement *colinfo;

	uint32 hweight = 0;
	uint32 vweight = 0;
	uint32 i,j;
	int16 total_bonus_h = _area_ch - (LOD->rows - 1)    * LOD->vspacing;
	int16 total_bonus_w = _area_cw - (LOD->columns - 1) * LOD->hspacing;
	int16 array_y = _area_cy;

	_sub_declare_all;

	if (LOD->rows == 0 || LOD->columns == 0) return;
	if (LOD->members % LOD->rows)            return;
	if (LOD->members % LOD->columns)         return;

	if ((rowinfo = IFEELIN F_NewP(CUD->Pool,(LOD->rows + LOD->columns) * sizeof (struct FeelinLayoutElement))) != NULL)
	{
		colinfo = (struct FeelinLayoutElement *)((uint32)(rowinfo) + LOD->rows * sizeof (struct FeelinLayoutElement));

///Precalc Rows

		node = (FWidgetNode *) IFEELIN F_Get(LOD->family, FA_Family_Head);

		/* for each row */

		for (i = 0 ; i < LOD->rows ; i++)
		{
			rowinfo[i].min = 0;
			rowinfo[i].max = FV_Area_Max;

			j = 0;

			while (node)
			{
				if (_sub_is_showable)
				{
					rowinfo[i].min     = MAX(rowinfo[i].min, _sub_minh);
					rowinfo[i].max     = MIN(rowinfo[i].max, _sub_maxh);
					rowinfo[i].weight += _sub_weight;

					if ((++j % LOD->columns) == 0)
					{
						node = node->Next; break;
					}
				}
				node = node->Next;
			}

			rowinfo[i].max = MAX(rowinfo[i].max, rowinfo[i].min);

			// process results for this row

			total_bonus_h -= rowinfo[i].min;

			if (rowinfo[i].min != rowinfo[i].max)
			{
				vweight += rowinfo[i].weight;
			}

			#ifdef DB_LAYOUT
			IFEELIN F_Log(0,"L1_ROW(%3ld) : Min %5ld - Max %5ld Weight %ld",i,rowinfo[i].min,rowinfo[i].max,rowinfo[i].weight);
			#endif
		}
//+
///Precalc Columns

		// for each col
		for (i = 0 ; i < LOD->columns ; i++)
		{
			// min and max widths

			colinfo[i].min = 0;
			colinfo[i].max = FV_Area_Max;

			j = 0;

			for (_each)
			{
				if (_sub_is_showable)
				{
					if (((++j - 1) % LOD->columns) == i)
					{
						colinfo[i].min     = MAX(colinfo[i].min, _sub_minw);
						colinfo[i].max     = MIN(colinfo[i].max, _sub_maxw);
						colinfo[i].weight += _sub_weight;
					}
				}
			}

			colinfo[i].max = MAX(colinfo[i].max, colinfo[i].min);

			// process results for this col

			total_bonus_w -= colinfo[i].min;

			if (colinfo[i].min != colinfo[i].max)
			{
				hweight += colinfo[i].weight;
			}
					
			#ifdef DB_LAYOUT
			IFEELIN F_Log(0,"L1_COL(%3ld) : Min %5ld Max %5ld Weight %ld",i,colinfo[i].min,colinfo[i].max,colinfo[i].weight);
			#endif
		}

//+

		if (vweight == 0) {/* y = total_bonus_h / 2;*/ vweight = 1; }
		if (hweight == 0) {/* x = total_bonus_w / 2;*/ hweight = 1; }

		#ifdef DB_LAYOUT
		IFEELIN F_Log(0,"total_bonus_w %ld - total_bonus_h %ld",total_bonus_w,total_bonus_h);
		#endif

		if (total_bonus_h < 0)
		{
			IFEELIN F_Log(0,"total_bonus_h %ld - probably minmax problem",total_bonus_h);
			total_bonus_h = 0;
		}

		if (total_bonus_w < 0)
		{
			IFEELIN F_Log(0,"total_bonus_w %ld - probably minmax problem",total_bonus_w);
			total_bonus_w = 0;
		}

///Calc Row & Columns dimensions

		// calc row heights

		for (i = 0 ; i < LOD->rows ; i++)
		{
			rowinfo[i].dim = rowinfo[i].min;

			if (rowinfo[i].min != rowinfo[i].max)
			{
				uint16 bonus_h   = total_bonus_h * rowinfo[i].weight / vweight;
				rowinfo[i].dim += bonus_h;

				#ifdef DB_LAYOUT
				IFEELIN F_Log(0,"CLAMP %ld,%ld,%ld",rowinfo[i].dim,rowinfo[i].min,rowinfo[i].max);
				#endif
				 
				rowinfo[i].dim  = CLAMP(rowinfo[i].dim,rowinfo[i].min,rowinfo[i].max);
				vweight        -= rowinfo[i].weight;
				total_bonus_h  -= bonus_h;
			}

			#ifdef DB_LAYOUT
			IFEELIN F_Log(0,"L2_ROW(%3ld) - %5ld",i,rowinfo[i].dim);
			#endif
		}

		/* compute columns widths */

		for (i = 0 ; i < LOD->columns ; i++)
		{
			colinfo[i].dim = colinfo[i].min;

			if (colinfo[i].min != colinfo[i].max)
			{
				uint16 bonus_w   = total_bonus_w * colinfo[i].weight / hweight;
				colinfo[i].dim += bonus_w;
				colinfo[i].dim  = CLAMP(colinfo[i].dim, colinfo[i].min, colinfo[i].max);
				hweight        -= colinfo[i].weight;
				total_bonus_w  -= bonus_w;
			}

			#ifdef DB_LAYOUT
			IFEELIN F_Log(0,"L2_COL(%3ld) - %5ld",i,colinfo[i].dim);
			#endif
		}
//+
///Distribute space

		/*
		 * pass 2 : distribute space
		*/

		node = (FWidgetNode *) IFEELIN F_Get(LOD->family,FA_Family_Head);

		// for each row

		for (i = 0 ; i < LOD->rows ; i++)
		{
			// max height for childs in this row
			int16 array_x = _area_cx;
			uint16 rowh = rowinfo[i].dim;
			j = 0;

			// for each column

			while (node)
			{
				if (_sub_is_showable)
				{
				  /* max width for childs in this column */

					uint16 colw = colinfo[j].dim;
					uint16 cwidth;
					uint16 cheight;

					/* center child if col width is bigger than child maxwidth */

					cwidth = MIN(_sub_maxw, colw);
					cwidth = MAX(cwidth, _sub_minw);

					/* center child if row height is bigger than child maxheight */

					cheight = MIN(_sub_maxh, rowh);
					cheight = MAX(cheight, _sub_minh);

					#ifdef DB_LAYOUT
					IFEELIN F_Log(0,"%ld : %ld - %ld x %ld - %s{%08lx}", cleft, ctop, cwidth, cheight,_object_classname(sub),sub);
					#endif
								
/* FIXME-061017

Currently alignment is only handled horizontaly. The purpose  of  the  align
keyword  (start,  center,  end) was to use them indeferently with horizontal
and vertical groups, what about arrays ?

*/

					switch (_sub_align)
					{
						case FV_Widget_Align_Start:
						{
							_sub_x = array_x;
						}
						break;

						case FV_Widget_Align_End:
						{
							_sub_x = array_x + colw - cwidth;
						}
						break;

						default:
						{
							_sub_x = array_x + (colw - cwidth) / 2;
						}
						break;
					}

					_sub_y = array_y + (rowh - cheight) / 2;
					_sub_w = cwidth;
					_sub_h = cheight;
				
					IFEELIN F_Do(node->Widget, FM_Area_Layout);

					array_x += LOD->hspacing + colw;

					if ((++j % LOD->columns) == 0)
					{
						node = node->Next; break;
					}
				}
				node = node->Next;
			}

			#ifdef DB_LAYOUT
			IFEELIN F_Log(0,"ARRAY_Y %ld >>> %ld (ROWH %ld)",array_y,array_y + LOD->vspacing + rowh,rowh);
			#endif

			array_y += LOD->vspacing + rowh;
		}
//+

		IFEELIN F_Dispose(rowinfo);
	}
	
	#ifdef DB_LAYOUT
	IFEELIN F_Log(0,"DONE");
	#endif
}
//+

#undef _each
#define _each                                   node = head ; node ; node  = node->Next

///code_layout_horizontal
F_HOOK(void,code_layout_horizontal)
{
	struct FeelinClass *Class = Hook->h_Data;
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *head;
	uint16 free_w = _area_cw;
	uint16 free_h = _area_ch;
	
	uint16 pad=0;
	uint32 total_widget_weight=0;
	uint32 count=0;
	
	_sub_declare_all;

	if ((head = (FWidgetNode *) IFEELIN F_Get(LOD->family,FA_Family_Head)) == NULL)
	{
		return;
	}

	for (_each)
	{
		if (_sub_is_showable)
		{
			uint16 h = _sub_fixh ? _sub_minh : ((_sub_maxh < free_h) ? _sub_maxh : free_h);

			_sub_h = h;
		
			count++;

			if (_sub_fixw)
			{
				_sub_w = _sub_minw;
				
				free_w -= _sub_w;
			}
			else
			{
				_sub_set_compute_w;

				if (_group_is_relsizing)
				{
					_sub_weight = _sub_minw;
				}
				
				total_widget_weight += _sub_weight;
			}
		}
	}

	free_w -= (count - 1) * LOD->hspacing;

	if (free_w)
	{
		
/* 2.1 : D'abord on va s'assurer  que  la  taille  minimale  et  la  taille
maximale des Objets est bien respectée. */
 
		node = head;
 
		while (node)
		{
			if (_sub_is_compute_w)
			{
				uint16 size = free_w * _sub_weight / total_widget_weight;

				if (_sub_minw > size)
				{
					_sub_w = _sub_minw;
					
					_sub_clear_compute_w;
				}
				else if (_sub_maxw < size)
				{
					_sub_w = _sub_maxw;
					 
					_sub_clear_compute_w;
				}
				else
				{
					node = node->Next; continue;
				}

				free_w -= _sub_w; total_widget_weight -= _sub_weight;

				node = head;
			}
			else
			{
				node = node->Next;
			}
		}

/* 2.2: A présent on s'occupe du reste. */

		for (_each)
		{
			if (_sub_is_compute_w)
			{
				uint16 w = free_w * _sub_weight / total_widget_weight;
				
				_sub_w = w;
				_sub_clear_compute_w;
				
				free_w -= _sub_w;
				total_widget_weight -= _sub_weight;
			}
		}

		if (free_w)
		{
			FWidgetNode *adjust_node=NULL;
			uint16 used_w=0;

			free_w = _area_cw - (count - 1) * LOD->hspacing;

			for (_each)
			{
				if (_sub_is_showable)
				{
					used_w += _sub_w;

					if (_sub_fixw == FALSE) adjust_node = node;
				}
			}

			pad = free_w - used_w;

			if (adjust_node)
			{
				adjust_node->AreaPublic->Box.w += pad; pad = 0;
			}
		}
	}

	{
		uint16 x = _area_cx;

		for (_each)
		{
			if (_sub_is_showable)
			{
				int16 y;

				switch (_sub_align)
				{
					case FV_Widget_Align_Start:
					{
						y = _area_cy;
					}
					break;

					case FV_Widget_Align_End:
					{
						y = _area_cy2 - _sub_h + 1;
					}
					break;

					default:
					{
						y = _area_cy + ((_sub_h < free_h) ? (free_h - _sub_h) / 2 : 0);
					}
					break;
				};

				#if 1 //debug

				if (_sub_w > FV_Area_Max)
				{
					IFEELIN F_Log(0, "vertical width (%ld) of %s{%08lx} is too big !", _sub_w, _object_classname(node->Widget), node->Widget);
				}

				#endif

				_sub_y = y;
				_sub_x = x;

				x += _sub_w + LOD->hspacing + pad; pad = 0;

				IFEELIN F_Do(node->Widget, FM_Area_Layout);
			}
		}
	}
}
//+
///code_layout_vertical
F_HOOK(void,code_layout_vertical)
{
	struct FeelinClass *Class = Hook->h_Data;
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	FWidgetNode *head = (FWidgetNode *) IFEELIN F_Get(LOD->family,FA_Family_Head);
	uint16 free_w = _area_cw;
	uint16 free_h = _area_ch;

	uint16 pad=0;
	uint32 total_widget_weight=0;
	uint32 count=0;

	_sub_declare_all;

	if (head == NULL)
	{
		return;
	}

	#if 0
	IFEELIN F_Log(0, "\n\n*** vertical layout, box (%ld x %ld)\n", _area_cw, _area_ch);
	#endif

	for (_each)
	{
		if (_sub_is_showable)
		{
			uint16 w = _sub_fixw ? _sub_minw : ((_sub_maxw < free_w) ? _sub_maxw : free_w);

			_sub_w = w;

			count++;

			#if 0

			IFEELIN F_Log(0, "%16.16s{%08lx} box (%ld x %ld) min (%ld x %ld) max (%ld x %ld)",

				_object_classname(node->Widget),
				node->Widget,
				_sub_w,
				_sub_h,
				_sub_minw,
				_sub_minh,
				_sub_maxw,
				_sub_maxh);

			#endif

			if (_sub_fixh)
			{
				_sub_h = _sub_minh;

				free_h -= _sub_h;
			}
			else
			{
				_sub_set_compute_h;

				if ((FF_GROUP_RELSIZING & LOD->flags) != 0)
				{
					_sub_weight = _sub_minh;
				}

				total_widget_weight += _sub_weight;
			}

			#if 0

			IFEELIN F_Log(0, "%16.16s{%08lx} box (%ld x %ld) min (%ld x %ld) max (%ld x %ld)",

				_object_classname(node->Widget),
				node->Widget,
				_sub_w,
				_sub_h,
				_sub_minw,
				_sub_minh,
				_sub_maxw,
				_sub_maxh);

			#endif
		}
	}

	free_h -= (count - 1) * LOD->vspacing;

	/* 2.1 : D'abord on va s'assurer que la taille  minimale  et  la  taille
	maximale des Objets est bien respectée. */

	if (free_h != 0)
	{
		for (node = head ; node ; )
		{
			if (_sub_is_compute_h)
			{
				uint16 size = free_h * _sub_weight / total_widget_weight;

				if (_sub_minh > size)
				{
					_sub_h = _sub_minh;
					_sub_clear_compute_h;
				}
				else if (_sub_maxh < size)
				{
					_sub_h = _sub_maxh;
					_sub_clear_compute_h;
				}
				else
				{
					node = node->Next;

					continue;
				}

				free_h -= _sub_h;
				total_widget_weight -= _sub_weight;

				node = head;
			}
			else
			{
				node = node->Next;
			}
		}

/* 2.2: A présent on s'occupe du reste. */

		for (_each)
		{
			if (_sub_is_compute_h)
			{
				_sub_h = free_h * _sub_weight / total_widget_weight;
				_sub_clear_compute_h;

				free_h -= _sub_h;
				total_widget_weight -= _sub_weight;
			}
		}

		if (free_h)
		{
			FWidgetNode *adjust_node=NULL;
			uint16 used_h=0;

			free_h = _area_ch - (count - 1) * LOD->vspacing;

			for (_each)
			{
				if (_sub_is_showable)
				{
					used_h += _sub_h;

					if (_sub_fixh == FALSE) adjust_node = node;
				}
			}

			pad = free_h - used_h;

			if (adjust_node)
			{
				adjust_node->AreaPublic->Box.h += pad; pad = 0;
			}
		}
	}

	{
		uint16 y = _area_cy;

		for (_each)
		{
			if (_sub_is_showable)
			{
				int16 x;

				switch (_sub_align)
				{
					case FV_Widget_Align_Start:
					{
						x = _area_cx;
					}
					break;

					case FV_Widget_Align_End:
					{
						x = _area_cx2 - _sub_w + 1;
					}
					break;

					default:
					{
						x = _area_cx + ((_sub_w < free_w) ? (free_w - _sub_w) / 2 : 0);
					}
					break;
				};

				#if 1 //debug

				if (_sub_h > FV_Area_Max)
				{
					IFEELIN F_Log(0, "vertical height (%ld) of %s{%08lx} is too big !", _sub_h, _object_classname(node->Widget), node->Widget);
				}

				#endif

				_sub_y = y;
				_sub_x = x;

				y += _sub_h + LOD->vspacing + pad; pad = 0;

				IFEELIN F_Do(node->Widget, FM_Area_Layout);
			}
		}
	}
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Group_Layout
F_METHOD(bool32,Group_Layout)
{
	struct LocalObjectData *LOD = F_LOD(Class, Obj);

	FObject head = (FWidgetNode *) IFEELIN F_Get(LOD->family, FA_Family_Head);

	FWidgetNode *node;

	if (F_SUPERDO() == FALSE)
	{
		return FALSE;
	}

	if (head == NULL)
	{
		return TRUE;
	}

	#ifdef F_GROUP_BUFFERED_REGIONS_ENABLED

/** dispose previous clear region **************************************************************/

	if (LOD->region != NULL)
	{
		IGRAPHICS DisposeRegion(LOD->region);

		LOD->region = NULL;
	}

	#endif

/** save previous box ***************************************************************************

	To avoid the overhead of checking box modifications while I'm  computing
	new box values, old box values are saved and compared once the layout is
	done, the 'damaged' flag accordingly.

	Note that we *never* clear the FF_Area_Damaged flag, which can *only* be
	cleared by the Area class, once the object has been drawn.

**/

	for (_each)
	{
		_sub_previous_x = _sub_x;
		_sub_previous_y = _sub_y;
		_sub_previous_w = _sub_w;
		_sub_previous_h = _sub_h;
	}

/** call the layout function *******************************************************************/
 
	if (LOD->LayoutHook != NULL)
	{
		if (LOD->virtual_data != NULL)
		{
///
			FWidgetNode *node;
			
			uint16 w = _area_w;
			uint16 h = _area_h;
			
			if (w < LOD->virtual_data->real_minw)
			{
				_area_w = LOD->virtual_data->real_minw;
			}

			if (h < LOD->virtual_data->real_minh)
			{
				_area_h = LOD->virtual_data->real_minh;
			}

			IUTILITY CallHookPkt(LOD->LayoutHook,Obj,Msg);
			
			_area_w = w;
			_area_h = h;
 
			for (_each)
			{
				if (_sub_x < _area_cx || _sub_x > _area_cx2 ||
					_sub_y < _area_cy || _sub_y > _area_cy2)
				{
					IFEELIN F_Log(0,"%s{%08lx} is not visible", _object_classname(node->Widget), node->Widget);
				}
				else
				{
					if (_sub_w > _area_cw)
					{
						/*
						IFEELIN F_Log(0,"%s{%08lx} trunc w %ld (%ld)",
							_object_classname(node->Widget), node->Widget, _area_cw,
							_sub_visible_w);*/
/*
						if (_sub_visible_w != _area_cw)
						{
							_sub_visible_w = _area_cw;
							
							_sub_set_damaged;
						}
*/
					}
				
					#if 0
					if (_sub_h > _area_ch)
					{
						IFEELIN F_Log(0,"%s{%08lx} trunc h %ld (%ld)",
							_object_classname(node->Widget), node->Widget, _area_ch,
							_sub_visible_h);

						if (_sub_visible_h != _area_ch)
						{
							_sub_visible_h = _area_ch;

							_sub_set_damaged;
						}
					} 
					#endif
				}
 
				//_sub_set_damaged;
			}
//+
		}
		else
		{
			IUTILITY CallHookPkt(LOD->LayoutHook, Obj, Msg);
		}
	}
	else
	{
		IFEELIN F_Log(0,"LayoutHook is NULL, please report");
	}
	
	#if 0
	if (LOD->virtual_data)
	{
		for (_each)
		{
			if (_sub_is_showable)
			{
				if (group_is_object_visible(Class, Obj, node))
				{
					IFEELIN F_Do(node->Widget, FM_Area_Show);
				}
				else
				{
					IFEELIN F_Log(0,"%s{%08lx} is not visible",_object_classname(node->Widget),node->Widget);
					
					IFEELIN F_Do(node->Widget, FM_Area_Hide);
				}
			}
		}
	}
	#endif

/** check modified boxes ************************************************************************

	The FF_Area_Damaged flags is used  to  avoid  unnecessary  redrawing  of
	objects,  mostly  after  a new layout is computed. Here, we check if the
	object's  box  was  modified,  and   set   the   FF_Area_Damaged   flags
	accordingly.

**/
 
	for (_each)
	{
		if ((_sub_x != _sub_previous_x) ||
			(_sub_y != _sub_previous_y) ||
			(_sub_w != _sub_previous_w) ||
			(_sub_h != _sub_previous_h))
		{
			_sub_set_damaged;

			#ifdef DB_DAMAGED
			IFEELIN F_Log(0,"%s{%08lx} is damaged (%ld : %ld, %ld x %ld)", _object_classname(node->Widget), node->Widget, _sub_x, _sub_y, _sub_w, _sub_h);
			#endif
		}
		#ifdef DB_NOT_DAMAGED
		else
		{
			IFEELIN F_Log(0,"%s{%08lx} no damage (%ld : %ld, %ld x %ld)",_object_classname(node->Widget),node->Widget,_sub_x,_sub_y,_sub_w,_sub_h);

		}
		#endif
	}

	return TRUE;
}
//+
