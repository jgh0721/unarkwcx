#pragma once

#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#pragma execution_character_set( "utf-8" )

/*!
    코드의 수행시간을 측정하기위한 매크로

    QueryPerformanceCounter 를 이용하여 측정함

    _checkTimeRun 변수에는 MS 단위의 실행시간이 float 형태의 값으로 들어있음, 값을 구할 수 없으면 0.0 이 초기값으로 들어감

    참조 : http://maytrees.tistory.com/81
*/

#define CHECK_TIME_START \
    __int64 _checkTimeStart = 0, _checkTimeFreq = 0, _checkTimeEnd = 0; \
    float _checkTimeRun = 0.0f; \
    BOOL _checkTimeCondition = FALSE; \
if( (_checkTimeCondition = QueryPerformanceFrequency( (_LARGE_INTEGER*)&_checkTimeFreq )) ) \
    {		QueryPerformanceCounter( (_LARGE_INTEGER*)&_checkTimeStart );	};


#define CHECK_TIME_END \
if( _checkTimeCondition != FALSE ) \
    { \
    QueryPerformanceCounter( (_LARGE_INTEGER*)&_checkTimeEnd ); \
    _checkTimeRun = (float)((double)(_checkTimeEnd - _checkTimeStart) / _checkTimeFreq * 1000); \
    };

namespace nsCommon
{
    namespace detail
    {
#pragma pack(push,8)
        typedef struct tagTHREADNAME_INFO
        {
            DWORD dwType; // Must be 0x1000.
            LPCSTR szName; // Pointer to name (in user addr space).
            DWORD dwThreadID; // Thread ID (-1=caller thread).
            DWORD dwFlags; // Reserved for future use, must be zero.
        } THREADNAME_INFO;
#pragma pack(pop)

        typedef BOOL( WINAPI *LPFN_ISWOW64PROCESS ) (HANDLE, PBOOL);
    }

    class CBenchTimer
    {
    public:
        CBenchTimer();
        ~CBenchTimer();

        void        step();
        // 이전에 호출한 step 과 step 사이의 경과 시간, 단위는 ms, step 를 한번밖에 호출하지 않았다면, 처음 부터의 경과시간을 구한다. 
        float       getElapsedTimeFromPrev();
        // 맨 처음 클래스를 생성한 시간부터의 경과 시간, 단위는 ms
        float       getElapsedTimeFromStart();

    private:
        float       _checkTimeRun;
        __int64     _checkTimeStart;
        __int64     _checkTimeFreq;
        __int64     _checkTimePrev;
        __int64     _checkTimeCurrent;
    };

#ifdef _MSC_VER
    // 디버거에서 확인할 수 있도록 스레드에 이름을 부여합니다. 스레드 ID = -1 => 호출한 스레드
    void                            SetThreadName( DWORD dwThreadID, LPCSTR pszThreadName );
#endif

    //설치된 OS가 64비트 인지 판단.
    bool					        is64BitWindow();	
    //장착된 CPU가 64비트를 지원하는지 판단.
    bool					        is64BitProcessor();	
    BOOL                            EnablePrivilege(LPCTSTR szPrivilege);
    std::wstring                    getOSVer();
    std::wstring                    getOSDisplayName();
    std::wstring                    getInternetExplorerVersion();
    std::wstring                    getEnvironmentVariable( const std::wstring& strEnvName );

#ifdef _SFC_
    bool                            IsProtectedFileByOS( const std::wstring& fileName );
#endif
    bool                            InCompressUsing7ZIP( const std::wstring& strPath, const std::wstring& strTargetFile, const std::wstring& strinCompressFileName, const std::wstring& str7zipPath );
    bool                            DeCompressUsing7ZIP( const std::wstring& strPath, const std::wstring& strFileName, const std::wstring& str7zipPath, const std::wstring& strPassword = L"dkdlahsdbwj!@#" );
    bool                            DeCompressUsing7ZIP( const std::wstring& strPath, const std::wstring& strFileName, const std::wstring& str7zipPath, const std::wstring& strUncompressPath, const std::wstring& strPassword = L"dkdlahsdbwj!@#" );
    /*!
        eventType
        #define EVENTLOG_SUCCESS                0x0000
        #define EVENTLOG_ERROR_TYPE             0x0001
        #define EVENTLOG_WARNING_TYPE           0x0002
        #define EVENTLOG_INFORMATION_TYPE       0x0004
        #define EVENTLOG_AUDIT_SUCCESS          0x0008
        #define EVENTLOG_AUDIT_FAILURE          0x0010
    */
    bool                            WriteEventLog( const std::wstring& programName, const std::wstring& logMessage, WORD eventType = EVENTLOG_INFORMATION_TYPE, WORD eventCatecory = 0, DWORD eventID = 0 );

#ifdef USE_OLE_SUPPORT
    std::string                     GetNewGUID();
#endif

    // 트레이아이콘에 강제종료 등으로 인해 남겨진 아이콘들 정리
    void                            sweepNotificationIcon( );
    void                            SetFroegroundWindowForce( HWND hWnd );

    // 해당 프로세스 PID 에서 소유한 모든 창 핸들 반환
    std::vector< HWND >             getWindHandle( DWORD dwProcessID );
    class CWindowList
    {
    public:
        CWindowList() {};
        ~CWindowList() {};

