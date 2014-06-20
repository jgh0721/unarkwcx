#include "stdafx.h"

#include <string>
#include <vector>

#include "cmnUtils.h"
#ifdef USE_WMI_SUPPORT
    #ifndef USE_COM_SUPPORT
    #define USE_COM_SUPPORT
    #endif
#include "cmnWMI.h"
using namespace nsCommon::nsCmnWMI;
#endif
#include "cmnConverter.h"
#include "cmnWinUtils.h"
#include "cmnFormatter.hpp"

#pragma warning( disable: 4996 )

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace detail
    {

    }

    CBenchTimer::CBenchTimer()
        : _checkTimeStart( 0 ), _checkTimePrev( 0 ), _checkTimeFreq( 0 ), _checkTimeCurrent( 0 ), _checkTimeRun( 0.0 )
    {
        ::QueryPerformanceFrequency( (_LARGE_INTEGER*)&_checkTimeFreq );
        if( _checkTimeFreq > 0 )
            ::QueryPerformanceCounter( (_LARGE_INTEGER*)&_checkTimeStart );
    }

    CBenchTimer::~CBenchTimer()
    {

    }

    void CBenchTimer::step()
    {
        if( _checkTimeCurrent > 0 )
            _checkTimePrev = _checkTimeCurrent;
        ::QueryPerformanceCounter( (_LARGE_INTEGER*)&_checkTimeCurrent );
    }

    float CBenchTimer::getElapsedTimeFromPrev()
    {
        if( _checkTimePrev == 0 )
            return getElapsedTimeFromStart();

        return _checkTimeRun = (float)((double)(_checkTimeCurrent - _checkTimePrev) / _checkTimeFreq * 1000);
    }

    float CBenchTimer::getElapsedTimeFromStart()
    {
        return _checkTimeRun = (float)((double)(_checkTimeCurrent - _checkTimeStart) / _checkTimeFreq * 1000);
    }

#ifdef _MSC_VER
    void SetThreadName( DWORD dwThreadID, LPCSTR pszThreadName )
    {
        const DWORD MS_VC_EXCEPTION = 0x406D1388;

        detail::THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = pszThreadName;
        info.dwThreadID = dwThreadID;
        info.dwFlags = 0;

        __try
        {
            RaiseException( MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (const ULONG_PTR*)&info );
        }
        __except( EXCEPTION_CONTINUE_EXECUTION )
        {
        }

    }
