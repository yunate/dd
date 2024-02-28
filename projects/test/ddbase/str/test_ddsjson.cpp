#include "test/stdafx.h"

#include "ddbase/str/ddtree.h"
#include "ddbase/str/ddstr.h"

#include "ddbase/ddtest_case_factory.h"

namespace NSP_DD {

DDTEST(test_ddtree, 1)
{
    s32 level = 10;
    s32 current_level = 0;
    auto sjson = ddtree::create(ddstr::format("level_%d_%d", ++current_level, 0));
    for (; current_level < level; ++current_level) {
        sjson->add_child(ddstr::format("level_%d_%d", current_level, 1));
    }

    std::string ss = sjson->to_string();
    std::string expect = R"(level_1_0 {
  level_1_1
  level_2_1
  level_3_1
  level_4_1
  level_5_1
  level_6_1
  level_7_1
  level_8_1
  level_9_1
}
)";
    expect = ddstr::replace(expect.c_str(), "\r\n", "\n");
    DDASSERT(ss == expect);
    std::string ss1 = sjson->clone()->to_string();
    DDASSERT(ss == ss1);
}


DDTEST(test_ddtree, 2)
{
    s32 level = 10;
    s32 current_level = 0;
    auto sjson = ddtree::create(ddstr::format("level_%d_%d", ++current_level, 0));
    auto child = sjson;
    for (; current_level < level; ++current_level) {
        auto tmp = ddtree::create(ddstr::format("level_%d_%d", current_level, 1));
        child->add_child(tmp);
        child = tmp;
    }

    std::string ss = sjson->to_string();
    std::string expect = R"(level_1_0 {
  level_1_1 {
    level_2_1 {
      level_3_1 {
        level_4_1 {
          level_5_1 {
            level_6_1 {
              level_7_1 {
                level_8_1 {
                  level_9_1
                }
              }
            }
          }
        }
      }
    }
  }
}
)";
    expect = ddstr::replace(expect.c_str(), "\r\n", "\n");
    DDASSERT(ss == expect);
    std::string ss1 = sjson->clone()->to_string();
    DDASSERT(ss == ss1);
}
} // namespace NSP_DD
