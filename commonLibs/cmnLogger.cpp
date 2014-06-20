#include "stdafx.h"

#include <io.h>
#include <conio.h>
#include <memory.h>
#include <cassert>

#ifndef USE_SHELL_SUPPORT
#define USE_SHELL_SUPPORT
#endif

#include "cmnLogger.hpp"
#include "cmnConverter.h"
#include "cmnDateTime.h"
#include "cmnPath.h"

#ifdef USE_CMNLOGGER_GPBERROR
#include "google/protobuf/stubs/common.h"
#endif

#ifdef USE_CMNLOGGER_QTDEBUG
#include <QtCore>
#include <QtDebug>
#endif

#pragma warning( disable: 4996 )

#pragma execution_character_set( "utf-8" )

using namespace nsCommon;
using namespace nsCommon::nsCmnLogger;
using namespace nsCommon::nsCmnConvert;
using namespace nsCommon::nsCmnDateTime;
using namespace nsCommon::nsCmnPath;

CRITICAL_SECTION            nsCommon::nsCmnLogger::CLogUtil::_csFile;
boost::unordered_map< std::wstring, TyPrFileToMutex >
                            nsCommon::nsCmnLogger::CLogUtil::_mapFileNameToDest;
nsCommon::CRoundRobinAllocator< LOG_CONTEXT > nsCommon::nsCmnLogger::CLogUtil::_rbAllocator( 256 );

volatile bool               nsCommon::nsCmnLogger::CLogFactory::_isExit = false;
volatile bool               nsCommon::nsCmnLogger::CLogFactory::_isInit = false;
CRITICAL_SECTION            nsCommon::nsCmnLogger::CLogFactory::_csFactory;
LoggerConfiguration         nsCmnLogger::CLogFactory::_defaultConfiguration;
boost::unordered_map< std::string, CLogger* >
    nsCommon::nsCmnLogger::CLogFactory::_mapNameToLogger;
boost::thread               nsCommon::nsCmnLogger::CLogFactory::_workerThread;
boost::thread               nsCommon::nsCmnLogger::CLogFactory::_workerFileThread;
CSharedQueue< LOG_CONTEXT* >
                            nsCommon::nsCmnLogger::CLogFactory::_queue;
boost::unordered_map< TyPrFileToMutex, CSharedQueue< std::wstring >* >
                            nsCommon::nsCmnLogger::CLogFactory::_fileWriteQueue;

CRITICAL_SECTION            nsCommon::nsCmnLogger::CLogUtil::_csCallback;
boost::unordered_map< std::string, TyPrCallbackAToMutex >    
							nsCommon::nsCmnLogger::CLogUtil::_mapFileNameToCallbackA;
boost::unordered_map< std::wstring, TyPrCallbackWToMutex >    
							nsCommon::nsCmnLogger::CLogUtil::_mapFileNameToCallbackW;
//////////////////////////////////////////////////////////////////////////
/// 다른 로거 시스템 지원

#ifdef USE_CMNLOGGER_GPBERROR

void GPBLogHandler( google::protobuf::LogLevel gpbLogLevel, const char* filename, int line, const std::string& message )
{
    switch( gpbLogLevel )
    {
        case google::protobuf::LOGLEVEL_INFO:
            LM_DEBUG_TO( "network/gpblibrary", L"[GPB ERROR] FileName = %1:%2, Message = %3", filename, line, message );
            break;

        case google::protobuf::LOGLEVEL_WARNING:
            LM_WARNING_TO( "network/gpblibrary", L"[GPB ERROR] FileName = %1:%2, Message = %3", filename, line, message );
            break;

        case google::protobuf::LOGLEVEL_ERROR:
            LM_ERROR_TO( "network/gpblibrary", L"[GPB ERROR] FileName = %1:%2, Message = %3", filename, line, message );
            break;

        case google::protobuf::LOGLEVEL_FATAL:
            LM_CRITICAL_TO( "network/gpblibrary", L"[GPB ERROR] FileName = %1:%2, Message = %3", filename, line, message );
            break;
    }
}

#endif

#ifdef USE_CMNLOGGER_QTDEBUG
void QtLogHandler( QtMsgType type, const QMessageLogContext& context, const QString& msg )
{
    switch( type )
    {
        case QtDebugMsg:
            LM_DEBUG_TO( "root/qtframework", L"[QT Framework] Category = %1, FileName = %2:%3, Function = %4, Message = %5",
                context.category, context.file, context.line, context.function, msg );
            break;
        case QtWarningMsg:
            LM_WARNING_TO( "root/qtframework", L"[QT Framework] Category = %1, FileName = %2:%3, Function = %4, Message = %5",
                context.category, context.file, context.line, context.function, msg );
            break;
        case QtCriticalMsg:
            LM_ERROR_TO( "root/qtframework", L"[QT Framework] Category = %1, FileName = %2:%3, Function = %4, Message = %5",
                context.category, context.file, context.line, context.function, msg );
            break;
        case QtFatalMsg:
            LM_CRITICAL_TO( "root/qtframework", L"[QT Framework] Category = %1, FileName = %2:%3, Function = %4, Message = %5",
                context.category, context.file, context.line, context.function, msg );
            break;
    }
}
#endif

