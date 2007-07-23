enum    {

        // convert

        FV_BitMap_AddProcessing_Convert = 1,

        // operations

        FV_BitMap_AddProcessing_Crop,
        FV_BitMap_AddProcessing_Tint,
        FV_BitMap_AddProcessing_Radial,
        FV_BitMap_AddProcessing_Scale,

        FV_BitMap_AddProcessing_Blur

        };

enum    {

        FA_BitMap_AddProcessing_Left = 1,
        FA_BitMap_AddProcessing_Top,
        FA_BitMap_AddProcessing_Width,
        FA_BitMap_AddProcessing_Height,

        FA_BitMap_AddProcessing_TintRatio

        };

struct  FS_BitMap_AddProcessing                 { uint32 ProcMethod; /* ... */ };
struct  FS_BitMap_AddProcessing_Crop            { uint32 ProcMethod; uint32 Left; uint32 Top; uint32 Width; uint32 Height; /* ... */ };
struct  FS_BitMap_AddProcessing_Tint            { uint32 ProcMethod; uint32 RGB; /* ... */ };
struct  FS_BitMap_AddProcessing_Blur            { uint32 ProcMethod; /* ... */ };

#define _msg_crop                               ((struct FS_BitMap_AddProcessing_Crop *)(Msg))
#define _msg_crop_left                          _msg_crop->Left
#define _msg_crop_top                           _msg_crop->Top
#define _msg_crop_width                         _msg_crop->Width
#define _msg_crop_height                        _msg_crop->Height

#define _msg_tint                               ((struct FS_BitMap_AddProcessing_Tint *)(Msg))
#define _msg_tint_rgb                           _msg_tint->RGB

typedef bool32 (*feelin_processing_func)        (FClass *Class, FObject Obj, APTR data);

// node

struct in_ProcessingNode
{
    struct in_ProcessingNode       *next;
    struct in_ProcessingNode       *prev;

    feelin_processing_func          func;
};

/*** convert ************************************************************************************

    convert a pixel size to another

*/

struct FS_BitMap_AddProcessing_Convert
{
    struct FS_BitMap_AddProcessing  Base;

    uint32                          PixelSize;

    /* ... */
};

#define _msg_convert                            ((struct FS_BitMap_AddProcessing_Convert *)(Msg))
#define _msg_convert_pixelsize                  _msg_convert->PixelSize

struct in_Processing_Convert
{
    uint32                          pixel_size;
};

bool32 bitmap_processing_convert(FClass *Class, FObject Obj, struct in_Processing_Convert *data);

/*** crop ***************************************************************************************



*/

struct in_Processing_Crop
{
    uint16                          x;
    uint16                          y;
    uint16                          w;
    uint16                          h;
};

bool32 bitmap_processing_crop(FClass *Class, FObject Obj, struct in_Processing_Crop *data);

/*** tint ***************************************************************************************



*/

struct in_Processing_Tint
{
    uint16                          x;
    uint16                          y;
    uint16                          w;
    uint16                          h;

    uint32                          rgb;
    uint8                           ratio;
};

bool32 bitmap_processing_tint(FClass *Class, FObject Obj, struct in_Processing_Tint *data);

/*** radial *************************************************************************************

Sets the alpha value low on the borders and  high  near  the  center.  Takes
vertical  and  horizontal  distance into account. 'val' is the maximum alpha
value. Only pixels with an existing alpha value are modified.

*/

struct in_Processing_Radial
{
    uint32                          dummy;
};

bool32 bitmap_processing_radial(FClass *Class, FObject Obj, struct in_Processing_Radial *data);

/*** scale **************************************************************************************


*/

struct FS_BitMap_AddProcessing_Scale
{
    struct FS_BitMap_AddProcessing  Base;

    uint32                          Width;
    uint32                          Height;

    /* ... */
};

#define _msg_scale                              ((struct FS_BitMap_AddProcessing_Scale *)(Msg))
#define _msg_scale_width                        _msg_scale->Width
#define _msg_scale_height                       _msg_scale->Height

struct in_Processing_Scale
{
    uint32                          Width;
    uint32                          Height;
};

bool32 bitmap_processing_scale(FClass *Class, FObject Obj, struct in_Processing_Scale *data);

/*** blur ***************************************************************************************

*/

struct in_Processing_Blur
{
    uint32                          dummy;
};

bool32 bitmap_processing_blur(FClass *Class, FObject Obj, struct in_Processing_Blur *data);
