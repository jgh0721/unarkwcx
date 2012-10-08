#include "stdafx.h"

#include "CommonLogger.h"

#include <io.h>
#include <process.h>
#include <iostream>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi")

#include "ConvertStr.h"

#pragma warning( disable: 4996 )

CLog* CLog::pClsLog = NULL;
volatile bool CLog::isExit = false;

CLog::CLog()
    : m_hProcessLOGThread( NULL )
    , m_eCurrentLogLevel( LL_NONE ), m_eLogLevel( LL_NONE )
    , m_lCurrentIndex( -1 )
    , m_eLogTransfer( LT_DEBUGGER )
    , m_nSplitCount(0)
    , m_IsFileSplit( true )
    , m_IsFileSplitWhenDate( true )
    , m_nFileLimitSize( 0 )
    , m_pLogFile( NULL )
{
    memset( m_szCurrentLogFile, '\0', _countof( m_szCurrentLogFile ) * sizeof( wchar_t ) );

    memset( m_szDateFormatA, '\0', _countof( m_szDateFormatA ) * sizeof( char ) );
    memset( m_szDateFormatW, '\0', _countof( m_szDateFormatW ) * sizeof( wchar_t ) );
    memset( m_szTimeFormatA, '\0', _countof( m_szTimeFormatA ) * sizeof( char ) );
    memset( m_szTimeFormatW, '\0', _countof( m_szTimeFormatW ) * sizeof( wchar_t ) );

    SetDateFormat( NULL );
    SetTimeFormat( NULL );

    InitializeCriticalSectionAndSpinCount( &m_cs, 4000 );
    m_hProcessLOGThread = (HANDLE)_beginthreadex( NULL, 0, &CLog::processLOGThread, this, 0, NULL );
    for( int idx = 0; idx < MAX_MESSAGE_COUNT; ++idx )
    {
        PLOGDATA pLogData = new LOGDATA;
        
        pLogData->dwBufferSizeA += MSG_BUFFER_SIZE;
        pLogData->dwBufferSizeW += MSG_BUFFER_SIZE;

        pLogData->szLogData = new char[ pLogData->dwBufferSizeA ];
        pLogData->wszLogData = new wchar_t[ pLogData->dwBufferSizeW ];

        pLogData->dwRemainBufferSizeA = pLogData->dwBufferSizeA;
        pLogData->dwRemainBufferSizeW = pLogData->dwBufferSizeW;

        pLogData->szNextMsgPtrA = pLogData->szLogData;
        pLogData->szNextMsgPtrW = pLogData->wszLogData;

        InterlockedExchange( &pLogData->isUse, 0 );

        m_vecLOGData.push_back( pLogData );
    }
}

CLog::~CLog()
{
    WaitForSingleObject( m_hProcessLOGThread, INFINITE );
    CloseHandle( m_hProcessLOGThread );
    m_hProcessLOGThread = NULL;

    for( size_t idx = 0; idx < m_vecLOGData.size(); ++idx )
    {
        if( m_vecLOGData[ idx ]->wszLogData != NULL )
            delete [] m_vecLOGData[ idx ]->wszLogData;
        if( m_vecLOGData[ idx ]->szLogData != NULL )
            delete [] m_vecLOGData[ idx ]->szLogData;
    }
}

CLog* CLog::GetInstance( void )
{
    if( CLog::pClsLog == NULL && CLog::isExit == false )
        CLog::pClsLog = new CLog();
    return CLog::pClsLog;
}

void CLog::CloseInstance( void )
{
    if( CLog::pClsLog )
        delete CLog::pClsLog;
    CLog::pClsLog = NULL;
    CLog::isExit = true;
}

bool CLog::IsLogging( CLOG_LEVEL logLevel )
{
    m_eCurrentLogLevel = logLevel;

    return m_eLogLevel >= m_eCurrentLogLevel;
}

void CLog::Lock()
{
    EnterCriticalSection( &m_cs );
}

void CLog::Unlock()
{
    LeaveCriticalSection( &m_cs );
}

size_t CLog::Log( const char* szFileName, const char* szFuncName, const wchar_t* wszFileName, const wchar_t* wszFuncName, unsigned int nLineNumber, const char* fmt, ... )
{
    size_t ret = 0;
    int logIndex = -1;

    InterlockedCompareExchange( &m_lCurrentIndex, -1, MAX_MESSAGE_COUNT );

    logIndex = InterlockedIncrement( &m_lCurrentIndex );
    PLOGDATA pLogDATA = m_vecLOGData[ logIndex ];
    while( InterlockedCompareExchange( &pLogDATA->isUse, 1, 0 ) != 0 && isExit == false )
        SleepEx( 0, TRUE );

    pLogDATA->isUnicode = false;
    
    pLogDATA->dwRemainBufferSizeA = pLogDATA->dwBufferSizeA;
    pLogDATA->szNextMsgPtrA = pLogDATA->szLogData;
    ZeroMemory( pLogDATA->szLogData, pLogDATA->dwBufferSizeA * sizeof( char ) );

    ret += log_arg_list( szFileName, szFuncName, nLineNumber, m_strPreFixA.c_str(), false, pLogDATA, NULL );

    va_list args;

    va_start( args, fmt );
    ret += log_arg_list( szFileName, szFuncName, nLineNumber, fmt, true, pLogDATA, args );
    va_end( args );

    Lock();
    m_queueLOGData.push( logIndex );
    Unlock();

    return ret;
}

