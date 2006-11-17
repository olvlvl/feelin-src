#include "Private.h"
#include "processing.h"

///typedefs

#define GSIMUL 4096

#define F_PIXEL_R(x)                            ((x & 0xFF000000) >> 24)
#define F_PIXEL_G(x)                            ((x & 0x00FF0000) >> 16)
#define F_PIXEL_B(x)                            ((x & 0x0000FF00) >>  8)
#define F_PIXEL_A(x)                            (x & 0x000000FF)

struct in_BitMapScale;

typedef uint32 (*FPixelFunc)                    (struct in_BitMapScale *fbs, uint32 *source, uint32 x, uint32 y);
typedef void (*FLineYFunc)                      (struct in_BitMapScale *fbs, uint32 *source, APTR target);
typedef void (*FLineXFunc)                      (struct in_BitMapScale *fbs, uint32 *source, APTR target, uint32 tpy);

typedef struct in_BitMapScale
{
    uint32                          smod;
    uint32                          source_w;
    uint32                          source_h;
    uint16                          xfrac;
    uint16                          yfrac;

    uint32                          target_w;
    uint32                          target_h;

    FPixelFunc                      code_render_pixel;
    FLineYFunc                      code_render_y;
    FLineXFunc                      code_render_x;
}
fbsender;

#define GSIMUL 4096

//+

///pixels_scale_bilinear
/*
 * v00|     |v01
 *   -+-----+-
 *    |     |      For 'x' we get the 4 corners with their intensity depending on the 'x' placement.
 *    |     |      We sum up the whole then divide. The resulting pixel has the correct value of
 *    |   x |      the 4 pixels surrounding it.
 *    |     |
 *   -+-----+-
 * v10|     |v11
 */
static uint32 pixels_scale_bilinear(fbsender *fbs, uint32 *s, uint32 x, uint32 y)
{
    uint32 v00, v10, v01, v11; /* ul, ll, ur, lr */
    uint32 v00m, v10m, v01m, v11m; /* multipliers */
    uint32 s00, s10, s01, s11;
    uint32 d;
    uint32 advx, advy;

    /* limit borders */

    if (x + 1 >= fbs->source_w)
    {
        advx = 0;
    }
    else
    {
        advx = sizeof (uint32);
    }

    if (y + 1 >= fbs->source_h)
    {
        advy = 0;
    }
    else
    {
        #if 0
        advy = fbs->smod;
        #else
        advy = fbs->smod;
        #endif
    }

    /* fetch pixels */

    s00 = *s;
    #if 0
    s01 = *(s + advx);
    s10 = *(s + advy);
    s11 = *(s + advy + advx);
    #else
    s01 = *((uint32 *)((uint32)(s) + advx));
    s10 = *((uint32 *)((uint32)(s) + advy));
    s11 = *((uint32 *)((uint32)(s) + advy + advx));
    #endif

    /* pre-multiply */

    v00m = (GSIMUL - fbs->xfrac) * (GSIMUL - fbs->yfrac);
    v01m = fbs->xfrac * (GSIMUL - fbs->yfrac);
    v10m = (GSIMUL - fbs->xfrac) * fbs->yfrac;
    v11m = fbs->xfrac * fbs->yfrac;

    /* A */

    v00 = v00m * F_PIXEL_A(s00);
    v01 = v01m * F_PIXEL_A(s01);
    v10 = v10m * F_PIXEL_A(s10);
    v11 = v11m * F_PIXEL_A(s11);
    d = ((v00 + v10 + v01 + v11 + GSIMUL * GSIMUL / 2) / (GSIMUL * GSIMUL)) << 24;

    /* R */

    v00 = v00m * F_PIXEL_R(s00);
    v01 = v01m * F_PIXEL_R(s01);
    v10 = v10m * F_PIXEL_R(s10);
    v11 = v11m * F_PIXEL_R(s11);
    d |= ((v00 + v10 + v01 + v11 + GSIMUL * GSIMUL / 2) / (GSIMUL * GSIMUL)) << 16;

    /* G */

    v00 = v00m * F_PIXEL_G(s00);
    v01 = v01m * F_PIXEL_G(s01);
    v10 = v10m * F_PIXEL_G(s10);
    v11 = v11m * F_PIXEL_G(s11);
    d |= ((v00 + v10 + v01 + v11 + GSIMUL * GSIMUL / 2) / (GSIMUL * GSIMUL)) << 8;

    /* B */

    v00 = v00m * F_PIXEL_B(s00);
    v01 = v01m * F_PIXEL_B(s01);
    v10 = v10m * F_PIXEL_B(s10);
    v11 = v11m * F_PIXEL_B(s11);
    d |= ((v00 + v10 + v01 + v11 + GSIMUL * GSIMUL / 2) / (GSIMUL * GSIMUL)) & 0xff;

    return (d);
}
//+
///pixels_scale_nearest
static uint32 pixels_scale_nearest(fbsender *fbs, uint32 *s, uint32 x, uint32 y)
{
    return *s;
}
//+
///pixels_scale_average
/*
 *    |   |   |
 * v00|v01|v02|v03
 * ---+---+---+---
 * v10|v11|v12|v13  For an 'x' in v11, we get the 4x4 grid surrounding then
 * ---+---+---+---  use the average of them.
 * v20|v21|v22|v23
 * ---+---+---+---
 * v30|v31|v32|v33
 *    |   |   |
 */
