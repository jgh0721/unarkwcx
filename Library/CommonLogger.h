#pragma once

#include <cassert>
#include <string>
#include <iostream>
#include <map>

#include <io.h>
#include <time.h>
#include <stdarg.h>

#include "ConvertStr.h"

//////////////////////////////////////////////////////////////////////////
///	디버그 로그를 남기기위한 공통 라이브러리
///	작성자 : 헬마
///	최초 작성일 : 2009-07-21
///
///	참조 소스코드 : ACE Library 의 Log_Msg.cpp, Log_Msg.h
///
///	지원 OS : 윈도 NT 계열
///
///	지원하는 서식 문자열
///	매개변수 필요없는 서식 문자열
///		%% - %문자
///		%T - 호출한 스레드ID ( 10진수 출력 )
///		%P - 호출한 프로세스ID ( 10진수 출력 )
///		%N - 호출한 소스코드가 위치한 소스파일 이름
///		%L - 호출한 소스코드가 위치한 줄번호
///     %F - 호출한 함수 이름
///		%D - 호출한 날짜	( 서식은 기본지정 되거나 별도로 변경가능 )
///		%t - 호출한 시간	( 서식은 기본지정 되거나 별도로 변경가능 )
///		%l - 호출한 로그 레벨 ( 문자열로 표현 )
///	매개변수 필요한 서식 문자열
///		%o - 8진수
///     %x(X) - 16진수 
///		%d - 32비트 부호 있는 정수
///		%u - 32비트 부호 없는 정수
///		%i(I)64d - 64비트 부호 있는 정수
///		%i(I)64u - 64비트 부호 없는 정수
///		%f - 부동소수점
///		%s - 문자열
///		%b(B) - bool 값을 받아서 TRUE, FALSE 문자열로 출력
///		%E - GetLastError() 가 반환한 오류코드를 받아서 문자열로 변환하여 출력
///		%H - HRESULT 형식의 오류코드를 받아서 문자열로 변환하여 출력
///
///		.2009-07-21		:	재설계시작
///		.2009-07-21		:	일부 서식화 문자열 변경
///		.2009-07-21		:	상수이름 변경
///		.2009-07-21		:	ANSI 문자열, UNICODE 문자열 모두를 받을 수 있도록 두벌의 함수를 작성
///		.2009-07-21		:	로그수준을 설정할 수 있도록 추가
///		.2009-07-21		:	로그수준을 실시간으로 변경할 수 있도록 함. 
///		.2009-07-21		:	동적 버퍼 관리 기능 추가
///		.2009-07-21		:	스트림으로 출력 떄는 파일 스트림이라도 용량제한등을 할 수 없음.
///		.2009-07-21		:	COM 오류 코드 지원추가, %H
///		.2009-07-21		:	부동소수점 숫자를 서식화할 때 _CVTBUFSIZE 를 사용하여 버퍼크기를 요청하도록 변경 
///		.2009-07-21		:	로그 클래스에 넘겨진 스트림클래스는 무조건 로그 클래스가 해제하도록 변경
///		.2009-07-21		:	로그 클래스에서 지원할 콜백함수 원형 추가
///
///		.2009-07-22		:	파일이름을 적으면 파일이름 뒤에 로그를 작성한 날짜와 분할파일 번호가 자동으로 붙도록 변경
///		.2009-07-22		:	로그레벨을 표시할 수 있는 서식화 문자열 추가
///		.2009-07-22		:	파일이 분할되어 있으면 가장 마지막 분할 파일에 이어서 붙이도록 함
///		.2009-07-22		:	로그 내용의 앞에 언제나 붙게될 내용을 설정할 수 있도록 변경
///			- 기본값 = [LEVEL=%I][DateTime=%D %T][PID=%P][TID=%t][File=%N-%L][Func=%F]
///			- 접두사 부분에는 매개변수가 필요없는 서식화 문자열만 가능.
///			- 기본값 중에서 골라서 사용하는 것을 권장
///		.2009-07-22		:	로그내용에 로그 레벨문자열이 정확히 출력되지 않는 문제 수정
///		.2009-07-22		:	ANSI 문자열에 대한 지원 보류
///		.2009-07-22		:	콘솔창 출력 기능 구현
///		.2009-07-24		:	if 문 안에 {} 없이 로그 매크로함수를 사용했을 때 경고가 나타나는 현상 수정
///			- 로그 매크로함수 끝에 반드시 ; 를 붙여서 문을 끝내야함
///		.2009-07-25		:	x64 환경에서 컴파일시 발생하는 경고 수정
///		.2009-08-17		:	로그 생성 후 하루가 지나면 자동으로 파일을 날짜에 맞춰서 나누도록 변경
///
///		.2011-02-XX		:	날짜, 시간 서식을 지정할 수 있도록함
///						:	속도 최적화를 위해 log_arg_list 가 string 를 반환하지 않음
///						:	몇몇 서식 문자열 변경
///						:	64비트 정수 지원 추가
///						:	로그 서식 문자열의 접두어 변경 () 
///						:	파일로 로그 저장할 때 날짜가 변경되면 로그파일이 분리되는 옵션 추가
///						:	주석 추가
///						:	checkFileExist() 함수에서 _wfindfirst 함수로 생성한 파일검색 핸들을 닫지 않던 버그 수정
///						:	로그 파일의 용량 제한이 무제한이더라도 날짜가 지나면 로그 파일이 분할될 수 있도록 수정
///		.2011-07-07		:	콜백함수를 호출할 때 콜백함수가 기록할 FILE* 또는 ofstream 을 전달하도록 수정
///						:	콜백함수를 호출할 때 prefix 부분과 data 부분의 문자열을 분리하여 전달하도록 수정
///						:	로그를 기록할 때 prefix 부분과 data 부분으로 나눠서 기록하도록 수정
//////////////////////////////////////////////////////////////////////////
/*

	간단한 로거 사용법

	ㄱ). 로그를 출력할 대상을 결정한다. ( 로그 출력 대상은 비트 연산자 "OR" 를 이용하여 복수개를 지정할 수 있음 )
		LM_INSTANCE()->SetLogTransfer()
	ㄴ). 로그를 출력할 대상에 파일 또는 스트림을 지정했다면 해당 대상을 지정한다
		LM_INSTANCE()->SetLogFile()
	ㄷ). 로거 사용이 완전히 끝나면
		LM_EXIT() 를 호출한다
*/

