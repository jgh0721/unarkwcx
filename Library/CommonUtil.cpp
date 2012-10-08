#include "stdafx.h"

#include <intrin.h>

#include "CommonUtil.h"
#include "ConvertStr.h"

#pragma comment( lib, "version.lib" )

const std::locale locEnUs( "English" );

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

bool IsAlphabet( const std::string& strString )
{
	for( std::string::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
		if( isalpha( *iterChar, locEnUs ) == true )
			return true;
	return false;
}

bool IsAlphabet( const std::wstring& strString )
{
	for( std::wstring::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
		if( isalpha( *iterChar, locEnUs ) == true )
			return true;
	return false;
}

bool IsNumber( const std::string& strString )
{
	for( std::string::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
		if( isdigit( *iterChar, locEnUs ) == false )
			return false;

	return true;
}

bool IsNumber( const std::wstring& strString )
{
	for( std::wstring::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
		if( isdigit( *iterChar, locEnUs ) == false )
			return false;
	return true;
}

bool IsDate( const std::wstring& strString )
{
	std::wstring::size_type pos = strString.find( L'-' );
	if( pos == std::wstring::npos )
	{
		if( strString.length() >= wcslen( L"YYYYmmdd" ) )
		{
			unsigned int nYear	= _wtoi( strString.substr( 0, 4 ).c_str() );
			unsigned int nMonth	= _wtoi( strString.substr( 4, 2 ).c_str() );
			unsigned int nDay	= _wtoi( strString.substr( 6, 2 ).c_str() );

			if( nYear <= 0 || nYear > 3000 )
				return false;
			if( nMonth <= 0 || nMonth > 12 )
				return false;
			if( nDay <= 0 || nDay > 31 )
				return false;

			return true;
		}
	}
	else
	{
		if( strString.length() >= wcslen( L"YYYY-mm-dd" ) )
		{
			unsigned int nYear	= _wtoi( strString.substr( 0, 4 ).c_str() );
			unsigned int nMonth	= _wtoi( strString.substr( 5, 2 ).c_str() );
			unsigned int nDay	= _wtoi( strString.substr( 8, 2 ).c_str() );

			if( nYear <= 0 || nYear > 3000 )
				return false;
			if( nMonth <= 0 || nMonth > 12 )
				return false;
			if( nDay <= 0 || nDay > 31 )
				return false;

			return true;
		}
	}

	return false;
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

std::wstring formatElapsedTime( unsigned int elapsedTimeSec, const std::wstring& fmt /*= L"%H 시간 %M 분 %S 초" */ )
{
	unsigned int hour = 0;
	unsigned int minute = 0;
	unsigned int sec = 0;

	sec = elapsedTimeSec % 60;
	minute = elapsedTimeSec / 60;
	if( minute >= 60 )
	{
		hour = minute / 60;
		minute = minute % 60;
	}

	std::wstring formatText;

	if( fmt.find( L"%H" ) == std::wstring::npos )
	{
		formatText = string_replace_all( formatText, L"%M", format(L"%d", minute + (hour * 60) ) );
	}
	else
	{
		formatText = string_replace_all( fmt, L"%H", format(L"%d",hour) );
		formatText = string_replace_all( formatText, L"%M", format(L"%d", minute) );
	}

	formatText = string_replace_all( formatText, L"%S", format(L"%d", sec) );
	
	return formatText;
}

std::wstring GetFormattedDateText( __time64_t tmTime, std::wstring fmt /*= L"%Y-%m-%d" */ )
{
	return GetFormattedDateTimeText( tmTime, fmt );
}

std::wstring GetFormattedTimeText( __time64_t tmTime, std::wstring fmt /*= L"%H:%M:%S" */ )
{
	return GetFormattedDateTimeText( tmTime, fmt );
}

std::wstring GetFormattedDateTimeText( __time64_t tmTime, std::wstring fmt )
{
	WCHAR wszBuffer[ 128 ] = {0,};

	struct tm localTime;
	errno_t errno_ = localtime_s( &localTime, &tmTime );
	if( errno_ != 0 )
		return L"";

	if( wcsftime( wszBuffer, _countof( wszBuffer ), fmt.c_str(), &localTime ) == 0 )
		return L"";

	return wszBuffer;
}

std::wstring GetWordsUsingDelimiter( const std::wstring& strMessage, const std::wstring& delimiter )
{
	bool isFoundQuote = false;

	size_t delimLength = delimiter.size();

	std::wstring::size_type startPos = strMessage.find( delimiter );
	std::wstring::size_type endPos = std::wstring::npos;

	if( startPos == std::wstring::npos )
		return L"";

	startPos += delimLength;

	if( strMessage[ startPos ] == L'\"' )
	{
		isFoundQuote = true;
		startPos++;
	}

	endPos = strMessage.find( isFoundQuote == false ? L" " : L"\"", startPos + 1 );
	if( endPos == std::wstring::npos )
		return strMessage.substr( startPos );

	return strMessage.substr( startPos, endPos - startPos );
}


int _wcsicmp( const std::wstring& lhs, const std::wstring& rhs )
{
	return _wcsicmp( lhs.c_str(), rhs.c_str() );
}
#ifdef _AFX
int _wcsicmp( const CString& lhs, const CString& rhs )
{
	return _wcsicmp( lhs.GetString(), rhs.GetString() );
}
#endif

int _wtoi( const std::wstring& lhs )
{
	return _wtoi( lhs.c_str() );
}

__int64 _wtoi64( const std::wstring& lhs )
{
	return _wtoi64( lhs.c_str() );
}

int u8sicmp( const std::string& lhs, const std::string& rhs )
{
	return _wcsicmp( CU82U( lhs ).c_str(), CU82U( rhs ).c_str() );
}

int u8toi( const std::string& lhs )
{
	return _wtoi( CU82U( lhs ).c_str() );
}

__int64 u8toi64( const std::string& lhs )
{
	return _wtoi64( CU82U( lhs ).c_str() );
}

__time64_t GetTimeValueFromFormat( const std::wstring& dateTimeText, const std::wstring& fmt /*= L"%Y-%m-%d %H:%M:%S" */ )
{
    if( dateTimeText.empty() == true || fmt.empty() == true )
        return 0;

	int nIdx = 0;
	struct tm tmDate;

	tmDate.tm_year = 0;		// 1900 년 이후 흘러간 연도 수
	tmDate.tm_mon = 0;		// 0 ~ 11 로 나타난 현재 월
	tmDate.tm_mday = 1;		// 1 ~ 31 로 나타난 현재 일

	tmDate.tm_hour = 0;		// 0 ~ 23
	tmDate.tm_min = 0;		// 0 ~ 59
	tmDate.tm_sec = 0;		// 0 ~ 59

	tmDate.tm_wday = 0;		// 0 ~ 6, 일주일 표시 0 = Sun
	tmDate.tm_yday = 0;		// 1월 1일을 0 으로 한 0 ~ 365
	tmDate.tm_isdst = 0;

	for( std::wstring::size_type nPos = 0; nPos < fmt.size(); ++nPos )
	{
		switch( fmt[ nPos ] )
		{
		case L'%':
			if( fmt[ nPos + 1 ] == L'Y' ) 
			{
				tmDate.tm_year		= _wtoi( dateTimeText.substr( nIdx, 4 ) ) - 1900;
				nIdx += 3;
				++nPos;
			}
			if( fmt[ nPos + 1 ] == L'm' )
			{
				tmDate.tm_mon		= _wtoi( dateTimeText.substr( nIdx, 2 ) ) - 1;
				nIdx += 1;
				++nPos;
			}
			if( fmt[ nPos + 1 ] == L'd' )
			{
				tmDate.tm_mday		= _wtoi( dateTimeText.substr( nIdx, 2 ) );
				nIdx += 1;
				++nPos;
			}

			if( fmt[ nPos + 1 ] == L'H' )
			{
				tmDate.tm_hour		= _wtoi( dateTimeText.substr( nIdx, 2 ) );
				nIdx += 1;
				++nPos;
			}

			if( fmt[ nPos + 1 ] == L'p' )
			{
				std::wstring strAM, strPM;
				if( strAM.empty() == true && strPM.empty() == true )
				{
					struct tm tmAM, tmPM;

					tmAM.tm_year = 2012 - 1900;		// 1900 년 이후 흘러간 연도 수
					tmAM.tm_mon = 1;		// 0 ~ 11 로 나타난 현재 월
					tmAM.tm_mday = 1;		// 1 ~ 31 로 나타난 현재 일

					tmAM.tm_hour = 1;		// 0 ~ 23
					tmAM.tm_min = 1;		// 0 ~ 59
					tmAM.tm_sec = 1;		// 0 ~ 59

					tmAM.tm_wday = 0;		// 0 ~ 6, 일주일 표시 0 = Sun
					tmAM.tm_yday = 0;		// 1월 1일을 0 으로 한 0 ~ 365
					tmAM.tm_isdst = 0;

					tmPM = tmAM;

					tmPM.tm_hour = 13;
					wchar_t szBuffer[ 64 ] = {0,};
					wcsftime( szBuffer, 64, L"%p", &tmAM );
					strAM = szBuffer;
					memset( szBuffer, '\0', sizeof(wchar_t) * 64 );

					wcsftime( szBuffer, 64, L"%p", &tmPM );
					strPM = szBuffer;
				}
                
                if( nIdx + strAM.size() < dateTimeText.size() && strAM.size() > 0 )
                {
                    if( _wcsicmp( dateTimeText.substr( nIdx, strAM.size() ), strAM ) == 0 )
                        tmDate.tm_hour = tmDate.tm_hour;
                }

                if( nIdx + strPM.size() < dateTimeText.size() && strPM.size() > 0 )
                {
                    if( _wcsicmp( dateTimeText.substr( nIdx, strPM.size() ), strPM ) == 0 )
                        tmDate.tm_hour += 12;
                }

				nIdx += 1;
				++nPos;
			}

			if( fmt[ nPos + 1 ] == L'I' )
			{
				tmDate.tm_hour		+= _wtoi( dateTimeText.substr( nIdx, 2 ) );
				nIdx += 1;
				++nPos;
			}

			if( fmt[ nPos + 1 ] == L'M' )
			{
				tmDate.tm_min		= _wtoi( dateTimeText.substr( nIdx, 2 ) );
				nIdx += 1;
				++nPos;
			}
			if( fmt[ nPos + 1 ] == L'S' )
			{
				tmDate.tm_sec		= _wtoi( dateTimeText.substr( nIdx, 2 ) );
				nIdx += 1;
				++nPos;
			}
		default:
			nIdx++;
		};

	}

	return _mktime64( &tmDate );
}

unsigned long getULONGFromIPAddress( BYTE lsb, BYTE lsbBy1, BYTE lsbBy2, BYTE lsbBy3 )
{
	unsigned long ipaddress = 0;

	ipaddress = lsb | ( lsbBy1 << 8 );
	ipaddress = ipaddress | ( lsbBy2 << 16 );
	ipaddress = ipaddress | ( lsbBy3 << 24 );

	return ipaddress;
}

std::string getIPAddressFromULONG( unsigned long ulIPaddress )
{
	if( ulIPaddress <= 0 )
		"";

	in_addr in;

	in.S_un.S_addr = ulIPaddress;
	return inet_ntoa( in );
}

bool is64BitOS()
{
	//Windows 2000 이하의 제품군은 64bit 가 존재 하지 않으므로 무조건 32bit 동작
	OSVERSIONINFO osi;
	ZeroMemory(&osi, sizeof(osi));
	osi.dwOSVersionInfoSize = sizeof(osi);

	GetVersionEx(&osi);

	if(osi.dwMajorVersion == 4)
		return false;

	if(osi.dwMajorVersion == 5 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		return false;

	BOOL bIsWoW64 = FALSE;

	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;

	HMODULE hModule = GetModuleHandle(TEXT("kernel32"));

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( hModule, "IsWow64Process") ;

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWoW64))
		{
			if( hModule )
				FreeLibrary( hModule );
			return false;
		}
	}

	if( hModule )
		FreeLibrary( hModule );

	return bIsWoW64 != FALSE ? true : false;
}

std::wstring GetFileInfomation( const std::wstring& filePath, const int selectInfo )
{
	if( filePath.empty() == true )
		return L"";

	std::wstring fileVersion;

	struct LANGCODEPAGE
	{
		WORD wLanguage;
		WORD wCodePage;
	};
	LANGCODEPAGE* lpLang = NULL;

	UINT uiTranslate = 0;
	UINT dwResultLenth = 0;  

	TCHAR* pszResult = NULL;
	TCHAR* pszVer  = NULL;
	TCHAR  szSub[MAX_PATH];

	INT iVerSize = ::GetFileVersionInfoSize( filePath.c_str(), NULL );

	if( iVerSize == 0 )
		return fileVersion;

	pszVer = (TCHAR*)malloc( sizeof(TCHAR) * iVerSize );
	memset( pszVer, 0, sizeof( TCHAR ) * iVerSize );

	if( ::GetFileVersionInfo( filePath.c_str(), 0, iVerSize, OUT pszVer ) == FALSE )
	{
		free( pszVer );
		return fileVersion;
	}

	if( ::VerQueryValue( pszVer, L"\\VarFileInfo\\Translation", OUT (LPVOID *)&lpLang, OUT &uiTranslate )  == FALSE )
	{
		free( pszVer );
		return fileVersion;
	}

	switch ( selectInfo )
	{
	case 1 : 
		wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\ProductVersion", lpLang[0].wLanguage, lpLang[0].wCodePage );
		break;
	case  2 :
		wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\FileVersion", lpLang[0].wLanguage, lpLang[0].wCodePage );
		break;
	case  3 :
		wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\Comments", lpLang[0].wLanguage, lpLang[0].wCodePage );
		break;
	case  4 :
		wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\CompanyName", lpLang[0].wLanguage, lpLang[0].wCodePage );
		break;
	default :
		wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\FileVersion", lpLang[0].wLanguage, lpLang[0].wCodePage );
		break;
	}

	if( ::VerQueryValue( pszVer, szSub, OUT (LPVOID*)&pszResult, OUT &dwResultLenth ) == FALSE )
	{
		free( pszVer );
		return fileVersion;
	}

	if (pszResult != NULL)
		fileVersion = pszResult;

	free( pszVer );

	return fileVersion;
}

IMON_DLL_API std::string getCPUBrandString()
{
	char CPUString[0x20];
	char CPUBrandString[0x40];
	int CPUInfo[4] = {-1};
	unsigned nExIds, i;

	// __cpuid with an InfoType argument of 0 returns the number of
	// valid Ids in CPUInfo[0] and the CPU identification string in
	// the other three array elements. The CPU identification string is
	// not in linear order. The code below arranges the information 
	// in a human readable form.
	__cpuid(CPUInfo, 0);
	memset(CPUString, 0, sizeof(CPUString));
	*((int*)CPUString) = CPUInfo[1];
	*((int*)(CPUString+4)) = CPUInfo[3];
	*((int*)(CPUString+8)) = CPUInfo[2];

	// Calling __cpuid with 0x80000000 as the InfoType argument
	// gets the number of valid extended IDs.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	// Get the information associated with each extended ID.
	for (i=0x80000000; i<=nExIds; ++i)
	{
		__cpuid(CPUInfo, i);

		// Interpret CPU brand string and cache information.
		if  (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if  (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if  (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}

	// Display all the information in user-friendly format.

	printf_s("\n\nCPU String: %s\n", CPUString);

	if( nExIds >= 0x80000004 )
		return CPUBrandString;
	else
		return  CPUString;
}