static uint32 pixels_scale_average(fbsender *fbs, uint32 *s, uint32 x, uint32 y)
{
    uint32 v00, v01, v02, v03;
    uint32 v10, v11, v12, v13;
    uint32 v20, v21, v22, v23;
    uint32 v30, v31, v32, v33;

    uint32 s00, s01, s02, s03;
    uint32 s10, s11, s12, s13;
    uint32 s20, s21, s22, s23;
    uint32 s30, s31, s32, s33;

    uint32 d;
    uint32 bckx;
    uint32 bcky;

    /* limit borders */

    if ((int32)(x) - 1 < 0)
    {
        bckx = 0;
    }
    else
    {
        bckx = sizeof (uint32);
    }

    if ((int32)(y) - 1 < 0)
    {
        bcky = 0;
    }
    else
    {
        bcky = fbs->smod;
    }

    /* fetch pixels */

    s00 = *((uint32 *)((uint32)(s) - bcky - bckx));
    s01 = *((uint32 *)((uint32)(s) - bcky));
    s02 = *((uint32 *)((uint32)(s) - bcky + 1 * sizeof (uint32)));
    s03 = *((uint32 *)((uint32)(s) - bcky + 2 * sizeof (uint32)));
    s10 = *((uint32 *)((uint32)(s) - bckx));
    s11 = *(s);
    s12 = *((uint32 *)((uint32)(s) + 1 * sizeof (uint32)));
    s13 = *((uint32 *)((uint32)(s) + 2 * sizeof (uint32)));
    s20 = *((uint32 *)((uint32)(s) + fbs->smod - bckx));
    s21 = *((uint32 *)((uint32)(s) + fbs->smod));
    s22 = *((uint32 *)((uint32)(s) + fbs->smod + 1 * sizeof (uint32)));
    s23 = *((uint32 *)((uint32)(s) + fbs->smod + 2 * sizeof (uint32)));
    s30 = *((uint32 *)((uint32)(s) + fbs->smod * 2 - bckx));
    s31 = *((uint32 *)((uint32)(s) + fbs->smod * 2));
    s32 = *((uint32 *)((uint32)(s) + fbs->smod * 2 + 1 * sizeof (uint32)));
    s33 = *((uint32 *)((uint32)(s) + fbs->smod * 2 + 2 * sizeof (uint32)));

    /* A */
    v00 = F_PIXEL_A(s00);
    v01 = F_PIXEL_A(s01);
    v02 = F_PIXEL_A(s02);
    v03 = F_PIXEL_A(s03);
    v10 = F_PIXEL_A(s10);
    v11 = F_PIXEL_A(s11);
    v12 = F_PIXEL_A(s12);
    v13 = F_PIXEL_A(s13);
    v20 = F_PIXEL_A(s20);
    v21 = F_PIXEL_A(s21);
    v22 = F_PIXEL_A(s22);
    v23 = F_PIXEL_A(s23);
    v30 = F_PIXEL_A(s30);
    v31 = F_PIXEL_A(s31);
    v32 = F_PIXEL_A(s32);
    v33 = F_PIXEL_A(s33);
    d = ((v00 + v01 + v02 + v03 + v10 + v11 + v12 + v13 + v20 + v21 + v22 + v23 + v30 + v31 + v32 + v33) / 16) << 24;

    /* R */
    v00 = F_PIXEL_R(s00);
    v01 = F_PIXEL_R(s01);
    v02 = F_PIXEL_R(s02);
    v03 = F_PIXEL_R(s03);
    v10 = F_PIXEL_R(s10);
    v11 = F_PIXEL_R(s11);
    v12 = F_PIXEL_R(s12);
    v13 = F_PIXEL_R(s13);
    v20 = F_PIXEL_R(s20);
    v21 = F_PIXEL_R(s21);
    v22 = F_PIXEL_R(s22);
    v23 = F_PIXEL_R(s23);
    v30 = F_PIXEL_R(s30);
    v31 = F_PIXEL_R(s31);
    v32 = F_PIXEL_R(s32);
    v33 = F_PIXEL_R(s33);
    d |= ((v00 + v01 + v02 + v03 + v10 + v11 + v12 + v13 + v20 + v21 + v22 + v23 + v30 + v31 + v32 + v33) / 16) << 16;

    /* G */
    v00 = F_PIXEL_G(s00);
    v01 = F_PIXEL_G(s01);
    v02 = F_PIXEL_G(s02);
    v03 = F_PIXEL_G(s03);
    v10 = F_PIXEL_G(s10);
    v11 = F_PIXEL_G(s11);
    v12 = F_PIXEL_G(s12);
    v13 = F_PIXEL_G(s13);
    v20 = F_PIXEL_G(s20);
    v21 = F_PIXEL_G(s21);
    v22 = F_PIXEL_G(s22);
    v23 = F_PIXEL_G(s23);
    v30 = F_PIXEL_G(s30);
    v31 = F_PIXEL_G(s31);
    v32 = F_PIXEL_G(s32);
    v33 = F_PIXEL_G(s33);
    d |= ((v00 + v01 + v02 + v03 + v10 + v11 + v12 + v13 + v20 + v21 + v22 + v23 + v30 + v31 + v32 + v33) / 16) << 8;

    /* B */
    v00 = F_PIXEL_B(s00);
    v01 = F_PIXEL_B(s01);
    v02 = F_PIXEL_B(s02);
    v03 = F_PIXEL_B(s03);
    v10 = F_PIXEL_B(s10);
    v11 = F_PIXEL_B(s11);
    v12 = F_PIXEL_B(s12);
    v13 = F_PIXEL_B(s13);
    v20 = F_PIXEL_B(s20);
    v21 = F_PIXEL_B(s21);
    v22 = F_PIXEL_B(s22);
    v23 = F_PIXEL_B(s23);
    v30 = F_PIXEL_B(s30);
    v31 = F_PIXEL_B(s31);
    v32 = F_PIXEL_B(s32);
    v33 = F_PIXEL_B(s33);
    d |= ((v00 + v01 + v02 + v03 + v10 + v11 + v12 + v13 + v20 + v21 + v22 + v23 + v30 + v31 + v32 + v33) / 16) & 0xff;

    return (d);
}
//+

