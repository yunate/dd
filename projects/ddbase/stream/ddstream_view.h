
#ifndef ddbase_stream_ddstream_view_h_
#define ddbase_stream_ddstream_view_h_

#include "ddbase/dddef.h"
#include "ddbase/stream/ddistream.h"
#include "ddbase/network/ddtls.h"
#include <memory>
#include <vector>

namespace NSP_DD {
class ddistream_view
{
public:
    virtual ~ddistream_view() { };

    // 获得当前位置 seek
    // @return 流当前位置, 小于0表示异常
    virtual s64 pos() const = 0;

    // 获得流的大小, 动态改变, 该值每次调用可能会变大变小,取决于具体的实现
    // @return 流的大小
    virtual s64 size() const = 0;

    // 从pos开始读取流,由于取到内存中因此不能超过2^31位 (会改变pos)
    // @param [out] buff 输出buff，内存需要自己管理
    // @param [in] count 期望读取的大小, buff的大小必须大于等于count
    // @return 读取的实际大小
    virtual s32 read(u8* const buff, s32 count) = 0;

    // 设置流当前位置
    // @param [int] offset 偏移
    // @param [int] origin 取如下值 SEEK_CUR SEEK_END SEEK_SET
    // @return 实际设置的位置
    virtual s64 seek(s64 offset, int origin) = 0;
};

class ddstream_view : public ddistream_view
{
public:
    static std::unique_ptr<ddistream_view> from(void* buff, s32 buff_size);
    static std::unique_ptr<ddistream_view> from(ddistream* stream_view);

    // 如果stream_view的数据需要加密, 可以使用这个函数, tls不能为nullptr
    static std::unique_ptr<ddistream_view> from(ddistream_view* stream_view, ddtls* tls);

    // good e.g.
    //   std::vector<std::unique_ptr<ddistream_view>> views;
    //   views.emplace_back(ddstream_view::from(nullptr, 0));
    //   ddstream_view::from(std::move(views));
    // not good e.g.
    //  ddstream_view::from({ ddstream_view::from(nullptr, 0) });
    static std::unique_ptr<ddistream_view> from(std::vector<std::unique_ptr<ddistream_view>>&& stream_views);
};
} // namespace NSP_DD
#endif // ddbase_stream_ddstream_view_h_
