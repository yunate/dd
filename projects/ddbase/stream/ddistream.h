
#ifndef ddbase_stream_ddistream_h_
#define ddbase_stream_ddistream_h_

#include "ddbase/dddef.h"
#include <stdio.h>

namespace NSP_DD {
class ddistream
{
public:
    virtual ~ddistream() { };

    // 获得当前位置 seek
    // @return 流当前位置, 小于0表示异常
    virtual s64 pos() const = 0;

    // 设置流当前位置
    // @param [int] offset 偏移
    // @param [int] origin 取如下值 SEEK_CUR SEEK_END SEEK_SET
    // @return 实际设置的位置
    virtual s64 seek(s64 offset, int origin) = 0;

    // 获得流的大小, 动态改变, 该值每次调用可能会变大变小,取决于具体的实现
    // @return 流的大小
    virtual s64 size() const = 0;

    /** 设置流的大小
    @param [in] newSize 流的大小
    @return 实际分配的大小
    */
    virtual s64 resize(s64 newSize) = 0;

    // 从pos开始读取流,由于取到内存中因此不能超过2^31位 (会改变pos)
    // @param [out] buff 输出buff，内存需要自己管理
    // @param [in] count 期望读取的大小, buff的大小必须大于等于count
    // @return 读取的实际大小
    virtual s32 read(u8* const buff, s32 count) = 0;

    // 从pos开始写入流，一次性写入不能超过32位(会改变pos)
    // @param [in] buff 写入buff，内存需要自己管理
    // @param [in] count 期望写入的大小, buff的大小必须大于等于count
    // @return 写入的实际大小
    virtual s32 write(const u8* const buff, s32 count) = 0;
};

} // namespace NSP_DD
#endif // ddbase_stream_ddistream_h_