// 로그 레벨을 나타내는 문자열
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
	LT_CALLBACK = 64
};

typedef void (*pfnLogCallbackW)
	( 
		PVOID pParam,
		FILE* fpLogFile,
		CLOG_LEVEL level,
		__time64_t tmTime,
		DWORD_PTR pid,
		DWORD_PTR tid,
		const wchar_t* wszFileName,
		const wchar_t* wszFuncName, 
		unsigned int lineNumber,
		const wchar_t* wszPrefix,
		const wchar_t* wszData
	);

typedef void (*pfnLogCallbackA)
	( 
	PVOID pParam,
	FILE* fpLogFile,
	CLOG_LEVEL level,
	__time64_t tmTime,
	DWORD_PTR pid,
	DWORD_PTR tid,
	const char* wszFileName,
	const char* wszFuncName, 
	unsigned int lineNumber,
	const char* wszPrefix,
	const char* wszData
	);


class CLog
{
public:
	~CLog();
	static CLog*	GetInstance( void );
	static void		CloseInstance( void );

	size_t CLog::Log( const wchar_t* fmt, ... );
	size_t CLog::Log( const char* fmt, ... );

	bool			IsLogging( CLOG_LEVEL logLevel );
	void			Lock();
	void			Unlock();

	// 콜백 추가, 삭제
	bool			AddCallbackA( pfnLogCallbackA pfnCallback, PVOID pParam );
	bool			AddCallbackW( pfnLogCallbackW pfnCallback, PVOID pParam );
	void			RemoveCallbackA( pfnLogCallbackA pfnCallback );
	void			RemoveCallbackW( pfnLogCallbackW pfnCallback );

	// 로그 출력대상 관련
	bool			SetLogFile(  const wchar_t* szLogPath, const wchar_t* szLogName, const wchar_t* szLogExt );
	void			SetFileLimitSize( unsigned int nFileLimitSize = 0, bool IsFileSplit = true, bool IsFileSplitWhenDate = true );
	unsigned int	GetFileLimitSize( void );

