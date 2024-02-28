
#include "ddtools/stdafx.h"

#include "ddbase/ddmini_include.h"
#include "ddbase/ddio.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddtools/project_file_adder/project_file_adder.h"

namespace NSP_DD {
    DDTEST(project_file_adder, main)
    {
        project_file_adder adder(LR"__(F:\My\ddwork_space\ddimage\projects\ddimage_base\ddimage_base.vcxproj)__", LR"__(F:\My\ddwork_space\ddimage\3rd\freeimage)__");
        if (adder.add()) {
            ddcout(ddconsole_color::green) << L"successful!\n";
        }
    }
} // namespace NSP_DD