//////////////////////////////////////////////////////////////////////////

LOG_CONTEXT* nsCommon::nsCmnLogger::CLogUtil::CheckOutLOGContext()
{
    LOG_CONTEXT* pContext = NULLPTR;

    do{ pContext = _rbAllocator.CheckOut(); if( pContext != NULLPTR ) break; SleepEx(1,FALSE); } while( pContext == NULLPTR ); 

    return pContext;
}

bool nsCommon::nsCmnLogger::CLogUtil::IsExistLOGFile()
{
    CScopedCriticalSection lck( _csFile );
    return _mapFileNameToDest.empty() == false ? true : false;
}

nsCommon::nsCmnLogger::TyPrFileToMutex nsCommon::nsCmnLogger::CLogUtil::GetFile( LoggerConfiguration* lc )
{
    CScopedCriticalSection lck( _csFile );
    lc->currentFullFileName = _mapFileNameToDest.begin()->first;
    return _mapFileNameToDest[ lc->currentFullFileName ];
}

TyPrFileToMutex nsCommon::nsCmnLogger::CLogUtil::GetFile( const std::wstring& fileName )
{
    CScopedCriticalSection lck( _csFile );
    return _mapFileNameToDest[ fileName ];
}

void nsCommon::nsCmnLogger::CLogUtil::CheckInLOGContext( LOG_CONTEXT* object )
{
    _rbAllocator.CheckIn( object );
}

nsCommon::nsCmnLogger::TyPrFileToMutex nsCommon::nsCmnLogger::CLogUtil::MakeLOGFile( LoggerConfiguration* lc )
{
    CScopedCriticalSection lck( _csFile );
    TyPrFileToMutex prFileToMutex( NULLPTR, NULLPTR );

    do 
    {
        if( CreateDirectoryRecursively( lc->filePath.c_str() ) == false )
            break;

        CheckOldFileExist( lc );
        CheckFileExist( lc );

        prFileToMutex.first = _wfopen( lc->currentFullFileName.c_str(), L"a+t, ccs=UTF-16LE" );
        if( prFileToMutex.first == NULLPTR )
        {
            lc->currentFullFileName = L"";
            break;
        }

        prFileToMutex.second = new CRITICAL_SECTION;
        ::InitializeCriticalSectionAndSpinCount( prFileToMutex.second, 4000 );

        static BYTE utf16_le_bom[] = { 0xFF, 0xFE };
        fwrite( utf16_le_bom, sizeof(BYTE), _countof( utf16_le_bom ), prFileToMutex.first );

        _mapFileNameToDest[ lc->currentFullFileName ] = prFileToMutex;
    } while (false);

    return prFileToMutex;
}