size_t CLog::Log( const char* szFileName, const char* szFuncName, const wchar_t* wszFileName, const wchar_t* wszFuncName, unsigned int nLineNumber, const wchar_t* fmt, ... )
{
    size_t ret = 0;
    int logIndex = -1;

    InterlockedCompareExchange( &m_lCurrentIndex, -1, MAX_MESSAGE_COUNT );

    logIndex = InterlockedIncrement( &m_lCurrentIndex );
    PLOGDATA pLogDATA = m_vecLOGData[ logIndex ];
    while( InterlockedCompareExchange( &pLogDATA->isUse, 1, 0 ) != 0 && isExit == false )
        SleepEx( 0, TRUE );
    
    pLogDATA->isUnicode = true;
    
    pLogDATA->dwRemainBufferSizeW = pLogDATA->dwBufferSizeW;
    pLogDATA->szNextMsgPtrW = pLogDATA->wszLogData;
    ZeroMemory( pLogDATA->wszLogData, pLogDATA->dwBufferSizeW * sizeof( wchar_t ) );

    ret += log_arg_list( wszFileName, wszFuncName, nLineNumber, m_strPreFixW.c_str(), false, pLogDATA, NULL );

    va_list args;

    va_start( args, fmt );
    ret += log_arg_list( wszFileName, wszFuncName, nLineNumber, fmt, true, pLogDATA, args );
    va_end( args );

    Lock();
    m_queueLOGData.push( logIndex );
    Unlock();

    return ret;
}

bool CLog::AddCallbackA( pfnLogCallbackA pfnCallback, PVOID pParam )
{
    if( pfnCallback == NULL )
        return false;

    bool isFound = false;

    // 콜백 함수가 이미 있는지 검사 후 추가
    for( std::map<pfnLogCallbackA, PVOID>::iterator iter = m_lstCallbackA.begin(); iter != m_lstCallbackA.end(); ++iter )
    {
        if( iter->first == pfnCallback )
            isFound = true;
    }

    Lock();

    if( isFound == false )
        m_lstCallbackA.insert( std::make_pair( pfnCallback, pParam ) );

    Unlock();

    return true;
}

bool CLog::AddCallbackW( pfnLogCallbackW pfnCallback, PVOID pParam )
{
    if( pfnCallback == NULL )
        return false;

    bool isFound = false;

    // 콜백 함수가 이미 있는지 검사 후 추가
    for( std::map<pfnLogCallbackW, PVOID>::iterator iter = m_lstCallbackW.begin(); iter != m_lstCallbackW.end(); ++iter )
    {
        if( iter->first == pfnCallback )
            isFound = true;
    }

    Lock();

    if( isFound == false )
        m_lstCallbackW.insert( std::make_pair( pfnCallback, pParam ) );

    Unlock();

    return true;

}

void CLog::RemoveCallbackA( pfnLogCallbackA pfnCallback )
{
    Lock();

    if( m_lstCallbackA.empty() )
    {
        return ;
        Unlock();
    }

    for( std::map<pfnLogCallbackA, PVOID>::iterator iter = m_lstCallbackA.begin(); iter != m_lstCallbackA.end(); ++iter )
    {
        if( iter->first == pfnCallback )
        {
            m_lstCallbackA.erase( iter );
            Unlock();
            return ;
        }
    }

    Unlock();
}

void CLog::RemoveCallbackW( pfnLogCallbackW pfnCallback )
{
    Lock();

    if( m_lstCallbackW.empty() )
    {
        return ;
        Unlock();
    }

    for( std::map<pfnLogCallbackW, PVOID>::iterator iter = m_lstCallbackW.begin(); iter != m_lstCallbackW.end(); ++iter )
    {
        if( iter->first == pfnCallback )
        {
            m_lstCallbackW.erase( iter );
            Unlock();
            return ;
        }
    }

    Unlock();
}

void CLog::SetLogLevel( CLOG_LEVEL logLevel )
{
    m_eLogLevel = logLevel;
}

void CLog::SetLogTransfer( DWORD logTransfer )
{
    m_eLogTransfer = logTransfer;
}

void CLog::SetLogPrefix( const wchar_t* pwszLogPrefix /* = NULL */  )
{
    if( pwszLogPrefix == NULL )
    {
        m_strPreFixA = "[LEVEL=%l][DateTime=%D %t][PID=%P][TID=%T][File=%N-%L][Func=%F]";
        m_strPreFixW = L"[LEVEL=%l][DateTime=%D %t][PID=%P][TID=%T][File=%N-%L][Func=%F]";
    }
    else
    {
        m_strPreFixW = pwszLogPrefix;
        m_strPreFixA = CU2A( pwszLogPrefix ).c_str();
    }
}

void CLog::SetDateFormat( const wchar_t* pwszDateFormat /* = NULL */ )
{
    if( pwszDateFormat == NULL )
    {
        strcpy( m_szDateFormatA, "%Y-%m-%d" );
        wcscpy( m_szDateFormatW, L"%Y-%m-%d" );
    }
    else
    {
        strcpy( m_szDateFormatA, CU2A( pwszDateFormat ) );
        wcscpy( m_szDateFormatW, pwszDateFormat );
    }
}

