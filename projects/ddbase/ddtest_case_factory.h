
#ifndef ddbase_ddtest_case_factory_h_
#define ddbase_ddtest_case_factory_h_
#include "ddbase/ddsingleton.hpp"

#include <functional>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace NSP_DD {

class dditest_case {
public:
    virtual void run() = 0;
};

class ddtest_case_factory {
public:
    inline void add_case(const std::string& name, dditest_case* testCase)
    {
        m_cases[name].push_back(testCase);
    }

    inline void insert_white_type(const std::string& name)
    {
        m_white_type.insert(name);
    }

    inline void run()
    {
        for (auto& it : m_cases) {
            if (m_white_type.find(it.first) != m_white_type.end()) {
                for (size_t i = 0; i < it.second.size(); ++i) {
                    dditest_case* testCase = it.second[i];
                    testCase->run();
                }
            }
        }
    }

private:
    std::unordered_map<std::string, std::vector<dditest_case*>> m_cases;
    std::unordered_set<std::string> m_white_type;
};

#define DDTCF ddsingleton<ddtest_case_factory>::get_instance()

#define DDTEST(ty, N) \
class dd ## ty ## N ## _test_case : public dditest_case \
{ \
public: \
    dd ## ty ## N ## _test_case() \
    { \
        m_dummy; \
        DDTCF.add_case(#ty, &m_dummy); \
    } \
    virtual void run() override; \
private: \
    static dd ## ty ## N ## _test_case m_dummy; \
}; \
dd ## ty ## N ## _test_case dd ## ty ## N ## _test_case::m_dummy; \
void dd ## ty ## N ## _test_case::run()

} // namespace NSP_DD
#endif // ddbase_ddtest_case_factory_h_