void nsCommon::nsCmnLogger::CLogUtil::WriteData( LOG_CONTEXT* logContext )
{
    assert( logContext != NULLPTR );
    assert( logContext->loggerConfiguration != NULLPTR );

    if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false )
        return;

    LoggerConfiguration* lc = logContext->loggerConfiguration;
    
    if( lc->logTransferMethods & LT_DEBUGGER )
    {
        if( logContext->isUnicodeData == true )
            ::OutputDebugStringW( logContext->logDataW );
        else
            ::OutputDebugStringA( logContext->logDataA );
    }

    if( lc->logTransferMethods & LT_STDERR )
    {
        if( logContext->isUnicodeData == true )
        {
            std::wcerr << logContext->logDataW;
            std::wcerr.flush();
        }
        else
        {
            std::cerr << logContext->logDataA;
            std::cerr.flush();
        }
    }

    if( lc->logTransferMethods & LT_STDOUT )
    {
        if( logContext->isUnicodeData == true )
        {
            std::wcout << logContext->logDataW;
            std::wcout.flush();
        }
        else
        {
            std::cout << logContext->logDataA;
            std::cout.flush();
        }

    }

    if( lc->logTransferMethods & LT_FILE )
    {
        /*!
            파일에 데이터를 기록하는 부분
            
            #. currentFullFileName 이 존재하는지 확인
            #. 존재하지 않으면 현재 기록할 로그 파일이 없으므로 로그 파일 생성
            #. 해당 파일에 대한 FILE* 를 얻음
            #. 파일의 용량과 날짜를 점검하여 파일을 나눌지 결정함
            #. 파일을 나누게 되면 기존 파일에 대해 반드시 fclose 을 호출해야함.
            #. 파일에 기록
        */
        TyPrFileToMutex prFileToMutex;
        LoggerConfiguration* logger = lc->isSplitByLogger == false ? &(CLogFactory::GetDefaultConfiguration()) : lc;

        if( logger->currentFullFileName.empty() == true )
        {
            if( logger->isSplitByLogger == true )
            {
                prFileToMutex = MakeLOGFile( logger );
            }
            else
            {
                prFileToMutex = IsExistLOGFile() == true ? GetFile( logger ) : MakeLOGFile( logger );
            }
        }
        else
        {
            prFileToMutex = GetFile( logger->currentFullFileName );
        }

        do 
        {
            if( prFileToMutex.first == NULLPTR || prFileToMutex.second == NULLPTR )
                break;

            if( CheckFileSizeAndDate( lc->currentFullFileName.c_str(), logger ) == true )
            {
                prFileToMutex = MakeLOGFile( logger );
                if( prFileToMutex.first == NULLPTR || prFileToMutex.second == NULLPTR )
                    break;
            }

            if( logger->isUseBatchFileWrite == true )
            {
                if( CLogFactory::_fileWriteQueue.count( prFileToMutex ) <= 0 )
                    CLogFactory::_fileWriteQueue[ prFileToMutex ] = new CSharedQueue< std::wstring >();

                CSharedQueue< std::wstring >* spQueue = CLogFactory::_fileWriteQueue[ prFileToMutex ];
                if( logContext->isUnicodeData == true )
                    spQueue->Push( logContext->logDataW );
                else
                    spQueue->Push( CA2U(logContext->logDataA).c_str() );
            }
            else
            {
                CScopedCriticalSection lck( *prFileToMutex.second );
                if( logContext->isUnicodeData == true )
                    fputws( logContext->logDataW, prFileToMutex.first );
                else
                    fputws( CA2U(logContext->logDataA), prFileToMutex.first );
                fflush( prFileToMutex.first );
            }

        } while (false);
    }

	if( lc->logTransferMethods & LT_CALLBACK )
	{
		if( logContext->isUnicodeData == true )
		{
			TyPrCallbackWToMutex _mapCallbackW = GetCallback( lc->loggerNameW );

			for( std::map< pfnLogCallbackW, PVOID >::iterator it = _mapCallbackW.first->begin(); it != _mapCallbackW.first->end(); ++it )
				(*it->first)( it->second, logContext->logLevel, logContext->logDataW );
		}
		else
		{
			TyPrCallbackAToMutex _mapCallbackA = GetCallback( lc->loggerNameA );

			for( std::map< pfnLogCallbackA, PVOID >::iterator it = _mapCallbackA.first->begin(); it != _mapCallbackA.first->end(); ++it )
				(*it->first)( it->second, logContext->logLevel, logContext->logDataA );
		}
	}
    
    _rbAllocator.CheckIn( logContext );
}

void nsCommon::nsCmnLogger::CLogUtil::CheckFileExist( LoggerConfiguration* lc )
{
    __time64_t tmCurrentTime = ::_time64(NULL);
    _localtime64_s( &lc->currentFileDate, &tmCurrentTime );

    std::wstring strCurrentDateText = nsCommon::nsCmnDateTime::GetFormattedDateText( ::_time64(NULL), L"%Y-%m-%d" );

    lc->currentFileIndex = 0;

    std::wstring strFileName;
    if( lc->isSplitByLogger == false )
    {
        strFileName = nsCmnFormatter::tsFormat( L"%1\\%2_%3-[*].%4", lc->filePath, lc->fileName, strCurrentDateText, lc->fileExt );
    }
    else
    {
        strFileName = nsCmnFormatter::tsFormat( L"%1\\%2_%3_%4-[*].%5", 
            lc->filePath, lc->fileName, string_replace_all( lc->loggerNameW, L"/", L"_" ), strCurrentDateText, lc->fileExt );
    }

    _wfinddata_t data;
    intptr_t hSearch = _wfindfirst( strFileName.c_str(), &data );

    do 
    {
        if( hSearch == -1 )
            break;
        
        ++lc->currentFileIndex;

        while( _wfindnext( hSearch, &data) != -1 )
            ++lc->currentFileIndex;

    } while (false);

    if( lc->isSplitByFileIndex == false
        && lc->currentFileIndex != 0 )
    {
        lc->currentFileIndex--;
    }

    bool bNextFile = false;
    do 
    {
        bNextFile = false;

        if( lc->isSplitByLogger == false )
        {
            lc->currentFullFileName = nsCmnFormatter::tsFormat( L"%1\\%2_%3-[%4].%5",
                lc->filePath, lc->fileName, strCurrentDateText, lc->currentFileIndex, lc->fileExt );
        }
        else
        {
            lc->currentFullFileName = nsCmnFormatter::tsFormat( L"%1\\%2_%3_%4-[%5].%6",
                lc->filePath, lc->fileName, string_replace_all( lc->loggerNameW, L"/", L"_" ), strCurrentDateText, lc->currentFileIndex, lc->fileExt );
        }

        if( CheckFileSizeAndDate( lc->currentFullFileName.c_str(), lc ) == true )
        {
            bNextFile = true;
            lc->currentFileIndex++;
        }

    } while( bNextFile == true );

    if( hSearch != -1 )
        _findclose( hSearch );
}

