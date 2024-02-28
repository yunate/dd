#ifndef test_stdafx_h_
#define test_stdafx_h_

#define  _CRT_SECURE_NO_WARNINGS
#define _CRT_RAND_S

// winsock
#define _WINSOCKAPI_
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

// std
#include <functional>
#include <memory>
#include <string>
#include <vector>

// windows
#include <malloc.h>
#include <tchar.h>
#include <windows.h>

#include "ddwin/ddmini_include.h"

// current
#include "ddbase/ddtest_case_factory.h"
#endif // test_stdafx_h_
