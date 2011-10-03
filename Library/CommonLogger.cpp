#include "stdafx.h"

#include "CommonLogger.h"

#pragma warning( disable: 4996 )

CLog* CLog::pClsLog = NULL;

CLog::CLog()
	: m_nLineNumber( 0 )
	, m_eLogLevel( LL_NONE )
	, m_szMsgBufferA( NULL )
	, m_szMsgBufferW( NULL )
	, m_eLogTransfer( LT_NONE )
	, m_nMsgBufferSizeA( 0 )
	, m_nMsgBufferSizeW( 0 )
	, m_nRemainBufferSizeA( 0 )
	, m_nRemainBufferSizeW( 0 )
	, m_szNextMsgPtrA( NULL )
	, m_szNextMsgPtrW( NULL )
	, m_nFileLimitSize( 0 )
	, m_IsFileSplit( true )
	, m_nSplitCount( 0 )
	, m_IsFileSplitWhenDate( true )
	, m_pLogFile( NULL )
{
	m_strPreFixA = "";
	m_strPreFixW = L"";

	memset( m_szCurrentLogFile, '\0', _countof( m_szCurrentLogFile ) * sizeof( wchar_t ) );
	memset( m_szDateFormatA, '\0', _countof( m_szDateFormatA ) * sizeof( char ) );
	memset( m_szDateFormatW, '\0', _countof( m_szDateFormatW ) * sizeof( wchar_t ) );
	memset( m_szTimeFormatA, '\0', _countof( m_szTimeFormatA ) * sizeof( char ) );
	memset( m_szTimeFormatW, '\0', _countof( m_szTimeFormatW ) * sizeof( wchar_t ) );

	SetDateFormat( NULL );
	SetTimeFormat( NULL );

	InitializeCriticalSectionAndSpinCount( &m_cs, 4000 );
}

CLog::~CLog()
{
	DeletePtrA< char* >( m_szMsgBufferA );
	DeletePtrA< wchar_t* >( m_szMsgBufferW );
}

CLog* CLog::GetInstance( void )
{
	if( CLog::pClsLog == NULL )
		CLog::pClsLog = new CLog();
	return CLog::pClsLog;
}

void CLog::CloseInstance( void )
{
	if( CLog::pClsLog )
		delete CLog::pClsLog;
	CLog::pClsLog = NULL;
}

void CLog::Lock()
{
	EnterCriticalSection( &m_cs );
}

void CLog::Unlock()
{
	LeaveCriticalSection( &m_cs );
}

void CLog::SetLogLevel( CLOG_LEVEL logLevel )
{
	m_eLogLevel = logLevel;
}

void CLog::SetLogTransfer( DWORD logTransfer )
{
	m_eLogTransfer = logTransfer;
}

void CLog::SetFileName( const char* szFileNameA, const wchar_t* szFileNameW )
{
	memset( m_szFileNameA, '\0', _countof( m_szFileNameA ) * sizeof( char ) );
	memset( m_szFileNameW, '\0', _countof( m_szFileNameW ) * sizeof( wchar_t ) );

	if( szFileNameA == NULL )
		strcpy( m_szFileNameA, "<Unknown File>" );
	else
		strcpy( m_szFileNameA, szFileNameA );

	if( szFileNameW == NULL )
		wcscpy( m_szFileNameW, L"<Unknown File>" );
	else
		wcscpy( m_szFileNameW, szFileNameW );
}

void CLog::SetFuncName( const char* szFuncNameA, const wchar_t* szFuncNameW )
{
	memset( m_szFuncNameA, '\0', _countof( m_szFuncNameA ) * sizeof( char ) );
	memset( m_szFuncNameW, '\0', _countof( m_szFuncNameW ) * sizeof( wchar_t ) );

	if( szFuncNameA == NULL )
		strcpy( m_szFuncNameA, "<Unknown Func>" );
	else
		strcpy( m_szFuncNameA, szFuncNameA );

	if( szFuncNameW == NULL )
		wcscpy( m_szFuncNameW, L"<Unknown Func>" );
	else
		wcscpy( m_szFuncNameW, szFuncNameW );
}

void CLog::SetLineNumber( unsigned int nLineNumber )
{
	m_nLineNumber = nLineNumber;
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
		m_strPreFixA = CU2A( pwszLogPrefix );
	}
}

