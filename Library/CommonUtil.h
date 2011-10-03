#pragma once

#include <string>
#include <cassert>
#include <stdarg.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
/// 프로젝트에서 공통적으로 사용되는 유틸리티 클래스, 함수 모음
/// 작성자 : 헬마
//////////////////////////////////////////////////////////////////////////

std::string format(const char *fmt, ...);
std::string format_arg_list(const char *fmt, va_list args);
std::wstring format(const wchar_t *fmt, ...);
std::wstring format_arg_list(const wchar_t *fmt, va_list args);

template <typename T> inline 
	void DeletePtr( T ptr ) 
{ 
	if( ptr != NULL )
	{
		delete ptr; 
		ptr = NULL; 
	}
}

template <typename T> inline
	void DeletePtrA( T ptr )
{
	if( ptr != NULL )
	{
		delete [] ptr;
		ptr = NULL;
	}
}

template <typename T> inline
	void ReleasePtr( T ptr )
{
	if( ptr != nullptr )
	{
		ptr->Release();
		delete ptr;
		ptr = nullptr;
	}
}

class lt_nocaseW
	: public std::binary_function< const std::wstring, const std::wstring, bool>
{
public:
	bool operator()( const std::wstring& x, const std::wstring& y ) const
	{
		return _wcsicmp( x.c_str(), y.c_str() ) < 0 ;
	}
};

class lt_nocaseA
	: public std::binary_function< char*, char*, bool>
{
public:
	bool operator()( std::string x, std::string y ) const
	{
		return _stricmp( x.c_str(), y.c_str() ) < 0;
	}
};

errno_t CopyAnsiString( char ** pszDest, const char * pszSrc, int srcLength = -1 );
errno_t CopyWideString( wchar_t ** pszDest, const wchar_t * pszSrc, int srcLength = -1 );

std::string string_replace_all( const std::string& src, const std::string& pattern, const std::string& replace );
std::wstring string_replace_all( const std::wstring& src, const std::wstring& pattern, const std::wstring& replace );

