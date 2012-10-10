// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define TEST_EXE_INCLUDE

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <Shlwapi.h>
#pragma comment( lib, "shlwapi.lib" )

#include "Library\wcxhead.h"
#include "Library\Ark.h"
#include "Library\ArkLib.h"

#include "Library\CommonUtil.h"
#include "Library\ConvertStr.h"
#include "Library\CommonLogger.h"


#include <Shlwapi.h>

#pragma comment( lib, "shlwapi" )


// TODO: reference additional headers your program requires here
