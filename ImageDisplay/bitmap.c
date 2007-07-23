#include "Private.h"

#include "Feelin:Sources/BitMap/processing.h"

//#define DB_CREATE

///typedef

struct in_Shared
{
	FNode							node;

	FAtom 						   *path;
	FObject							picture;

	uint32							refs;
	
	uint32                          width; // *must* be uint32
	uint32                          height; // *must* be uint32
	bits32							color_type;
};

typedef struct in_Image_BitMap
{
	struct in_Shared			   *shared;

	APTR							background;
	struct in_CodeTable            *background_methods;

	uint8                          	repeat;
	bool8							multiple;

	uint16							position_x;
	uint16							position_y;
	bits32						    position_x_type;
	bits32							position_y_type;

	FObject                         rendered;
	uint32                          rendered_width;
	uint32                          rendered_height;
}
FImage;

//+

/************************************************************************************************
*** shared pictures *****************************************************************************
************************************************************************************************/

///id_obtain_picture

/* the function is not thread safe, you must use an arbiter */

struct in_Shared *id_obtain_picture(FClass *Class, STRPTR URL)
{
	BPTR lock;
	STRPTR buf;
	FAtom *atom;
	struct in_Shared *shared;

/*** lock the file *****************************************************************************/

 	lock = IDOS_ Lock(URL, SHARED_LOCK);

//	  IFEELIN F_Log(0, "lock (0x%08lx)", lock);

	if (lock == NULL)
	{
		IFEELIN F_Log(FV_LOG_USER, "Unable to lock (%s)", URL);

		return NULL;
	}

/*** create name buffer ************************************************************************/

	buf = IFEELIN F_NewP(CUD->pool, 1024);

//	  IFEELIN F_Log(0, "buf (0x%08lx)", buf);

	if (buf == NULL)
	{
		IDOS_ UnLock(lock);

		return NULL;
	}

	IDOS_ NameFromLock(lock, buf, 1024);

//	  IFEELIN F_Log(0, "name (%s)", buf);

	IDOS_ UnLock(lock);

/*** obtain atom *******************************************************************************/

	atom = IFEELIN F_AtomObtain(buf, ALL);

//	  IFEELIN F_Log(0, "atom (0x%08lx)", atom);

	IFEELIN F_Dispose(buf);

	if (atom == NULL)
	{
		return NULL;
	}

/*** search match ******************************************************************************/

	for (shared = (struct in_Shared *) CUD->shared_bitmap_list.Head ; shared ; shared = (struct in_Shared *) shared->node.Next)
	{
		if (shared->path == atom)
		{
			break;
		}
	}

//	  IFEELIN F_Log(0, "result: (0x%08lx)", shared);

	if (shared != NULL)
	{
		IFEELIN F_AtomRelease(atom);

		shared->refs++;

//		  IFEELIN F_Log(0, "refs (%ld)", shared->refs);

		return shared;
	}

/*** create shared picture *********************************************************************/

	shared = IFEELIN F_NewP(CUD->pool, sizeof (struct in_Shared));

//	  IFEELIN F_Log(0, "shared (0x%08lx)", shared);

	if (shared == NULL)
	{
		return NULL;
	}

/*** create picture ****************************************************************************/

	shared->picture = PictureObject,

		"FA_Picture_Name", atom->Key,

		End;

//	  IFEELIN F_Log(0, "picture (0x%08lx)", shared->picture);

	if (shared->picture == NULL)
	{
		IFEELIN F_Dispose(shared);
		
		return NULL;
	}

	shared->path = atom;
	shared->refs = 1;

	IFEELIN F_Do
	(
		shared->picture, FM_Get,

		F_AUTO_ID(WIDTH),  		&shared->width,
		F_AUTO_ID(HEIGHT), 		&shared->height,
		F_AUTO_ID(COLORTYPE),	&shared->color_type,

		TAG_DONE
	);

//	  IFEELIN F_Log(0, "got width (%ld) height (%ld) colortype (0x%08lx)", shared->width, shared->height, shared->color_type);

	IFEELIN F_LinkHead(&CUD->shared_bitmap_list, (FNode *) shared);

//	  IFEELIN F_Log(0, "return (0x%08lx)", shared);

	return shared;
}
//+
///id_release_picture

/* the function is not thread safe, you must use an arbiter */

void id_release_picture(struct in_Shared *Shared)
{
	Shared->refs--;

	if (Shared->refs != 0)
	{
		return;
	}

	IFEELIN F_LinkRemove(&CUD->shared_bitmap_list, (FNode *) Shared);

	IFEELIN F_DisposeObj(Shared->picture);

	IFEELIN F_AtomRelease(Shared->path);

	IFEELIN F_Dispose(Shared);
}
//+

