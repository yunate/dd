
#include "ddtools/stdafx.h"

#include "ddbase/ddmini_include.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddio.h"

#include "ddtools/sln_maker/solusion_maker.h"

namespace NSP_DD {
    DDTEST(sln_maker, main)
    {
        ddsolusion_maker_param param;
        param.sln_name = L"dd";
        param.proj_name = L"ddpra";
        param.base_dir = ddpath::join(LR"--(E:\ddworkspace)--", param.sln_name);

        param.proj_type = ddproject_type::lib;
        param.mt_d = (u32)DDMT_D::MT | (u32)DDMT_D::MD;
        param.use_vcpkg = true;
        ddsolusion_maker maker(param);
        (void)maker.make();
    }
} // namespace NSP_DD

