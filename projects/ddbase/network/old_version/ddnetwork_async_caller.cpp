#include "ddbase/stdafx.h"
#include "ddbase/network/ddnetwork_async_caller.hpp"
namespace NSP_DD {
std::function<void(const std::function<void()>&)> ddnetwork_async_caller::s_caller = nullptr;
bool ddnetwork_async_caller::s_has_set_async = false;
} // namespace NSP_DD