bool nsCommon::nsCmnLogger::CLogUtil::CheckFileSizeAndDate( const wchar_t* wszFullFileName, LoggerConfiguration* lc )
{
    bool isNeedSplit = false;

    do 
    {
        // 파일 용량 관련 점검
        if( lc->fileSizeLimit == 0 )
            break;

        static const __int64 MEGABYTE = 1024 * 1024;

        struct __stat64 statFile;
        if( _wstat64( wszFullFileName, &statFile ) == -1 )
            break;

        __int64 llFileSize = statFile.st_size;

        if( llFileSize >= ( lc->fileSizeLimit * MEGABYTE ) )
            isNeedSplit = true;

    } while (false);

    do 
    {
        // 날짜 관련 점검
        if( lc->isSplitWhenDate == false )
            break;

        __time64_t currentTime = ::_time64(NULL);
        struct tm tmCurrentTime;

        errno_t err = _localtime64_s( &tmCurrentTime, &currentTime );
        if( err != 0 )
            break;

        if( tmCurrentTime.tm_yday > lc->currentFileDate.tm_yday )
            isNeedSplit = true;

    } while( false );

    return isNeedSplit;
}

void nsCommon::nsCmnLogger::CLogUtil::CheckOldFileExist( LoggerConfiguration* lc )
{
    // 0 이면 삭제하지 않음
    if( lc->maxLogFileDateCount <= 0 )
        return;

    _wfinddata_t data;
    __time64_t tmCurrentTime = ::_time64(NULL);
    _localtime64_s( &lc->currentFileDate, &tmCurrentTime );

    std::wstring strFileName = nsCmnFormatter::tsFormat( L"%1\\%2_*.%3", lc->filePath, lc->fileName, lc->fileExt );
    intptr_t hSearch = _wfindfirst( strFileName.c_str(), &data );

    unsigned int maxLogTimeSpan = lc->maxLogFileDateCount * 60 * 60 * 24;

    do 
    {
        if( hSearch == -1 )
            break;

		do 
		{
			if( (tmCurrentTime - data.time_write) >= maxLogTimeSpan )
				DeleteFile( nsCmnFormatter::tsFormat( L"%1\\%2", lc->filePath, data.name ).c_str() );

		} while( _wfindnext( hSearch, &data) != -1 );

    } while (false);

    if( hSearch != -1 )
        _findclose( hSearch );
}

const char* nsCommon::nsCmnLogger::CLogUtil::GetLoglevelTextA( CLOG_LEVEL logLevel )
{
    switch( logLevel )
    {
    case LL_NONE:
        return "LL_NONE";
        break;
    case LL_CRITICAL:
        return "LL_CRITICAL";
        break;
    case LL_ERROR:
        return "LL_ERROR";
        break;
    case LL_WARNING:
        return "LL_WARNING";
        break;
    case LL_DEBUG:
        return "LL_DEBUG";
        break;
    case LL_TRACE:
        return "LL_TRACE";
        break;
    default:
        return "Unknown Level";
    }
}

const wchar_t* nsCommon::nsCmnLogger::CLogUtil::GetLoglevelTextW( CLOG_LEVEL logLevel )
{
    switch( logLevel )
    {
    case LL_NONE:
        return L"LL_NONE";
        break;
    case LL_CRITICAL:
        return L"LL_CRITICAL";
        break;
    case LL_ERROR:
        return L"LL_ERROR";
        break;
    case LL_WARNING:
        return L"LL_WARNING";
        break;
    case LL_DEBUG:
        return L"LL_DEBUG";
        break;
    case LL_TRACE:
        return L"LL_TRACE";
        break;
    default:
        return L"Unknown Level";
    }
}

TyPrCallbackAToMutex nsCommon::nsCmnLogger::CLogUtil::GetCallback( const std::string& fileName )
{
	CScopedCriticalSection lck( _csCallback );
	
	if( _mapFileNameToCallbackA.count( fileName ) < 1 )
		return MakeCallback( fileName );

	return _mapFileNameToCallbackA[ fileName ];
}