bool CLog::IsLogging( CLOG_LEVEL logLevel )
{
	m_eCurrentLogLevel = logLevel;

	return m_eLogLevel >= m_eCurrentLogLevel;
}

bool CLog::getBufferA( UINT_PTR nRequiredSize, ptrdiff_t nBetween, char** next_msg )
{
	if( m_nRemainBufferSizeA > nRequiredSize )
		return true;

	m_nMsgBufferSizeA += MSG_BUFFER_SIZE;
	m_nRemainBufferSizeA += MSG_BUFFER_SIZE;

	char* pBuffer = new char[ m_nMsgBufferSizeA ];
	if( pBuffer == NULL )
	{
		m_nMsgBufferSizeA -= MSG_BUFFER_SIZE;
		m_nRemainBufferSizeA -= MSG_BUFFER_SIZE;
		return false;
	}

	ZeroMemory( pBuffer, m_nMsgBufferSizeA * sizeof(char) );
	strcpy( pBuffer, m_szMsgBufferA );
	delete [] m_szMsgBufferA;
	m_szMsgBufferA = pBuffer;

	*next_msg = m_szMsgBufferA + nBetween;
	return true;
}

bool CLog::getBufferW( UINT_PTR nRequiredSize, ptrdiff_t nBetween, wchar_t** next_msg )
{
	if( m_nRemainBufferSizeW > nRequiredSize )
		return true;

	m_nMsgBufferSizeW += MSG_BUFFER_SIZE;
	m_nRemainBufferSizeW += MSG_BUFFER_SIZE;

	wchar_t* pBuffer = new wchar_t[ m_nMsgBufferSizeW ];
	if( pBuffer == NULL )
	{
		m_nMsgBufferSizeW -= MSG_BUFFER_SIZE;
		m_nRemainBufferSizeW -= MSG_BUFFER_SIZE;
		return false;
	}

	ZeroMemory( pBuffer, m_nMsgBufferSizeW * sizeof(wchar_t) );
	wcscpy( pBuffer, m_szMsgBufferW );
	delete [] m_szMsgBufferW;
	m_szMsgBufferW = pBuffer;

	*next_msg = m_szMsgBufferW + nBetween;
	return true;
}

size_t CLog::Log( const char* fmt, ...  )
{
	m_tmCurrentTime = ::_time64( NULL );
	m_dwProcessID = ::GetCurrentProcessId();
	m_dwThreadID = ::GetCurrentThreadId();

	if( m_szMsgBufferA == NULL )
	{
		m_nMsgBufferSizeA = MSG_BUFFER_SIZE;
		m_szMsgBufferA = new char[ m_nMsgBufferSizeA ];
	}

	memset( m_szMsgBufferA, '\0', m_nMsgBufferSizeA * sizeof(char) );
	m_nRemainBufferSizeA = m_nMsgBufferSizeA;
	m_szNextMsgPtrA = m_szMsgBufferA;

	int nRetSize = log_arg_list( m_strPreFixA.c_str(), NULL );

	m_strPrefixLogA = m_szMsgBufferA;

	memset( m_szMsgBufferA, '\0', m_nMsgBufferSizeA * sizeof(char) );
	m_nRemainBufferSizeA = m_nMsgBufferSizeA;
	m_szNextMsgPtrA = m_szMsgBufferA;

	va_list			args;

	va_start( args, fmt );
	nRetSize += log_arg_list( fmt, args );
	va_end( args );

	if( m_eLogTransfer & LT_STDERR )
	{
		std::cerr << m_strPrefixLogA << m_szMsgBufferA << std::endl;
		std::cerr.flush();
	}

	if( m_eLogTransfer & LT_DEBUGGER )
	{
		OutputDebugStringA( m_strPrefixLogA.c_str() );
		OutputDebugStringA( m_szMsgBufferA );
	}

	if( m_eLogTransfer & LT_FILE )
	{
		if( m_pLogFile == NULL )
			createLogFile();

		if( checkFileSizeAndTime( GetFileName().c_str() ) == true )
		{
			fputws( CA2U(m_strPrefixLogA), m_pLogFile );
			fputws( CA2U(m_szMsgBufferA), m_pLogFile );
			fflush( m_pLogFile );
		}
	}

	if( m_eLogTransfer & LT_CALLBACK )
	{
		for( std::map<pfnLogCallbackA, PVOID>::iterator iter = m_lstCallbackA.begin(); iter != m_lstCallbackA.end(); ++iter )
		{
			(*iter->first)
			(	
				iter->second,
				m_pLogFile,
				m_eLogLevel, 
				m_tmCurrentTime, 
				m_dwProcessID, 
				m_dwThreadID, 
				m_szFileNameA, 
				m_szFuncNameA, 
				m_nLineNumber, 
				m_strPrefixLogA.c_str(),
				m_szMsgBufferA		
			);
		}
	}

	return nRetSize;
}