/************************************************************************************************
*** picture *************************************************************************************
************************************************************************************************/

///id_picture_create
STATIC F_CODE_CREATE(id_picture_create)
{
	FImage *image;
	
	struct in_Shared * shared;

	/* obtain a shared Picture object */

	F_LOCK_ARBITER;

	shared = id_obtain_picture(Class, Parse->bitmap_url);

	F_UNLOCK_ARBITER;
	
	if (shared == NULL)
	{
		return NULL;
	}

	/* create our private FImage structure */

	image = IFEELIN F_NewP(Pool, sizeof (FImage));

	if (image == NULL)
	{
		return NULL;
	}

	image->shared = shared;

	if (Parse->colors_count != 0)
	{
		image->background_methods = &id_color_table;
		image->background = image->background_methods->create(Obj, Class, Parse, Pool);

		if (image->background == NULL)
		{
			image->background_methods = NULL;
		}
	}

	image->repeat = Parse->bitmap_repeat;
	image->multiple = Parse->bitmap_multiple;

	if (Parse->bitmap_position_x_type == 0)
	{
		image->position_x = 0;
		image->position_x_type = FV_TYPE_PIXEL;
	}
	else
	{
		image->position_x = Parse->bitmap_position_x;
		image->position_x_type = Parse->bitmap_position_x_type;
	}

	if (Parse->bitmap_position_y_type == 0)
	{
		image->position_y = 0;
		image->position_y_type = FV_TYPE_PIXEL;
	}
	else
	{
		image->position_y = Parse->bitmap_position_y;
		image->position_y_type = Parse->bitmap_position_y_type;
	}

	#if 0

	IFEELIN F_Log(0, "position: x (%ld%s) y (%ld%s)",
		image->position_x,
		image->position_x_type == FV_TYPE_PIXEL ? "px" : "%",
		image->position_y,
		image->position_y_type == FV_TYPE_PIXEL ? "px" : "%");

	#endif

	#ifdef DB_CREATE
	IFEELIN F_Log(0,"pic (0x%08lx)(%s) (%ld x %ld)", image->Pic, Parse->url->atom->Key, image->width, image->height);
	#endif

	return image;
}
//+
///id_picture_delete
STATIC F_CODE_DELETE(id_picture_delete)
{
	if (image->background_methods && image->background_methods->delete)
	{
		image->background_methods->delete(image->background);
	}

	F_LOCK_ARBITER;

	id_release_picture(image->shared);

	F_UNLOCK_ARBITER;

	IFEELIN F_DisposeObj(image->rendered);
	IFEELIN F_Dispose(image);
}
//+
///id_picture_setup
STATIC F_CODE_SETUP(id_picture_setup)
{
	if (image->background_methods != NULL)
	{
		image->background_methods->setup(Class, image->background, Render);
	}


	#if 0
	return (int32) IFEELIN F_Do(image->Pic,F_IDO(FM_BitMap_Render),
   
		F_IDO(FA_BitMap_TargetScreen), IFEELIN F_Get(Render->Display, F_IDO(FA_Display_Screen)),
		F_IDO(FA_BitMap_RenderedWidth), &image->width,
		F_IDO(FA_BitMap_RenderedHeight), &image->height,
		
		TAG_DONE);
	#endif

	return TRUE;
}
//+
///id_picture_cleanup
STATIC F_CODE_CLEANUP(id_picture_cleanup)
{
	if (image->background_methods != NULL)
	{
		image->background_methods->cleanup(Class, image->background, Render);
	}
}
//+
///id_picutre_get
STATIC F_CODE_GET(id_picture_get)
{
	bool32 rc=0;

	switch (Attribute)
	{
		case FA_ImageDisplay_Height:
		{
		   rc = image->shared->height;
		}
		break;

		case FA_ImageDisplay_Width:
		{
			if (image->multiple)
			{
				rc = image->shared->width / 4;
			}
			else
			{
			   	rc = image->shared->width;
			}
		}
		break;
	 
		case FA_ImageDisplay_Mask:
		{
			if ((FF_COLOR_TYPE_ALPHA & image->shared->color_type) != 0)
			{
				rc = TRUE;
			}
		}
		break;
	}

	return rc;
}
//+
///id_picture_draw