void CLog::SetTimeFormat( const wchar_t* pwszTimeFormat /* = NULL */ )
{
    if( pwszTimeFormat == NULL )
    {
        strcpy( m_szTimeFormatA, "%H:%M:%S" );
        wcscpy( m_szTimeFormatW, L"%H:%M:%S" );
    }
    else
    {
        strcpy( m_szTimeFormatA, CU2A(pwszTimeFormat) );
        wcscpy( m_szTimeFormatW, pwszTimeFormat );
    }
}

bool CLog::SetLogFile( const wchar_t* szLogPath, const wchar_t* szLogName, const wchar_t* szLogExt )
{
    assert( m_szLogName != NULL );
    assert( m_szLogPath != NULL );
    assert( m_szLogExt != NULL );

    memset( m_szLogPath, '\0', _countof( m_szLogPath ) * sizeof( wchar_t ) );
    memset( m_szLogName, '\0', _countof( m_szLogPath ) * sizeof( wchar_t ) );
    memset( m_szLogExt, '\0', _countof( m_szLogPath ) * sizeof( wchar_t ) );

    wcscpy( m_szLogPath, szLogPath );
    if( m_szLogPath[0] == L'.' )
    {
        TCHAR wszBuffer[MAX_PATH] = {0,};
        GetModuleFileName( NULL, wszBuffer, MAX_PATH );
        PathRemoveFileSpec( wszBuffer );

        wcscpy( m_szLogPath, string_replace_all( m_szLogPath, L".", wszBuffer ).c_str() );
    }

	if( m_szLogPath[ wcslen(m_szLogPath) - 1 ] != L'\\'  )
		wcscat( m_szLogPath, L"\\" );
	
	wcscpy( m_szLogName, szLogName );
	
	if( szLogExt[0] == L'.' )
		wcscpy( m_szLogExt, szLogExt+1 );
	else
		wcscpy( m_szLogExt, szLogExt );

	// 디렉토리 생성
    
	if( checkDirectory( m_szLogPath ) != ERROR_SUCCESS && wcsicmp( m_szLogPath, L".\\" ) != 0 )
		return false;

    return createLogFile();
}

unsigned int CLog::GetFileLimitSize( void )
{
    return m_nSplitCount;
}

void CLog::SetFileLimitSize( unsigned int nFileLimitSize /*= 0*/, bool IsFileSplit /*= true*/, bool IsFileSplitWhenDate /*= true */ )
{
    m_nFileLimitSize = nFileLimitSize;
    m_IsFileSplitWhenDate = IsFileSplitWhenDate;
    m_IsFileSplit = IsFileSplit;
}

DWORD CLog::checkDirectory( const wchar_t* szDirectory )
{
    const unsigned int MAX_FOLDER_DEPTH = 122;

    DWORD dwRet = ERROR_SUCCESS;

    size_t nLen = wcslen( szDirectory );
    if( nLen < (_MAX_DRIVE + 1) || nLen > (MAX_PATH - 1) ) 
        return ERROR_BAD_PATHNAME;

    wchar_t szPath[MAX_PATH] = {0,};
    INT_PTR offset = _MAX_DRIVE;

    for(int i = 0; i < MAX_FOLDER_DEPTH; i++)
    {
        if( (UINT_PTR)offset >= nLen ) 
            break;

        const wchar_t * pos = szDirectory + offset;
        const wchar_t * dst = wcschr(pos, L'\\');

        if( dst == NULL ) 
        {
            wcsncpy_s(szPath, MAX_PATH, szDirectory, nLen);
            i = MAX_FOLDER_DEPTH;
        }
        else 
        {
            INT_PTR cnt = dst - szDirectory;
            wcsncpy_s(szPath, MAX_PATH, szDirectory, cnt);

            offset = cnt + 1;
        }

        if( ::CreateDirectoryW(szPath, NULL) == FALSE ) 
        {
            DWORD dwErr = GetLastError();
            if( dwErr != ERROR_ALREADY_EXISTS  ) 
            {
                dwRet = dwErr;
                break;
            }
        }
    }

    return dwRet;
}

std::wstring CLog::GetFileName( void )
{
    if( m_szCurrentLogFile[ 0 ] != L'\0' )
        return m_szCurrentLogFile;

    TCHAR szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

    if( !wcsftime( szBuffer, MAX_TIME_BUFFER_SIZE, L"%Y-%m-%d", &m_tmCreateLogTime ) )
        szBuffer[0] = L'\0';

    wcscpy( m_szCurrentLogFile, format( L"%s%s_%s-[%02d].%s", m_szLogPath, m_szLogName, szBuffer[0] == NULL ? L"Unknown Date" : szBuffer, m_nSplitCount, m_szLogExt ).c_str() );
    return m_szCurrentLogFile;
}

void CLog::checkFileExist( void )
{
    wchar_t szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

    if( !wcsftime( szBuffer, MAX_TIME_BUFFER_SIZE, L"%Y-%m-%d", &m_tmCreateLogTime ) )
        szBuffer[0] = L'\0';

    m_nSplitCount = 0;

    std::wstring strFile = format( L"%s%s_%s-[*].%s", m_szLogPath, m_szLogName, szBuffer[0] == NULL ? L"Unknown Date" : szBuffer, m_szLogExt );
    _wfinddata_t data;
    intptr_t hSearch = _wfindfirst( strFile.c_str(), &data );
    if( hSearch == -1 )
        return;

    while( _wfindnext( hSearch, &data) != -1 )
        m_nSplitCount++;

    _findclose( hSearch );
}

