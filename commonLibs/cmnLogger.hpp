#pragma once

#ifndef CMNLOGGER_HPP
#define CMNLOGGER_HPP

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <crtdefs.h>

#include "cmnUtils.h"
#include "cmnConcurrent.h"
#include "cmnFormatter.hpp"

#include <boost/preprocessor.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lockfree/queue.hpp>

#pragma execution_character_set( "utf-8" )

/*!
    로그 라이브러리

    로거 이름 
        로거 이름은 "/" 구분자를 이용하여 계층적으로 구성될 수 있으며 모든 로거는 "root" 이름을 가지는 로거의 하위 로거이다. 
        하위 로거는 상위 로거의 설정을 자동적으로 승계하여 따라간다. 
        각 로거별로 설정을 변경할 수 있다. 
        로거 이름의 길이는 최대 128 글자로 제한된다. 

    로거 환경설정
        로거의 환경은 InitLogger 와 함께 기본으로 생성되는 "root" 를 시작으로 모든 하위 로거에 재귀적으로 적용된다. 
            예). root/a/b/c 가 있을 때 a/b 의 로거를 수정하면 a/b/c 의 로거의 환경설정도 함께 수정된다. 

    로거 파일 생성 규칙
        경로/파일이름_날짜[색인번호].확장자

*/

 
/*!
        로그 문자열은 PreFix 문자열과 Contents 문자열의 두 부분으로 나뉘어짐. 
        PreFix 문자열 : 모든 로그 문자열에 앞에 자동으로 붙게 되는 문자열
        Contents 문자열 : 사용자가 출력하려는 문자열
        
    간략한 사용방법 :
        어플리케이션이 최초 시작하는 부분에 InitLogger() 를 실행하여 로그 라이브러리를 초기화함

        어플리케이션이 종료 되는 부분에 StopLogger() 를 실행하여 로그 라이브러리를 해제함

        생성되는 로그 파일 패턴 : 기본값, 로그 통합
            로그 통합 사용( isSplitByLogger = false ), 경로\_defaultLogFileName_로그생성날짜-[색인].확장자
            로그 분리 사용( isSplitByLogger = true ), 경로\_defaultLogFileName_로거이름_로그생성날짜-[색인].확장자

    잘못된 사용 예). 

        1. LM_TRACE((  어쩌구 저쩌구...  )) 
            => 예전처럼 괄호 2개로 묶어서 매크로를 사용하면 안됨. 무시무시한 컴파일 오류가 발생함. 
        2. LM_TRACE( L"어쩌구 저쩌구" )
            => 반드시 처음에는 서식화 문자열이, 두번째에는 인자들이 와야함. 
                ==> LM_TRACE( L"%1", L"어쩌구 저쩌구" ) 와 같이 수정필요
        3. LM_TRACE( L"%s", "fsdfsd" )
            => 변경된 로거는 %s 등의 기존 C/C++ 서식화 형식을 지원하지 않으므로 해당 매크로는 "%s" 라는 문자열을 로그로 출력하게 된다!!!
*/
 
/*!
    2013-05-27 
*/

namespace nsCommon
{
    namespace nsCmnLogger
    {
        enum CLOG_LEVEL
        {
            LL_NONE = 0,
            LL_CRITICAL,
            LL_ERROR,
            LL_WARNING,
            LL_DEBUG,
            LL_TRACE
        };

        enum CLOG_TRANSFER
        {
            LT_NONE = 0,
            LT_STDERR = 1,
            LT_STREAM = 2,
            LT_FILE = 4,
            LT_EVENTLOG = 8,
            LT_CONSOLE = 16,
            LT_DEBUGGER = 32,
            LT_CALLBACK = 64,
            LT_STDOUT = 128
        };

        const unsigned int MAX_LOGGERNAME_SIZE = 128;
        const std::string DEFAULT_ROOT_LOGGER = "root";

