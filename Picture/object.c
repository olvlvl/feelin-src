#include "Private.h"

//#define DB_OPEN
//#define DB_CLOSE

/************************************************************************************************
*** Private *************************************************************************************
************************************************************************************************/

///png_sig_cmp
int32 png_sig_cmp(uint8 *sig, uint32 start, uint32 num_to_check)
{
	uint8 png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

	if (num_to_check > 8)
	{
		num_to_check = 8;
	}
	else if (num_to_check < 1)
	{
		return (0);
	}

	if (start > 7)
	{
		return (0);
	}

	if (start + num_to_check > 8)
	{
		num_to_check = 8 - start;
	}

	return IFEELIN F_StrCmp(&sig[start], &png_signature[start], num_to_check);
}
//+

/************************************************************************************************
*** Methods *************************************************************************************
************************************************************************************************/

///picture_create_with_core_png
uint32 picture_create_with_core_png(FClass *Class, FObject Obj, uint32 Method, APTR Msg, STRPTR Name)
{
	FObject png = CorePNGObject,

		"Source", Name,
		"SourceType", FV_CoreMedia_SourceType_File,

		End;

	if (png)
	{
		uint32 rc = IFEELIN F_Do(Obj, F_RESOLVED_ID(STEAL), png);

		IFEELIN F_DisposeObj(png);

		if (rc)
		{
			return F_SUPERDO();
		}
	}
	else
	{
		IFEELIN F_Log(FV_LOG_USER, "Unable to decore picture (%s) with CorePNG", Name);
	}

	return 0;
}
//+
///picture_create_with_datatypes
uint32 picture_create_with_datatypes(FClass *Class, FObject Obj, uint32 Method, APTR Msg, STRPTR Name)
{
	int32 rc = FALSE;

	APTR dt = IDATATYPES NewDTObject(Name,

		DTA_SourceType, DTST_FILE,
		DTA_GroupID, GID_PICTURE,
		PDTA_Remap, FALSE,
		PDTA_DestMode, PMODE_V43,

		TAG_DONE);

	if (dt)
	{
		struct BitMapHeader *header=NULL;
		struct ColorRegister *color_array=NULL;

		uint16  bitmap_width            = 0;
		uint16  bitmap_height           = 0;
		uint8   bitmap_depth            = 0;

		uint8   bitmap_pixel_size       = 0;
		APTR    bitmap_pixel_array      = NULL;
		uint16  bitmap_pixel_array_mod  = 0;

		bits8   bitmap_color_type       = 0;
		uint32  bitmap_color_count      = 0;
		uint32 *bitmap_color_array      = NULL;

		IDATATYPES GetDTAttrs(dt,

			PDTA_BitMapHeader, (uint32) &header,
			PDTA_ColorRegisters, (uint32) &color_array,
			PDTA_NumColors, (uint32) &bitmap_color_count,

			TAG_DONE);

		if (header)
		{
			bitmap_width = header->bmh_Width;
			bitmap_height = header->bmh_Height;
			bitmap_depth = header->bmh_Depth;

			// colors

			bitmap_color_type = (bitmap_depth > 8) ? FV_COLOR_TYPE_RGB : FV_COLOR_TYPE_PALETTE;

			if ((color_array != NULL) && (bitmap_color_count != 0))
			{
				#ifdef DB_OPEN
				F_Log(0,"creating FV_COLOR_TYPE_PALETTE (0x%08lx)(%ld)", color_array, bitmap_color_count);
				#endif

				bitmap_color_array = IFEELIN F_New(bitmap_color_count * sizeof (uint32));

				if (bitmap_color_array)
				{
					uint32 *target = bitmap_color_array;
					uint32 transp_index = (header->bmh_Masking == mskHasTransparentColor) ? header->bmh_Transparent : 0xFFFFFFFF;
					uint32 i;

					#ifdef DB_OPEN
					IFEELIN F_Log(0,"transp_index %ld",transp_index);
					#endif

					if (transp_index != 0xFFFFFFFF)
					{
						bitmap_color_type |= FF_COLOR_TYPE_ALPHA;
					}

					for (i = 0 ; i < bitmap_color_count ; i++, color_array++)
					{
						uint32 rgba;

						rgba = color_array->red;
						rgba <<= 8;

						rgba |= color_array->green;
						rgba <<= 8;

						rgba |= color_array->blue;
						rgba <<= 8;

						if (i == transp_index)
						{
							rgba |= 0x00;
						}
						else
						{
							rgba |= 0xFF;
						}

						#ifdef DB_OPEN
						IFEELIN F_Log(0,"col(%03ld) 0x%08lx",i, rgba);
						#endif

						*target++ = rgba;
					}
				}
			}

			// pixels

			if ((bitmap_depth > 8) ||
				((bitmap_depth < 9) && (bitmap_color_array != NULL)))
			{
				bitmap_pixel_size       = (bitmap_depth > 8) ? 3 : 1;
				bitmap_pixel_array_mod  = bitmap_pixel_size * bitmap_width;
				bitmap_pixel_array      = IFEELIN F_New(bitmap_pixel_array_mod * bitmap_height);
			}

			// copy

			if (bitmap_pixel_array)
			{
				#ifdef __amigaos4__
				IINTUITION IDoMethod
				#else
				DoMethod
				#endif
				(
					dt, PDTM_READPIXELARRAY,

					bitmap_pixel_array,

					(bitmap_depth > 8) ? PBPAFMT_RGB : PBPAFMT_LUT8,

					bitmap_pixel_array_mod,
					0,0, bitmap_width, bitmap_height
				);

				rc = TRUE;
			}
			else
			{
				IFEELIN F_Log(FV_LOG_USER,"Unable to allocate PixelArray for (%s)", Name);
			}
		}

		IDATATYPES DisposeDTObject(dt);

		if (rc)
		{
			return IFEELIN F_SuperDo(Class,Obj,Method,

				F_RESOLVED_ID(WIDTH),         bitmap_width,
				F_RESOLVED_ID(HEIGHT),        bitmap_height,
				F_RESOLVED_ID(DEPTH),         bitmap_depth,

				F_RESOLVED_ID(PIXELSIZE),     bitmap_pixel_size,
				F_RESOLVED_ID(PIXELARRAY),    bitmap_pixel_array,
				F_RESOLVED_ID(PIXELARRAYMOD), bitmap_pixel_array_mod,

				F_RESOLVED_ID(COLORTYPE),     bitmap_color_type,
				F_RESOLVED_ID(COLORARRAY),    bitmap_color_array,
				F_RESOLVED_ID(COLORCOUNT),    bitmap_color_count,

				TAG_MORE, Msg);
		}
	}
	else
	{
		IFEELIN F_Log(FV_LOG_USER,"Unable to open \"%s\" through DataTypes", Name);
	}

	return 0;
}
//+

///Picture_New
F_METHOD(uint32,Picture_New)
{
	struct TagItem *Tags = Msg,item;

	STRPTR name = NULL;

	while (IFEELIN F_DynamicNTI(&Tags,&item,Class))
	switch (item.ti_Tag)
	{
		case FV_ATTRIBUTE_NAME:
		{
			name = (STRPTR) item.ti_Data;
		}
		break;
	}

	if (name)
	{
		BPTR lock = IDOS_ Lock(name, ACCESS_READ);

		if (lock)
		{
			bool32 use_core_png = FALSE;

			BPTR file = IDOS_ Open(name, MODE_OLDFILE);

			if (file)
			{
				uint8 signature[8];

				IDOS_ Read(file, signature, 8);

				if (png_sig_cmp(signature, 0, 8) == 0)
				{
					use_core_png = TRUE;
				}

				IDOS_ Close(file);
			}

			IDOS_ UnLock(lock);

			if (use_core_png)
			{
				return picture_create_with_core_png(Class, Obj, Method, Msg, name);
			}
			else if (DataTypesBase)
			{
				return picture_create_with_datatypes(Class, Obj, Method, Msg, name);
			}
		}
	}

	return NULL;
}
//+