bool CLog::checkFileSizeAndTime( const wchar_t* szFileName )
{
    bool isSplit = false;

    do 
    {
        // 파일 용량 관련 점검
        if( m_nFileLimitSize == 0 )
            break;

        static const __int64 MEGABYTE = 1024 * 1024;

        struct __stat64 statFile;
        if( _wstat64( szFileName, &statFile ) == -1 )
            break;

        __int64 llFileSize = statFile.st_size;

        if( llFileSize >= ( m_nFileLimitSize * MEGABYTE ) )
        {
            m_nSplitCount++;
            isSplit = true;
        }

    } while( false );

    do 
    {
        // 날짜 관련 점검
        if( m_IsFileSplitWhenDate == false )
            break;

        __time64_t currentTime = ::_time64(NULL);
        struct tm tmCurrentTime;

        errno_t err = _localtime64_s( &tmCurrentTime, &currentTime );
        if( err != 0 )
            break;

        if( tmCurrentTime.tm_yday > m_tmCreateLogTime.tm_yday )
        {
            m_tmCreateLogTime = tmCurrentTime;
            m_nSplitCount = 0;
            isSplit = true;
        }

    } while( false );

    if( isSplit == true )
        return createLogFile();

    return true;
}


bool CLog::createLogFile()
{
    bool isSuccess = false;

    do 
    {
        __time64_t tmCurrentTime = ::_time64( NULL );
        errno_t err = _localtime64_s( &m_tmCreateLogTime, &tmCurrentTime );
        if( err != 0 )
            break;

        checkFileExist();

        m_pLogFile = _wfopen( GetFileName().c_str(), L"a+t, ccs=UTF-16LE" );
        if( m_pLogFile == NULL )
            break;

        BYTE utf16_le_bom[] = { 0xFF, 0xFE };
        fwrite( utf16_le_bom, sizeof(BYTE), _countof( utf16_le_bom ), m_pLogFile );

        isSuccess = true;
    } while( false );


    return isSuccess;
}

unsigned int __stdcall CLog::processLOGThread( PVOID pParam )
{
    CLog* pLogger = reinterpret_cast< CLog* >( pParam );

    LOGDATA* logData = NULL;
    int nLOGDataIndex = -1;
    while( isExit == false )
    {
        do 
        {
            pLogger->Lock();
            if( pLogger->m_queueLOGData.empty() == false )
            {
                nLOGDataIndex = pLogger->m_queueLOGData.front();
                pLogger->m_queueLOGData.pop();
                pLogger->Unlock();
            }
            else
            {
                pLogger->Unlock();
                break;
            }

            if( nLOGDataIndex < 0 )
                break;

            logData = pLogger->m_vecLOGData[ nLOGDataIndex ];

            nLOGDataIndex = -1;
            // 로그 처리
            if( logData->isUnicode == true )
            {
                if( pLogger->m_eLogTransfer & LT_DEBUGGER )
                    OutputDebugStringW( logData->wszLogData );

                if( pLogger->m_eLogTransfer & LT_STDERR )
                    std::wcerr << logData->wszLogData << std::endl;

                if( pLogger->m_eLogTransfer & LT_STDOUT )
                    std::wcout << logData->wszLogData << std::endl;

                if( pLogger->m_eLogTransfer & LT_FILE  )
                {
                    if( pLogger->m_pLogFile == NULL )
                        pLogger->createLogFile();

                    if( pLogger->checkFileSizeAndTime( pLogger->GetFileName().c_str() ) == true )
                    {
                        fputws( logData->wszLogData, pLogger->m_pLogFile );
                        fflush( pLogger->m_pLogFile );
                    }
                }

                if( pLogger->m_eLogTransfer & LT_CALLBACK )
                {
                    pLogger->Lock();
                    for( std::map< pfnLogCallbackW, PVOID >::iterator it = pLogger->m_lstCallbackW.begin(); it != pLogger->m_lstCallbackW.end(); ++it )
                        (*it->first)( it->second, pLogger->m_eLogLevel, pLogger->m_eCurrentLogLevel, logData->wszLogData );
                    for( std::map< pfnLogCallbackA, PVOID >::iterator it = pLogger->m_lstCallbackA.begin(); it != pLogger->m_lstCallbackA.end(); ++it )
                        (*it->first)( it->second, pLogger->m_eLogLevel, pLogger->m_eCurrentLogLevel, CU2A(logData->wszLogData).c_str() );
                    pLogger->Unlock();
                }
            }
            else
            {
                if( pLogger->m_eLogTransfer & LT_DEBUGGER )
                    OutputDebugStringA( logData->szLogData );

                if( pLogger->m_eLogTransfer & LT_STDERR )
                    std::cerr << logData->szLogData << std::endl;

                if( pLogger->m_eLogTransfer & LT_STDOUT )
                    std::cout << logData->szLogData << std::endl;

                if( pLogger->m_eLogTransfer & LT_FILE  )
                {
                    if( pLogger->m_pLogFile == NULL )
                        pLogger->createLogFile();

                    if( pLogger->checkFileSizeAndTime( pLogger->GetFileName().c_str() ) == true )
                    {
                        fputws( CA2U(logData->szLogData), pLogger->m_pLogFile );
                        fflush( pLogger->m_pLogFile );
                    }
                }

                if( pLogger->m_eLogTransfer & LT_CALLBACK )
                {
                    pLogger->Lock();
                    for( std::map< pfnLogCallbackW, PVOID >::iterator it = pLogger->m_lstCallbackW.begin(); it != pLogger->m_lstCallbackW.end(); ++it )
                        (*it->first)( it->second, pLogger->m_eLogLevel, pLogger->m_eCurrentLogLevel, CA2U(logData->szLogData).c_str() );
                    for( std::map< pfnLogCallbackA, PVOID >::iterator it = pLogger->m_lstCallbackA.begin(); it != pLogger->m_lstCallbackA.end(); ++it )
                        (*it->first)( it->second, pLogger->m_eLogLevel, pLogger->m_eCurrentLogLevel, logData->szLogData);
                    pLogger->Unlock();
                }
            }

            InterlockedExchange( &logData->isUse, 0 );
        } while (false);

        SleepEx( 10, TRUE );
    }

    return 0;
}

