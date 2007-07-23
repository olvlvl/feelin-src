#include "Private.h"
#include "processing.h"

///BitMap_AddProcessing
F_METHODM(bool32,BitMap_AddProcessing,FS_BitMap_AddProcessing)
{
	struct LocalObjectData *LOD = F_LOD(Class,Obj);

	struct in_ProcessingNode *node = NULL;

	switch (Msg->ProcMethod)
	{
		///convert
		case FV_BitMap_AddProcessing_Convert:
		{
			node = IFEELIN F_New(sizeof (struct in_ProcessingNode) + sizeof (struct in_Processing_Convert));

			if (node)
			{
				struct in_Processing_Convert *data = (struct in_Processing_Convert *) ((uint32)(node) + sizeof (struct in_ProcessingNode));

				node->func = (feelin_processing_func) bitmap_processing_convert;

				data->pixel_size = _msg_convert_pixelsize;
			}
		}
		break;
		///+

		///crop
		case FV_BitMap_AddProcessing_Crop:
		{
			node = IFEELIN F_New(sizeof (struct in_ProcessingNode) + sizeof (struct in_Processing_Crop));

			if (node)
			{
				struct in_Processing_Crop *data = (struct in_Processing_Crop *) ((uint32)(node) + sizeof (struct in_ProcessingNode));

				node->func = (feelin_processing_func) bitmap_processing_crop;

				data->x = _msg_crop_left;
				data->y = _msg_crop_top;
				data->w = _msg_crop_width;
				data->h = _msg_crop_height;
			}
		}
		break;
		///+
		///tint
		case FV_BitMap_AddProcessing_Tint:
		{
			node = IFEELIN F_New(sizeof (struct in_ProcessingNode) + sizeof (struct in_Processing_Tint));

			if (node)
			{
				struct in_Processing_Tint *data = (struct in_Processing_Tint *) ((uint32)(node) + sizeof (struct in_ProcessingNode));
				struct TagItem *tags = (struct TagItem *)((uint32)(Msg) + sizeof (struct FS_BitMap_AddProcessing_Tint));
				struct TagItem item;

				node->func = (feelin_processing_func) bitmap_processing_tint;

				data->x = 0;
				data->y = 0;
				data->w = 0xFFFF;
				data->h = 0xFFFF;

				data->rgb = _msg_tint_rgb;
				data->ratio = 128;

				while (IFEELIN F_DynamicNTI(&tags,&item,Class))
				switch (item.ti_Tag)
				{
					case FA_BitMap_AddProcessing_Left:
					{
						data->x = item.ti_Data;
					}
					break;

					case FA_BitMap_AddProcessing_Top:
					{
						data->y = item.ti_Data;
					}
					break;

					case FA_BitMap_AddProcessing_Width:
					{
						data->w = item.ti_Data;
					}
					break;

					case FA_BitMap_AddProcessing_Height:
					{
						data->h = item.ti_Data;
					}
					break;

					case FA_BitMap_AddProcessing_TintRatio:
					{
						data->ratio = item.ti_Data;
					}
					break;
				}
			}
		}
		break;
		//+
		///radial
		case FV_BitMap_AddProcessing_Radial:
		{
			node = IFEELIN F_New(sizeof (struct in_ProcessingNode) + sizeof (struct in_Processing_Radial));

			if (node)
			{
				//struct in_Processing_Radial *data = (struct in_Processing_Radial *) ((uint32)(node) + sizeof (struct in_ProcessingNode));

				node->func = (feelin_processing_func) bitmap_processing_radial;
			}
		}
		break;
		//+

		///scale
		case FV_BitMap_AddProcessing_Scale:
		{
			node = IFEELIN F_New(sizeof (struct in_ProcessingNode) + sizeof (struct in_Processing_Scale));

			if (node)
			{
				struct in_Processing_Scale *data = (struct in_Processing_Scale *) ((uint32)(node) + sizeof (struct in_ProcessingNode));

				node->func = (feelin_processing_func) bitmap_processing_scale;

				data->Width = _msg_scale_width;
				data->Height = _msg_scale_height;
			}
		}
		break;
		//+

		///blur
		case FV_BitMap_AddProcessing_Blur:
		{
			node = IFEELIN F_New(sizeof (struct in_ProcessingNode) + sizeof (struct in_Processing_Blur));

			if (node)
			{
				//struct in_Processing_Blur *data = (struct in_Processing_Blur *) ((uint32)(node) + sizeof (struct in_ProcessingNode));

				node->func = (feelin_processing_func) bitmap_processing_blur;
			}
		}
		break;
		///+
	}

	if (node)
	{
		IFEELIN F_LinkTail(&LOD->processing, (FNode *) node);

		return TRUE;
	}

	return FALSE;
}
//+
///BitMap_RemProcessing
F_METHOD(uint32,BitMap_RemProcessing)
{
	IFEELIN F_Log(0,"not implemented");

	return 0;
}
//+

