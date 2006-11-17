#include "Private.h"

//#define DB_ASKMINMAX_ARRAY
//#define DB_ASKMINMAX_MAIN

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///AskMinMax_Simple
STATIC void AskMinMax_Simple(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	uint16 w=0;
	uint16 h=0;
	uint16 omw;
	uint16 omh;
	uint16 maxw=0;
	uint16 maxh=0;
	uint32 nObj=0;

	_sub_declare_all;

	for (_each)
	{
		if (_sub_is_showable)
		{
			nObj++;

			omw = _sub_minw;
			omh = _sub_minh;

			if (_area_is_horizontal)
			{
				h = MAX(h,omh); w = (FF_GROUP_SAMEWIDTH & LOD->flags) ? MAX(w,omw) : w + omw;
				if (maxw < 10000) maxw += _sub_maxw; maxh = MAX(maxh,_sub_maxh);
			}
			else
			{
				if (maxh < 10000) maxh += _sub_maxh; maxw = MAX(maxw,_sub_maxw);
				w = MAX(w,omw); h = (FF_GROUP_SAMEHEIGHT & LOD->flags) ? MAX(h,omh) : h + omh;
			}
		}
	}

/* 3: Fix & Set values */

	if (nObj != 0)
	{
		if (_area_is_horizontal)
		{
			_area_maxw  = (nObj - 1) * LOD->hspacing + _area_minw + maxw;
			_area_minw  = (nObj - 1) * LOD->hspacing + _area_minw + ((FF_GROUP_SAMEWIDTH  & LOD->flags) ? w * nObj : w);
			_area_maxh  = _area_minh + maxh;
			_area_minh += h;
		}
		else
		{
			_area_maxw  = _area_minw + maxw;
			_area_minw += w;
			_area_maxh  = (nObj - 1) * LOD->vspacing + _area_minh + maxh;
			_area_minh  = (nObj - 1) * LOD->vspacing + _area_minh + ((FF_GROUP_SAMEHEIGHT & LOD->flags) ? h * nObj : h);
		}
	}
	else
	{
		_area_maxw = 0; _area_minw = 0;
		_area_maxh = 0; _area_minh = 0;
	}

	if (_area_maxw > FV_Area_Max) _area_maxw = FV_Area_Max;
	if (_area_maxh > FV_Area_Max) _area_maxh = FV_Area_Max;
}
//+
///AskMinMax_Array
STATIC void AskMinMax_Array(FClass *Class, FObject Obj)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);
	uint16 i,j;
	uint16 samw=0,samh=0;

	_sub_declare_all;

	for (_each)
	{
		if (_sub_is_showable)
		{
			if ((FF_GROUP_SAMEWIDTH | FF_GROUP_SAMEHEIGHT) & LOD->flags)
			{
				samw = MAX(samw, _sub_minw);
				samh = MAX(samh, _sub_minh);
			}
		}
	}

	if (_group_is_columns)
	{
		if ((LOD->members % LOD->columns) != 0)
		{
			IFEELIN F_Log(0, "(%ld) Members are not even with (%ld) Columns", LOD->members, LOD->columns);

			LOD->columns = 1;
			LOD->rows = LOD->members;
		}
		else
		{
			LOD->rows = LOD->members / LOD->columns;
		}
	}
	else
	{
		if ((LOD->members % LOD->rows) != 0)
		{
			IFEELIN F_Log(0, "(%ld) Members are not even with (%ld) Rows", LOD->members, LOD->rows);

			LOD->rows = 1;
			LOD->columns = LOD->members;
		}
		else
		{
			LOD->columns = LOD->members / LOD->rows;
		}
	}

	_area_minw = _area_maxw = (LOD->columns - 1) * LOD->hspacing;
	_area_minh = _area_maxh = (LOD->rows    - 1) * LOD->vspacing;

	#ifdef DB_ASKMINMAX_ARRAY
	IFEELIN F_Log(0,"\nMIN_MAX_W %ld - MIN_MAX_H %ld",_area_minw,_area_minh);
	#endif