#endif

    bool is64BitWindow()
    {
#if defined(_WIN64)
        return true;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
        // 32-bit programs run on both 32-bit and 64-bit Windows
        // so must sniff

        typedef BOOL( WINAPI *LPFN_ISWOW64PROCESS ) (HANDLE, PBOOL);

        BOOL b64BitWindows = FALSE;

        HMODULE hKernel32 = NULL;
        hKernel32 = LoadLibrary( L"kernel32.dll" );

        if( hKernel32 == NULL )	//Kernel32.dll 로드 실패시 FALSE로 간주함.
            return b64BitWindows == TRUE ? true : false;

        BOOL f64 = FALSE;
        detail::LPFN_ISWOW64PROCESS fnIsWow64Process;

        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( hKernel32, "IsWow64Process" );
        if( NULL != fnIsWow64Process )
            b64BitWindows = (fnIsWow64Process( GetCurrentProcess(), &f64 ) && f64);

        if( hKernel32 != NULL )
            FreeLibrary( hKernel32 );

        return b64BitWindows == TRUE ? true : false;
#else
        return false; // Win64 does not support Win16
#endif
    }

    bool is64BitProcessor()
    {
        //Windows 2000 이하의 제품군은 64bit 가 존재 하지 않으므로 무조건 32bit 동작
        SYSTEM_INFO sysInfo;
        memset( &sysInfo, '\0', sizeof(sysInfo) );

        GetNativeSystemInfo( &sysInfo );

        return sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? true : false;
    }

    BOOL EnablePrivilege( LPCTSTR szPrivilege )
    {
        BOOL bResult = FALSE;
        HANDLE hToken = NULL;
        TOKEN_PRIVILEGES tpOld, tpCurrent;

        if( !OpenProcessToken( ::GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken ) )
            return bResult;

        tpCurrent.PrivilegeCount = 1;
        tpCurrent.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;

        if( ::LookupPrivilegeValue( NULL, szPrivilege, &tpCurrent.Privileges[ 0 ].Luid ) )
        {
            DWORD dwOld = sizeof(TOKEN_PRIVILEGES);
            if( ::AdjustTokenPrivileges( hToken, FALSE, &tpCurrent, dwOld, &tpOld, &dwOld ) )
                bResult = TRUE;
            else
                bResult = FALSE;
        }
        else
            bResult = FALSE;

        CloseHandle( hToken );
        return bResult;
    }

    std::wstring getOSVer()
    {
        OSVERSIONINFOEX osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if( GetVersionEx((OSVERSIONINFO*) &osvi) == FALSE )
            return L"0.0";

        return format( L"%d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion );
    }

    std::wstring getOSDisplayName()
    {
        std::wstring osDPName;

        const int BUFSIZE = 256;

        typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
        typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

        OSVERSIONINFOEX osvi;
        SYSTEM_INFO si;
        PGNSI pGNSI;
        PGPI pGPI;
        DWORD dwType = 0;

        ZeroMemory(&si, sizeof(SYSTEM_INFO));
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if( GetVersionEx( (OSVERSIONINFO*)&osvi ) == FALSE )
            return L"Unknown OS";

        // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

        pGNSI = (PGNSI)GetProcAddress( GetModuleHandle( TEXT( "kernel32.dll" ) ), "GetNativeSystemInfo" );
        if( NULL != pGNSI )
            pGNSI( &si );
        else
            GetSystemInfo( &si );

        if( (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) ||
            (osvi.dwMajorVersion <= 4) )
            return L"Windows 9x Family";

        osDPName += L"Microsoft ";

        if( osvi.dwMajorVersion == 6 )
        {
            switch( osvi.dwMinorVersion )
            {
                case 0:
                    if( osvi.wProductType == VER_NT_WORKSTATION )
                        osDPName += L"Windows Vista ";
                    else
                        osDPName += L"Windows Server 2008 ";
                    break;
                case 1:
                    if( osvi.wProductType == VER_NT_WORKSTATION )
                        osDPName += L"Windows 7 ";
                    else
                        osDPName += L"Windows Server 2008 R2 ";
                    break;
                case 2:
                    if( osvi.wProductType == VER_NT_WORKSTATION )
                        osDPName += L"Windows 8 ";
                    else
                        osDPName += L"Windows Server 2012";
                    break;
            }

            pGPI = (PGPI)GetProcAddress( GetModuleHandle( TEXT( "kernel32.dll" ) ), "GetProductInfo" );

            pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType );

            switch( dwType )
            {
                case PRODUCT_ULTIMATE:
                    osDPName += L"Ultimate Edition";
                    break;
                case 0x00000030: // PRODUCT_PROFESSIONAL
                    osDPName += L"Professional";
                    break;
                case PRODUCT_HOME_PREMIUM:
                    osDPName += L"Home Premium Edition";
                    break;
                case PRODUCT_HOME_BASIC:
                    osDPName += L"Home Basic Edition";
                    break;
                case PRODUCT_ENTERPRISE:
                    osDPName += L"Enterprise Edition";
                    break;
                case PRODUCT_BUSINESS:
                    osDPName += L"Business Edition";
                    break;
                case PRODUCT_STARTER:
                    osDPName += L"Starter Edition";
                    break;
                case PRODUCT_CLUSTER_SERVER:
                    osDPName += L"Cluster Server Edition";
                    break;
                case PRODUCT_DATACENTER_SERVER:
                    osDPName += L"Datacenter Edition";
                    break;
                case PRODUCT_DATACENTER_SERVER_CORE:
                    osDPName += L"Datacenter Edition (core installation)";
                    break;
                case PRODUCT_ENTERPRISE_SERVER:
                    osDPName += L"Enterprise Edition";
                    break;
                case PRODUCT_ENTERPRISE_SERVER_CORE:
                    osDPName += L"Enterprise Edition (core installation)";
                    break;
                case PRODUCT_ENTERPRISE_SERVER_IA64:
                    osDPName += L"Enterprise Edition for Itanium-based Systems";
                    break;
                case PRODUCT_SMALLBUSINESS_SERVER:
                    osDPName += L"Small Business Server";
                    break;
                case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                    osDPName += L"Small Business Server Premium Edition";
                    break;
                case PRODUCT_STANDARD_SERVER:
                    osDPName += L"Standard Edition";
                    break;
                case PRODUCT_STANDARD_SERVER_CORE:
                    osDPName += L"Standard Edition (core installation)";
                    break;
                case PRODUCT_WEB_SERVER:
                    osDPName += L"Web Server Edition";
                    break;
            }
        }
        else if( (osvi.dwMajorVersion == 5) &&
                 (osvi.dwMinorVersion == 2) )
        {
#if _WIN32_WINNT == 0x0500
#define SM_SERVERR2 89
#endif
            if( GetSystemMetrics( SM_SERVERR2 ) )
                osDPName += L"Windows Server 2003 R2, ";
            else if( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
                osDPName += L"Windows Storage Server 2003";
            else if( osvi.wSuiteMask & 0x00008000 /* VER_SUITE_WH_SERVER */ )
                osDPName += L"Windows Home Server";
            else if( osvi.wProductType == VER_NT_WORKSTATION &&
                     si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
                     osDPName += L"Windows XP Professional x64 Edition";
            else
                osDPName += L"Windows Server 2003, ";

            // Test for the server type.
            if( osvi.wProductType != VER_NT_WORKSTATION )
            {
                if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
                {
                    if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        osDPName += L"Datacenter Edition for Itanium-based Systems";
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        osDPName += L"Enterprise Edition for Itanium-based Systems";
                }
                else if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
                {
                    if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        osDPName += L"Datacenter x64 Edition";
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        osDPName += L"Enterprise x64 Edition";
                    else
                        osDPName += L"Standard x64 Edition";
                }
                else
                {
                    if( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                        osDPName += L"Compute Cluster Edition";
                    else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        osDPName += L"Datacenter Edition";
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        osDPName += L"Enterprise Edition";
                    else if( osvi.wSuiteMask & VER_SUITE_BLADE )
                        osDPName += L"Web Edition";
                    else
                        osDPName += L"Standard Edition";
                }
            }
        }
        else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
        {
            osDPName += L"Windows XP ";
            if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                osDPName += L"Home Edition";
            else
                osDPName += L"Professional";
        }
        else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
        {
            osDPName += L"Windows 2000 ";

            if( osvi.wProductType == VER_NT_WORKSTATION )
            {
                osDPName += L"Professional";
            }
            else
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                    osDPName += L"Datacenter Server";
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                    osDPName += L"Advanced Server";
                else
                    osDPName += L"Server";
            }
        }

        if( wcslen( osvi.szCSDVersion ) > 0 )
        {
            osDPName += L" ";
            osDPName += osvi.szCSDVersion;
        }

        osDPName += format( L" (build %d", osvi.dwBuildNumber );

        if( osvi.dwMajorVersion >= 6 )
        {
            if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
                osDPName += L", 64-bit";
            else if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
                osDPName += L", 32-bit";
        }

        return osDPName;
    }

    std::wstring	getInternetExplorerVersion()
    {
        WCHAR ieVersion[ 64 ] = { 0, };
        DWORD dwVersion = _countof( ieVersion ) * sizeof(WCHAR);
        HKEY hKEY = NULL;

        do
        {
            LONG lRet = ERROR_SUCCESS;

            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_READ, &hKEY );
            if( lRet != ERROR_SUCCESS )
                break;

            lRet = RegQueryValueEx( hKEY, L"svcVersion", NULL, NULL, (LPBYTE)ieVersion, &dwVersion );
            if( lRet == ERROR_SUCCESS )
                break;

            memset( ieVersion, '\0', sizeof(WCHAR)* _countof( ieVersion ) );

            lRet = RegQueryValueEx( hKEY, L"Version", NULL, NULL, (LPBYTE)ieVersion, &dwVersion );
            if( lRet == ERROR_SUCCESS )
                break;

        } while( false );

        if( hKEY != NULL )
            RegCloseKey( hKEY );

        return ieVersion;
    }

    std::wstring getEnvironmentVariable( const std::wstring& strEnvName )
    {
        if( strEnvName.empty() )
            return L"";

        WCHAR wszEnvBuffer[ _MAX_ENV ] = { 0, };
        DWORD dwRet = GetEnvironmentVariableW( strEnvName.c_str(), wszEnvBuffer, _MAX_ENV );
        if( dwRet == 0 )
            return L"";

        return wszEnvBuffer;
    }

#ifdef _SFC_
    bool IsProtectedFileByOS( const std::wstring& fileName )
    {
        return SfcIsFileProtected( NULL, fileName.c_str() ) != FALSE ? true : false;
    }
#endif

    bool InCompressUsing7ZIP( const std::wstring& strPath, const std::wstring& strTargetFile, const std::wstring& strinCompressFileName, const std::wstring& str7zipPath )
    {
        bool isResult = false;

        std::wstring str7zFile;

        if( *str7zipPath.rbegin() == L'\\' || *str7zipPath.rbegin() == L'/' )
            str7zFile = nsCmnFormatter::tsFormat( L"\"%1%2\"", str7zipPath, L"7za.exe" );
        else
            str7zFile = nsCmnFormatter::tsFormat( L"\"%1\\%2\"", str7zipPath, L"7za.exe" );

        std::wstring strParam = nsCmnFormatter::tsFormat( L" a -y \"%1\" \"%2\"", strinCompressFileName, strTargetFile );

        std::wstring strtmp = nsCmnFormatter::tsFormat( L"%1 %2", str7zFile, strParam );

        WCHAR* wsz7zFile = _wcsdup( strtmp.c_str() );

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if( CreateProcessW( NULL,   // No module name (use command line). 
            wsz7zFile, // Command line. 
            NULL,             // Process handle not inheritable. 
            NULL,             // Thread handle not inheritable. 
            FALSE,            // Set handle inheritance to FALSE. 
            0,                // No creation flags. 
            NULL,             // Use parent's environment block. 
            NULL,             // Use parent's starting directory. 
            &si,              // Pointer to STARTUPINFO structure.
            &pi              // Pointer to PROCESS_INFORMATION structure.
            ) )
        {
            isResult = true;
        }
        else
        {
            free( wsz7zFile );

            return isResult;
        }

        WaitForSingleObject( pi.hProcess, INFINITE );

        if( wsz7zFile )
            free( wsz7zFile );

        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );

        return isResult;
    }

    bool DeCompressUsing7ZIP( const std::wstring& strPath, const std::wstring& strFileName, const std::wstring& str7zipPath, const std::wstring& strPassword /* = L"dkdlahsdbwj!@#" */ )
    {
        bool isResult = false;

        std::wstring str7zFile;
        std::wstring strParam;

        if( *str7zipPath.rbegin() == L'\\' || *str7zipPath.rbegin() == L'/' )
            str7zFile = nsCmnFormatter::tsFormat( L"\"%1%2\"", str7zipPath, L"7za.exe" );
        else
            str7zFile = nsCmnFormatter::tsFormat( L"\"%1\\%2\"", str7zipPath, L"7za.exe" );

        if( strPassword.empty() == false )
            strParam = nsCmnFormatter::tsFormat( L" x -y -p%1 -o\"%2\" \"%2\\%3", strPassword, strPath, strFileName );
        else
            strParam = nsCmnFormatter::tsFormat( L" x -y -o\"%1\" \"%1\\%2", strPath, strFileName );

        std::wstring strtmp = str7zFile + strParam;

        strtmp = nsCommon::nsCmnConvert::string_replace_all( strtmp, L"/", L"\\" );
        WCHAR* wsz7zFile = new WCHAR[ strtmp.size() + 1 ];
        wcscpy_s( wsz7zFile, strtmp.size() + 1, strtmp.c_str() );

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        do
        {
            if( CreateProcessW( NULL,   // No module name (use command line). 
                wsz7zFile, // Command line. 
                NULL,             // Process handle not inheritable. 
                NULL,             // Thread handle not inheritable. 
                FALSE,            // Set handle inheritance to FALSE. 
                0,                // No creation flags. 
                NULL,             // Use parent's environment block. 
                NULL,             // Use parent's starting directory. 
                &si,              // Pointer to STARTUPINFO structure.
                &pi              // Pointer to PROCESS_INFORMATION structure.
                ) == FALSE )
            {
                break;
            }

            WaitForSingleObject( pi.hProcess, INFINITE );

            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );

            isResult = true;
        } while( false );

        if( wsz7zFile )
            delete[] wsz7zFile;

        return isResult;
    }

    bool DeCompressUsing7ZIP( const std::wstring& strPath, const std::wstring& strFileName, const std::wstring& str7zipPath, const std::wstring& strUncompressPath, const std::wstring& strPassword /*= L"dkdlahsdbwj!@#" */ )
    {
        bool isResult = false;

        std::wstring str7zFile;
        std::wstring strParam;

        if( *str7zipPath.rbegin() == L'\\' || *str7zipPath.rbegin() == L'/' )
            str7zFile = nsCmnFormatter::tsFormat( L"\"%1%2\"", str7zipPath, L"7za.exe" );
        else
            str7zFile = nsCmnFormatter::tsFormat( L"\"%1\\%2\"", str7zipPath, L"7za.exe" );

        if( strPassword.empty() == false )
            strParam = nsCmnFormatter::tsFormat( L" x -y -p%1 -o\"%4\" \"%2\\%3", strPassword, strPath, strFileName, strUncompressPath );
        else
            strParam = nsCmnFormatter::tsFormat( L" x -y -o\"%3\" \"%1\\%2", strPath, strFileName, strUncompressPath );

        std::wstring strtmp = str7zFile + strParam;

        strtmp = nsCommon::nsCmnConvert::string_replace_all( strtmp, L"/", L"\\" );
        WCHAR* wsz7zFile = new WCHAR[ strtmp.size() + 1 ];
        wcscpy_s( wsz7zFile, strtmp.size() + 1, strtmp.c_str() );

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        do
        {
            if( CreateProcessW( NULL,   // No module name (use command line). 
                wsz7zFile, // Command line. 
                NULL,             // Process handle not inheritable. 
                NULL,             // Thread handle not inheritable. 
                FALSE,            // Set handle inheritance to FALSE. 
                0,                // No creation flags. 
                NULL,             // Use parent's environment block. 
                NULL,             // Use parent's starting directory. 
                &si,              // Pointer to STARTUPINFO structure.
                &pi              // Pointer to PROCESS_INFORMATION structure.
                ) == FALSE )
            {
                break;
            }

            WaitForSingleObject( pi.hProcess, INFINITE );

            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );

            isResult = true;
        } while( false );

        if( wsz7zFile )
            delete[] wsz7zFile;

        return isResult;
    }

    bool WriteEventLog( const std::wstring& programName, const std::wstring& logMessage, WORD eventType, WORD eventCatecory, DWORD eventID )
    {
        bool isWriteSuccess = false;

        HANDLE hEvent = NULL;
        PSID currentSID = NULL;

        hEvent = RegisterEventSourceW( NULL, programName.c_str() );

        if( hEvent == NULL )
            return isWriteSuccess;

#ifdef _AFX
        ATL::CAccessToken accessToken;
        ATL::CSid currentUserSid;
        if( accessToken.GetProcessToken( TOKEN_READ | TOKEN_QUERY ) && accessToken.GetUser( &currentUserSid ) )
            ConvertStringSidToSid( currentUserSid.Sid(), &currentSID );
#endif
        LPCWSTR pInsertStrings[ 1 ] = { NULL };
        pInsertStrings[ 0 ] = (LPCWSTR)logMessage.c_str();

        if( ReportEventW( hEvent, eventType, eventCatecory, eventID, currentSID, 1, 0, pInsertStrings, NULL ) == TRUE )
            isWriteSuccess = true;

        if( hEvent != NULL )
            DeregisterEventSource( hEvent );

        if( currentSID != NULL )
            LocalFree( currentSID );

        return isWriteSuccess;
    }