///bitmap_render_x_scale
void bitmap_render_x_scale(struct in_BitMapScale *fbs, uint32 *source, uint32 *target, uint32 tpy)
{
    uint32 spx = fbs->source_w * GSIMUL / fbs->target_w;
    uint32 tpx;
    uint32 j;

    for (j = 0 ; j < fbs->target_w ; j++)
    {
        tpx = (spx * j);
        fbs->xfrac = tpx & (GSIMUL - 1);
        tpx /= GSIMUL;

        *target++ = fbs->code_render_pixel(fbs, source + tpx, tpx, tpy);
    }
}
//+
///bitmap_render_x_plain
void bitmap_render_x_plain(struct in_BitMapScale *fbs, uint32 *source, uint32 *target, uint32 tpy)
{
    uint32 j;

    for (j = 0 ; j < fbs->target_w ; j++)
    {
        *target++ = fbs->code_render_pixel(fbs, source + j, j, tpy);
    }
}
//+

///bitmap_render_y_scale
void bitmap_render_y_scale(fbsender *fbs, uint32 *source, uint32 *target)
{
    uint32 spy = (fbs->source_h * GSIMUL) / fbs->target_h;
    uint32 tpy;
    uint32 *sp;
    uint32 i;

    uint32 modulo = fbs->smod;

    for (i = 0; i < fbs->target_h; i++)
    {
        tpy = (spy * i);
        fbs->yfrac = tpy & (GSIMUL - 1);
        tpy /= GSIMUL;

        sp = (APTR)((uint32)(source) + tpy * modulo);

        fbs->code_render_x
        (
            fbs,
            sp,
            target,
            tpy
        );

        target += fbs->target_w;
    }
}
//+
///bitmap_render_y_plain
void bitmap_render_y_plain(fbsender *fbs, uint32 *source, uint32 *target)
{
    uint32 i;

    for (i = 0; i < fbs->target_h; i++)
    {
        fbs->code_render_x
        (
            fbs,
            source + i * fbs->source_w,
            target + i * fbs->target_w,
            i
        );
    }
}
//+

