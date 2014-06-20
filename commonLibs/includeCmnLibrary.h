#pragma once

#pragma execution_character_set( "utf-8" )

// cmdDetectSetup 를 사용할 때 lib7Zip 을 이용하여 압축파일 등을 판단하려면 USE_LIB7ZIP_DETECT 를 선언한다

#ifdef USE_WMI_SUPPORT
    #ifndef USE_COM_SUPPORT
    #define USE_COM_SUPPORT
    #endif
#endif

#ifdef USE_COM_SUPPORT

    #include <comutil.h>

    #ifdef _DEBUG
        #pragma comment(lib, "comsuppwd")
    #else
        #pragma comment(lib, "comsuppw")
#endif

#endif

#ifdef USE_SHELL_SUPPORT
    #include <WinBase.h>
    #include <ShlObj.h>
    #include <Shlwapi.h>
    #include <Shellapi.h>
    #pragma comment(lib, "shlwapi")
    #pragma comment(lib, "shell32")
#endif

#ifdef USE_NETWORK_SUPPORT
    #include <WinSock2.h>
    #pragma comment(lib, "ws2_32")
    #include <UrlMon.h>
    #pragma comment(lib, "urlmon" )
    #include <WinInet.h>
    #pragma comment(lib, "wininet")
    #include <IPHlpApi.h>
    #pragma comment(lib, "iphlpapi" )
#endif

#ifdef USE_OLE_SUPPORT
    #include <objbase.h>
    #pragma comment(lib, "ole32")
#endif

#ifdef USE_SERVICE_SUPPORT
    #include <WinSvc.h>
#endif

#ifdef USE_FILEVER_CERTINFO_SUPPORT
    #include <Winver.h>
    #pragma comment(lib, "version")
    #include <WinCrypt.h>
    #include <SoftPub.h>
    #include <WinTrust.h>
    #pragma comment( lib, "WinTrust" )
    #include <WinCrypt.h>
    #pragma comment( lib, "crypt32" )
#endif

#include "cmnUtils.h"
#include "cmnWinUtils.h"
#include "cmnPath.h"
#include "cmnConcurrent.h"
#include "cmnNetwork.h"
#include "cmnDateTime.h"
#include "cmnConverter.h"
#include "cmnFormatter.hpp"
#include "cmnLogger.hpp"
#include "cmnPCInfo.h"

#ifdef USE_WMI_SUPPORT
    #include "cmnWMI.h"
#endif