#ifdef USE_OLE_SUPPORT
    std::string GetNewGUID()
    {
        GUID guid;
        std::string formattedGUID;
        if( SUCCEEDED( CoCreateGuid( &guid ) ) )
        {
            formattedGUID =
                format( "%.8X-%.4X-%.4X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X",
                guid.Data1, guid.Data2, guid.Data3, guid.Data4[ 0 ],
                guid.Data4[ 1 ], guid.Data4[ 2 ], guid.Data4[ 3 ], guid.Data4[ 4 ],
                guid.Data4[ 5 ], guid.Data4[ 6 ], guid.Data4[ 7 ]
                );
        }

        return formattedGUID;
    }
#endif

    void sweepNotificationIcon()
    {
        HWND h = FindWindow( L"Shell_TrayWnd", NULL );

        if( h )
        {
            if( (h = FindWindowEx( h, NULL, L"TrayNotifyWnd", NULL )) != NULL )
            {
                if( FindWindowEx( h, NULL, L"ToolbarWindow32", NULL ) != NULL )
                {
                    RECT r;
                    long x;

                    GetWindowRect( h, &r );

                    int height = 16;
                    int loop = (r.bottom - r.top) / height;

                    for( int i = 0; i < loop; i++ )
                    {
                        for( x = r.left; x < r.right; x += 16 )
                        {
                            SendMessage( h, WM_MOUSEMOVE, 0, (height * i) * 0x10000 + x - r.left );
                        }
                    }

                }

                if( FindWindowEx( h, NULL, L"SysPager", NULL ) != NULL )
                {
                    h = FindWindowEx( h, NULL, L"SysPager", NULL );
                    h = FindWindowEx( h, NULL, L"ToolbarWindow32", NULL );

                    RECT r;
                    long x;

                    GetWindowRect( h, &r );

                    int height = 16;
                    int loop = (r.bottom - r.top) / height;

                    for( int i = 0; i < loop; i++ )
                    {
                        for( x = r.left; x < r.right; x += 16 )
                        {
                            SendMessage( h, WM_MOUSEMOVE, 0, (height * i) * 0x10000 + x - r.left );
                        }
                    }
                }
                else
                {
                    h = FindWindowEx( h, NULL, L"ToolbarWindow32", NULL );

                    RECT r;
                    long x;

                    GetWindowRect( h, &r );

                    int height = 16;
                    int loop = (r.bottom - r.top) / height;

                    for( int i = 0; i < loop; i++ )
                    {
                        for( x = r.left; x < r.right; x += 16 )
                        {
                            SendMessage( h, WM_MOUSEMOVE, 0, (height * i) * 0x10000 + x - r.left );
                        }
                    }

                }
            }

            //숨겨진 아이콘 표시 잔상 제거
            h = FindWindow( L"NotifyIconOverflowWindow", NULL );

            if( h )
            {
                if( (h = FindWindowEx( h, NULL, L"ToolbarWindow32", NULL )) != NULL )
                {
                    RECT r;
                    long x;

                    GetWindowRect( h, &r );

                    int height = 16;
                    int loop = (r.bottom - r.top) / height;

                    for( int i = 0; i < loop; i++ )
                    {
                        for( x = r.left; x < r.right; x += 16 )
                        {
                            SendMessage( h, WM_MOUSEMOVE, 0, (height * i) * 0x10000 + x - r.left );
                        }
                    }

                }

            }
        }
    }

    void SetFroegroundWindowForce( HWND hWnd )
    {
        HWND hFHwnd;
        DWORD dwID, dwFid;

        hFHwnd = GetForegroundWindow();
        if( hFHwnd == hWnd )
            return;

        dwFid = GetWindowThreadProcessId( hFHwnd, NULL );
        dwID = GetWindowThreadProcessId( hWnd, NULL );
        if( AttachThreadInput( dwFid, dwID, TRUE ) )
        {
            SetForegroundWindow( hWnd );
            BringWindowToTop( hWnd );
            SetFocus( hWnd );
            AttachThreadInput( dwFid, dwID, FALSE );
        }
    }

    std::vector< HWND > getWindHandle( DWORD dwProcessID )
    {
        std::vector<HWND> vecHwnd;
        vecHwnd.clear();

        HWND tempHwnd = FindWindow( NULL, NULL );

        while( tempHwnd != NULL )
        {
            ULONG idProc;
            GetWindowThreadProcessId( tempHwnd, &idProc );

            if( dwProcessID == idProc )
            {
                vecHwnd.push_back( tempHwnd );
            }

            tempHwnd = GetWindow( tempHwnd, GW_HWNDNEXT );
        }
        return vecHwnd;
    }

    std::vector< std::pair< HWND, std::wstring > > CWindowList::GetWindowList()
    {
        _vecWindowList.clear();

        EnumChildWindows( GetDesktopWindow(), (WNDENUMPROC)CWindowList::enumChildWindowProc, (LPARAM)this );

        return _vecWindowList;
    }

    BOOL CALLBACK CWindowList::enumChildWindowProc( HWND hWnd, LPARAM lParam )
    {
        CWindowList* pWindwList = reinterpret_cast<CWindowList*>(lParam);

        WCHAR	szBuffer[ 1024 + 1 ] = { 0, };

        GetWindowTextW( hWnd, szBuffer, 1024 );

        pWindwList->_vecWindowList.push_back( std::make_pair( hWnd, (std::wstring)szBuffer ) );

        return TRUE;
    }

    HWND CWindowList::isExistWindow( DWORD processID, std::wstring windowText )
    {
        DWORD dwPID = 0;

        for( std::vector< std::pair< HWND, std::wstring > >::iterator It = _vecWindowList.begin(); It != _vecWindowList.end(); ++It )
        {
            GetWindowThreadProcessId( It->first, &dwPID );

            if( dwPID != processID )
                continue;

            if( wcsstr( It->second.c_str(), windowText.c_str() ) != NULL )
                return It->first;
        }

        return NULL;
    }

    int CWindowList::getWindowState( HWND hWnd )
    {
        if( hWnd == NULL )
            return 0;

        int nState = 1;

        // Is it visible?
        if( IsWindowVisible( hWnd ) )
            nState |= 2;

        // Is it enabled?
        if( IsWindowEnabled( hWnd ) )
            nState |= 4;

        // Is it active?
        if( GetForegroundWindow() == hWnd )
            nState |= 8;

        // Is it minimized?
        if( IsIconic( hWnd ) )
            nState |= 16;

        // Is it maximized?
        if( IsZoomed( hWnd ) )
            nState |= 32;

        return nState;
    }

    std::wstring QueryRegValueWithStr( HKEY rootKey, const std::wstring& subkey, const std::wstring& valueName, bool isRead32Key /*= true */ )
    {
        std::vector< wchar_t > retVal;
        HKEY key = NULL;

        do
        {
            DWORD dwFlags = KEY_READ;

            if( is64BitWindow() == true )
            {
                if( isRead32Key == true )
                    dwFlags |= KEY_WOW64_32KEY;
                else
                    dwFlags |= KEY_WOW64_64KEY;
            }

            if( RegOpenKeyExW( rootKey, subkey.c_str(), 0, dwFlags, &key ) != ERROR_SUCCESS )
                break;

            DWORD dwDataLength = 0;
            if( RegQueryValueExW( key, valueName.c_str(), NULL, NULL, NULL, &dwDataLength ) != ERROR_SUCCESS )
                break;

            retVal.reserve( dwDataLength + 1 );
            retVal.resize( dwDataLength + 1 );

            if( RegQueryValueExW( key, valueName.c_str(), NULL, NULL, (LPBYTE)&retVal[ 0 ], &dwDataLength ) != ERROR_SUCCESS )
                break;

        } while( false );

        if( key != NULL )
            RegCloseKey( key );

        if( retVal.empty() == true )
            return L"";

        return std::wstring( &retVal[ 0 ] );
    }

    void SetRegValueWithStr( HKEY rootKey, const std::wstring& subkey, const std::wstring& valueName, const std::wstring& valueData, bool isWrite32Key /*= true */ )
    {
        HKEY key = NULL;

        do
        {
            DWORD dwFlags = KEY_WRITE;

            if( is64BitWindow() == true )
            {
                if( isWrite32Key == true )
                    dwFlags |= KEY_WOW64_32KEY;
                else
                    dwFlags |= KEY_WOW64_64KEY;
            }

            if( RegOpenKeyExW( rootKey, subkey.c_str(), 0, dwFlags, &key ) != ERROR_SUCCESS )
                break;

            // SetValueEx 를 할 때 길이는 NULL 문자를 포함한 길이를 바이트 단위로 넘겨야하기 때문에 +1 을 해야함
            RegSetValueExW( key, valueName.c_str(), 0, REG_SZ, (PBYTE)(valueData.c_str()), sizeof(WCHAR)* (DWORD)((valueData.size() + 1)) );

        } while( false );

        if( key != NULL )
            RegCloseKey( key );
    }

    std::wstring QueryOSStringFromRegistry()
    {
        const wchar_t* UNKNOWN_OS = L"Unknown OS";

        std::wstring wsResult = UNKNOWN_OS;

        wsResult = QueryRegValueWithStr( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName", is64BitWindow() == true ? false : true );
        if( wsResult.empty() == true )
            wsResult = UNKNOWN_OS;

        return wsResult;
    }

    std::wstring QueryOSStringFromREG()
    {
        OSVERSIONINFOEX osi;
        ZeroMemory( &osi, sizeof(osi) );
        osi.dwOSVersionInfoSize = sizeof(osi);

        GetVersionEx( (LPOSVERSIONINFO)&osi );

        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T( "Windows NT 4.0" );

        if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T( "Windows 2000" );

        if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 1 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T( "Windows XP" );

        if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 2 && (GetSystemMetrics( SM_SERVERR2 ) == 0) )
            return _T( "Windows 2003" );

        if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 2 && (GetSystemMetrics( SM_SERVERR2 ) != 0) )
            return _T( "Windows 2003 R2" );

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 0 && osi.wProductType == VER_NT_WORKSTATION )
            return _T( "Windows Vista" );

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 0 && osi.wProductType != VER_NT_WORKSTATION )
            return _T( "Windows Server 2008" );

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 1 && osi.wProductType == VER_NT_WORKSTATION )
            return _T( "Windows 7" );

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 1 && osi.wProductType != VER_NT_WORKSTATION )
            return _T( "Windows Server 2008 R2" );

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 2 && osi.wProductType == VER_NT_WORKSTATION )
            return _T( "Windows 8" );

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 2 && osi.wProductType != VER_NT_WORKSTATION )
            return _T( "Windows Server 2012" );

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 3 && osi.wProductType == VER_NT_WORKSTATION )
            return _T( "Windows 8.1" );

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 3 && osi.wProductType != VER_NT_WORKSTATION )
            return _T( "Windows Server 2012 R2" );

        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
            return _T( "Windows 95" );

        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 10 && osi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
            return _T( "Windows 98" );

        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 90 && osi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
            return _T( "Windows Me" );

        return _T( "Unknown OS" );
    }

    BOOL SetRegistyStartProgram( BOOL bAutoExec, LPCWSTR lpValueName, LPCWSTR lpExeFileName )
    {
        HKEY hKey;
        LONG lRes;
        if( bAutoExec )
        {
            if( lpValueName == NULL || lpExeFileName == NULL )
                return FALSE;

            if( RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                0L,
                KEY_WRITE,
                &hKey
                ) != ERROR_SUCCESS )
                return FALSE;

            lRes = RegSetValueExW( hKey,
                                  lpValueName,
                                  0,
                                  REG_SZ,
                                  (BYTE*)lpExeFileName,
                                  (DWORD)(wcslen( lpExeFileName ) * 2 + 1) );

            RegCloseKey( hKey );

            if( lRes != ERROR_SUCCESS )
                return FALSE;
        }
        else
        {
            if( RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                0, KEY_ALL_ACCESS,
                &hKey ) != ERROR_SUCCESS )
                return FALSE;

            lRes = RegDeleteValueW( hKey, lpValueName );

            RegCloseKey( hKey );

            if( lRes != ERROR_SUCCESS )
                return FALSE;
        }

        return TRUE;
    }

    bool VerifyVersionInfoEx( DWORD dwMajorVersion, int dwMinorVersion /*= 0*/, const std::string& strCondition /*= "=" */ )
    {
        bool isMatchCondition = false;

        do 
        {
            OSVERSIONINFOEXW osvi;
            DWORDLONG dwlConditionMask = 0;

            ZeroMemory( &osvi, sizeof( OSVERSIONINFOEXW ) );
            osvi.dwOSVersionInfoSize = sizeof( osvi );
            osvi.dwMajorVersion = dwMajorVersion;

            int condition = VER_EQUAL;

            if( strCondition == "=" )
                condition = VER_EQUAL;
            else if( strCondition == ">=" )
                condition = VER_GREATER_EQUAL;
            else if( strCondition == ">" )
                condition = VER_GREATER;
            else if( strCondition == "<=" )
                condition = VER_LESS_EQUAL;
            else if( strCondition == "<" )
                condition = VER_LESS;

            VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, condition );

            if( dwMinorVersion < 0 )
            {
                isMatchCondition = VerifyVersionInfoW( &osvi, VER_MAJORVERSION, dwlConditionMask ) != FALSE ? true : false;
            }
            else
            {
                osvi.dwMinorVersion = dwMinorVersion;
                VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, condition );
                isMatchCondition = VerifyVersionInfoW( &osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask ) != FALSE ? true : false;

            }

        } while (false);

        return isMatchCondition;
    }