#if 0
///bitmap_scale_2x
#define SUB_MODULO (q ? -source_pixel_array_mod : 0)
#define ADD_MODULO ((q < height - 1) ? source_pixel_array_mod : 0)
#define SUB_ONE (r ? -1 : 0)
#define ADD_ONE ((r < width - 1) ? 1 : 0)

STATIC void bitmap_scale_2x
(
    uint8 *source_pixel_array,
    uint32 source_pixel_array_mod,

    uint8 *target_pixel_array,
    uint32 target_pixel_array_mod,

    uint32 width,
    uint32 height
)
{
    uint32 x, y, x2, y2;
    uint32 tgt_w, tgt_h;

}
//+
#endif

///scale
void scale(struct LocalObjectData *LOD, struct in_Processing_Scale *data, APTR target)
{
    uint32 *source = LOD->cpp.PixelArray;

    fbsender fbs;

    fbs.smod = LOD->cpp.PixelArrayMod;
    fbs.source_w = LOD->cpp.Width;
    fbs.source_h = LOD->cpp.Height;
    fbs.xfrac = 0;
    fbs.yfrac = 0;
    fbs.target_w = data->Width;
    fbs.target_h = data->Height;

    fbs.code_render_x = (data->Width == LOD->cpp.Width) ? bitmap_render_x_plain : bitmap_render_x_scale;
    fbs.code_render_y = (data->Height == LOD->cpp.Height) ? bitmap_render_y_plain : bitmap_render_y_scale;

    fbs.code_render_pixel = pixels_scale_bilinear;

    if (fbs.code_render_y &&
        fbs.code_render_x && source && target)
    {
        fbs.code_render_y(&fbs, source, target);
    }
}
//+

///bitmap_processing_scale
bool32 bitmap_processing_scale(FClass *Class, FObject Obj, struct in_Processing_Scale *data)
{
    struct LocalObjectData *LOD = F_LOD(Class,Obj);

    uint32 *buffer;

    // FIXME: we shouldn't have to request RGBA

    if (LOD->cpp.PixelSize != 4)
    {
        struct in_Processing_Convert msg = { 4 };

        if (bitmap_processing_convert(Class,Obj,&msg) == FALSE)
        {
            return FALSE;
        }
    }

    #if 0

    buffer = IFEELIN F_New(LOD->cpp.Width * data->Height * LOD->cpp.PixelSize * 4);

    if (buffer)
    {
        bitmap_scale_2x
        (
            LOD->cpp.PixelArray,
            LOD->cpp.PixelArrayMod,

            buffer,
            LOD->cpp.Width * 2 * 4,

            LOD->cpp.Width * 2,
            LOD->cpp.Height * 2
        );

        IFEELIN F_Dispose(LOD->cpp.PixelArray);

        LOD->cpp.Width  *= 2;
        LOD->cpp.Height *= 2;

        LOD->cpp.PixelArray     = buffer;
        LOD->cpp.PixelArrayMod  *= 2;
    }

    #else

    buffer = IFEELIN F_New(data->Width * data->Height * LOD->cpp.PixelSize);

    #if 0

    IFEELIN F_Log
    (
        0, "scale (0x%08lx)(%ld x %ld) >> (0x%08lx)(%ld x %ld)",

        LOD->cpp.PixelArray,
        LOD->cpp.Width,
        LOD->cpp.Height,

        data->Width,
        data->Height
    );

    #endif

    if (buffer)
    {
        scale(LOD, data, buffer);

        IFEELIN F_Dispose(LOD->cpp.PixelArray);

        LOD->cpp.Width  = data->Width;
        LOD->cpp.Height = data->Height;

        LOD->cpp.PixelArray     = buffer;
        LOD->cpp.PixelArrayMod  = data->Width * 4;
    }

    #endif

    return TRUE;
}
//+