size_t CLog::log_arg_list( const char* szFileName, const char* szFuncName, unsigned int nLineNumber, const char* fmt, bool isAppendNewLine, LOGDATA* pLOGDATA, va_list args )
{
    assert( fmt != NULL );

    int nRetSize = 0;

    while( *fmt != '\0' )
    {
        if( (*fmt != '%') || 
            (*fmt == '%' && fmt[1] == '%') )
        {
            // % 가 없으면 %가 나타날때까지 복사
            *pLOGDATA->szNextMsgPtrA++ = *fmt++;
            if( *fmt == '%' && fmt[1] == '%' )
                ++fmt;
            --pLOGDATA->dwRemainBufferSizeA;
            nRetSize++;
        }
        else
        {
            bool isDone = false;
            const char* start_format = fmt;
            char* fp = NULL;
            char szFormat[ 64 ] = {0,};
            int nResult = 0; // snprintf 가 기록한 양 또는 기록하는데 필요한 양

            fp = szFormat;
            *fp++ = *fmt++; // % 문자를 넣음
            while( isDone == false )
            {
                isDone = true;
                nResult = 0;

                switch( *fmt )
                {
                case 'b':
                case 'B':
                    strcpy( fp, "s" );
                    if( getBufferA( 6 /* FALSE 길이 */, pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, va_arg( args, bool ) == true ? "TRUE" : "FALSE" );
                    break;

                case 'D':				// 날짜 출력
                    {
                        strcpy( fp, "s" );
                        struct tm tmTemp;
                        __time64_t tmCurrentTime = _time64( NULL );
                        errno_t err = ::_localtime64_s( &tmTemp, &tmCurrentTime );
                        char szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

                        if( err != 0 || !strftime( szBuffer, MAX_TIME_BUFFER_SIZE, m_szDateFormatA, &tmTemp ) )
                        {
                            if( getBufferA( 15 /* L"<Unknown Date>" 의 길이 */, pLOGDATA ) == false )
                                break;

                            nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, "<Unknown Date>" );
                        }
                        else
                        {
                            if( getBufferA( strlen( szBuffer ), pLOGDATA ) == false )
                                break;

                            nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, szBuffer );
                        }
                    }
                    break;
                case 'E':				// GetLastError() 의 오류코드를 문자열 설명으로 변경
                case 'H':				// HRESULT 형식의 오류코드를 문자열 설명으로 변경
                    {
                        strcpy( fp, "s" );
                        HLOCAL hLocal = NULL;
                        BOOL IsResult = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            NULL, va_arg( args, int ), MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ), (PSTR)&hLocal, 0, NULL );

                        if( IsResult != FALSE )
                        {
                            PSTR pszBuffer = (PSTR)::LocalLock( hLocal );

                            if( getBufferA( strlen( pszBuffer ), pLOGDATA ) == false )
                                break;
                            nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, pszBuffer );
                            LocalFree( hLocal );
                        }
                        else
                        {
                            if( getBufferA( strlen("<FormatMessage Fail>"), pLOGDATA ) == false )
                                break;

                            nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, "<FormatMessage Fail>" );
                        }
                    }
                    break;

                case 'f':				// 부동소수점, C 언어는 인자를 받을 때 float 와 double 을 모두 double 로 상승시켜서 받음
                    fp[0] = *fmt;
                    fp[1] = '\0';

                    if( getBufferA( _CVTBUFSIZE, pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, va_arg( args, double ) );
                    break;

                case 'F':				// 소스코드 함수이름
                    strcpy( fp, "s" );
                    if( getBufferA( strlen( szFuncName ), pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, szFuncName );
                    break;

                case 'i':				// 64비트 정수
                case 'I':
                    if( ( fmt[ 1 ] == '6' ) && ( fmt[ 2 ] == '4' ) && ( fmt[ 3 ] == 'd' || fmt[ 3 ] == 'u' ) )
                    {
                        *fp++ = 'I';
                        *fp++ = fmt[1];
                        *fp++ = fmt[2];
                        *fp++ = fmt[3];
                        *fp = '\0';

                        if( getBufferA( 21 /* 2 ^ 64 최대 정수형의 자리수 */, pLOGDATA ) == false )
                            break;

                        nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, va_arg( args, __int64 ) );
                    }
                    break;

                case 'l':				// 로그 레벨 문자열로
                    strcpy( fp, "s" );
                    if( getBufferA( 14 /* 로그 레벨 최대 길이 */, pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, getLogLevelA() );
                    break;

                case 'L':				// 소스코드 줄 번호
                    strcpy( fp, "u" );
                    if( getBufferA( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, nLineNumber );
                    break;

                case 'N':				// 소스코드 파일이름
                    strcpy( fp, "s" );
                    if( getBufferA( strlen( szFileName ), pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, szFileName );
                    break;

                case 'P':				// 프로세스 ID
                    strcpy( fp, "u" );
                    if( getBufferA( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, ::GetCurrentProcessId() );
                    break;

                case 't':				// 시간 출력
                    {
                        strcpy( fp, "s" );
                        struct tm tmTemp;
                        __time64_t tmCurrentTime = _time64( NULL );
                        errno_t err = ::_localtime64_s( &tmTemp, &tmCurrentTime );
                        char szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

                        if( err != 0 || !strftime( szBuffer, MAX_TIME_BUFFER_SIZE, m_szTimeFormatA, &tmTemp ) )
                        {
                            if( getBufferA( 15 /* L"<Unknown Date>" 의 길이 */, pLOGDATA ) == false )
                                break;

                            nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, "<Unknown Time>" );
                        }
                        else
                        {
                            if( getBufferA( strlen( szBuffer ), pLOGDATA ) == false )
                                break;

                            nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, szBuffer );
                        }
                    }
                    break;

                case 'T':				// 스레드 ID
                    strcpy( fp, "u" );
                    if( getBufferA( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, ::GetCurrentThreadId() );
                    break;

                case 's':				// 문자열
                    {
                        char* pszArgs = va_arg( args, char* );
                        strcpy( fp, "s" );
                        if( pszArgs == NULL )
                        {
                            if( getBufferA( 7 /* L"(null)" 의 길이 */, pLOGDATA ) == false )
                                break;

                            nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, "(null)" );
                        }
                        else
                        {
                            if( getBufferA( strlen( pszArgs ) + 1, pLOGDATA ) == false )
                                break;

                            nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, pszArgs );
                        }
                    }
                    break;

                case L'o':				// 8진수
                case L'x':				// 16진수, 문자는 소문자
                case L'X':				// 16진수, 문자는 대문자
                case L'd':				// 부호있는 정수
                case L'u':				// 부호없는 정수
                    fp[0] = *fmt;
                    fp[1] = '\0';
                    if( getBufferA( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, pLOGDATA ) == false )
                        break;

                    nResult = _snprintf( pLOGDATA->szNextMsgPtrA, pLOGDATA->dwRemainBufferSizeA, szFormat, va_arg( args, int ) );
                    break;

                default:
                    while( start_format != fmt && pLOGDATA->dwRemainBufferSizeA > 0 )
                    {
                        *pLOGDATA->szNextMsgPtrA++ = *start_format++;
                        --pLOGDATA->dwRemainBufferSizeA;
                    }

                    if( pLOGDATA->dwRemainBufferSizeA > 0 )
                    {
                        *pLOGDATA->szNextMsgPtrA++ = *fmt;
                        --pLOGDATA->dwRemainBufferSizeA;
                    }
                    break;
                } // switch

                if( nResult > 0 ) // snprintf 에 기록된 양이 있으면 그만큼 버퍼 포인터 이동 및 남은 버퍼크기 감소 
                {
                    pLOGDATA->szNextMsgPtrA += nResult;
                    pLOGDATA->dwRemainBufferSizeA -= nResult;
                    nRetSize += nResult;
                }

                ++fmt;
            } // while

        } // if

    } // while

    if( isAppendNewLine == true && 
        getBufferA( 1 /* L"\n" 의 길이 */, pLOGDATA ) == true )
    {
        strcpy( pLOGDATA->szNextMsgPtrA, "\n" );
        pLOGDATA->szNextMsgPtrA += 1;
        nRetSize += 1;
        pLOGDATA->dwRemainBufferSizeA -= 1;
    }

    return nRetSize;
}

