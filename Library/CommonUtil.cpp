#include "stdafx.h"

#include "CommonUtil.h"

/* 유용한 std::string 에 대한 formatting 함수 */
__inline std::string format_arg_list(const char *fmt, va_list args)
{
	if (!fmt) return "";
	int   result = -1, length = 512;
	char *buffer = 0;
	while (result == -1)    {
		if (buffer)
			delete [] buffer;
		buffer = new char [length + 1];
		memset(buffer, 0, (length + 1) * sizeof(char) );

		// remove deprecate warning
		//result = _vsnprintf(buffer, length, fmt, args);

		result = _vsnprintf_s(buffer, length, _TRUNCATE, fmt, args);
		length *= 2;
	}
	std::string s(buffer);
	delete [] buffer;
	return s;
}

__inline std::wstring format_arg_list(const wchar_t *fmt, va_list args)
{
	if (!fmt) return L"";
	int   result = -1, length = 512;
	wchar_t *buffer = 0;
	while (result == -1)    {
		if (buffer)
			delete [] buffer;
		buffer = new wchar_t [length + 1];
		memset(buffer, 0, (length + 1) * sizeof(wchar_t) );

		// remove deprecate warning
		//result = _vsnprintf(buffer, length, fmt, args);

		result = _vsnwprintf_s(buffer, length, _TRUNCATE, fmt, args);
		length *= 2;
	}
	std::wstring s(buffer);
	delete [] buffer;
	return s;
}

std::string format(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	std::string s = format_arg_list(fmt, args);
	va_end(args);
	return s;
}

std::wstring format(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	std::wstring s = format_arg_list(fmt, args);
	va_end(args);
	return s;
}

errno_t CopyAnsiString( char ** pszDest, const char * pszSrc, int srcLength /* = -1 */ )
{
	assert( pszSrc != NULL );
	assert( *pszDest == NULL );

	int nSize = srcLength == -1 ? strlen( pszSrc ) + 1 : srcLength + 1;
	*pszDest = new char[ nSize ];
	memset( *pszDest, '\0', nSize * sizeof( char ) );
	return strcpy_s( *pszDest, nSize, pszSrc );
}

errno_t CopyWideString( wchar_t ** pszDest, const wchar_t * pszSrc, int srcLength /* = -1 */ )
{
	assert( pszSrc != NULL );
	assert( *pszDest == NULL );

	int nSize = srcLength == -1 ? wcslen( pszSrc ) + 1 : srcLength + 1;
	*pszDest = new wchar_t[ nSize ];
	memset( *pszDest, '\0', nSize * sizeof( wchar_t ) );
	return wcscpy_s( *pszDest, nSize, pszSrc );
}

std::string string_replace_all( const std::string& src, const std::string& pattern, const std::string& replace )
{
	std::string result = src;    
	std::string::size_type pos = 0;    
	std::string::size_type offset = 0;    
	std::string::size_type pattern_len = pattern.size();    
	std::string::size_type replace_len = replace.size();    

	while ( ( pos = result.find( pattern, offset ) ) != std::string::npos )    
	{    
		result.replace( result.begin() + pos,     
			result.begin() + pos + pattern_len,     
			replace );    
		offset = pos + replace_len;    
	}    
	return result;    
}

std::wstring string_replace_all( const std::wstring& src, const std::wstring& pattern, const std::wstring& replace )
{
	std::wstring result = src;    
	std::wstring::size_type pos = 0;    
	std::wstring::size_type offset = 0;    
	std::wstring::size_type pattern_len = pattern.size();    
	std::wstring::size_type replace_len = replace.size();    

	while ( ( pos = result.find( pattern, offset ) ) != std::wstring::npos )    
	{    
		result.replace( result.begin() + pos,     
			result.begin() + pos + pattern_len,     
			replace );    
		offset = pos + replace_len;    
	}    
	return result;    

}
