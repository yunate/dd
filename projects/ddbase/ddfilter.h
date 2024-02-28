#ifndef ddbase_ddfilter_h_
#define ddbase_ddfilter_h_
#include "ddbase/dddef.h"

#include <functional>
namespace NSP_DD {

template<class F, class F1>
class ddcluser_with_filter
{
public:
     ddcluser_with_filter(F callable, F1 filter) : m_callable(callable), m_filter(filter) { }

     template<typename ...V>
     inline auto operator()(V&&... a)
     {
         using R = decltype(m_callable(std::forward<V>(a)...));
         if (m_filter(std::forward<V>(a)...)) {
             return m_callable(std::forward<V>(a)...);
         }
         return R();
     }

     inline ddcluser_with_filter& operator*()
     {
         return *this;
     }

private:
    F m_callable;
    F1 m_filter;
};

template<class F, class F1>
inline auto create_ddcluser_with_filter(F callable, F1 filter)
{
    return ddcluser_with_filter<F, F1>(callable, filter);
}

} // namespace NSP_DD
#endif // ddbase_ddfilter_h_