        std::vector< std::pair< HWND, std::wstring > > GetWindowList();
        HWND isExistWindow( DWORD processID, std::wstring windowText );

        // 0 = not found
        // 1 = exists
        // 2 = visible
        // 4 = enabled
        // 8 = active
        // 16 = minimized
        // 32 = maximized
        int getWindowState( HWND hWnd );

    private:
        static BOOL CALLBACK enumChildWindowProc( HWND hWnd, LPARAM lParam );
        std::vector< std::pair< HWND, std::wstring > > _vecWindowList;
    };

    /*!
        레지스트리의 특정 키에서 문자열을 읽어온다.
        대상 컴퓨터가 64비트 OS 일때 isRead32Key == true 이면 레지스트리의 32bit view 를 사용하고 isRead32Key == false 면 64bit view 를 사용한다
    */
    std::wstring                    QueryRegValueWithStr( HKEY rootKey, const std::wstring& subkey, const std::wstring& valueName, bool isRead32Key = true );
    void                            SetRegValueWithStr( HKEY rootKey, const std::wstring& subkey, const std::wstring& valueName, const std::wstring& valueData, bool isWrite32Key = true );
    std::wstring                    QueryOSStringFromRegistry();
    std::wstring                    QueryOSStringFromREG();
    BOOL                            SetRegistyStartProgram( BOOL bAutoExec, LPCWSTR lpValueName, LPCWSTR lpExeFileName );

    // 비교하려는 버전 숫자와 조건를 넣어 해당 조건에 부합하는지 테스트한다. 
    // 지원하는 조건 문자, "=", "<=", ">=", ">", "<", 윈도가 이렇게만 지원함... != 는 어디갔는지 묻지 말아주세요
    // dwMinorVersion 에 -1 (기본값) 을 넣으면 dwMajorVersion 만 사용하여 비교를 수행한다. 
    bool                            VerifyVersionInfoEx( DWORD dwMajorVersion, int dwMinorVersion = -1, const std::string& strCondition = "=" );
    

#ifdef USE_SERVICE_SUPPORT
    void                            SetServiceToAutoRestart( const std::wstring& serviceName, int restartDelayMs = 1000 * 60, int resetPeriodSec = 60 );
    bool                            StartServiceFromName( LPCTSTR pszServiceName, UINT nWaitSecs = 5 );
    bool                            StopServiceFromName( LPCTSTR pszServiceName, UINT nWaitSecs = 5, DWORD dwCode = 0x00000001 );
    bool                            GetServiceStatus( LPCTSTR pszServiceName, SERVICE_STATUS& status, QUERY_SERVICE_CONFIG* pConfig = NULL, DWORD dwBufSize = 1024 );
#endif

#ifdef USE_WMI_SUPPORT
    std::wstring                    GetWMIInfoToStr( const std::wstring& className, const std::wstring& propertyName );
    std::wstring                    QueryOSStringFromWMI();
    std::wstring                    QueryBiosSerialFromWMI();
    std::wstring                    QueryComputerUUIDFromWMI( );
    std::wstring                    QueryOSStringFromWMINonArch();
    std::wstring                    QueryBaseBoardSerialFromWMI();

    namespace detail
    {
        typedef struct tag_WMI_NIC_INFO
        {
            std::wstring    sAdapterType;
            int             nAdapterTypeID;
            std::wstring    sCaption;
            std::wstring    sDescription;
            std::wstring    sDeviceID;
            std::wstring    sGUID;
            int             nIndex;
            bool            bInstalled;
            int             nInterfaceIndex;
            std::wstring    sManufacturer;
            std::wstring    sName;
            std::wstring    sNetConnectionID;
            int             nNetConnectionStatus;
            bool            bNetEnabled;   //네트워크 카드 활성화 비활성화
            bool            bPhysicalAdapter;
            std::wstring    sPNPDeviceID;
            std::wstring    sProductName;
            std::wstring    sServiceName;

            //////////////////////////////////////////////////////////////////////////
            std::wstring    sDefalutGateWayIP;
            bool            bDHCPEnable;
            std::wstring    sDHCPServerIP;
            std::wstring    sDNSDomain;
            std::wstring    sDNSHostName;
            std::wstring    sIPAddress;
            std::wstring    sLinkLocalIPv6;
            bool            bIPEnable;
            std::wstring    sIPSubnet;
            std::wstring    sMACAddress;

            tag_WMI_NIC_INFO()
                : nAdapterTypeID(-1), nIndex(-1), bInstalled(false), nInterfaceIndex(-1), nNetConnectionStatus(-1),
                bNetEnabled(false), bPhysicalAdapter(false), bDHCPEnable(false), bIPEnable(false)
            {            };

        } WMI_NIC_INFO;
    }

    std::vector< detail::WMI_NIC_INFO > QueryNICInfoFromWMI( bool bVitualExclude = false );
    std::vector< detail::WMI_NIC_INFO > QueryWin32NetworkAdapterFromWMI();
    std::vector< detail::WMI_NIC_INFO > QueryWin32NetworkAdapterConfigurationFromWMI();

#endif

}