        typedef struct tagLoggerConfiguration
        {
			std::wstring		loggerNameW;
			std::string			loggerNameA;

            unsigned int        logTransferMethods;
            CLOG_LEVEL          logLevel;

            bool                isSynchronousOp;
            bool                isSplitWhenDate;
            unsigned int        fileSizeLimit;          // 로그 파일 크기 제한, 0 = 무제한, MB 단위

            std::wstring        fileName;
            std::wstring        filePath;
            std::wstring        fileExt;

            struct tm           currentFileDate;
            unsigned int        currentFileIndex;       // 로거 내부적으로 사용
            std::wstring        currentFullFileName;    // 로거 내부적으로 사용

            std::string         prefixLogA;             // 모든 출력 문자열 앞 부분에 항상 출력되는 서식
            std::wstring        prefixLogW;

            bool                isAppendNewline;

            unsigned int        maxLogFileDateCount;    // 최대 며칠치의 로그 파일을 보관할 것인지 설정, 기본값 7일

            bool                isSplitByLogger;        // 각 로거별로 별도의 로그파일을 생성할 지 결정함, 각 로거 이름별로 별도의 로그를 생성하도록 하면 로그 파일 이름에 로거 이름이 추가로 기록됨
            bool                isSplitByFileIndex;      // 프로세스 재시작 마다 새로운 로그 파일 생성 여부. 기존에 분할된 로그가 있으면 마지막 인덱스에 추가되어 기록됨, 기본값 true

            // 로그의 배치 기록
            bool                isUseBatchFileWrite;
            unsigned int        nBatchFileWriteIntervalsSec;

            tagLoggerConfiguration() : 
                logTransferMethods( LT_DEBUGGER ), logLevel( LL_DEBUG ),
                isSynchronousOp( true ), isSplitWhenDate( true ), fileSizeLimit(0),
                currentFileIndex(0),
                isAppendNewline(true),
                maxLogFileDateCount(7), isSplitByLogger(false),
                isUseBatchFileWrite( false ), nBatchFileWriteIntervalsSec( 10 ), isSplitByFileIndex( true )
            {};

        } LoggerConfiguration;

        typedef struct tagLogContext
        {
			CLOG_LEVEL		logLevel;
            char*           logDataA;
            wchar_t*        logDataW;
            size_t			logDataSizeA;
            size_t			logDataSizeW;
            bool            isUnicodeData;

            /* 
                LoggerConfiguration 에 대한 포인터는 CLogger 클래스에서 관리하고 이곳은 링크가 걸린다. 
                따라서 이곳에서 해당 포인터는 절대 해제하면 안됨
            */
            LoggerConfiguration*
                            loggerConfiguration;

            tagLogContext() : logLevel( LL_NONE ), logDataA( NULLPTR ), logDataW( NULLPTR ), isUnicodeData(false), logDataSizeA(0), logDataSizeW(0) {};
            ~tagLogContext() 
            { 
                DeletePtrA< char* >( logDataA ); DeletePtrA< wchar_t* >( logDataW ); 
            };

        } LOG_CONTEXT, *PLOG_CONTEXT;

		typedef void (*pfnLogCallbackW)	( PVOID pParam, CLOG_LEVEL logLevel, const wchar_t* wszData );
		typedef void (*pfnLogCallbackA)	( PVOID pParam,	CLOG_LEVEL logLevel, const char* szData );
        
		typedef std::map< pfnLogCallbackA, PVOID >	       TyMapCallbackA;
		typedef std::map< pfnLogCallbackW, PVOID >	       TyMapCallbackW;

		typedef std::pair< TyMapCallbackA*, CRITICAL_SECTION* >	TyPrCallbackAToMutex;
		typedef std::pair< TyMapCallbackW*, CRITICAL_SECTION* >	TyPrCallbackWToMutex;

		typedef std::pair< FILE*, CRITICAL_SECTION* >       TyPrFileToMutex;

        class CLogFactory;

        class CLogUtil
        {
        public:
            static LOG_CONTEXT*             CheckOutLOGContext();
            static void                     CheckInLOGContext( LOG_CONTEXT* object );

			static TyPrCallbackAToMutex       GetCallback( const std::string& fileName );
			static TyPrCallbackWToMutex       GetCallback( const std::wstring& fileName );

			static TyPrCallbackAToMutex       MakeCallback( const std::string& fileName );
			static TyPrCallbackWToMutex       MakeCallback( const std::wstring& fileName );