size_t CLog::Log( const wchar_t* fmt, ... )
{
	m_tmCurrentTime = ::_time64( NULL );
	m_dwProcessID = ::GetCurrentProcessId();
	m_dwThreadID = ::GetCurrentThreadId();

	if( m_szMsgBufferW == NULL )
	{
		m_nMsgBufferSizeW = MSG_BUFFER_SIZE;
		m_szMsgBufferW = new wchar_t[ m_nMsgBufferSizeW ];
	}

	memset( m_szMsgBufferW, '\0', m_nMsgBufferSizeW * sizeof(wchar_t) );
	m_nRemainBufferSizeW = m_nMsgBufferSizeW;
	m_szNextMsgPtrW = m_szMsgBufferW;

	int nRetSize = log_arg_list( m_strPreFixW.c_str(), NULL );

	m_strPrefixLogW = m_szMsgBufferW;

	memset( m_szMsgBufferW, '\0', m_nMsgBufferSizeW * sizeof(wchar_t) );
	m_nRemainBufferSizeW = m_nMsgBufferSizeW;
	m_szNextMsgPtrW = m_szMsgBufferW;

	va_list			args;

	va_start( args, fmt );
	nRetSize += log_arg_list( fmt, args );
	va_end( args );

	if( m_eLogTransfer & LT_STDERR )
	{
		std::wcerr << m_strPrefixLogW << m_szMsgBufferW << std::endl;
		std::wcerr.flush();
	}

	if( m_eLogTransfer & LT_DEBUGGER )
	{
		OutputDebugStringW( m_strPrefixLogW.c_str() );
		OutputDebugStringW( m_szMsgBufferW );
	}

	if( m_eLogTransfer & LT_FILE  )
	{
		if( m_pLogFile == NULL )
			createLogFile();
		
		if( checkFileSizeAndTime( GetFileName().c_str() ) == true )
		{
			fputws( m_strPrefixLogW.c_str(), m_pLogFile );
			fputws( m_szMsgBufferW, m_pLogFile );
			fflush( m_pLogFile );
		}
	}

	if( m_eLogTransfer & LT_CALLBACK )
	{
		if( m_pLogFile == NULL )
			createLogFile();

		if( checkFileSizeAndTime( GetFileName().c_str() ) == true )
		{
			for( std::map<pfnLogCallbackW, PVOID>::iterator iter = m_lstCallbackW.begin(); iter != m_lstCallbackW.end(); ++iter )
			{
				(*iter->first)
					(	
					iter->second,
					m_pLogFile,
					m_eLogLevel, 
					m_tmCurrentTime, 
					m_dwProcessID, 
					m_dwThreadID, 
					m_szFileNameW, 
					m_szFuncNameW, 
					m_nLineNumber, 
					m_strPrefixLogW.c_str(),
					m_szMsgBufferW		
					);
			}
		}
	}

	return nRetSize;
}

