
#pragma once

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <strsafe.h>
#include <shlobj.h> 
#include "shlwapi.h"
#pragma comment (lib, "Shlwapi.lib")

#include <gdiplus.h>
#pragma comment (lib, "Gdiplus.lib")
using namespace Gdiplus;

#include "wininet.h" 
#pragma comment(lib,"wininet.lib")

#include <time.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>