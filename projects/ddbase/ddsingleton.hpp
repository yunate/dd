#ifndef ddbase_ddsingleton_hpp_
#define ddbase_ddsingleton_hpp_

#include "ddbase/dddef.h"
namespace NSP_DD {

// 为了能够让T类的构造函数写成protect
template<class T>
class ddsingleton_wrapper : public T {};

// 饿汉模式
template <class T>
class ddsingleton
{
public:
    static T& get_instance()
    {
        // 为了能够在main函数前初始化s_pInstance
        s_pInstance;
        static ddsingleton_wrapper<T> instance;
        return *static_cast<T *>(&instance);
    }

private:
    ddsingleton() {}
    ddsingleton(const ddsingleton&) {}
    ddsingleton& operator = (const ddsingleton&) {}
    ~ddsingleton() {}

    // 强制饿汉模式，这种写法主要是为了防止初始化依赖问题
    static T* s_pInstance;
};

template<class T>
T* ddsingleton<T>::s_pInstance = &ddsingleton<T>::get_instance();

} // namespace NSP_DD
#endif // ddbase_ddsingleton_hpp_