/*** Compute MinW & MinH if Group has FA_Group_SameXxx *****************************************/

	j = 0;

	node = (FWidgetNode *) IFEELIN F_Get(LOD->family, FA_Family_Head);

	for (i = 0 ; i < LOD->rows ; i++)
	{
		uint16 minh = 0;
		uint16 maxh = FV_Area_Max;

		while (node)
		{
			if (_sub_is_showable)
			{
				if (FF_GROUP_SAMEHEIGHT & LOD->flags)
				{
					_sub_minh = MIN(samh, _sub_maxh);
				}

				#ifdef DB_ASKMINMAX_ARRAY
				IFEELIN F_Log(0,"Min %5ld x %5ld - Max %5ld x %5ld - %s{%08lx}", _sub_minw, _sub_minh, _sub_maxw, _sub_maxh, _object_classname(node->Widget), node->Widget);
				#endif

				minh = MAX(minh,_sub_minh);
				maxh = MIN(maxh,_sub_maxh);

				node = node->Next;

				if ((++j % LOD->columns) == 0)
				{
					#ifdef DB_ASKMINMAX_ARRAY
					IFEELIN F_Log(0,"ROW %ld END - MinH %ld - MaxH %ld",i,minh,maxh);
					#endif

					_area_minh += minh;
					_area_maxh += MAX(minh,maxh);

					break;
				}
			}
			else
			{
				node = node->Next;
			}
		}
	}

	j = 0;

	for (i = 0 ; i < LOD->columns ; i++)
	{
		uint16 minw = 0;
		uint16 maxw = FV_Area_Max;

		for (_each)
		{
			if (_sub_is_showable)
			{
				if (((++j - 1) % LOD->columns) == i)
				{
					if (FF_GROUP_SAMEWIDTH & LOD->flags)
					{
						_sub_minw = MIN(samw,_sub_maxw);
					}

					minw = MAX(minw,_sub_minw);
					maxw = MIN(maxw,_sub_maxw);

					#ifdef DB_ASKMINMAX_ARRAY
					IFEELIN F_Log(0,"(%ld,%ld) SUB 0x%08lx - MinW %5ld (%5ld) MaxW %5ld (%5ld)",i,j,node->Widget,_sub_minw,minw,_sub_maxw,maxw);
					#endif
				}
			}
		}
		maxw = MAX(maxw, minw); _area_minw += minw; _area_maxw += maxw;

		#ifdef DB_ASKMINMAX_ARRAY
		IFEELIN F_Log(0,"COL(%2ld) - MinW %5ld - MaxW %5ld",i,minw,maxw);
		#endif
	}

	#ifdef DB_ASKMINMAX_ARRAY
	IFEELIN F_Log(0,">>> MIN %6ld x %6ld - MAX %6ld x %6ld\n",_area_minh,_area_minw,_area_maxh,_area_maxw);
	#endif
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///Group_AskMinMax
F_METHOD(uint32,Group_AskMinMax)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	_sub_declare_all;

//	  LOD->members = 0;

	/* ask min & max to all children */

	for (_each)
	{

		#warning virtual

/*
		* strange to see virtual related code here, but it's the best place
		I  found.  Because  a  group  can  be  virtualized  at  anytime, the
		FM_AskMinMax method will always be called the rethink the layout. *
 
		if (_widget_is_virtualized && _widget_is_virtual)
		{
			_sub_set_virtualized;
		}
		else
		{
			_sub_clear_virtualized;
		}
*/
		
		_sub_minw = _sub_minh = 0;
		_sub_maxw = _sub_maxh = FV_Area_Max;
		
		IFEELIN F_Do(node->Widget, FM_Area_AskMinMax);
		
//		  LOD->members++;
	}

	/* call appropriate minmax function */

	if (LOD->MinMaxHook != NULL)
	{
		IUTILITY CallHookPkt(LOD->MinMaxHook, Obj, Msg);
	}
	else if ((LOD->rows > 1) || (LOD->columns > 1))
	{
		AskMinMax_Array(Class,Obj);
	}
	else
	{
		AskMinMax_Simple(Class,Obj);
	}

	#ifdef DB_ASKMINMAX_MAIN
	{
		for (_each)
		{
			IFEELIN F_Log(0," %s{%08lx} min (%ld x %ld) max (%ld x %ld)", _object_classname(node->Widget), node->Widget, _sub_minw, _sub_minh, _sub_maxw, _sub_maxh);
		}
	}
	#endif

	return F_SUPERDO();

/*
	if (LOD->virtual_data != NULL)
	{
		LOD->virtual_data->real_minw = _area_minw;
		LOD->virtual_data->real_minh = _area_minh;
				
		_area_minw = 0;
		_area_minh = 0;
		
		F_SUPERDO();
		
		LOD->virtual_data->real_minw += _area_minw;
		LOD->virtual_data->real_minh += _area_minh;
 
		_area_minw = MAX(MIN(20, _area_maxw), _area_minw);
		_area_minh = MAX(MIN(20, _area_maxh), _area_minh);
		
		#ifdef DB_ASKMINMAX_MAIN
		IFEELIN F_Log(0,"min (%ld x %ld) real (%ld x %ld)",_area_minw, _area_minh, LOD->virtual_data->real_minw, LOD->virtual_data->real_minh);
		#endif
	}
	else
	{
 
		* FC_Area will finish everything now,  taking  care  of  FA_MinXxx,
		FA_MaxXxx and FA_FixedXxx *

		return F_SUPERDO();
	}
*/
}
//+