size_t CLog::log_arg_list( const wchar_t* fmt, va_list args )
{
	assert( fmt != NULL );

	int nRetSize = 0;

	while( *fmt != L'\0' )
	{
		if( *fmt != L'%' )
		{
			// % 가 없으면 %가 나타날때까지 복사
			*m_szNextMsgPtrW++ = *fmt++;
			--m_nRemainBufferSizeW;
			nRetSize++;
		}
		else if( fmt[1] == L'%' )
		{
			// %% 로 %가 연속으노 나타나면 %를 그대로 저장
			*m_szNextMsgPtrW++ = *fmt++;
			++fmt;
			--m_nRemainBufferSizeW;
			nRetSize++;
		}
		else
		{
			wchar_t szFormat[ 128 ] = {0,}; // snprintf 에 넣을 서식화 문자열 버퍼
			const wchar_t* start_format = fmt;
			wchar_t* fp = NULL;
			bool IsDone = false;
			int nResult = 0; // snprintf 가 기록한 양 또는 기록하는데 필요한 양

			fp = szFormat;
			*fp++ = *fmt++;	// % 문자를 넣음
			while( IsDone == false )
			{
				IsDone = true;
				nResult = 0;

				switch( *fmt )
				{
				case L'b':				// bool 값을 문자열로
				case L'B':
					wcscpy( fp, L"s" );
					if( getBufferW( 6 /* FALSE 길이 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, va_arg( args, bool ) == true ? L"TRUE" : L"FALSE" );
					break;

				case L'D':				// 날짜 출력
					{
						wcscpy( fp, L"s" );
						struct tm tmTemp;
						errno_t err = ::_localtime64_s( &tmTemp, &m_tmCurrentTime );
						wchar_t szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

						if( err != 0 || !wcsftime( szBuffer, MAX_TIME_BUFFER_SIZE, m_szDateFormatW, &tmTemp ) )
						{
							if( getBufferW( 15 /* L"<Unknown Date>" 의 길이 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
								break;

							nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, L"<Unknown Date>" );
						}
						else
						{
							if( getBufferW( wcslen( szBuffer ), m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
								break;

							nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, szBuffer );
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

							if( getBufferW( wcslen( pwszBuffer ), m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
								break;
							nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, pwszBuffer );
							LocalFree( hLocal );
						}
						else
						{
							if( getBufferW( wcslen(L"<FormatMessage Fail>"), m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
								break;

							nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, L"<FormatMessage Fail>" );
						}
					}
					break;

				case L'f':				// 부동소수점, C 언어는 인자를 받을 때 float 와 double 을 모두 double 로 상승시켜서 받음
					fp[0] = *fmt;
					fp[1] = L'\0';

					if( getBufferW( _CVTBUFSIZE, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, va_arg( args, double ) );
					break;

				case L'F':				// 소스코드 함수이름
					wcscpy( fp, L"s" );
					if( getBufferW( wcslen(m_szFuncNameW), m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, m_szFuncNameW );
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

						if( getBufferW( 21 /* 2 ^ 64 최대 정수형의 자리수 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
							break;
						
						nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, va_arg( args, __int64 ) );
					}
					break;

				case L'l':				// 로그 레벨 문자열로
					wcscpy( fp, L"s" );
					if( getBufferW( 14 /* 로그 레벨 최대 길이 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, getLogLevelW() );
					break;
 
				case L'L':				// 소스코드 줄 번호
					wcscpy( fp, L"u" );
					if( getBufferW( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, m_nLineNumber );
					break;

				case L'N':				// 소스코드 파일이름
					wcscpy( fp, L"s" );
					if( getBufferW( wcslen(m_szFileNameW), m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, m_szFileNameW );
					break;

				case L'P':				// 프로세스 ID
					wcscpy( fp, L"u" );
					if( getBufferW( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, m_dwProcessID );
					break;

				case L't':				// 시간 출력
					{
						wcscpy( fp, L"s" );
						struct tm tmTemp;
						errno_t err = ::_localtime64_s( &tmTemp, &m_tmCurrentTime );
						wchar_t szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

						if( err != 0 || !wcsftime( szBuffer, MAX_TIME_BUFFER_SIZE, m_szTimeFormatW, &tmTemp ) )
						{
							if( getBufferW( 15 /* L"<Unknown Date>" 의 길이 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
								break;

							nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, L"<Unknown Time>" );
						}
						else
						{
							if( getBufferW( wcslen( szBuffer ), m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
								break;

							nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, szBuffer );
						}
					}
					break;

				case L'T':				// 스레드 ID
					wcscpy( fp, L"u" );
					if( getBufferW( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, m_dwThreadID );
					break;

				case L's':				// 문자열
					{
						wchar_t* pszArgs = va_arg( args, wchar_t* );
						wcscpy( fp, L"s" );
						if( pszArgs == NULL )
						{
							if( getBufferW( 7 /* L"(null)" 의 길이 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
								break;

							nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, L"(null)" );
						}
						else
						{
							if( getBufferW( wcslen( pszArgs ) + 1, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
								break;

							nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, pszArgs );
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
					if( getBufferW( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == false )
						break;

					nResult = _snwprintf( m_szNextMsgPtrW, m_nRemainBufferSizeW, szFormat, va_arg( args, int ) );
					break;

				default:
					while( start_format != fmt && m_nRemainBufferSizeW > 0 )
					{
						*m_szNextMsgPtrW++ = *start_format++;
						--m_nRemainBufferSizeW;
					}

					if( m_nRemainBufferSizeW > 0 )
					{
						*m_szNextMsgPtrW++ = *fmt;
						--m_nRemainBufferSizeW;
					}

					break;
				} // switch

				if( nResult > 0 )	// snprintf 에 기록된 양이 있으면 그만큼 버퍼 포인터 이동 및 남은 버퍼크기 감소 
				{
					m_szNextMsgPtrW += nResult;
					m_nRemainBufferSizeW -= nResult;
					nRetSize += nResult;
				}

				++fmt;
			} // while

		} // if

	} // while 

	if( getBufferW( 2 /* L"\r\n" 의 길이 */, m_szNextMsgPtrW - m_szMsgBufferW, &m_szNextMsgPtrW ) == true )
	{
		wcscpy( m_szNextMsgPtrW, L"\r\n" );
		m_szNextMsgPtrW += 2;
		nRetSize += 2;
		m_nRemainBufferSizeW -= 2;
	}

	return nRetSize;
}

size_t CLog::log_arg_list( const char* fmt, va_list args )
{
	assert( fmt != NULL );

	int nRetSize = 0;

	while( *fmt != '\0' )
	{
		if( *fmt != '%' )
		{
			// % 가 없으면 %가 나타날때까지 복사
			*m_szNextMsgPtrA++ = *fmt++;
			--m_nRemainBufferSizeA;
			nRetSize++;
		}
		else if( fmt[1] == L'%' )
		{
			// %% 로 %가 연속으노 나타나면 %를 그대로 저장
			*m_szNextMsgPtrA++ = *fmt++;
			++fmt;
			--m_nRemainBufferSizeA;
			nRetSize++;
		}
		else
		{
			char szFormat[ 128 ] = {0,}; // snprintf 에 넣을 서식화 문자열 버퍼
			const char* start_format = fmt;
			char* fp = NULL;
			bool IsDone = false;
			int nResult = 0; // snprintf 가 기록한 양 또는 기록하는데 필요한 양

			fp = szFormat;
			*fp++ = *fmt++;	// % 문자를 넣음
			while( IsDone == false )
			{
				IsDone = true;
				nResult = 0;

				switch( *fmt )
				{
				case 'b':				// bool 값을 문자열로
				case 'B':
					strcpy( fp, "s" );
					if( getBufferA( 6 /* FALSE 길이 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, va_arg( args, bool ) == true ? "TRUE" : "FALSE" );
					break;

				case 'D':				// 날짜 출력
					{
						strcpy( fp, "s" );
						struct tm tmTemp;
						errno_t err = ::_localtime64_s( &tmTemp, &m_tmCurrentTime );
						char szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

						if( err != 0 || !strftime( szBuffer, MAX_TIME_BUFFER_SIZE, m_szDateFormatA, &tmTemp ) )
						{
							if( getBufferA( 15 /* L"<Unknown Date>" 의 길이 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
								break;

							nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, "<Unknown Date>" );
						}
						else
						{
							if( getBufferA( strlen( szBuffer ), m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
								break;

							nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, szBuffer );
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

							if( getBufferA( strlen( pszBuffer ), m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
								break;
							nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, pszBuffer );
							LocalFree( hLocal );
						}
						else
						{
							if( getBufferA( strlen("<FormatMessage Fail>"), m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
								break;

							nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, "<FormatMessage Fail>" );
						}
					}
					break;

				case 'f':				// 부동소수점, C 언어는 인자를 받을 때 float 와 double 을 모두 double 로 상승시켜서 받음
					fp[0] = *fmt;
					fp[1] = '\0';

					if( getBufferA( _CVTBUFSIZE, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, va_arg( args, double ) );
					break;

				case 'F':				// 소스코드 함수이름
					strcpy( fp, "s" );
					if( getBufferA( strlen(m_szFuncNameA), m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, m_szFuncNameA );
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

						if( getBufferA( 21 /* 2 ^ 64 최대 정수형의 자리수 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
							break;

						nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, va_arg( args, __int64 ) );
					}
					break;

				case 'l':				// 로그 레벨 문자열로
					strcpy( fp, "s" );
					if( getBufferA( 14 /* 로그 레벨 최대 길이 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, getLogLevelA() );
					break;

				case 'L':				// 소스코드 줄 번호
					strcpy( fp, "u" );
					if( getBufferA( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, m_nLineNumber );
					break;

				case 'N':				// 소스코드 파일이름
					strcpy( fp, "s" );
					if( getBufferA( strlen(m_szFileNameA), m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, m_szFileNameA );
					break;

				case 'P':				// 프로세스 ID
					strcpy( fp, "u" );
					if( getBufferA( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, m_dwProcessID );
					break;

				case 't':				// 시간 출력
					{
						strcpy( fp, "s" );
						struct tm tmTemp;
						errno_t err = ::_localtime64_s( &tmTemp, &m_tmCurrentTime );
						char szBuffer[ MAX_TIME_BUFFER_SIZE ] = {0,};

						if( err != 0 || !strftime( szBuffer, MAX_TIME_BUFFER_SIZE, m_szTimeFormatA, &tmTemp ) )
						{
							if( getBufferA( 15 /* L"<Unknown Date>" 의 길이 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
								break;

							nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, "<Unknown Time>" );
						}
						else
						{
							if( getBufferA( strlen( szBuffer ), m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
								break;

							nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, szBuffer );
						}
					}
					break;

				case 'T':				// 스레드 ID
					strcpy( fp, "u" );
					if( getBufferA( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, m_dwThreadID );
					break;

				case L's':				// 문자열
					{
						char* pszArgs = va_arg( args, char* );
						strcpy( fp, "s" );
						if( pszArgs == NULL )
						{
							if( getBufferA( 7 /* L"(null)" 의 길이 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
								break;

							nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, "(null)" );
						}
						else
						{
							if( getBufferA( strlen( pszArgs ) + 1, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
								break;

							nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, pszArgs );
						}
					}
					break;

				case 'o':				// 8진수
				case 'x':				// 16진수, 문자는 소문자
				case 'X':				// 16진수, 문자는 대문자
				case 'd':				// 부호있는 정수
				case 'u':				// 부호없는 정수
					fp[0] = *fmt;
					fp[1] = '\0';
					if( getBufferA( 11 /* 2 ^ 32, 최대 정수형의 자리수 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == false )
						break;

					nResult = _snprintf( m_szNextMsgPtrA, m_nRemainBufferSizeA, szFormat, va_arg( args, int ) );
					break;

				default:
					while( start_format != fmt && m_nRemainBufferSizeA > 0 )
					{
						*m_szNextMsgPtrA++ = *start_format++;
						--m_nRemainBufferSizeA;
					}

					if( m_nRemainBufferSizeA > 0 )
					{
						*m_szNextMsgPtrA++ = *fmt;
						--m_nRemainBufferSizeA;
					}

					break;
				} // switch

				if( nResult > 0 )	// snprintf 에 기록된 양이 있으면 그만큼 버퍼 포인터 이동 및 남은 버퍼크기 감소 
				{
					m_szNextMsgPtrA += nResult;
					m_nRemainBufferSizeA -= nResult;
					nRetSize += nResult;
				}

				++fmt;
			} // while

		} // if

	} // while 

	if( getBufferA( 2 /* L"\r\n" 의 길이 */, m_szNextMsgPtrA - m_szMsgBufferA, &m_szNextMsgPtrA ) == true )
	{
		strcpy( m_szNextMsgPtrA, "\r\n" );
		m_szNextMsgPtrA += 2;
		nRetSize += 2;
		m_nRemainBufferSizeA -= 2;
	}

	return nRetSize;
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

bool CLog::SetLogFile( const wchar_t* szLogPath, const wchar_t* szLogName, const wchar_t* szLogExt )
{
	assert( m_szLogName != NULL );
	assert( m_szLogPath != NULL );
	assert( m_szLogExt != NULL );

	memset( m_szLogPath, '\0', _countof( m_szLogPath ) * sizeof( wchar_t ) );
	memset( m_szLogName, '\0', _countof( m_szLogPath ) * sizeof( wchar_t ) );
	memset( m_szLogExt, '\0', _countof( m_szLogPath ) * sizeof( wchar_t ) );

	wcscpy( m_szLogPath, szLogPath );
	if( m_szLogPath[ wcslen(m_szLogPath) - 1 ] != L'\\'  )
		wcscat( m_szLogPath, L"\\" );
	
	wcscpy( m_szLogName, szLogName );
	
	if( szLogExt[0] == L'.' )
		wcscpy( m_szLogExt, szLogExt+1 );
	else
		wcscpy( m_szLogExt, szLogExt );

	// 디렉토리 생성
	if( checkDirectory( m_szLogPath ) != ERROR_SUCCESS )
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