#ifdef USE_SERVICE_SUPPORT
    void SetServiceToAutoRestart( const std::wstring& serviceName, int restartDelayMs /*= 1000 * 60*/, int resetPeriodSec /*= 60 */ )
    {
        SERVICE_FAILURE_ACTIONS sfa;
        SC_HANDLE hSCM = NULL;
        SC_HANDLE hService = NULL;

        do
        {
            hSCM = OpenSCManager( NULL, NULL, NULL );
            if( hSCM == NULL )
                break;

            hService = OpenService( hSCM, serviceName.c_str(), SC_MANAGER_ALL_ACCESS );
            if( hService == NULL )
                break;

            SC_ACTION sca;
            sca.Type = SC_ACTION_RESTART;
            sca.Delay = restartDelayMs;

            sfa.dwResetPeriod = resetPeriodSec;
            sfa.lpRebootMsg = NULL;
            sfa.lpCommand = NULL;
            sfa.lpsaActions = &sca;
            sfa.cActions = 1;

            ChangeServiceConfig2( hService, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa );

        } while( false );

        if( hService != NULL )
            CloseServiceHandle( hService );
        hService = NULL;

        if( hSCM != NULL )
            CloseServiceHandle( hSCM );
        hSCM = NULL;

    }

    bool StartServiceFromName( LPCTSTR pszServiceName, UINT nWaitSecs /* = 5 */ )
    {
        bool			IsResult = false;
        SC_HANDLE		hManager = NULL;
        SC_HANDLE		hService = NULL;

        hManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if( hManager == NULL )
            return IsResult;

        hService = ::OpenService( hManager, pszServiceName, SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP );
        if( hService == NULL )
            goto CLEANUP;

        if( ::StartService( hService, 0, NULL ) != FALSE )
        {
            SERVICE_STATUS ss;
            for( UINT nSec = 0; nSec < nWaitSecs; ++nSec )
            {
                ::QueryServiceStatus( hService, &ss );
                if( ss.dwCurrentState == SERVICE_RUNNING )
                {
                    IsResult = true;
                    break;
                }
                Sleep( 1000 );
            }
        }

    CLEANUP:
        if( hManager )
            CloseServiceHandle( hManager );
        if( hService )
            CloseServiceHandle( hService );

        return IsResult;
    }

    bool StopServiceFromName( LPCTSTR pszServiceName, UINT nWaitSecs /* = 5 */, DWORD dwCode /* = SERVICE_CONTROL_STOP*/ )
    {
        bool			IsResult = false;
        SC_HANDLE		hManager = NULL;
        SC_HANDLE		hService = NULL;

        hManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if( hManager == NULL )
            return IsResult;

        hService = ::OpenService( hManager, pszServiceName, SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP );
        if( hService == NULL )
            goto CLEANUP;

        SERVICE_STATUS ss;
        ::QueryServiceStatus( hService, &ss );
        if( ss.dwCurrentState != SERVICE_STOPPED )
        {
            if( ::ControlService( hService, dwCode, &ss ) != FALSE )
            {
                for( UINT nSec = 0; nSec < nWaitSecs; ++nSec )
                {
                    ::QueryServiceStatus( hService, &ss );
                    if( ss.dwCurrentState == SERVICE_STOPPED )
                    {
                        IsResult = true;
                        break;
                    }
                    Sleep( 1000 );
                }
            }
            else
            {
                IsResult = ::GetLastError() == ERROR_SERVICE_NOT_ACTIVE ? true : false;
            }
        }
        else
        {
            IsResult = true;
        }

    CLEANUP:
        if( hManager )
            CloseServiceHandle( hManager );
        if( hService )
            CloseServiceHandle( hService );

        return IsResult;
    }

    bool GetServiceStatus( LPCTSTR pszServiceName, SERVICE_STATUS& status, QUERY_SERVICE_CONFIG* pConfig /*= NULL*/, DWORD dwBufSize /*= 1024*/ )
    {
        bool			IsResult = false;
        SERVICE_STATUS  ss;
        SC_HANDLE		hManager = NULL;
        SC_HANDLE		hService = NULL;
        DWORD           dwBytesNeeded = 0;

        hManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if( hManager == NULL )
            return IsResult;

        hService = ::OpenService( hManager, pszServiceName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG );
        if( hService == NULL )
            goto CLEANUP;

        if( ::QueryServiceStatus( hService, &ss ) == TRUE )
        {
            IsResult = true;
            status = ss;
        }

        if( pConfig != NULL )
        {
            if( ::QueryServiceConfig( hService, pConfig, dwBufSize, &dwBytesNeeded ) == FALSE )
            {
                IsResult = false;
                goto CLEANUP;
            }
        }

    CLEANUP:
        if( hManager != NULL )
            CloseServiceHandle( hManager );
        if( hService != NULL )
            CloseServiceHandle( hService );

        return IsResult;
    }

