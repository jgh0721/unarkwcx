#pragma once

#include <locale>
#include <string>
#include <cassert>
#include <stdarg.h>
#include <stdio.h>

#include <WinSock2.h>
#pragma comment( lib, "ws2_32.lib" )


//////////////////////////////////////////////////////////////////////////
/// 프로젝트에서 공통적으로 사용되는 유틸리티 클래스, 함수 모음
/// 작성자 : 헬마
//////////////////////////////////////////////////////////////////////////

#if defined(TEST_EXE_INCLUDE)
#define IMON_DLL_API
#else
#define IMON_DLL_API	AFX_EXT_CLASS
#endif

IMON_DLL_API std::string format(const char *fmt, ...);
IMON_DLL_API std::string format_arg_list(const char *fmt, va_list args);
IMON_DLL_API std::wstring format(const wchar_t *fmt, ...);
IMON_DLL_API std::wstring format_arg_list(const wchar_t *fmt, va_list args);

IMON_DLL_API extern const std::locale locEnUs;

IMON_DLL_API bool IsAlphabet( const std::string& strString );
IMON_DLL_API bool IsAlphabet( const std::wstring& strString );

IMON_DLL_API bool IsNumber( const std::string& strString );
IMON_DLL_API bool IsNumber( const std::wstring& strString );

/*
전송받은 문자열이 날짜형식인지 점검
점검 문자열 형식 "%Y-%m-%d", "%Y%m%d"
*/
IMON_DLL_API bool IsDate( const std::wstring& strString );

// 초단위의 숫자를 넘겨서 경과시간을 %H %M %S  를 사용한 문자열로 서식화, 
IMON_DLL_API std::wstring formatElapsedTime( unsigned int elapsedTimeSec, const std::wstring& fmt = L"%H 시간 %M 분 %S 초" );

template <typename T> inline 
	void DeletePtr( T& ptr ) 
{ 
	if( ptr != NULL )
	{
		delete ptr; 
		ptr = NULL; 
	}
}

template <typename T> inline
	void DeletePtrA( T& ptr )
{
	if( ptr != NULL )
	{
		delete [] ptr;
		ptr = NULL;
	}
}

template <typename T> inline
	void ReleasePtr( T& ptr )
{
	if( ptr != nullptr )
	{
		ptr->Release();
		delete ptr;
		ptr = nullptr;
	}
}

template< typename T > inline
	void DeleteMapContainerPointerValue( T container )
{
	for( typename T::iterator it = container.begin(); it != container.end(); ++it )
		delete (*it).second;
}

IMON_DLL_API errno_t CopyAnsiString( char ** pszDest, const char * pszSrc, int srcLength = -1 );
IMON_DLL_API errno_t CopyWideString( wchar_t ** pszDest, const wchar_t * pszSrc, int srcLength = -1 );

IMON_DLL_API std::string string_replace_all( const std::string& src, const std::string& pattern, const std::string& replace );
IMON_DLL_API std::wstring string_replace_all( const std::wstring& src, const std::wstring& pattern, const std::wstring& replace );

IMON_DLL_API std::wstring GetFormattedDateTimeText( __time64_t tmTime, std::wstring fmt = L"%Y-%m-%d %H:%M:%S" );

IMON_DLL_API std::wstring GetFormattedDateText( __time64_t tmTime, std::wstring fmt = L"%Y년 %m월 %d일" );
IMON_DLL_API std::wstring GetFormattedTimeText( __time64_t tmTime, std::wstring fmt = L"%H시 %M분" );

IMON_DLL_API __time64_t	GetTimeValueFromFormat( const std::wstring& dateTimeText, const std::wstring& fmt = L"%Y-%m-%d %H:%M:%S" );

// 문자열에서 공백으로 구분된 후 특정 delmiter 를 제거한 단어를 반환한다. 
// /MessageText:"fsdfsdfds" /MessageCode:232 
// delimiter = /MessageText: 라면 "fsdfsdfds" 반환
// delimiter = /MessageCode: 라면 "232" 반환
IMON_DLL_API std::wstring GetWordsUsingDelimiter( const std::wstring& strMessage, const std::wstring& delimiter );


IMON_DLL_API int						_wcsicmp( const std::wstring& lhs, const std::wstring& rhs );
#ifdef _AFX
IMON_DLL_API int						_wcsicmp( const CString& lhs, const CString& rhs );
#endif
IMON_DLL_API int						_wtoi( const std::wstring& lhs );
IMON_DLL_API __int64					_wtoi64( const std::wstring& lhs );

IMON_DLL_API int						u8sicmp( const std::string& lhs, const std::string& rhs );
IMON_DLL_API int						u8toi( const std::string& lhs );
IMON_DLL_API __int64					u8toi64( const std::string& lhs );

// IP 주소로부터 해당하는 U_INT 숫자를 반환, 네트워크 바이트 순서로 반환됨
IMON_DLL_API unsigned long	getULONGFromIPAddress( BYTE lsb, BYTE lsbBy1, BYTE lsbBy2, BYTE lsbBy3 );
// 네트워크 바이트 순서로 된 ULONG 을 받아 문자열된 IP 문자열 반환
IMON_DLL_API std::string		getIPAddressFromULONG( unsigned long ulIPaddress );
IMON_DLL_API std::string		getCPUBrandString();

// namespace boost
// {
// 	template<>
// 	inline int lexical_cast( const std::wstring& arg )
// 	{
// 		return _wtoi( arg.c_str() );
// 	}
// 
// 	template<>
// 	inline double lexical_cast( const std::wstring& arg )
// 	{
// 		return _wtof( arg.c_str() );
// 	}
// }

IMON_DLL_API bool					is64BitOS();

// 파일의 버전 정보 및 회사 정보를 구할수 있다.
// selectInfo 1 ~ 4 : Default : 2( ProductVersion )
// 1 : ProductVersion, 2 : FileVersion, 3 : Comments, 4 : CompanyName
IMON_DLL_API std::wstring GetFileInfomation( const std::wstring& filePath, const int selectInfo = 1 );