            static bool                     IsExistLOGFile();
            static TyPrFileToMutex          GetFile( LoggerConfiguration* lc );
            static TyPrFileToMutex          GetFile( const std::wstring& fileName );
            static TyPrFileToMutex          MakeLOGFile( LoggerConfiguration* lc );

            static void                     WriteData( LOG_CONTEXT* logContext );
            static const char*              GetLoglevelTextA( CLOG_LEVEL logLevel );
            static const wchar_t*           GetLoglevelTextW( CLOG_LEVEL logLevel );
        private:
            static void                     CheckFileExist( LoggerConfiguration* lc );
            static bool                     CheckFileSizeAndDate( const wchar_t* wszFullFileName, LoggerConfiguration* lc );
            static void                     CheckOldFileExist( LoggerConfiguration* lc );

            friend class                    CLogFactory;
            static CRITICAL_SECTION                                         _csFile;
            static boost::unordered_map< std::wstring, TyPrFileToMutex >    _mapFileNameToDest;
            static CRoundRobinAllocator<LOG_CONTEXT>                        _rbAllocator;

			static CRITICAL_SECTION													_csCallback;
			static boost::unordered_map< std::string, TyPrCallbackAToMutex >		_mapFileNameToCallbackA;
			static boost::unordered_map< std::wstring, TyPrCallbackWToMutex >		_mapFileNameToCallbackW;
        };

        class CLogger
        {
        public:
            ~CLogger();

			bool				AddCallback( pfnLogCallbackA pfnCallback, PVOID pParam );
			bool				AddCallback( pfnLogCallbackW pfnCallback, PVOID pParam );
			void				RemoveCallback( pfnLogCallbackA pfnCallback );
			void				RemoveCallback( pfnLogCallbackW pfnCallback );

            void                SetLoggerConfiguration( const LoggerConfiguration& loggerConfiguration );
            LoggerConfiguration GetLoggerConfiguration() { return _loggerConfiguration; };

            void                AppendChildLogger( CLogger* logger )
            { assert( logger != NULLPTR ); _vecChilds.push_back( logger ); };

            bool                IsLogging( CLOG_LEVEL wantLevel );

            /*!
                prefix 데이터에 대해 문자열을 생성한 후 입력한 문자열에 대해 contents 를 생성하여 둘을 한번에 출력함
            */
#if _MSC_VER >= 1800
            template< typename ... Args >
            size_t Log( CLOG_LEVEL currentLogLevel, const std::pair< const char*, const wchar_t* >& prFileName, const std::pair< const char*, const wchar_t* >& prFuncName, unsigned int lineNumber, 
                        const char* format, Args ... args )
            {
                if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false )
                    return -1;

                size_t lRet = 0;
                nsCommon::nsCmnFormatter::nsCmnFmtTypes::TyTpLoggerType tpLoggerType( prFileName.first, prFileName.second, prFuncName.first, prFuncName.second, _loggerNameA, _loggerNameW, CLogUtil::GetLoglevelTextA(currentLogLevel), CLogUtil::GetLoglevelTextW(currentLogLevel), lineNumber );
                LOG_CONTEXT* pLC = retrieveLOGContext( false, &_loggerConfiguration );
                pLC->logLevel = currentLogLevel; 
                if( _loggerConfiguration.prefixLogA.empty() == false )
                { 
                    nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< char* > out( pLC->logDataA, pLC->logDataSizeA, pLC->logDataSizeA );    
                    lRet = nsCommon::nsCmnFormatter::nsCmnFmtDetail::tsFormatLoggerBuffer( out, tpLoggerType, _loggerConfiguration.prefixLogA.c_str(), args... );
                } 

                nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< char* > out( pLC->logDataA, pLC->logDataSizeA, pLC->logDataSizeA - lRet );    
                lRet += nsCommon::nsCmnFormatter::nsCmnFmtDetail::tsFormatLoggerBuffer( out, tpLoggerType, format, args... );
                appendNewLineToLOGDATA( pLC, lRet ); 
                writeData( pLC ); 

                return lRet;
            }

            template< typename ... Args >
            size_t Log( CLOG_LEVEL currentLogLevel, const std::pair< const char*, const wchar_t* >& prFileName, const std::pair< const char*, const wchar_t* >& prFuncName, unsigned int lineNumber, 
                        const wchar_t* format, Args ... args )
            {
                if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false )
                    return -1;