#endif

#ifdef USE_WMI_SUPPORT
    std::wstring GetWMIInfoToStr( const std::wstring& className, const std::wstring& propertyName )
    {
        std::wstring retVal;
    
        do
        {
            CWbemLocator wbemLocator;
            CWbemService wbemService;

            if( FAILED( wbemLocator.ConnectServer( wbemService ) ) )
                break;

            CWbemClassEnumerator wbemClasEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( className, wbemClasEnumerator ) ) )
                break;

            wbemClasEnumerator.SetObjectProperty( TyVecWbemProperty( 1, propertyName ) );
            if( FAILED( wbemClasEnumerator.RefreshEnum() ) )
                break;

            VARIANT vtData;
            VariantInit( &vtData );

            // 항상 첫 번째 인스턴스의 값 가져옴
            if( SUCCEEDED( wbemClasEnumerator.GetItem( 0, propertyName, vtData ) ) )
            {
                if( vtData.vt == VT_BSTR )
                {
                    retVal = V_BSTR( &vtData );
                }
                else
                {
                    VARIANT vtDest;
                    VariantInit( &vtDest );
                    if( SUCCEEDED( VariantChangeType( &vtData, &vtDest, VARIANT_ALPHABOOL | VARIANT_NOUSEROVERRIDE, VT_BSTR ) ) )
                        retVal = V_BSTR( &vtDest );
                    VariantClear( &vtDest );
                }
            }

            VariantClear( &vtData );

        } while( false );

        return retVal;
    }

    std::wstring QueryOSStringFromWMI()
    {
        //const LPTSTR UNKNOWN_OS = _T("Unknown OS");

        std::wstring wsResult = QueryOSStringFromRegistry();
        std::wstring sOSArchitecture;
        sOSArchitecture.clear();

        CoInitializeEx( 0, COINIT_MULTITHREADED );

        {
            nsCmnWMI::CWbemLocator wbemLocator;
            nsCmnWMI::CWbemService wbemService;

            if( FAILED( wbemLocator.ConnectServer( wbemService ) ) )
                return wsResult;

            nsCmnWMI::CWbemClassEnumerator wbemClsEnumerator;
            
            if( FAILED( wbemService.GetInstanceEnumerator( L"Win32_OperatingSystem", wbemClsEnumerator ) ) )
                return wsResult;

            wbemClsEnumerator.SetObjectPropertyBuiltIn();
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                _variant_t vtItem;
                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Caption", vtItem.GetVARIANT() ) ) )
                {
                    wsResult = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str();
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"OSArchitecture", vtItem.GetVARIANT() ) ) )
                {
                    sOSArchitecture = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }
            }

        }

        CoUninitialize();

        if( sOSArchitecture.empty() == false )
            wsResult = wsResult + L" " + sOSArchitecture;

        return wsResult;
    }

    std::wstring QueryBiosSerialFromWMI()
    {
        std::wstring wsResult = L"";

        CoInitializeEx( 0, COINIT_MULTITHREADED );

        {
            nsCmnWMI::CWbemLocator wbemLocator;
            nsCmnWMI::CWbemService wbemService;

            if( FAILED( wbemLocator.ConnectServer( wbemService ) ) )
                return wsResult;

            nsCmnWMI::CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( L"Win32_Bios", wbemClsEnumerator ) ) )
                return wsResult;

            wbemClsEnumerator.SetObjectPropertyBuiltIn( );
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                _variant_t vtItem;
                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"SerialNumber", vtItem.GetVARIANT() ) ) )
                {
                    wsResult = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                    break;
                }
            }
        }

        CoUninitialize();

        return wsResult;
    }

    std::wstring QueryComputerUUIDFromWMI()
    {
        std::wstring wsResult = L"";

        CoInitializeEx( 0, COINIT_MULTITHREADED );

        {
            nsCmnWMI::CWbemLocator wbemLocator;
            nsCmnWMI::CWbemService wbemService;

            if( FAILED( wbemLocator.ConnectServer( wbemService ) ) )
                return wsResult;

            nsCmnWMI::CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( L"Win32_ComputerSystemProduct", wbemClsEnumerator ) ) )
                return wsResult;

            wbemClsEnumerator.SetObjectPropertyBuiltIn( );
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                _variant_t vtItem;
                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"UUID", vtItem ) ) )
                {
                    wsResult = nsCommon::nsCmnConvert::ConvertToStringW( vtItem, L"," ).c_str( );
                    break;
                }
            }

        }

        CoUninitialize();

        return wsResult;
    }

    std::wstring QueryOSStringFromWMINonArch()
    {
        std::wstring wsResult = QueryOSStringFromRegistry();

        CoInitializeEx( 0, COINIT_MULTITHREADED );

        {
            nsCmnWMI::CWbemLocator wbemLocator;
            nsCmnWMI::CWbemService wbemService;

            if( FAILED( wbemLocator.ConnectServer( wbemService ) ) )
                return wsResult;

            nsCmnWMI::CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( L"Win32_OperatingSystem", wbemClsEnumerator ) ) )
                return wsResult;

            wbemClsEnumerator.SetObjectPropertyBuiltIn( );
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                _variant_t vtItem;
                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Caption", vtItem.GetVARIANT( ) ) ) )
                {
                    wsResult = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT( ), L"," ).c_str( );
                }
            }

        }

        CoUninitialize();

        return wsResult;
    }

    std::wstring QueryBaseBoardSerialFromWMI()
    {
        std::wstring wsResult = L"";

        CoInitializeEx( 0, COINIT_MULTITHREADED );

        {
            nsCmnWMI::CWbemLocator wbemLocator;
            nsCmnWMI::CWbemService wbemService;

            if( FAILED( wbemLocator.ConnectServer( wbemService ) ) )
                return wsResult;

            nsCmnWMI::CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( L"Win32_BaseBoard", wbemClsEnumerator ) ) )
                return wsResult;

            wbemClsEnumerator.SetObjectPropertyBuiltIn( );
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                _variant_t vtItem;
                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"SerialNumber", vtItem.GetVARIANT() ) ) )
                {
                    wsResult = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                    break;
                }
            }

        }

        CoUninitialize();

        return wsResult;
    }

    std::vector< detail::WMI_NIC_INFO > QueryNICInfoFromWMI( bool bVitualExclude /*= false*/ )
    {
        std::vector< detail::WMI_NIC_INFO > vecNICs;

        // 1. Win32_NetworkAdapter
        std::vector< detail::WMI_NIC_INFO > vecNetworkAdapters;
        vecNetworkAdapters = QueryWin32NetworkAdapterFromWMI();

        // 2. Win32_NetworkAdapterConfiguration
        std::vector< detail::WMI_NIC_INFO > vecNetworkAdapterConfigs;
        vecNetworkAdapterConfigs = QueryWin32NetworkAdapterConfigurationFromWMI();

        // 1 + 2
        // Ethernet 802.3, Wireless 만 필요.( ID : 0, 9 )
        for( auto it = vecNetworkAdapters.begin(); it != vecNetworkAdapters.end(); ++it )
        {
            if( it->nAdapterTypeID != 0 && it->nAdapterTypeID != 9 )
                continue;

            if( bVitualExclude == true &&
                ( StrStrI( it->sDescription.c_str(), L"vmware" ) != NULL
                || StrStrI( it->sDescription.c_str(), L"virtualbox" ) != NULL ) )
                continue;

            for( auto it2 = vecNetworkAdapterConfigs.begin(); it2 != vecNetworkAdapterConfigs.end(); ++it2 )
            {
                if( it->nInterfaceIndex != it2->nInterfaceIndex )
                    continue;

                it->sDefalutGateWayIP = it2->sDefalutGateWayIP;
                it->bDHCPEnable = it2->bDHCPEnable;
                it->sDHCPServerIP = it2->sDHCPServerIP;
                it->sDNSDomain = it2->sDNSDomain;
                it->sDNSHostName = it2->sDNSHostName;
                it->sIPAddress = it2->sIPAddress;
                it->sLinkLocalIPv6 = it2->sLinkLocalIPv6;
                it->bIPEnable = it2->bIPEnable;
                it->sIPSubnet = it2->sIPSubnet;
                it->sMACAddress = it2->sMACAddress;

                vecNICs.push_back( *it );
            }
        }

        return vecNICs;
    }

    std::vector< detail::WMI_NIC_INFO > QueryWin32NetworkAdapterFromWMI()
    {
        std::vector< detail::WMI_NIC_INFO > vecNICs;

        CoInitializeEx( 0, COINIT_MULTITHREADED );

        do 
        {
            nsCmnWMI::CWbemLocator wbemLocator;
            nsCmnWMI::CWbemService wbemService;

            if( FAILED( wbemLocator.ConnectServer( wbemService ) ) )
                break;

            nsCmnWMI::CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( L"Win32_NetworkAdapter", wbemClsEnumerator ) ) )
                break;

            wbemClsEnumerator.SetObjectPropertyBuiltIn( );
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                break;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                detail::WMI_NIC_INFO item;

                _variant_t vtItem;
                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"AdapterType", vtItem.GetVARIANT() ) ) )
                {
                    item.sAdapterType = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"AdapterTypeId", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_I4 )
                        item.nAdapterTypeID = (int)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Caption", vtItem.GetVARIANT() ) ) )
                {
                    item.sCaption = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Description", vtItem.GetVARIANT() ) ) )
                {
                    item.sDescription = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"DeviceID", vtItem.GetVARIANT() ) ) )
                {
                    item.sDeviceID = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"GUID", vtItem.GetVARIANT() ) ) )
                {
                    item.sGUID = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Index", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_I4 )
                        item.nIndex = (int)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Installed", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_BOOL )
                        item.bInstalled = (bool)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"InterfaceIndex", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_I4 )
                        item.nInterfaceIndex = (int)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Manufacturer", vtItem.GetVARIANT() ) ) )
                {
                    item.sManufacturer = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Name", vtItem.GetVARIANT() ) ) )
                {
                    item.sName = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"NetConnectionID", vtItem.GetVARIANT() ) ) )
                {
                    item.sNetConnectionID = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"NetConnectionStatus", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_I4 )
                        item.nNetConnectionStatus = (int)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"NetEnabled", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_BOOL )
                        item.bNetEnabled = (bool)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"PhysicalAdapter", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_BOOL )
                        item.bPhysicalAdapter = (bool)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"PNPDeviceID", vtItem.GetVARIANT() ) ) )
                {
                    item.sPNPDeviceID = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"ProductName", vtItem.GetVARIANT() ) ) )
                {
                    item.sProductName = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"ServiceName", vtItem.GetVARIANT() ) ) )
                {
                    item.sServiceName = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                vecNICs.push_back( item );
            }

        } while ( false );

        CoUninitialize();

        return vecNICs;
    }

    std::vector< detail::WMI_NIC_INFO > QueryWin32NetworkAdapterConfigurationFromWMI()
    {
        std::vector< detail::WMI_NIC_INFO > vecNICs;

        CoInitializeEx( 0, COINIT_MULTITHREADED );

        do 
        {
            nsCmnWMI::CWbemLocator wbemLocator;
            nsCmnWMI::CWbemService wbemService;

            if( FAILED( wbemLocator.ConnectServer( wbemService ) ) )
                break;

            nsCmnWMI::CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( L"Win32_NetworkAdapterConfiguration", wbemClsEnumerator ) ) )
                break;

            wbemClsEnumerator.SetObjectPropertyBuiltIn( );
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                break;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                detail::WMI_NIC_INFO item;

                _variant_t vtItem;
                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Caption", vtItem.GetVARIANT() ) ) )
                {
                    item.sCaption = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Description", vtItem.GetVARIANT() ) ) )
                {
                    item.sDescription = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"DefaultIPGateway", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == ( VT_ARRAY | VT_BSTR ) )
                    {
                        SAFEARRAY *pSA = vtItem.parray;

                        long lLower, lUpper; 
                        SafeArrayGetLBound( pSA, 1, &lLower );
                        SafeArrayGetUBound( pSA, 1, &lUpper );

                        for( long idx = lLower; idx <= lUpper; idx++ )
                        {
                            BSTR bstr = NULL;

                            SafeArrayGetElement( pSA, &idx, &bstr );

                            SysFreeString( bstr );

                            if( idx == 0 )
                                item.sDefalutGateWayIP = bstr;
                        }
                    }
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"DHCPEnabled", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_BOOL )
                        item.bDHCPEnable = (bool)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"DHCPServer", vtItem.GetVARIANT() ) ) )
                {
                    item.sDHCPServerIP = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"DNSDomain", vtItem.GetVARIANT() ) ) )
                {
                    item.sDNSDomain = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"DNSHostName", vtItem.GetVARIANT() ) ) )
                {
                    item.sDNSHostName = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"Index", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_I4 )
                        item.nIndex = (int)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"InterfaceIndex", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_I4 )
                        item.nInterfaceIndex = (int)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"IPAddress", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == ( VT_ARRAY | VT_BSTR ) )
                    {
                        SAFEARRAY *pSA = vtItem.parray;

                        long lLower, lUpper; 
                        SafeArrayGetLBound( pSA, 1, &lLower );
                        SafeArrayGetUBound( pSA, 1, &lUpper );

                        for( long idx = lLower; idx <= lUpper; idx++ )
                        {
                            BSTR bstr = NULL;

                            SafeArrayGetElement( pSA, &idx, &bstr );

                            SysFreeString( bstr );

                            if( idx == 0 )
                                item.sIPAddress = bstr;
                            else if( idx == 1 )
                                item.sLinkLocalIPv6 = bstr;
                        }
                    }
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"IPEnabled", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == VT_BOOL )
                        item.bIPEnable = (bool)vtItem;
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"IPSubnet", vtItem.GetVARIANT() ) ) )
                {
                    if( vtItem.vt == ( VT_ARRAY | VT_BSTR ) )
                    {
                        SAFEARRAY *pSA = vtItem.parray;

                        long lLower, lUpper; 
                        SafeArrayGetLBound( pSA, 1, &lLower );
                        SafeArrayGetUBound( pSA, 1, &lUpper );

                        for( long idx = lLower; idx <= lUpper; idx++ )
                        {
                            BSTR bstr = NULL;

                            SafeArrayGetElement( pSA, &idx, &bstr );

                            SysFreeString( bstr );

                            if( idx == 0 )
                                item.sIPSubnet = bstr;
                        }
                    }
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"MACAddress", vtItem.GetVARIANT() ) ) )
                {
                    item.sMACAddress = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                if( SUCCEEDED( wbemClsEnumerator.GetItem( nIDX, L"ServiceName", vtItem.GetVARIANT() ) ) )
                {
                    item.sServiceName = nsCommon::nsCmnConvert::ConvertToStringW( vtItem.GetVARIANT(), L"," ).c_str( );
                }

                vecNICs.push_back( item );
            }

        } while ( false );

        CoUninitialize();

        return vecNICs;
    }
#endif

}
