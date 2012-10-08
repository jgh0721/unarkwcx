#pragma once

#include <locale>
#include <string>
#include <cassert>
#include <stdarg.h>
#include <stdio.h>

#include <WinSock2.h>
#pragma comment( lib, "ws2_32.lib" )


//////////////////////////////////////////////////////////////////////////
/// ������Ʈ���� ���������� ���Ǵ� ��ƿ��Ƽ Ŭ����, �Լ� ����
/// �ۼ��� : �︶
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
���۹��� ���ڿ��� ��¥�������� ����
���� ���ڿ� ���� "%Y-%m-%d", "%Y%m%d"
*/
IMON_DLL_API bool IsDate( const std::wstring& strString );

// �ʴ����� ���ڸ� �Ѱܼ� ����ð��� %H %M %S  �� ����� ���ڿ��� ����ȭ, 
IMON_DLL_API std::wstring formatElapsedTime( unsigned int elapsedTimeSec, const std::wstring& fmt = L"%H �ð� %M �� %S ��" );

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

IMON_DLL_API std::wstring GetFormattedDateText( __time64_t tmTime, std::wstring fmt = L"%Y�� %m�� %d��" );
IMON_DLL_API std::wstring GetFormattedTimeText( __time64_t tmTime, std::wstring fmt = L"%H�� %M��" );

IMON_DLL_API __time64_t	GetTimeValueFromFormat( const std::wstring& dateTimeText, const std::wstring& fmt = L"%Y-%m-%d %H:%M:%S" );

// ���ڿ����� �������� ���е� �� Ư�� delmiter �� ������ �ܾ ��ȯ�Ѵ�. 
// /MessageText:"fsdfsdfds" /MessageCode:232 
// delimiter = /MessageText: ��� "fsdfsdfds" ��ȯ
// delimiter = /MessageCode: ��� "232" ��ȯ
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

// IP �ּҷκ��� �ش��ϴ� U_INT ���ڸ� ��ȯ, ��Ʈ��ũ ����Ʈ ������ ��ȯ��
IMON_DLL_API unsigned long	getULONGFromIPAddress( BYTE lsb, BYTE lsbBy1, BYTE lsbBy2, BYTE lsbBy3 );
// ��Ʈ��ũ ����Ʈ ������ �� ULONG �� �޾� ���ڿ��� IP ���ڿ� ��ȯ
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

// ������ ���� ���� �� ȸ�� ������ ���Ҽ� �ִ�.
// selectInfo 1 ~ 4 : Default : 2( ProductVersion )
// 1 : ProductVersion, 2 : FileVersion, 3 : Comments, 4 : CompanyName
IMON_DLL_API std::wstring GetFileInfomation( const std::wstring& filePath, const int selectInfo = 1 );