	// 로깅정보 설정
	void			SetLogLevel( CLOG_LEVEL logLevel );
	void			SetLogTransfer( DWORD logTransfer);
	void			SetFileName( const char* szFileNameA, const wchar_t* szFileNameW );
	void			SetFuncName( const char* szFuncNameA, const wchar_t* szFuncNameW );
	void			SetLineNumber( unsigned int nLineNumber );
	// 로그 문자열을 생성할 때 사용할 날짜, 시간 서식 문자열, NULL 을 지정하면 기본 서식 지정
	void			SetDateFormat( const wchar_t* pwszDateFormat = NULL );
	void			SetTimeFormat( const wchar_t* pwszTimeFormat = NULL );
	// 로그 문자열을 생성할 때 항상 앞에 생성될 문자열 설정, NULL 을 지정하면 기본값 설정, 설정하지 않으면 L"" 를 설정
	void			SetLogPrefix( const wchar_t* pwszLogPrefix = NULL );

	// 현재 저장되고 있는 로그 파일이름( 완전한 경로 포함 )
	std::wstring	GetFileName( void );

private:
	CLog( void );
	CLog& operator =( const CLog& );
	CLog( const CLog& );

	bool			getBufferW( UINT_PTR nRequiredSize, ptrdiff_t nBetween, wchar_t** next_msg );
	bool			getBufferA( UINT_PTR nRequiredSize, ptrdiff_t nBetween, char** next_msg );
	// 가변인자를 받아 실제로 로그 서식문자열을 생성, 반환값은 생성된 문자 수
	size_t			log_arg_list( const wchar_t* fmt, va_list args );
	size_t			log_arg_list( const char* fmt, va_list args );
	const wchar_t*	getLogLevelW();
	const char*		getLogLevelA();
	// 디렉토리 존재여부를 확인한 후 재귀적으로 디렉토리 생성
	DWORD			checkDirectory( const wchar_t* szDirectory );
	// 분할된 파일의 존재여부를 검사한 후 가장 마지막에 생성되어야할 분할 파일번호를 찾음
	void			checkFileExist( void );
	// 파일 크기 점검, false = 함수 실패, 파일 기록 불가, true = 함수 성공, 파일 기록 가능
	bool			checkFileSizeAndTime( const wchar_t* szFileName );
	bool			createLogFile();

	// 내부 변수
	static	CLog*			pClsLog;

	// 최대 로그 파일 크기, MB 단위, 0 = 무제한
	unsigned int			m_nFileLimitSize;
	// 최대 로그 파일 크기에 도달하면 파일을 분할할 지 여부, false 이고 꽉 차면 더 이상 기록안함
	bool					m_IsFileSplit;
	// 분할된 로그 파일 수
	unsigned int			m_nSplitCount;
	// 날짜가 바뀌면 로그 파일을 분할할지 여부 
	bool					m_IsFileSplitWhenDate;
	// 로그파일을 생성한 시간
	struct tm				m_tmCreateLogTime;
	// 저장될 로그파일 관련 변수들
	wchar_t					m_szLogPath[ MAX_PATH ];
	wchar_t					m_szLogName[ MAX_PATH ];
	wchar_t					m_szLogExt[ MAX_PATH ];
	wchar_t					m_szCurrentLogFile[ MAX_PATH ];	// 현재 로그가 기록중인 파일
	// 파일 구조체
	FILE*									m_pLogFile;
	std::map<pfnLogCallbackA, PVOID>		m_lstCallbackA;
	std::map<pfnLogCallbackW, PVOID>		m_lstCallbackW;

	// 버퍼관련 변수들
	// 초기 버퍼 크기 및 버퍼 확장단위
	static const unsigned int		MSG_BUFFER_SIZE = 2048;
	static const unsigned int		MAX_TIME_BUFFER_SIZE = 128;
	unsigned int			m_nMsgBufferSizeA;
	unsigned int			m_nMsgBufferSizeW;
	unsigned int			m_nRemainBufferSizeA;
	unsigned int			m_nRemainBufferSizeW;
	char					m_szDateFormatA[64];
	wchar_t					m_szDateFormatW[64];
	char					m_szTimeFormatA[64];
	wchar_t					m_szTimeFormatW[64];