size_t CLog::log_arg_list( const wchar_t* wszFileName, const wchar_t* wszFuncName, unsigned int nLineNumber, const wchar_t* fmt, bool isAppendNewLine, LOGDATA* pLOGDATA, va_list args )
{
    assert( fmt != NULL );

    int nRetSize = 0;

    while( *fmt != L'\0' )
    {
        if( (*fmt != L'%') || 
            (*fmt == L'%' && fmt[1] == L'%') )
        {
            // % 가 없으면 %가 나타날때까지 복사
            *pLOGDATA->szNextMsgPtrW++ = *fmt++;
            if( *fmt == L'%' && fmt[1] == L'%' )
                ++fmt;
            --pLOGDATA->dwRemainBufferSizeW;
            nRetSize++;
        }
        else
        {
            bool isDone = false;
            const wchar_t* start_format = fmt;
            wchar_t* fp = NULL;
            wchar_t szFormat[ 64 ] = {0,};
            int nResult = 0; // snprintf 가 기록한 양 또는 기록하는데 필요한 양

            fp = szFormat;
            *fp++ = *fmt++; // % 문자를 넣음
            while( isDone == false )
            {
                isDone = true;
                nResult = 0;

                switch( *fmt )
                {
                case L'b':
                case L'B':
                    wcscpy( fp, L"s" );
                    if( getBufferW( 6 /* FALSE 길이 */, pLOGDATA ) == false )
                        break;
                    
                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, va_arg( args, bool ) == true ? L"TRUE" : L"FALSE" );
                    break;

                case L'D':				// 날짜 출력
                    {
                        wcscpy( fp, L"s" );
                        struct tm tmTemp;
                        __time64_t tmCurrentTime = _time64( NULL );
                        errno_t err = ::_localtime64_s( &tmTemp, &tmCurrentTime );
                        wchar_t szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

                        if( err != 0 || !wcsftime( szBuffer, MAX_TIME_BUFFER_SIZE, m_szDateFormatW, &tmTemp ) )
                        {
                            if( getBufferW( 15 /* L"<Unknown Date>" 의 길이 */, pLOGDATA ) == false )
                                break;

                            nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, L"<Unknown Date>" );
                        }
                        else
                        {
                            if( getBufferW( wcslen( szBuffer ), pLOGDATA ) == false )
                                break;

                            nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, szBuffer );
                        }
                    }
                    break;
                case L'E':				// GetLastError() 의 오류코드를 문자열 설명으로 변경
                case L'H':				// HRESULT 형식의 오류코드를 문자열 설명으로 변경
                    {
                        wcscpy( fp, L"s" );
                        HLOCAL hLocal = NULL;
                        BOOL IsResult = FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            NULL, va_arg( args, int ), MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ), (PWSTR)&hLocal, 0, NULL );

                        if( IsResult != FALSE )
                        {
                            PWSTR pwszBuffer = (PWSTR)::LocalLock( hLocal );

                            if( getBufferW( wcslen( pwszBuffer ), pLOGDATA ) == false )
                                break;
                            nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, pwszBuffer );
                            LocalFree( hLocal );
                        }
                        else
                        {
                            if( getBufferW( wcslen(L"<FormatMessage Fail>"), pLOGDATA ) == false )
                                break;

                            nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, L"<FormatMessage Fail>" );
                        }
                    }
                    break;

                case L'f':				// 부동소수점, C 언어는 인자를 받을 때 float 와 double 을 모두 double 로 상승시켜서 받음
                    fp[0] = *fmt;
                    fp[1] = L'\0';

                    if( getBufferW( _CVTBUFSIZE, pLOGDATA ) == false )
                        break;

                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, va_arg( args, double ) );
                    break;

                case L'F':				// 소스코드 함수이름
                    wcscpy( fp, L"s" );
                    if( getBufferW( wcslen( wszFuncName ), pLOGDATA ) == false )
                        break;

                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, wszFuncName );
                    break;

                case L'i':				// 64비트 정수
                case L'I':
                    if( ( fmt[ 1 ] == L'6' ) && ( fmt[ 2 ] == L'4' ) && ( fmt[ 3 ] == L'd' || fmt[ 3 ] == L'u' ) )
                    {
                        *fp++ = L'I';
                        *fp++ = fmt[1];
                        *fp++ = fmt[2];
                        *fp++ = fmt[3];
                        *fp = L'\0';

                        if( getBufferW( 21 /* 2 ^ 64 최대 정수형의 자리수 */,pLOGDATA ) == false )
                            break;

                        nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, va_arg( args, __int64 ) );
                    }
                    break;

                case L'l':				// 로그 레벨 문자열로
                    wcscpy( fp, L"s" );
                    if( getBufferW( 14 /* 로그 레벨 최대 길이 */, pLOGDATA ) == false )
                        break;

                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, getLogLevelW() );
                    break;

                case L'L':				// 소스코드 줄 번호
                    wcscpy( fp, L"u" );
                    if( getBufferW( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, pLOGDATA ) == false )
                        break;

                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, nLineNumber );
                    break;

                case L'N':				// 소스코드 파일이름
                    wcscpy( fp, L"s" );
                    if( getBufferW( wcslen( wszFileName ), pLOGDATA ) == false )
                        break;

                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, wszFileName );
                    break;

                case L'P':				// 프로세스 ID
                    wcscpy( fp, L"u" );
                    if( getBufferW( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, pLOGDATA ) == false )
                        break;

                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, ::GetCurrentProcessId() );
                    break;

                case L't':				// 시간 출력
                    {
                        wcscpy( fp, L"s" );
                        struct tm tmTemp;
                        __time64_t tmCurrentTime = _time64( NULL );
                        errno_t err = ::_localtime64_s( &tmTemp, &tmCurrentTime );
                        wchar_t szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

                        if( err != 0 || !wcsftime( szBuffer, MAX_TIME_BUFFER_SIZE, m_szTimeFormatW, &tmTemp ) )
                        {
                            if( getBufferW( 15 /* L"<Unknown Date>" 의 길이 */, pLOGDATA ) == false )
                                break;

                            nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, L"<Unknown Time>" );
                        }
                        else
                        {
                            if( getBufferW( wcslen( szBuffer ), pLOGDATA ) == false )
                                break;

                            nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, szBuffer );
                        }
                    }
                    break;

                case L'T':				// 스레드 ID
                    wcscpy( fp, L"u" );
                    if( getBufferW( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, pLOGDATA ) == false )
                        break;

                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, ::GetCurrentThreadId() );
                    break;

                case L's':				// 문자열
                    {
                        wchar_t* pszArgs = va_arg( args, wchar_t* );
                        wcscpy( fp, L"s" );
                        if( pszArgs == NULL )
                        {
                            if( getBufferW( 7 /* L"(null)" 의 길이 */, pLOGDATA ) == false )
                                break;

                            nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, L"(null)" );
                        }
                        else
                        {
                            if( getBufferW( wcslen( pszArgs ) + 1, pLOGDATA ) == false )
                                break;

                            nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, pszArgs );
                        }
                    }
                    break;

                case L'o':				// 8진수
                case L'x':				// 16진수, 문자는 소문자
                case L'X':				// 16진수, 문자는 대문자
                case L'd':				// 부호있는 정수
                case L'u':				// 부호없는 정수
                    fp[0] = *fmt;
                    fp[1] = L'\0';
                    if( getBufferW( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, pLOGDATA ) == false )
                        break;

                    nResult = _snwprintf( pLOGDATA->szNextMsgPtrW, pLOGDATA->dwRemainBufferSizeW, szFormat, va_arg( args, int ) );
                    break;

                default:
                    while( start_format != fmt && pLOGDATA->dwRemainBufferSizeW > 0 )
                    {
                        *pLOGDATA->szNextMsgPtrW++ = *start_format++;
                        --pLOGDATA->dwRemainBufferSizeW;
                    }

                    if( pLOGDATA->dwRemainBufferSizeW > 0 )
                    {
                        *pLOGDATA->szNextMsgPtrW++ = *fmt;
                        --pLOGDATA->dwRemainBufferSizeW;
                    }
                    break;
                } // switch

                if( nResult > 0 ) // snprintf 에 기록된 양이 있으면 그만큼 버퍼 포인터 이동 및 남은 버퍼크기 감소 
                {
                    pLOGDATA->szNextMsgPtrW += nResult;
                    pLOGDATA->dwRemainBufferSizeW -= nResult;
                    nRetSize += nResult;
                }

                ++fmt;
            } // while

        } // if

    } // while

    if( isAppendNewLine == true && 
        getBufferW( 1 /* L"\n" 의 길이 */, pLOGDATA ) == true )
    {
        wcscpy( pLOGDATA->szNextMsgPtrW, L"\n" );
        pLOGDATA->szNextMsgPtrW += 1;
        nRetSize += 1;
        pLOGDATA->dwRemainBufferSizeW -= 1;
    }
    
    return nRetSize;
}