TyPrCallbackWToMutex nsCommon::nsCmnLogger::CLogUtil::GetCallback( const std::wstring& fileName )
{
	CScopedCriticalSection lck( _csCallback );

	if( _mapFileNameToCallbackW.count( fileName ) < 1 )
		return MakeCallback( fileName );

	return _mapFileNameToCallbackW[ fileName ];
}

TyPrCallbackAToMutex nsCommon::nsCmnLogger::CLogUtil::MakeCallback( const std::string& fileName )
{
	CScopedCriticalSection lck( _csCallback );

	TyPrCallbackAToMutex prCallbackToMutex( NULLPTR, NULLPTR );

	do 
	{
		prCallbackToMutex.first = new TyMapCallbackA;
		prCallbackToMutex.second = new CRITICAL_SECTION;
		::InitializeCriticalSectionAndSpinCount( prCallbackToMutex.second, 4000 );

		_mapFileNameToCallbackA[ fileName ] = prCallbackToMutex;
	
	} while( false );	

	return _mapFileNameToCallbackA[ fileName ];
}

TyPrCallbackWToMutex nsCommon::nsCmnLogger::CLogUtil::MakeCallback( const std::wstring& fileName )
{
	CScopedCriticalSection lck( _csCallback );

	TyPrCallbackWToMutex prCallbackToMutex( NULLPTR, NULLPTR );

	do 
	{
		prCallbackToMutex.first = new TyMapCallbackW;
		prCallbackToMutex.second = new CRITICAL_SECTION;
		::InitializeCriticalSectionAndSpinCount( prCallbackToMutex.second, 4000 );

		_mapFileNameToCallbackW[ fileName ] = prCallbackToMutex;

	} while( false );	

	return _mapFileNameToCallbackW[ fileName ];
}

//////////////////////////////////////////////////////////////////////////

nsCommon::nsCmnLogger::CLogger::CLogger( const char* loggerNameA, const wchar_t* loggerNameW )
{
    memset( _loggerNameA, '\0', sizeof( char ) * MAX_LOGGERNAME_SIZE );
    memset( _loggerNameW, '\0', sizeof( wchar_t ) * MAX_LOGGERNAME_SIZE );

    assert( loggerNameA != NULLPTR );
    assert( loggerNameW != NULLPTR );

    strncpy_s( _loggerNameA, MAX_LOGGERNAME_SIZE, loggerNameA, _TRUNCATE );
    wcsncpy_s( _loggerNameW, MAX_LOGGERNAME_SIZE, loggerNameW, _TRUNCATE );

	_loggerConfiguration.loggerNameA = _loggerNameA;
	_loggerConfiguration.loggerNameW = _loggerNameW;
}

nsCommon::nsCmnLogger::CLogger::CLogger( const char* loggerNameA, const wchar_t* loggerNameW, const LoggerConfiguration& loggerConfiguration )
{
    memset( _loggerNameA, '\0', sizeof( char ) * MAX_LOGGERNAME_SIZE );
    memset( _loggerNameW, '\0', sizeof( wchar_t ) * MAX_LOGGERNAME_SIZE );

    assert( loggerNameA != NULLPTR );
    assert( loggerNameW != NULLPTR );

    strncpy_s( _loggerNameA, MAX_LOGGERNAME_SIZE, loggerNameA, _TRUNCATE );
    wcsncpy_s( _loggerNameW, MAX_LOGGERNAME_SIZE, loggerNameW, _TRUNCATE );

	_loggerConfiguration.loggerNameA = _loggerNameA;
	_loggerConfiguration.loggerNameW = _loggerNameW;

    SetLoggerConfiguration( loggerConfiguration );
}

nsCommon::nsCmnLogger::CLogger::~CLogger()
{

}

