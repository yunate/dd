
#ifndef ddgif_ddgifdef_h_
#define ddgif_ddgifdef_h_

#include <vector>
#include <string>

namespace DogGifNSP
{
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned long;
#pragma pack(push, 1)

/** 数据头
47 49 46 38 39 61
*/
struct GifHead
{
    /** 文件署名
    */
    u8 signature[3];

    /** 文件版本
    */
    u8 version[3];

    bool IsGifFile()
    {
        if (signature[0] != 'G' ||
            signature[1] != 'I' ||
            signature[2] != 'F' ||
            version[0] != '8' ||
            (version[1] != '7' && version[1] != '9') ||
            version[2] != 'a')
        {
            return false;
        }

        return true;
    }
};

/** 逻辑屏幕标识符(Logical Screen Descriptor)
f0 00 b4 00 f7 8b 00
*/
struct GifLogicalScreenDescriptor
{
    /** 宽度（像素）
    00f0
    */
    u16 m_width;

    /** 高度（像素）
    00b4
    */
    u16 m_height;

    /** 颜色描述
       f7 1 111 0 111
        m 7 - 全局颜色列表标志(Global Color Table Flag)，当置位时表示有全局颜色列表，pixel值有意义.
        cr 6 5 4 - 颜色深度(Color ResoluTion)，cr+1确定图象的颜色深度.
        s 3 - 分类标志(Sort Flag)，0 则 Global Color Table 不进行排序，为 1 则表示 Global Color Table 按照降序排列，出现频率最多的颜色排在最前面.
        pixel 2 1 0 - 全局颜色列表大小，pixel+1确定颜色列表的索引数（2的pixel+1次方)
    */
    u8 m_colorDes;

    /** 背景色
    8b
        为背景颜色指向全局色表。背景颜色是指那些没有被图像覆盖的视屏部分的颜色。若全局色表标志位置为0，则该字段也被赋值0，并且被忽略。
    */
    u8 m_bgColor;

    /** 像素宽高比
    00
        如果该字段的值为非0，则象素的高宽比由下面的公式计算：
        高宽比 = (象素高宽比 + 15) / 64
        该字段的取值范围从最宽的比值4：1到最高的比值1：4，递增的步幅为1/64。取值： 0 - 没有比值，1～255 - 用于计算的值。
    */
    u8 m_pixelTo;
};

/** 扩展块
*/
struct ExtendBlock
{
    /** 0x21 标记这是一个扩展块
    */
    u8 m_flag;

    /** 0xf9 标记这是一个图形扩展块
    */
    u8 m_controlLabel;

    /** 块大小 固定4
    */
    u8 m_blockSize;

    /** 用户输入标记
        从左边数一，二，三位表示Reserved for Future Use，即保留位，暂无用处。
        从左边数四，五，六位表示 Disposal Method，表示在进行逐帧渲染时，前一帧留下的图像作何处理：0：不做任何处理。1：保留前一帧图像，在此基础上进行渲染。2：渲染前将图像置为背景色。3：将被下一帧覆盖的图像重置。
        从右数第二位表示 User Input Flag，表示是否需要在得到用户的输入时才进行下一帧的输入（具体用户输入指什么视应用而定）。0 表示无需用户输入。1 表示需要用户输入。
        最右边一位，表示 Transparent Flag，当该值为 1 时，后面的 Transparent Color Index 指定的颜色将被当做透明色处理。为 0 则不做处理。
    */
    u8 m_userFlag;

    /** 播放时延
    */
    u16 m_delayTime;

    /** 透明色索引
    */
    u8 m_TranColorIndex;

    /** 终结束标记 固定为0
    */
    u8 m_endFlag;
};

/** 图象标识符
    包含处理一个基于图像的表的必要参数，它是一幅图所必需的，图象标识符以0x2C(',')字符开始。
    一幅图像对应一个图象标识符，图象标识符后面跟着对应的图像数据。
*/
struct ImageDescriptor
{
    /** 图像的开始，固定为 ','
    */
    u8 m_start;

    /** x方向偏移量
    */
    u16 m_left;

    /** y方向偏移量
    */
    u16 m_top;

    /** 图像宽度
    */
    u16 m_width;

    /** 图像高度
    */
    u16 m_height;

