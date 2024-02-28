#ifndef ddbase_str_ddtree_h_
#define ddbase_str_ddtree_h_

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include "ddbase/ddnocopyable.hpp"
#include <string>
#include <vector>
#include <memory>

namespace NSP_DD {
class ddtree
{
    DDNO_COPY_MOVE(ddtree);
public:
    std::shared_ptr<ddtree> clone();
    inline static std::shared_ptr<ddtree> create(const std::string& str)
    {
        std::shared_ptr<ddtree> sjson(new ddtree());
        sjson->m_str = str;
        return sjson;
    }

    inline void add_child(const std::shared_ptr<ddtree>& clild) { m_childs.push_back(clild); }
    inline void add_child(const std::string& str) { m_childs.push_back(create(str)); }
    inline void add_children(const std::vector<std::string>& strs)
    {
        for (const auto& str : strs) {
            m_childs.push_back(create(str));
        }
    }
    inline const std::vector<std::shared_ptr<ddtree>>& get_childs() { return m_childs; }
    inline void set_str(const std::string& str) { m_str = str; }
    inline std::string get_str() { return m_str; }

    std::string to_string();
    static std::shared_ptr<ddtree> from_string(const std::string& sjson);
private:
    ddtree() = default;
    std::string m_str;
    std::vector<std::shared_ptr<ddtree>> m_childs;
};
} // namespace NSP_DD
#endif // ddbase_str_ddstr_h_