void nsCommon::nsCmnLogger::CLogger::SetLoggerConfiguration( const LoggerConfiguration& loggerConfiguration )
{
    _loggerConfiguration = loggerConfiguration;

    for( size_t idx = 0; idx < _vecChilds.size(); ++idx )
    {
        if( wcsicmp( _vecChilds[ idx ]->_loggerNameW, loggerConfiguration.loggerNameW.c_str() ) == 0 )
            continue;
        _vecChilds[ idx ]->SetLoggerConfiguration( loggerConfiguration );
    }
    
    if( _loggerConfiguration.filePath.empty() == true )
        _loggerConfiguration.filePath = L".";

    if( *_loggerConfiguration.filePath.begin() == L'.' )
    {
        std::wstring currentPath = nsCmnPath::GetCurrentPath();
        _loggerConfiguration.filePath = _loggerConfiguration.filePath.replace( 0, currentPath.size(), currentPath );
        _loggerConfiguration.filePath = nsCmnPath::CanonicalizePath( _loggerConfiguration.filePath );
    }

    if( _loggerConfiguration.prefixLogA.empty() == true && _loggerConfiguration.prefixLogW.empty() == false )
        _loggerConfiguration.prefixLogA = CU2A( _loggerConfiguration.prefixLogW );
    if( _loggerConfiguration.prefixLogW.empty() == true && _loggerConfiguration.prefixLogA.empty() == false )
        _loggerConfiguration.prefixLogW = CA2U( _loggerConfiguration.prefixLogA );

	_loggerConfiguration.loggerNameA = _loggerNameA;
	_loggerConfiguration.loggerNameW = _loggerNameW;
}

void nsCommon::nsCmnLogger::CLogger::appendNewLineToLOGDATA( LOG_CONTEXT* lc, size_t& dataSize )
{
    if( lc->isUnicodeData == true )
    {
        lc->logDataW[ dataSize++ ] = L'\r';
        lc->logDataW[ dataSize++ ] = L'\n';
    }
    else
    {
        lc->logDataA[ dataSize++ ] = '\r';
        lc->logDataA[ dataSize++ ] = '\n';
    }
}

void nsCommon::nsCmnLogger::CLogger::writeData( LOG_CONTEXT* lc )
{
    assert( lc != NULLPTR );
    assert( lc->loggerConfiguration != NULLPTR );

    if( lc->loggerConfiguration->isSynchronousOp == true )
        CLogUtil::WriteData( lc );
    else
        CLogFactory::PushLOG_CONTEXT( lc );
}

LOG_CONTEXT* nsCommon::nsCmnLogger::CLogger::retrieveLOGContext( bool isUnicodeData, LoggerConfiguration* lc )
{
    LOG_CONTEXT* plc = CLogUtil::CheckOutLOGContext();

    if( plc != NULLPTR )
    {
        plc->isUnicodeData = isUnicodeData;
        plc->loggerConfiguration = lc;

        if( plc->logDataA != NULLPTR && plc->logDataSizeA > 0 )
            ::memset( plc->logDataA, '\0', plc->logDataSizeA * sizeof( char ) );

        if( plc->logDataW != NULLPTR && plc->logDataSizeW > 0 ) 
            ::memset( plc->logDataW, '\0', plc->logDataSizeW * sizeof( wchar_t ) ); 
    }

    return plc;
}

bool nsCommon::nsCmnLogger::CLogger::AddCallback( pfnLogCallbackA pfnCallback, PVOID pParam )
{
	if( pfnCallback == NULLPTR )
		return false;

	TyPrCallbackAToMutex _mapCallbackA = CLogUtil::GetCallback( _loggerNameA );
	 
	for( std::map< pfnLogCallbackA, PVOID >::iterator iter = _mapCallbackA.first->begin(); iter != _mapCallbackA.first->end(); ++iter )
	{
		if( iter->first == pfnCallback )
			return true;
	}

	{
		CScopedCriticalSection( *_mapCallbackA.second );
		_mapCallbackA.first->insert( std::make_pair( pfnCallback, pParam ) );
	}
		
	for( size_t idx = 0; idx < _vecChilds.size(); ++idx )
		_vecChilds[ idx ]->AddCallback( pfnCallback, pParam );
	
	return true;
}

bool nsCommon::nsCmnLogger::CLogger::AddCallback( pfnLogCallbackW pfnCallback, PVOID pParam )
{
	if( pfnCallback == NULLPTR )
		return false;

	TyPrCallbackWToMutex _mapCallbackW = CLogUtil::GetCallback( _loggerNameW );

	for( std::map< pfnLogCallbackW, PVOID >::iterator iter = _mapCallbackW.first->begin(); iter != _mapCallbackW.first->end(); ++iter )
	{
		if( iter->first == pfnCallback )
			return true;
	}

	{
		CScopedCriticalSection( *_mapCallbackW.second );
		_mapCallbackW.first->insert( std::make_pair( pfnCallback, pParam ) );	
	}

	for( size_t idx = 0; idx < _vecChilds.size(); ++idx )
		_vecChilds[ idx ]->AddCallback( pfnCallback, pParam );

	return true;
}

