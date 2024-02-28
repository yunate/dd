#include "ddbase/stdafx.h"
#include "ddbase/network/http/ddhttp_utils.h"
#include "ddbase/str/ddstr.h"
namespace NSP_DD {

std::string ddhttp_utils::get_content_type(const std::string& suffix)
{
    if (ddstr::endwith(suffix.c_str(), "html")) {
        return "text/html; charset=utf-8";
    } else if (ddstr::endwith(suffix.c_str(), "json")) {
        return "application/json; charset=utf-8";
    } else if (ddstr::endwith(suffix.c_str(), "xml")) {
        return "application/xml; charset=utf-8";
    } else if (ddstr::endwith(suffix.c_str(), "jpeg")) {
        return "image/jpeg";
    } else if (ddstr::endwith(suffix.c_str(), "mp4")) {
        return "video/mp4";
    } else if (ddstr::endwith(suffix.c_str(), "pdf")) {
        return "application/pdf";
    } else if (ddstr::endwith(suffix.c_str(), "css")) {
        return "text/css; charset=utf-8";
    } else if (ddstr::endwith(suffix.c_str(), "js")) {
        return "application/javascript; charset=utf-8";
    } else {
        return "text/plain; charset=utf-8";
    }
}
} // namespace NSP_DD