	// 생성된 로그 문자열 버퍼
	char*					m_szNextMsgPtrA;
	wchar_t*				m_szNextMsgPtrW;
	char*					m_szMsgBufferA;
	wchar_t*				m_szMsgBufferW;

	CLOG_LEVEL				m_eLogLevel;
	DWORD					m_eLogTransfer;

	CRITICAL_SECTION		m_cs;

	// 로깅정보 데이터 변수
	char					m_szFileNameA[ MAX_PATH ];
	wchar_t					m_szFileNameW[ MAX_PATH ];
	char					m_szFuncNameA[ MAX_PATH ];
	wchar_t					m_szFuncNameW[ MAX_PATH ];
	unsigned int			m_nLineNumber;
	CLOG_LEVEL				m_eCurrentLogLevel;
	__time64_t				m_tmCurrentTime;
	DWORD					m_dwProcessID;
	DWORD					m_dwThreadID;

	std::string				m_strPreFixA;
	std::wstring			m_strPreFixW;

	std::string				m_strPrefixLogA;
	std::wstring			m_strPrefixLogW;
};

// 편의 매크로 모음
#define LM_INSTANCE() CLog::GetInstance()

#define LM_TRACE(arg) \
	do { \
		if( LM_INSTANCE()->IsLogging( LL_TRACE ) ) \
		{	\
			LM_INSTANCE()->Lock();	\
			LM_INSTANCE()->SetFileName( __FILE__, __FILEW__ ); \
			LM_INSTANCE()->SetFuncName( __FUNCTION__, __FUNCTIONW__ ); \
			LM_INSTANCE()->SetLineNumber( __LINE__ ); \
			LM_INSTANCE()->Log arg; \
			LM_INSTANCE()->Unlock();	\
		}	\
	} while(0)

#define LM_DEBUG(arg) \
	do { \
	if( LM_INSTANCE()->IsLogging( LL_DEBUG ) ) \
		{	\
		LM_INSTANCE()->Lock();	\
		LM_INSTANCE()->SetFileName( __FILE__, __FILEW__ ); \
		LM_INSTANCE()->SetFuncName( __FUNCTION__, __FUNCTIONW__ ); \
		LM_INSTANCE()->SetLineNumber( __LINE__ ); \
		LM_INSTANCE()->Log arg; \
		LM_INSTANCE()->Unlock();	\
}	\
	} while(0)

#define LM_WARNING(arg) \
	do { \
	if( LM_INSTANCE()->IsLogging( LL_WARNING ) ) \
		{	\
		LM_INSTANCE()->Lock();	\
		LM_INSTANCE()->SetFileName( __FILE__, __FILEW__ ); \
		LM_INSTANCE()->SetFuncName( __FUNCTION__, __FUNCTIONW__ ); \
		LM_INSTANCE()->SetLineNumber( __LINE__ ); \
		LM_INSTANCE()->Log arg; \
		LM_INSTANCE()->Unlock();	\
}	\
	} while(0)

#define LM_ERROR(arg) \
	do { \
	if( LM_INSTANCE()->IsLogging( LL_ERROR ) ) \
		{	\
		LM_INSTANCE()->Lock();	\
		LM_INSTANCE()->SetFileName( __FILE__, __FILEW__ ); \
		LM_INSTANCE()->SetFuncName( __FUNCTION__, __FUNCTIONW__ ); \
		LM_INSTANCE()->SetLineNumber( __LINE__ ); \
		LM_INSTANCE()->Log arg; \
		LM_INSTANCE()->Unlock();	\
}	\
	} while(0)

#define LM_CRITICAL(arg) \
	do { \
	if( LM_INSTANCE()->IsLogging( LL_CRITICAL ) ) \
		{	\
		LM_INSTANCE()->Lock();	\
		LM_INSTANCE()->SetFileName( __FILE__, __FILEW__ ); \
		LM_INSTANCE()->SetFuncName( __FUNCTION__, __FUNCTIONW__ ); \
		LM_INSTANCE()->SetLineNumber( __LINE__ ); \
		LM_INSTANCE()->Log arg; \
		LM_INSTANCE()->Unlock();	\
}	\
	} while(0)

#define LM_EXIT() \
	do { \
	CLog::CloseInstance(); \
	} while( 0 )