                size_t lRet = 0;
                nsCommon::nsCmnFormatter::nsCmnFmtTypes::TyTpLoggerType tpLoggerType( prFileName.first, prFileName.second, prFuncName.first, prFuncName.second, _loggerNameA, _loggerNameW, CLogUtil::GetLoglevelTextA(currentLogLevel), CLogUtil::GetLoglevelTextW(currentLogLevel), lineNumber );
                LOG_CONTEXT* pLC = retrieveLOGContext( true, &_loggerConfiguration );
                pLC->logLevel = currentLogLevel; 
                if( _loggerConfiguration.prefixLogW.empty() == false )
                { 
                    nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< wchar_t* > out( pLC->logDataW, pLC->logDataSizeW, pLC->logDataSizeW );
                    lRet = nsCommon::nsCmnFormatter::nsCmnFmtDetail::tsFormatLoggerBuffer( out, tpLoggerType, _loggerConfiguration.prefixLogW.c_str(), args... );
                } 

                nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< wchar_t* > out( pLC->logDataW, pLC->logDataSizeW, pLC->logDataSizeW - lRet );
                lRet += nsCommon::nsCmnFormatter::nsCmnFmtDetail::tsFormatLoggerBuffer( out, tpLoggerType, format, args... );
                appendNewLineToLOGDATA( pLC, lRet ); 
                writeData( pLC ); 

