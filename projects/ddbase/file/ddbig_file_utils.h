
#ifndef ddbase_file_ddbig_file_utils_h_
#define ddbase_file_ddbig_file_utils_h_

#include "ddbase/ddnocopyable.hpp"
#include "ddbase/dddef.h"
#include <memory>

namespace NSP_DD {

/** 默认文件块大小10M
*/
static const s32 g_file_block_default_size = 1024 * 1024 * 10;

class ddbig_file_mapper;

/** 一个大文件的文件块
*/
class ddfile_block
{
    DDNO_COPY_MOVE(ddfile_block);
public:
    ddfile_block() = default;

    /** 析构函数
    */
    ~ddfile_block();

    /** 获得文件块大小
    @return 文件块的大小
    */
    inline s32 get_size()
    {
        return m_blockSize;
    }

    /** 获得真正的文件块起始地址
    @return 文件块的起始地址
    */
    inline char* get_block_addr()
    {
        return m_pBlock + m_offSet;
    }

private:
    /** 文件块起始地址
    */
    char* m_pBlock = nullptr;

    /** 由于文件映射要求64k对齐，所以这儿的offset才是真正的开始
    */
    s32 m_offSet = 0;

    /** 文件块大小
    */
    s32 m_blockSize = 0;

    friend ddbig_file_mapper;
};

using sp_file_block = std::shared_ptr<ddfile_block>;

/** 内存映射文件来处理大文件，可读写，但是在文件大小改变了的话需要重新映射
*/
class ddbig_file_mapper
{
    DDNO_COPY_MOVE(ddbig_file_mapper);
public:
    /** 构造函数
    */
    ddbig_file_mapper();

    /** 析构函数
    */
    ~ddbig_file_mapper();

public:
    /** 初始化，重复调用会清空上一次的数据
    @param [in] file_path 文件路径
    @return 是否成功
    */
    bool map_file(const std::wstring& file_path);

    /** 获得文件大小
    @return 文件大小
    */
    inline s64 get_file_size()
    {
        return m_fileSize;
    }

    /** 获得文件块的下一块
    @param [in] size 指定大小（B），如果小于等于0或者大于1G的话使用默认大小，如果有末尾对齐（align != 0）返回的block大小可能要小于size
    @param [in] align size 指定的大小可能不在一个句子的尾巴，所以会向前找到第一个出现align（包括）的字符处,align == 0 说明不对齐，默认"\n"
    @return 文件块的share_ptr，如果为空说明map到文件尾部了
    */
    sp_file_block get_next_block(s32 sizeRaw = g_file_block_default_size, char align = '\n');

    /** 获得文件块
    @param [in] beginPos 开始位置
    @param [in] size 指定大小（B），如果小于等于0或者大于1G的话使用默认大小，如果有末尾对齐（align != 0）返回的block大小可能要小于size
    @param [in] align size 指定的大小可能不在一个句子的尾巴，所以会向前找到第一个出现align（包括）的字符处,align == 0 说明不对齐，默认"\n"
    @return 文件块的share_ptr，如果为空说明map到文件尾部了
    */
    sp_file_block get_block(s64 beginPos, s32 sizeRaw = g_file_block_default_size, char align = '\n');

private:
    /** 文件映射内存的句柄
    */
    void* m_hFileMap = nullptr;

    /** 映射的位置
    */
    s64 m_bginPos;

    /** 文件小
    */
    s64 m_fileSize;
};

} // namespace NSP_DD
#endif // ddbase_file_ddbig_file_utils_h_