void nsCommon::nsCmnLogger::CLogger::RemoveCallback( pfnLogCallbackA pfnCallback )
{
	TyPrCallbackAToMutex _mapCallbackA = CLogUtil::GetCallback( _loggerNameA );
	
	{
		CScopedCriticalSection( *_mapCallbackA.second );

		if( _mapCallbackA.first->count( pfnCallback ) < 1 )
			return;

		for( std::map< pfnLogCallbackA, PVOID >::iterator iter = _mapCallbackA.first->begin(); iter != _mapCallbackA.first->end(); ++iter )
		{
			if( iter->first == pfnCallback )
			{
				_mapCallbackA.first->erase( iter );
				break;
			}
		}
	}

	for( size_t idx = 0; idx < _vecChilds.size(); ++idx )
		_vecChilds[ idx ]->RemoveCallback( pfnCallback );
}

void nsCommon::nsCmnLogger::CLogger::RemoveCallback( pfnLogCallbackW pfnCallback )
{
	TyPrCallbackWToMutex _mapCallbackW = CLogUtil::GetCallback( _loggerNameW );

	{
		CScopedCriticalSection( *_mapCallbackW.second );

		if( _mapCallbackW.first->count( pfnCallback ) < 1 )
			return;

		for( std::map< pfnLogCallbackW, PVOID >::iterator iter = _mapCallbackW.first->begin(); iter != _mapCallbackW.first->end(); ++iter )
		{
			if( iter->first == pfnCallback )
			{
				_mapCallbackW.first->erase( iter );
				break;
			}
		}
	}
	
	for( size_t idx = 0; idx < _vecChilds.size(); ++idx )
		_vecChilds[ idx ]->RemoveCallback( pfnCallback );
}