                return lRet;
            }
#else
#define FORMAT_FUNCS(z, n, unused) \
            template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
            size_t Log( CLOG_LEVEL currentLogLevel, const std::pair< const char*, const wchar_t* >& prFileName, const std::pair< const char*, const wchar_t* >& prFuncName, unsigned int lineNumber, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ), \
                    typename boost::enable_if_c< boost::is_same< CharT, char >::value >::type * = 0 ) \
            {                      \
                if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) return -1; \
                size_t lRet = 0;     \
                nsCommon::nsCmnFormatter::nsCmnFmtTypes::TyTpLoggerType tpLoggerType( prFileName.first, prFileName.second, prFuncName.first, prFuncName.second, _loggerNameA, _loggerNameW, CLogUtil::GetLoglevelTextA(currentLogLevel), CLogUtil::GetLoglevelTextW(currentLogLevel), lineNumber ); \
                LOG_CONTEXT* pLC = retrieveLOGContext( false, &_loggerConfiguration ); \
				pLC->logLevel = currentLogLevel; \
                if( _loggerConfiguration.prefixLogA.empty() == false ) \
                { \
                    nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< CharT* > out( pLC->logDataA, pLC->logDataSizeA, pLC->logDataSizeA );    \
                    lRet = nsCommon::nsCmnFormatter::nsCmnFmtDetail::format_internal_logger( out, tpLoggerType, _loggerConfiguration.prefixLogA.c_str() BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
                } \
                nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< CharT* > out( pLC->logDataA, pLC->logDataSizeA, pLC->logDataSizeA - lRet );    \
                lRet += nsCommon::nsCmnFormatter::nsCmnFmtDetail::format_internal_logger( out, tpLoggerType, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
                appendNewLineToLOGDATA( pLC, lRet ); \
                writeData( pLC ); \
                return lRet;        \
            }

            BOOST_PP_REPEAT( BOOST_PP_ADD( 15, 1 ), FORMAT_FUNCS, 0 )

#undef FORMAT_FUNCS

#define FORMAT_FUNCS(z, n, unused) \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        size_t Log( CLOG_LEVEL currentLogLevel, const std::pair< const char*, const wchar_t* >& prFileName, const std::pair< const char*, const wchar_t* >& prFuncName, unsigned int lineNumber, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ), \
        typename boost::enable_if_c< boost::is_same< CharT, wchar_t >::value >::type * = 0 ) \
        {                      \
            if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) return -1; \
            size_t lRet = 0;     \
            nsCommon::nsCmnFormatter::nsCmnFmtTypes::TyTpLoggerType tpLoggerType( prFileName.first, prFileName.second, prFuncName.first, prFuncName.second, _loggerNameA, _loggerNameW, CLogUtil::GetLoglevelTextA(currentLogLevel), CLogUtil::GetLoglevelTextW(currentLogLevel), lineNumber ); \
            LOG_CONTEXT* pLC = retrieveLOGContext( true, &_loggerConfiguration ); \
			pLC->logLevel = currentLogLevel; \
            if( _loggerConfiguration.prefixLogW.empty() == false ) \
            { \
                nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< CharT* > out( pLC->logDataW, pLC->logDataSizeW, pLC->logDataSizeW );    \
                lRet = nsCommon::nsCmnFormatter::nsCmnFmtDetail::format_internal_logger( out, tpLoggerType, _loggerConfiguration.prefixLogW.c_str() BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
            } \
            nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< CharT* > out( pLC->logDataW, pLC->logDataSizeW, pLC->logDataSizeW - lRet );    \
            lRet += nsCommon::nsCmnFormatter::nsCmnFmtDetail::format_internal_logger( out, tpLoggerType, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
            appendNewLineToLOGDATA( pLC, lRet ); \
            writeData( pLC ); \
            return lRet;        \
        }

        BOOST_PP_REPEAT( BOOST_PP_ADD( 15, 1 ), FORMAT_FUNCS, 0 )
#undef FORMAT_FUNCS
                
#endif
        private:
            CLogger( const char* loggerNameA, const wchar_t* loggerNameW );
            CLogger( const char* loggerNameA, const wchar_t* loggerNameW, const LoggerConfiguration& loggerConfiguration );
            LOG_CONTEXT*        retrieveLOGContext( bool isUnicodeData, LoggerConfiguration* lc );
            void                appendNewLineToLOGDATA( LOG_CONTEXT* lc, size_t& dataSize );
            void                writeData( LOG_CONTEXT* lc );

            friend class        CLogFactory;

            char                _loggerNameA[ MAX_LOGGERNAME_SIZE ];
            wchar_t             _loggerNameW[ MAX_LOGGERNAME_SIZE ];

            std::vector< CLogger* > _vecChilds;
            LoggerConfiguration _loggerConfiguration;
        };

        class CLogFactory
        {
        public:
            // nBatchWriteLOGIntervalsSec = 0 이면 큐에서 데이터를 꺼내면 즉시 기록, 그외는 해당 초마다 기록
            static void InitLogger( const LoggerConfiguration& loggerConfiguration );
            static void StopLogger();

            static CLogger&             GetLogger( const std::string& loggerName = DEFAULT_ROOT_LOGGER );
            static void                 PushLOG_CONTEXT( LOG_CONTEXT* logContext )
            { _queue.Push( logContext ); };

            static volatile bool                                    _isInit;
            static LoggerConfiguration& GetDefaultConfiguration() { return _defaultConfiguration; };

        private:
            friend class                                            CLogUtil;
            static void                 workerThread();
            static void                 workerFileThread( unsigned int nBatchWriteLOGIntervalsSec );
            static volatile bool                                    _isExit;
            static CRITICAL_SECTION                                 _csFactory;
            static LoggerConfiguration                              _defaultConfiguration;
            static boost::unordered_map< std::string, CLogger* >    _mapNameToLogger;
            static boost::thread                                    _workerThread;
            static boost::thread                                    _workerFileThread;
            static CSharedQueue< LOG_CONTEXT* >                     _queue;
            static boost::unordered_map< TyPrFileToMutex, CSharedQueue< std::wstring >* >
                                                                    _fileWriteQueue;
        };

#define LM_INSTANCE nsCommon::nsCmnLogger::CLogFactory

#define LM_TRACE( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_TRACE ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_TRACE, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_TRACE_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_TRACE ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_TRACE, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_DEBUG( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_DEBUG ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_DEBUG, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_DEBUG_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_DEBUG ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_DEBUG, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_ERROR( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_ERROR ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_ERROR, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_ERROR_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_ERROR ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_ERROR, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_WARNING( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_WARNING ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_WARNING, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_WARNING_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_WARNING ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_WARNING, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_CRITICAL( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_CRITICAL ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_CRITICAL, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_CRITICAL_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_CRITICAL ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_CRITICAL, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)


    } // namespace nsCmnLogger

} // namespace nsCommon

#endif