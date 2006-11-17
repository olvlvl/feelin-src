/************************************************************************************************
*** Includes ************************************************************************************
************************************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/feelin.h>

#include <exec/memory.h>
#include <intuition/intuitionbase.h>
#include <libraries/feelin.h>

#include <feelin/string.h>
#include <feelin/preference.h>
#include <feelin/coremedia.h>
#include <feelin/rtg.h>

extern struct ClassUserData        *CUD;

/************************************************************************************************
*** Attributes & Methods ************************************************************************
************************************************************************************************/

enum    {

		FV_AUTO_RENDER,
		FV_AUTO_BLIT,
		FV_AUTO_WIDTH,
		FV_AUTO_HEIGHT,
		FV_AUTO_COLORTYPE,
		FV_AUTO_COLORARRAY,
		FV_AUTO_COLORCOUNT,

		FV_AUTO_BLITMODE,
		FV_AUTO_BACKGROUND,

		#if 0
		FV_AUTO_SCALEFILTER,
		#endif

		FV_AUTO_SCREEN

		};

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

struct in_DrawMessage
{
	FRender                        *Render;
	FBox                           *Origin;
	FPalette					   *Palette;
	
	FRect                          *Rect;

	bits32                          Flags;
};

#define FV_IMAGEDISPLAY_COLORS_MAX              4
 
struct in_Parse
{
	bits32                          flags;

	uint32							colors_value[FV_IMAGEDISPLAY_COLORS_MAX];
	uint8							colors_type[FV_IMAGEDISPLAY_COLORS_MAX];
	uint8                           colors_count;

	uint8                          	pattern;
	bits8							pattern_size;

	uint16                          gradient_angle;
	bool8							gradient_clockwise;

	uint8							_pad0;

	struct Hook *                   hook;

	/* bitmap */

	STRPTR							bitmap_url;
	uint32							bitmap_url_length;

	uint16							bitmap_position_x;
	uint16							bitmap_position_y;
	bits32						    bitmap_position_x_type;
	bits32							bitmap_position_y_type;

	uint8							bitmap_repeat;
	bool8							bitmap_multiple;
};

/* HOMBRE @ GOFROMIEL : J'ai déplacé struct in_parse ci-dessus car utilisée ci-dessous */

typedef   APTR (in_Code_Create)                  (FObject Obj, FClass *Class, struct in_Parse *Parse, APTR Pool);
typedef   void (in_Code_Delete)                  (APTR Image);
typedef bool32 (in_Code_Setup)                   (FClass *Class, APTR Image, FRender *Render);
typedef   void (in_Code_Cleanup)                 (FClass *Class, APTR Image, FRender *Render);
typedef bool32 (in_Code_Get)                     (APTR Image,FClass *Class, uint32 Attribute);
typedef bool32 (in_Code_Draw)                    (APTR Image, struct in_DrawMessage *Msg, FClass *Class);

/* Except for 'delete', ALL members MUST be filled !! */
 
struct in_CodeTable
{
	in_Code_Create                 *create;
	in_Code_Delete                 *delete;
	in_Code_Setup                  *setup;
	in_Code_Cleanup                *cleanup;
	in_Code_Get                    *get;
	in_Code_Draw                   *draw;
};

/* use the following macros to create image type functions */

#define F_CODE_CREATE(name)                     APTR  name(FObject Obj, FClass *Class, struct in_Parse *Parse, APTR Pool)
#define F_CODE_DELETE(name)                     void  name(FImage *image)
#define F_CODE_SETUP(name)                      bool32 name(FClass *Class, FImage *image, FRender *Render)
#define F_CODE_CLEANUP(name)                    void  name(FClass *Class, FImage *image,FRender *Render)
#define F_CODE_GET(name)                        bool32 name(FImage *image,FClass *Class, uint32 Attribute)
#define F_CODE_DRAW(name)                       bool32 name(FImage *image, struct in_DrawMessage *Msg, FClass *Class)

/************************************************************************************************
*** Class ***************************************************************************************
************************************************************************************************/

struct ClassUserData
{
	FList							shared_bitmap_list;
	APTR                            pool;
};

#define F_LOCK_ARBITER							IFEELIN F_OPool(CUD->pool)
#define F_UNLOCK_ARBITER						IFEELIN F_RPool(CUD->pool)

/************************************************************************************************
*** Types ***************************************************************************************
************************************************************************************************/

enum	{

		FV_COLORTYPE_NONE,
		FV_COLORTYPE_RGB,
		FV_COLORTYPE_ENTRY,
		FV_COLORTYPE_PEN

		};

enum    {

		FV_PATTERN_DOTS = 1,
		FV_PATTERN_GROOVE,
		FV_PATTERN_STRIPES,
		FV_PATTERN_OBLIQUE

		};

#define FF_PATTERN_XSMALL				(1 << 0)
#define FF_PATTERN_SMALL				(1 << 1)
#define FF_PATTERN_MEDIUM				(1 << 2)
#define FF_PATTERN_LARGE				(1 << 3)

/** Bitmap **************************************************************************************

	positions:

		* aucune position n'est définie, le bitmap est déssiné en (0, 0).

		* une seule position est définie, l'autre position est mise à 50%.

		pour les pourcentages,  la  position  est  calculée  par  rapport  à
		l'espace  disponible  auquel  est  soustrait  la  taille de l'image.
		Ainsi, une position 'y' à 100% alignera l'image totalement à droite.

		mots clefs: 'left' x à 0%, 'right' x à 100%, 'top' y à 0%,  'bottom'
		y  à  0%, 'center' x et y à 50%. NOTE: x ou y ne sont pas surchargés
		par 'center' si déjà définis, ainsi on peut utiliser: "center left",
		ou "left center".

		les positions sont valides même lorsque le bitmap est répété.

*/

enum	{

		FV_BITMAP_REPEAT,
		FV_BITMAP_REPEAT_X,
		FV_BITMAP_REPEAT_Y,
		FV_BITMAP_REPEAT_NO

		};

/************************************************************************************************
*** Object **************************************************************************************
************************************************************************************************/

struct LocalObjectData
{
	APTR                            image;
	struct in_CodeTable            *image_methods;

	FRender                        *render;
	FBox                           *origin;
	
	FObject							owner;
	FAreaPublic					   *owner_public;
};

/*** Prototypes ********************************************************************************/

extern struct in_CodeTable          id_pattern_table;
extern struct in_CodeTable          id_color_table;
extern struct in_CodeTable          id_gradient_table;
extern struct in_CodeTable          id_brush_table;
extern struct in_CodeTable          id_picture_table;
extern struct in_CodeTable          id_hook_table;

bool32 id_draw(APTR Image, in_Code_Draw *Func, FRender *Render, FBox *Origin, FPalette *Palette, FRect *Rect, bits32 Flags, FClass *Class);

#ifndef __amigaos4__
/* HOMBRE @ GOFRMIEL : GCC ne veut pas du proto ci-dessous car FImage est indéfini
			J'ai vu qu'il y a 2 déclaration de FImage (une dans bitmp.c et l'autre
			dans color.c
			Sans doute est-ce pour cela que tu as mis APTR au lieu de (FImage *), mais
			GCC n'en veut pas non plus :-)
			Je préfère donc ignorer le proto, même si cela génère un warning, au moins
			ça compile ! */
uint32 id_color_get_rgb(APTR image, FRender *Render);
#endif

#ifndef F_NEW_STYLES

F_HOOKM_PROTO(int32,code_image_construct,FS_Associated_Construct);
F_HOOKM_PROTO(void,code_image_destruct,FS_Associated_Destruct);

#endif