    /** 局部颜色列表标记
        m 7 - 局部颜色列表标志(Local Color Table Flag) 置位时标识紧接在图象标识符之后有一个局部颜色列表，供紧跟在它之后的一幅图象使用；值否时使用全局颜色列表，忽略pixel值。
        i 6 - 交织标志(Interlace Flag)，示是否需要隔行扫描。1 为需要，0 为不需要。
        s 5 - 分类标志(Sort Flag)，如果置位表示紧跟着的局部颜色列表分类排列.
        r 4 3 - 保留，必须初始化为0.
        pixel 2 1 0 - 局部颜色列表大小(Size of Local Color Table)，pixel+1就为颜色列表的位数
    */
    u8 m_localColorFlag;
};

/** 颜色表
*/
struct DogGifColor
{
    DogGifColor(u8 r, u8 g, u8 b, u8 a) :
        m_r(r),
        m_g(g),
        m_b(b),
        m_a(a)
    {

    }

    DogGifColor()
    {

    }

    /** blue
    */
    u8 m_b = 0;

    /** green
    */
    u8 m_g = 0;

    /** red
    */
    u8 m_r = 0;

    /** a
    */
    u8 m_a = 0xff;
};

/** 每一帧信息
*/
struct DogGifFrame
{
    /** 0：不做任何处理。1：保留前一帧图像，在此基础上进行渲染。2：渲染前将图像置为背景色。3：将被下一帧覆盖的图像重置。
    */
    u8 m_disposalMethod;

    /** 用户输入标记
    */
    u8 m_userInputFlag = 0;

    /** 当该值为 1 时，后面的 m_tranColorIndex 指定的颜色将被当做透明色处理。为 0 则不做处理。
    */
    u8 m_tranFlag;

    /** 播放时延
    */
    u16 m_delayTime = 0;

    /** 透明色索引
        置位表示使用透明颜色
    */
    u8 m_tranColorIndex = 0;

    /** x方向偏移量
    */
    u16 m_left = 0;

    /** y方向偏移量
    */
    u16 m_top = 0;

    /** 图像宽度
    */
    u16 m_width = 0;

    /** 图像高度
    */
    u16 m_height = 0;

    /** 是否有局部颜色表
    */
    u8 m_hasLocalColorTable = 0;

    /** 交织标志(Interlace Flag)，置位时图象数据使用交织方式排列，否则使用顺序排列。
    */
    u8 m_interlaceFlag = 0;

    /** 分类标志(Sort Flag)，如果置位表示紧跟着的局部颜色列表分类排列.
    */
    u8 m_sortFlag = 0;

    /** 局部颜色列表颜色个数
    */
    u16 m_LocalColorTableBit = 0;

    /** 局部色表
    */
    std::vector<DogGifColor> m_localColorTable;

    /** 编码长度
    */
    u8 m_codeLen = 0;

    /** frame数据
    */
    std::vector<u8> m_frameData;
};

/** gif全局信息
*/
struct DogGifGolInfo
{
    /** gif标识
        GIF89a 或者 GIF87a
    */
    std::string m_gifHeadSignaturl = "";

    /** 宽度
    */
    u16 m_width = 0;

    /** 高度度
    */
    u16 m_height = 0;

    /** 是否有全局颜色表
    */
    u8 m_hasGolColorTable = 0;

    /** 颜色深度
    */
    u8 m_colorDepth = 0;

    /** 全局颜色列表是否分类排列
    */
    u8 m_isGolColorTableSorted = 0;

    /** 全局颜色列表颜色个数
    */
    u16 m_golColorTableBit = 0;

    /** 背景色位置
    */
    u8 m_bgColorIndex;

    /** 象素的高宽比
        如果该字段的值为非0，则象素的高宽比由下面的公式计算：
        高宽比 = (象素高宽比 + 15) / 64
        该字段的取值范围从最宽的比值4：1到最高的比值1：4，递增的步幅为1/64。取值： 0 - 没有比值，1～255 - 用于计算的值。
    */
    u8 m_pixelToWidthHeight = 0;

    /** 全局色表
    */
    std::vector<DogGifColor> m_golColorTable;

    /** 每一帧数据
    */
    std::vector<DogGifFrame*> m_frameData;

    ~DogGifGolInfo()
    {
        for (auto it : m_frameData)
        {
            delete it;
        }

        m_golColorTable.clear();
    }
};

#pragma pack(pop)
}

#endif // ddgif_ddgifdef_h_

