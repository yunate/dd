
#ifndef ddgif_ddgif_h_
#define ddgif_ddgif_h_

/*********************************************************************
// 感谢参考博客：
// https://www.jianshu.com/p/4fabac6b15b3
// https://www.jianshu.com/p/62ad4f953660
**********************************************************************/

#include "ddgif/ddgifdef.h"
#include "ddgif/string_table.h"
#include <string>

namespace DogGifNSP
{

class DogGif
{
public:
    /** 构造函数
    */
    DogGif();

    /** 析构函数
    */
    ~DogGif();

    // 不允许拷贝
    DogGif& operator=(const DogGif&) = delete;
    DogGif(const DogGif&) = delete;
    DogGif(DogGif&&) = delete;

public:
    /** 初始化
    @note pBuff生命周期保证在Init函数执行期间有效即可
    @param [in] pBuff 图片的二进制数据（文件中读出来的原始数据）
    @paran [in] buffLen pBuff长度
    @return 是否成功
    */
    bool Init(u8* pBuff, u32 buffLen);

    /** 返回下一帧，线程不安全，如果成功内部帧index加一
    @param [out] ppBuff 输出颜色数组，内存不要自己释放
    @param [out] buffLen 数组长度
    @reutrn 是否成功
    */
    bool GetNextFrame(DogGifColor ** ppBuff, u32 & buffLen);

    /** 返回当前帧，线程不安全
    @param [out] ppBuff 输出颜色数组，内存不要自己释放
    @param [out] buffLen 数组长度
    @reutrn 是否成功
    */
    bool GetCurrentFrame(DogGifColor ** ppBuff, u32 & buffLen);

    /** 获得全局宽度
    @return 全局宽度
    */
    u32 GetGolWidth();

    /** 获得全局高度
    @return 全局高度
    */
    u32 GetGolHeight();

    /** 获得全局时间间隔，取第0帧的
    @return 全局时间间隔
    */
    u32 GetGolTimeDelay();
    u32 GetCurTimeDelay();
    u32 GetTimeDelay(u32 frameIdx);

    /** 是否初始化了
    @return 是否初始化了
    */
    bool HasInit();

    /** 一共有多少帧
    @return 帧数目
    */
    u32 GetFrameCount();

    /** 获得当前第几帧
    @return 第几帧
    */
    u32 GetCurFrameIndex();

private:
    /** 读gif头
    @pram [in, out] ppBuff head的开始指针，读取过后自动向后移动响应位置
    @pram [in, out] buffLen ppBuff长度，读取过后自动减少读取的个数
    @return 是否成功
    */
    bool ReadHead(u8 ** ppBuff, u32& buffLen);

    /** 读LSD
    @pram [in, out] ppBuff head的开始指针，读取过后自动向后移动响应位置
    @pram [in, out] buffLen ppBuff长度，读取过后自动减少读取的个数
    @return 是否成功
    */
    bool ReadLsd(u8 ** ppBuff, u32& buffLen);

    /** 读全局色表
    @pram [in, out] ppBuff head的开始指针，读取过后自动向后移动响应位置
    @pram [in, out] buffLen ppBuff长度，读取过后自动减少读取的个数
    @return 是否成功
    */
    bool ReadGolColorTable(u8** ppBuff, u32& buffLen);

    /** 读一帧
    @pram [in, out] ppBuff head的开始指针，读取过后自动向后移动响应位置
    @pram [in, out] buffLen ppBuff长度，读取过后自动减少读取的个数
    @return 成功返回DogGifFrame对象，否则NULL
    */
    DogGifFrame* ReadFrameData(u8** ppBuff, u32& buffLen);

private:
    /** 解密m_curFrame索引的帧。
    @note 要求m_frameBuff存储的是上一帧，连续调用而没有改变m_curFrame，那么很有可能出错
    @return 是否成功
    */
    bool DecodeFrame();

private:
    /** 当前帧的图片buff，在DecodeFrame函数内表示上一帧
    */
    std::vector<DogGifColor> m_frameBuff;

    /** 当前帧的索引
    */
    u32 m_curFrame;

    /** 全局gif信息
    */
    DogGifGolInfo m_gifGolInfo;

    /** 加解密帧用
    */
    StringTable* m_pStringTable;

    /** 是否已经初始化了
    */
    bool m_hasInit;
};

}
#endif // ddgif_ddgif_h_