bool CLog::getBufferA( UINT_PTR nRequiredSize, LOGDATA* pLOGDATA )
{
    bool isSuccess = true;

    do 
    {
        if( pLOGDATA->dwRemainBufferSizeA > nRequiredSize )
            break;

        do 
        {
            pLOGDATA->dwBufferSizeA += MSG_BUFFER_SIZE;
            pLOGDATA->dwRemainBufferSizeA += MSG_BUFFER_SIZE;

        } while ( ( pLOGDATA->dwRemainBufferSizeA < nRequiredSize ) );

        char* pBuffer = new char[ pLOGDATA->dwBufferSizeA ];
        if( pBuffer == NULL )
        {
            isSuccess = false;
            break;
        }

        ZeroMemory( pBuffer, pLOGDATA->dwBufferSizeA * sizeof( char ) );
        strcpy( pBuffer, pLOGDATA->szLogData );

        ptrdiff_t nBetween = pLOGDATA->szNextMsgPtrA - pLOGDATA->szLogData;
        delete [] pLOGDATA->szLogData;
        pLOGDATA->szLogData = pBuffer;

        pLOGDATA->szNextMsgPtrA = pLOGDATA->szLogData + nBetween;

    } while (false);

    return isSuccess;
}

bool CLog::getBufferW( UINT_PTR nRequiredSize, LOGDATA* pLOGDATA )
{
    bool isSuccess = true;

    do 
    {
        if( pLOGDATA->dwRemainBufferSizeW > nRequiredSize )
            break;

        do 
        {
            pLOGDATA->dwBufferSizeW += MSG_BUFFER_SIZE;
            pLOGDATA->dwRemainBufferSizeW += MSG_BUFFER_SIZE;

        } while ( ( pLOGDATA->dwRemainBufferSizeW < nRequiredSize ) );

        wchar_t* pBuffer = new wchar_t[ pLOGDATA->dwBufferSizeW ];
        if( pBuffer == NULL )
        {
            isSuccess = false;
            break;
        }

        ZeroMemory( pBuffer, pLOGDATA->dwBufferSizeW * sizeof( wchar_t ) );
        wcscpy( pBuffer, pLOGDATA->wszLogData );

        ptrdiff_t nBetween = pLOGDATA->szNextMsgPtrW - pLOGDATA->wszLogData;
        delete [] pLOGDATA->wszLogData;
        pLOGDATA->wszLogData = pBuffer;

        pLOGDATA->szNextMsgPtrW = pLOGDATA->wszLogData + nBetween;

    } while (false);

    return isSuccess;
}

const char* CLog::getLogLevelA()
{
    switch( m_eCurrentLogLevel )
    {
    case LL_NONE:
        return "LL_NONE";
        break;
    case LL_CRITICAL:
        return "LL_CRITICAL";
        break;
    case LL_TRACE:
        return "LL_TRACE";
        break;
    case LL_DEBUG:
        return "LL_DEBUG";
        break;
    case LL_WARNING:
        return "LL_WARNING";
        break;
    case LL_ERROR:
        return "LL_ERROR";
        break;
    }

    return "UNKNOWN LEVEL";
}

const wchar_t* CLog::getLogLevelW()
{
    switch( m_eCurrentLogLevel )
    {
    case LL_NONE:
        return L"LL_NONE";
        break;
    case LL_CRITICAL:
        return L"LL_CRITICAL";
        break;
    case LL_TRACE:
        return L"LL_TRACE";
        break;
    case LL_DEBUG:
        return L"LL_DEBUG";
        break;
    case LL_WARNING:
        return L"LL_WARNING";
        break;
    case LL_ERROR:
        return L"LL_ERROR";
        break;
    }

    return L"UNKNOWN LEVEL";
}