STATIC F_CODE_DRAW(id_picture_draw)
{
	uint32 bx;
	uint32 by;
	uint32 px;
	uint32 py;

	FRect o;

	uint32 background_rgb = 0;

	uint32 image_w = image->shared->width;
	uint32 image_h = image->shared->height;

	uint32 index_pad = 0;

	if (image->multiple)
	{
		image_w /= 4;

		if (FF_ImageDisplay_Draw_Ghost & Msg->Flags)
		{
			index_pad = image_w * 3;
		}
		else if (FF_ImageDisplay_Draw_Focus & Msg->Flags)
		{
			index_pad = image_w * 2;
		}
		else if (FF_ImageDisplay_Draw_Touch & Msg->Flags)
		{
			index_pad = image_w;
		}
	}

	if (Msg->Origin != NULL)
	{
		o.x1 = Msg->Origin->x;
		o.y1 = Msg->Origin->y;
		o.x2 = o.x1 + Msg->Origin->w - 1;
		o.y2 = o.y1 + Msg->Origin->h - 1;
	}
	else
	{
		o.x1 = Msg->Rect->x1;
		o.y1 = Msg->Rect->y1;
		o.x2 = Msg->Rect->x2;
		o.y2 = Msg->Rect->y2;
	}

/*** adjust positions ***************************************************************************

	We compute the coordinates of the bitmap within the origin box.  Regions
	will  then  be  used  to  clear the background and blit the image within
	restricted rectangles. These coordinates are also used when tiling.

*/

	if (image->position_x_type == FV_TYPE_PERCENTAGE)
	{
		uint32 ow = o.x2 - o.x1 + 1;

		if (image_w < ow)
		{
			bx = o.x1 + (ow - image_w) * image->position_x / 100;
		}
		else
		{
			bx = o.x1;
		}
	}
	else
	{
		bx = o.x1 + image->position_x;
	}

	if (image->position_y_type == FV_TYPE_PERCENTAGE)
	{
		uint32 oh = o.y2 - o.y1 + 1;

		if (image_h < oh)
		{
			by = o.y1 + (oh - image_h) * image->position_y / 100;
		}
		else
		{
			by = o.y1;
		}
	}
	else
	{
		by = o.y1 + image->position_y;
	}

#if 0
///scale
		case FV_PICTURE_MODE_SCALE:
		{
			if (Msg->Origin != NULL)
			{
				#if 1

				if ((image->rendered_width != Msg->Origin->w) ||
					(image->rendered_height != Msg->Origin->h))
				{
					IFEELIN F_DisposeObj(image->rendered);

					//IFEELIN F_Log(0,"scale image (%ld x %ld) to (%ld x %ld)", image->width, image->height, Msg->Origin->w, Msg->Origin->h);

					image->rendered = (FObject) IFEELIN F_Do(image->Pic, (uint32) "FM_BitMap_Clone");

					//IFEELIN F_Log(0,"cloned: 0x%08lx", image->rendered);

					if (image->rendered != NULL)
					{
						IFEELIN F_Do(image->rendered, (uint32) "AddProcessing", FV_BitMap_AddProcessing_Scale, Msg->Origin->w, Msg->Origin->h, TAG_DONE);
						IFEELIN F_Do(image->rendered, (uint32) "Render");
					}

					/*
					IFEELIN F_Do(image->Pic,F_IDO(FM_BitMap_Render),

						F_IDO(FA_BitMap_TargetScreen), IFEELIN F_Get(Msg->Render->Display, F_IDO(FA_Display_Screen)),
						F_IDO(FA_BitMap_TargetWidth), Msg->Origin->w,
						F_IDO(FA_BitMap_TargetHeight), Msg->Origin->h,
						F_IDO(FA_BitMap_RenderedWidth), &image->width,
						F_IDO(FA_BitMap_RenderedHeight), &image->height,

						TAG_DONE);
					*/
				}

				#endif
			}
			else
			{
		
				/* We *need* origins to scale the picture, if we don't have
				them we only perform a regular blit */
			
				goto __no_repeat;

			}
		}
//+
#endif

	#if 1

	/* background */

	if ((image->repeat == FV_BITMAP_REPEAT_NO) && (image->background != NULL))
	{
		#if 0

		id_draw(image->background, image->background_methods->draw, Msg->Render, Msg->Origin, Msg->Rect, 0, Class);

		#else

		struct Region *region = IGRAPHICS NewRegion();

		if (region != NULL)
		{
			FRect r;

			r.x1 = bx;
			r.y1 = by;
			r.x2 = bx + image_w - 1;
			r.y2 = by + image_h - 1;
			
			IGRAPHICS OrRectRegion(region, (struct Rectangle *) Msg->Rect);
			IGRAPHICS ClearRectRegion(region, (struct Rectangle *) &r);
		}

		id_draw(image->background, image->background_methods->draw, Msg->Render, Msg->Origin, Msg->Palette, (FRect *) region, FF_ImageDisplay_Draw_Region, Class);

		if (region != NULL)
		{
			IGRAPHICS DisposeRegion(region);
		}

		background_rgb = id_color_get_rgb(image->background, Msg->Render);

		#endif
	}

	#endif

	/* picture */
 
	//IFEELIN F_Log(0,"offsets (%3ld : %3ld) box (%3ld : %3ld) (%3ld x %3ld)", px, py, b.x,b.y,b.w,b.h);

	/* check if the bitmap box is within rectangle boudaries */

	#if 0

	if (bx > Msg->Rect->x2)
	{
//        IFEELIN F_Log(0, "over x2");

		return TRUE;
	}

	if (bx + b.w - 1 < Msg->Rect->x1)
	{
//		  IFEELIN F_Log(0, "below x1");

		return TRUE;
	}

	if (by > Msg->Rect->y2)
	{
//		  IFEELIN F_Log(0, "over y2");

		return TRUE;
	}

	if (by + b.h - 1 < Msg->Rect->y1)
	{
//		  IFEELIN F_Log(0, "below y1");

		return TRUE;
	}

	#endif

	if (image->repeat == FV_BITMAP_REPEAT_NO)
	{
		struct Region *region = IGRAPHICS NewRegion();

		if (region != NULL)
		{
			struct RegionRectangle *rr;

			FRect r;
			FBox d;

			r.x1 = bx;
			r.y1 = by;
			r.x2 = bx + image_w - 1;
			r.y2 = by + image_h - 1;

	        IGRAPHICS OrRectRegion(region, (struct Rectangle *) &r);
			IGRAPHICS AndRectRegion(region, (struct Rectangle *) Msg->Rect);

			for (rr = region->RegionRectangle ; rr ; rr = rr->Next)
			{
				int16 x1 = region->bounds.MinX + rr->bounds.MinX;
				int16 y1 = region->bounds.MinY + rr->bounds.MinY;
				int16 x2 = region->bounds.MinX + rr->bounds.MaxX;
				int16 y2 = region->bounds.MinY + rr->bounds.MaxY;

				if ((x1 <= x2) && (y1 <= y2))
				{
					d.x = x1;
					d.y = y1;
					d.w = x2 - x1 + 1;
					d.h = y2 - y1 + 1;

					//IFEELIN F_Log(0, "blit (%ld : %ld, %ld x %ld)", db.x, db.y, db.w, db.h);

					IFEELIN F_Do( image->rendered ? image->rendered : image->shared->picture, F_AUTO_ID(BLIT),

						Msg->Render->RPort,

						(x1 - bx) % image_w + index_pad,
						(y1 - by) % image_h,
						&d,

						F_AUTO_ID(BLITMODE), FV_BitMap_BlitMode_Copy,
		    			(image->background != NULL) ? F_AUTO_ID(BACKGROUND) : TAG_IGNORE, background_rgb,

						TAG_DONE);
				}
				#ifdef DB_VERBOSE_BAD_COORDINATES

				else
				{
					IFEELIN F_Log(0, "invalid coordinates: x (%ld, %ld) y (%ld, %ld) (%ld x %ld)", x1, x2, y1, y2, x2 - x1, y2 - y1);
				}

				#endif
			}

			IGRAPHICS DisposeRegion(region);
		}
	}
	else
	{
		FBox d;

		uint32 mode;

		uint16 x1 = Msg->Rect->x1;
		uint16 y1 = Msg->Rect->y1;
		uint16 x2 = Msg->Rect->x2;
		uint16 y2 = Msg->Rect->y2;

		int32 cmp_x = x1 - bx;
		int32 cmp_y = y1 - by;
			
		if (cmp_x < 0)
		{
			px = image_w - (-cmp_x % image_w);
		}
		else
		{
			px = cmp_x % image_w;
		}

		if (cmp_y < 0)
		{
			py = image_h - (-cmp_y % image_h);
		}
		else
		{
			py = cmp_y % image_h;
		}

		d.x = x1;
		d.y = y1;
		d.w = x2 - x1 + 1;
		d.h = y2 - y1 + 1;

		mode = FV_BitMap_BlitMode_Tile;

		IFEELIN F_Do(image->rendered ? image->rendered : image->shared->picture, F_AUTO_ID(BLIT),

			Msg->Render->RPort,

			px + index_pad, py, &d,

			F_AUTO_ID(BLITMODE), mode,
		    (image->background != NULL) ? F_AUTO_ID(BACKGROUND) : TAG_IGNORE, background_rgb,

			TAG_DONE);

	}
	
	return TRUE;
}
//+

struct in_CodeTable id_picture_table =
{
	(in_Code_Create *)   &id_picture_create,
	(in_Code_Delete *)   &id_picture_delete,
	(in_Code_Setup *)    &id_picture_setup,
	(in_Code_Cleanup *)  &id_picture_cleanup,
	(in_Code_Get *)      &id_picture_get,
	(in_Code_Draw *)     &id_picture_draw
};