bool nsCommon::nsCmnLogger::CLogger::IsLogging( CLOG_LEVEL wantLevel )
{
    if( _loggerConfiguration.logLevel >= wantLevel )
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////

void nsCommon::nsCmnLogger::CLogFactory::InitLogger( const LoggerConfiguration& loggerConfiguration )
{
    if( _isInit == true )
        return;

    ::InitializeCriticalSectionAndSpinCount( &_csFactory, 4000 );
    ::InitializeCriticalSectionAndSpinCount( &CLogUtil::_csFile, 4000 );
	::InitializeCriticalSectionAndSpinCount( &CLogUtil::_csCallback, 4000 );
   
    EnterCriticalSection( &_csFactory );

    CLogUtil::_rbAllocator.CreatePool();

    _defaultConfiguration = loggerConfiguration;
    
    if( _defaultConfiguration.filePath.empty() == true )
        _defaultConfiguration.filePath = L".";

    if( *_defaultConfiguration.filePath.begin() == L'.' )
    {
        std::wstring currentPath = nsCmnPath::GetCurrentPath();
        _defaultConfiguration.filePath = _defaultConfiguration.filePath.replace( 0, currentPath.size(), currentPath );
        _defaultConfiguration.filePath = nsCmnPath::CanonicalizePath( _defaultConfiguration.filePath );
    }

#ifdef USE_CMNLOGGER_GPBERROR
    google::protobuf::SetLogHandler( &GPBLogHandler );
#endif

#ifdef USE_CMNLOGGER_QTDEBUG
    qInstallMessageHandler( &QtLogHandler );
#endif

    _isExit = false;
    _isInit = true;

    LeaveCriticalSection( &_csFactory );

    UNREFERENCED_PARAMETER( GetLogger( "root" ) );

    if( _defaultConfiguration.isSynchronousOp == false )
        _workerThread = boost::thread( boost::bind( &nsCommon::nsCmnLogger::CLogFactory::workerThread ) );
    if( _defaultConfiguration.isUseBatchFileWrite == true )
        _workerFileThread = boost::thread( boost::bind( &nsCommon::nsCmnLogger::CLogFactory::workerFileThread, _defaultConfiguration.nBatchFileWriteIntervalsSec ) );
}

void nsCommon::nsCmnLogger::CLogFactory::StopLogger()
{
    if( _isInit == false )
        return ;

    ::EnterCriticalSection( &_csFactory );

    _isExit = true;
    _isInit = false;
    _workerThread.join();
    _workerFileThread.join();

    for( boost::unordered_map< std::string, CLogger* >::iterator it = _mapNameToLogger.begin();
        it != _mapNameToLogger.end(); ++it )
    {
        delete it->second;
    }

    _mapNameToLogger.clear();

    for( boost::unordered_map< TyPrFileToMutex, CSharedQueue< std::wstring >* >::iterator it = _fileWriteQueue.begin();
        it != _fileWriteQueue.end(); ++it )
        delete it->second;

    // 기록중이던 파일 모두 닫음
    for( boost::unordered_map< std::wstring, TyPrFileToMutex >::iterator it = CLogUtil::_mapFileNameToDest.begin();
        it != CLogUtil::_mapFileNameToDest.end(); ++it )
    {
        if( it->second.first != NULLPTR )
        {
            ::fclose( it->second.first );
            ::DeleteCriticalSection( it->second.second );
        }
    }

	for( boost::unordered_map< std::string, TyPrCallbackAToMutex >::iterator it = CLogUtil::_mapFileNameToCallbackA.begin();
		it != CLogUtil::_mapFileNameToCallbackA.end(); ++it )
	{
		if( it->second.first != NULLPTR )
		{
			delete it->second.first;
			::DeleteCriticalSection( it->second.second );
		}
	}

	for( boost::unordered_map< std::wstring, TyPrCallbackWToMutex >::iterator it = CLogUtil::_mapFileNameToCallbackW.begin();
		it != CLogUtil::_mapFileNameToCallbackW.end(); ++it )
	{
		if( it->second.first != NULLPTR )
		{
			delete it->second.first;
			::DeleteCriticalSection( it->second.second );
		}
	}

    _fileWriteQueue.clear();
	CLogUtil::_mapFileNameToCallbackA.clear();
	CLogUtil::_mapFileNameToCallbackW.clear();
    CLogUtil::_mapFileNameToDest.clear();
    CLogUtil::_rbAllocator.DeletePool();

    ::LeaveCriticalSection( &_csFactory );

    ::DeleteCriticalSection( &_csFactory );
    ::DeleteCriticalSection( &CLogUtil::_csFile );
	::DeleteCriticalSection( &CLogUtil::_csCallback );
}

void nsCommon::nsCmnLogger::CLogFactory::workerThread()
{
    // 로그를 기록하는 스레드
    LOG_CONTEXT* lc;

    do 
    {
        while( _queue.Try_and_pop( lc ) == false && _isExit == false )
            SleepEx( 1, FALSE );

        if( _isExit == true )
            break;

        CLogUtil::WriteData( lc );

    } while ( nsCommon::nsCmnLogger::CLogFactory::_isExit == false );
}

void nsCommon::nsCmnLogger::CLogFactory::workerFileThread( unsigned int nBatchWriteLOGIntervalsSec )
{
    const unsigned int nMaxCountPerOnce = 256;

    do 
    {
        std::wstring writeData;

        if( _isExit == true )
            break;

        SleepEx( nBatchWriteLOGIntervalsSec * 1000, TRUE );
    
        {
            CScopedCriticalSection lck( _csFactory );
            for( boost::unordered_map< TyPrFileToMutex, CSharedQueue< std::wstring >* >::iterator it = _fileWriteQueue.begin(); it != _fileWriteQueue.end(); ++it )
            {
                unsigned int nCurrentCount = 0;
                while( it->second->Try_and_pop( writeData ) == true )
                {
                    CScopedCriticalSection lck( *it->first.second );
                    fputws( writeData.c_str(), it->first.first );
                    if( ++nCurrentCount >= nMaxCountPerOnce )
                        break;

                }
                if( nCurrentCount > 0 )
                    fflush( it->first.first );
            }
        }

    } while ( nsCommon::nsCmnLogger::CLogFactory::_isExit == false );
}


CLogger& nsCommon::nsCmnLogger::CLogFactory::GetLogger( const std::string& loggerName )
{
    if( _isInit == false )
        return *_mapNameToLogger[ "" ];

    CScopedCriticalSection lck( _csFactory );

    if( _mapNameToLogger.find( loggerName ) != _mapNameToLogger.end() )
        return *_mapNameToLogger[ loggerName ];

    _mapNameToLogger[ loggerName ] = new CLogger( loggerName.c_str(), CA2U( loggerName ), _defaultConfiguration );
    if( loggerName.find( '/' ) != std::string::npos )
    {
        std::string parentName;
        std::string childName;
        std::string::size_type startPos = 0;
        std::string::size_type pos = loggerName.find( '/' );

        while( pos != std::string::npos )
        {
             childName = loggerName.substr( 0, pos );
            
            if( _mapNameToLogger.find( childName ) == _mapNameToLogger.end() )
            {
                _mapNameToLogger[ childName ] = new CLogger( childName.c_str(), CA2U( childName ), _defaultConfiguration );
                if( parentName.empty() == true )
                    _mapNameToLogger[ DEFAULT_ROOT_LOGGER ]->AppendChildLogger( _mapNameToLogger[ childName ] );
                else
                    _mapNameToLogger[ parentName ]->AppendChildLogger( _mapNameToLogger[ childName ] );
            }
            
            parentName = childName;
            pos = loggerName.find( '/', pos + 1 );
        }
    }
    else if( loggerName != DEFAULT_ROOT_LOGGER )
    {
        _mapNameToLogger[ DEFAULT_ROOT_LOGGER ]->AppendChildLogger( _mapNameToLogger[ loggerName ] );
    }
    return *_mapNameToLogger[ loggerName ];
}
