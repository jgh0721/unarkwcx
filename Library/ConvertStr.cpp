#include "stdafx.h"

#include "ConvertStr.h"

int getActualUTF8Length( const char* szUTF8 );
int convertUTF8toUTF16( const char* pszUTF8, wchar_t* pwszUTF16 );
int convertUTF16toUTF8( wchar_t* pwszUTF16, char* pszUTF8 );
int getBytesUTF16toUTF8( wchar_t* pwszUTF16 );

int getActualUTF8Length( const char* szUTF8 )
{
	int nLength = 0;
	while( *szUTF8 ) 
		nLength += (*szUTF8++ & 0xc0) != 0x80;

	return nLength;
}

int convertUTF8toUTF16( const char* pszUTF8, wchar_t* pwszUTF16 )
{
	int nRetSize = 0;
	int iIndex = 0;
	WCHAR wChar;

	while( 0 != pszUTF8[iIndex] )
	{
		if( ( 0xE0 == ( pszUTF8[iIndex] & 0xE0 ) ) )
		{
			wChar = ( ( pszUTF8[iIndex] & 0x0f ) << 12 ) | 
				( ( pszUTF8[iIndex+1]&0x3F ) << 6 ) | 
				( pszUTF8[iIndex+2] & 0x3F );

			iIndex += 3;
		} 
		else if( 0xC0 == ( pszUTF8[iIndex] & 0xC0 ) )
		{
			wChar = ( ( pszUTF8[iIndex] & 0x1F ) << 6 ) |
				( pszUTF8[iIndex+1] & 0x3F );

			iIndex += 2;
		} 
		else
		{
			wChar = pszUTF8[iIndex] & 0x7F;

			iIndex++;
		}

		pwszUTF16[nRetSize] = wChar;

		nRetSize++;
	}

	pwszUTF16[nRetSize] = 0;

	return nRetSize;
}

int getBytesUTF16toUTF8( wchar_t* pwszUTF16 )
{
	int nBytes = 1; // NULL - Terminating 을 위한 1바이트 미리 잡음
	wchar_t* wszBuffer = pwszUTF16;

	while( *wszBuffer != L'\0' )
	{
		if( *wszBuffer < 0x80 )
			nBytes += 1;
		else if( *wszBuffer < 0x800 )
			nBytes += 2;
		else 
			nBytes += 3;

		wszBuffer++;
	}

	return nBytes;
}

int convertUTF16toUTF8( wchar_t* pwszUTF16, char* pszUTF8 )
{
	assert( pwszUTF16 != NULL );
	assert( pszUTF8 != NULL );

	int nTotalBytes = 0;
	int nBytes = -1;
	BYTE szBytes[ 4 ] = {0,};
	WCHAR wChar;
	wchar_t* pwszBuffer = pwszUTF16;

	while( *pwszBuffer != L'\0' )
	{
		wChar = *pwszBuffer;

		if( wChar < 0x80 )
		{
			nBytes = 1;
			szBytes[ 0 ] = (BYTE)wChar;
		}
		else if( wChar < 0x800 )
		{
			nBytes = 2;
			szBytes[ 1 ] = (wChar & 0x3f) | 0x80;
			szBytes[ 0 ] = ( (wChar << 2) & 0xcf00 | 0xc000 ) >> 8;
		}
		else
		{
			nBytes = 3;
			szBytes[2] = ( wChar & 0x3f ) | 0x80;
			szBytes[1] = ( ( wChar << 2 ) & 0x3f00 | 0x8000) >> 8;
			szBytes[0] = ( ( wChar << 4 ) & 0x3f0000 | 0xe00000) >> 16;
		}

		for( int j = 0; j < nBytes; ++j )
			pszUTF8[ nTotalBytes++ ] = szBytes[ j ];

		pwszBuffer++;
	}

	pszUTF8[ nTotalBytes ] = '\0';

	return nTotalBytes;
}

CA2U::CA2U( const std::string& str )
	: m_pszBuffer(NULL),m_pwszBuffer(NULL)
{
	CopyAnsiString( &m_pszBuffer, str.c_str(), str.length() );
}

CA2U::CA2U( const char* pszStr )
	: m_pszBuffer(NULL),m_pwszBuffer(NULL)
{
	assert( pszStr != NULL );

	CopyAnsiString( &m_pszBuffer, pszStr, -1 );
}

CA2U::operator const wchar_t*()
{
	return c_str();
}

const wchar_t* CA2U::c_str()
{
	assert( m_pszBuffer != NULL );

	if( m_pwszBuffer != NULL )
		return m_pwszBuffer;

#ifdef _WINDOWS_
	int nReqSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, m_pszBuffer, -1, m_pwszBuffer, 0 );
	m_pwszBuffer = new wchar_t[ nReqSize ];
	memset( m_pwszBuffer, '\0', sizeof(wchar_t) * nReqSize );

	int nRetSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, m_pszBuffer, -1, m_pwszBuffer, nReqSize );
	if( nRetSize > 0 )
		return m_pwszBuffer;
#else
	int nReqSize = mbstowcs( NULL, m_pszBuffer, 0 );
	m_pwszBuffer = new wchar_t[ ++nReqSize ];
	memset( m_pwszBuffer, '\0', sizeof(wchar_t) * nReqSize );

	int nRetSize = mbstowcs( m_pwszBuffer, m_pszBuffer, nReqSize );
	if( nRetSize >= 0 )
		return m_pwszBuffer;
#endif

	return L"";
}

CA2U& CA2U::operator =( const char* pszStr )
{
	assert( pszStr != NULL );

	DeletePtrA< wchar_t* >( m_pwszBuffer );
	DeletePtrA< char* >( m_pszBuffer );

	CopyAnsiString( &m_pszBuffer, pszStr );

	return *this;
}

CA2U& CA2U::operator =( const std::string& str )
{
	DeletePtrA< wchar_t* >( m_pwszBuffer );
	DeletePtrA< char* >( m_pszBuffer );

	CopyAnsiString( &m_pszBuffer, str.c_str(), str.length() );

	return *this;
}

//////////////////////////////////////////////////////////////////////////

CU2A::CU2A( const std::wstring& str )
	: m_pszBuffer(NULL),m_pwszBuffer(NULL)
{

	CopyWideString( &m_pwszBuffer, str.c_str(), str.length() );
}

CU2A::CU2A( const wchar_t* pwszStr )
	: m_pszBuffer(NULL),m_pwszBuffer(NULL)
{
	assert( pwszStr != NULL );

	CopyWideString( &m_pwszBuffer, pwszStr );
}

CU2A::operator const char*()
{
	return c_str();
}

const char* CU2A::c_str()
{
	assert( m_pwszBuffer != NULL );

	if( m_pszBuffer != NULL )
		return m_pszBuffer;

#ifdef _WINDOWS_
	int nReqSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, m_pwszBuffer, -1, m_pszBuffer, 0, NULL, NULL );
	m_pszBuffer = new char[ nReqSize ];
	memset( m_pszBuffer, '\0', nReqSize * sizeof( char ) );

	int nRetSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, m_pwszBuffer, -1, m_pszBuffer, nReqSize, NULL, NULL );
	if( nRetSize > 0 )
		return m_pszBuffer;
#else
	int nReqSize = wcstombs( NULL, m_pwszBuffer, 0 );
	m_pszBuffer = new char[ ++nReqSize ];
	memset( m_pszBuffer, '\0', nReqSize * sizeof( char ) );

	int nRetSize = wcstombs( m_pszBuffer, m_pwszBuffer, nReqSize );
	if( nRetSize >= 0 )
		return m_pszBuffer;
#endif

	return "";
}

CU2A& CU2A::operator =( const wchar_t* pszStr )
{
	assert( pszStr != NULL );

	DeletePtrA< wchar_t* >( m_pwszBuffer );
	DeletePtrA< char* >( m_pszBuffer );

	CopyWideString( &m_pwszBuffer, pszStr );

	return *this;
}

CU2A& CU2A::operator =( const std::wstring& str )
{
	DeletePtrA< wchar_t* >( m_pwszBuffer );
	DeletePtrA< char* >( m_pszBuffer );

	CopyWideString( &m_pwszBuffer, str.c_str(), str.length() );

	return *this;
}

//////////////////////////////////////////////////////////////////////////

CU82U::CU82U( const char* pszStr )
	: m_pszBuffer(NULL),m_pwszBuffer(NULL)
{
	assert( pszStr != NULL );

	CopyAnsiString( &m_pszBuffer, pszStr );
}

CU82U::CU82U( const std::string& str )
	: m_pszBuffer(NULL),m_pwszBuffer(NULL)
{
	CopyAnsiString( &m_pszBuffer, str.c_str() );
}

CU82U::operator const wchar_t*()
{
	return c_str();
}

const wchar_t* CU82U::c_str()
{
	assert( m_pszBuffer != NULL );

	if( m_pwszBuffer != NULL )
		return m_pwszBuffer;

#ifdef _WINDOWS_
	int nReqSize = MultiByteToWideChar( CP_UTF8, 0, m_pszBuffer, -1, m_pwszBuffer, 0 );
	m_pwszBuffer = new wchar_t[ nReqSize ];
	memset( m_pwszBuffer, '\0', nReqSize * sizeof( wchar_t ) );

	int nRetSize = MultiByteToWideChar( CP_UTF8, 0, m_pszBuffer, -1, m_pwszBuffer, nReqSize );
	if( nRetSize > 0 )
		return m_pwszBuffer;
#else
	// UTF-8 문자열의 실제 길이 구하기
	int nReqSize = getActualUTF8Length( m_pszBuffer );

	if( nReqSize <= 0 )
		return L"";

	m_pwszBuffer = new wchar_t[ nReqSize + 1 ];
	memset( m_pwszBuffer, '\0', nReqSize * sizeof( wchar_t ) );

	// 변환시작
	int nRetSize = convertUTF8toUTF16( m_pszBuffer, m_pwszBuffer );
	if( nRetSize > 0 )
		return m_pwszBuffer;
#endif

	return L"";
}

CU82U& CU82U::operator =( const char* pszStr )
{
	assert( pszStr != NULL );

	DeletePtrA< char* >( m_pszBuffer );
	DeletePtrA< wchar_t* >( m_pwszBuffer );

	CopyAnsiString( &m_pszBuffer, pszStr );

	return *this;
}

CU82U& CU82U::operator =( const std::string& pszStr )
{

	DeletePtrA< char* >( m_pszBuffer );
	DeletePtrA< wchar_t* >( m_pwszBuffer );

	CopyAnsiString( &m_pszBuffer, pszStr.c_str() );

	return *this;
}

//////////////////////////////////////////////////////////////////////////

CU2U8::CU2U8( const wchar_t* pwszStr )
	: m_pszBuffer(NULL),m_pwszBuffer(NULL)
{
	assert( pwszStr != NULL );

	CopyWideString( &m_pwszBuffer, pwszStr );
}

CU2U8::CU2U8( const std::wstring& str )
	: m_pszBuffer(NULL),m_pwszBuffer(NULL)
{
	CopyWideString( &m_pwszBuffer, str.c_str() );
}

CU2U8::operator const char*()
{
	return c_str();
}

const char* CU2U8::c_str()
{
	assert( m_pwszBuffer != NULL );

	if( m_pszBuffer != NULL )
		return m_pszBuffer;

#ifdef _WINDOWS_
	int nReqSize = WideCharToMultiByte( CP_UTF8, 0, m_pwszBuffer, -1, m_pszBuffer, 0, NULL, NULL );
	m_pszBuffer = new char[ nReqSize ];
	memset( m_pszBuffer, '\0', nReqSize * sizeof( char ) );

	int nRetSize = WideCharToMultiByte( CP_UTF8, 0, m_pwszBuffer, -1, m_pszBuffer, nReqSize, NULL, NULL );
	if( nReqSize > 0 )
		return m_pszBuffer;
#else
	int nReqSize = getBytesUTF16toUTF8( m_pwszBuffer );
	int nRetSize = convertUTF16toUTF8( m_pwszBuffer, m_pszBuffer );

	if( nRetSize > 0 )
		return m_pszBuffer;
#endif

	return "";
}

CU2U8& CU2U8::operator =( const wchar_t* pwszStr )
{
	assert( pwszStr != NULL );

	CopyWideString( &m_pwszBuffer, pwszStr );

	return *this;
}

CU2U8& CU2U8::operator =( const std::wstring& str )
{
	CopyWideString( &m_pwszBuffer, str.c_str(), str.length() );

	return *this;
}

//////////////////////////////////////////////////////////////////////////

CA2U8::CA2U8( const char* pszStr )
	: m_pszBufferA(NULL), m_pszBufferU8(NULL)
{
	assert( pszStr != NULL );

	CopyAnsiString( &m_pszBufferA, pszStr );
}

CA2U8::CA2U8(  const std::string& str )
	: m_pszBufferA(NULL), m_pszBufferU8(NULL)
{
	CopyAnsiString( &m_pszBufferA, str.c_str() );
}

CA2U8::operator const char*()
{
	return c_str();
}

const char* CA2U8::c_str()
{
	assert( m_pszBufferA != NULL );

	if( m_pszBufferU8 != NULL )
		return m_pszBufferU8;

	wchar_t* pwszBuffer = NULL;
	// 윈도 환경에서는 ANSI -> UTF-16 -> UTF-8 로 변환해야함
#ifdef _WINDOWS_
	int nReqSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, m_pszBufferA, -1, pwszBuffer, 0 );
	pwszBuffer = new wchar_t[ nReqSize ];
	memset( pwszBuffer, '\0', sizeof(wchar_t) * nReqSize );

	int nRetSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, m_pszBufferA, -1, pwszBuffer, nReqSize );
	if( nRetSize > 0 )
	{
		nReqSize = WideCharToMultiByte( CP_UTF8, 0, pwszBuffer, -1, m_pszBufferU8, 0, NULL, NULL );
		m_pszBufferU8 = new char[ nReqSize ];
		memset( m_pszBufferU8, '\0', nReqSize * sizeof( char ) );

		nRetSize = WideCharToMultiByte( CP_UTF8, 0, pwszBuffer, -1, m_pszBufferU8, nReqSize, NULL, NULL );
		DeletePtrA< wchar_t *>( pwszBuffer );

		if( nRetSize > 0 )
			return m_pszBufferU8;
	}
#else
	int nReqSize = mbstowcs( NULL, m_pszBuffer, 0 );
	pwszBuffer = new wchar_t[ ++nReqSize ];
	memset( pwszBuffer, '\0', sizeof(wchar_t) * nReqSize );

	int nRetSize = mbstowcs( pwszBuffer, m_pszBuffer, nReqSize );
	if( nRetSize >= 0 )
	{
		int nBytes = -1;
		BYTE szBytes[ 4 ] = {0,};
		int iLength = wcslen( pwszBuffer );
		WCHAR wChar;
		int nLen = 0;

		// 2바이트 한글자는 최대 4바이트로 커질 수 있으므로 넉넉하게 선언
		m_pszBufferU8 = new char[ 3 * wcslen( pwszBuffer ) ];
		memset( m_pszBufferU8, '\0', 3 * wcslen( pwszBuffer ) * sizeof( char ) );

		for( int idx = 0; idx < iLength; ++idx )
		{
			wChar = pwszBuffer[ idx ];

			if( wChar < 0x80 )
			{
				nBytes = 1;
				szBytes[ 0 ] = (BYTE)wChar;
			}
			else if( wChar < 0x800 )
			{
				nBytes = 2;
				szBytes[ 1 ] = (wChar & 0x3f) | 0x80;
				szBytes[ 0 ] = ( (wChar << 2) & 0xcf00 | 0xc000 ) >> 8;
			}
			else
			{
				nBytes = 3;
				szBytes[2] = ( wChar & 0x3f ) | 0x80;
				szBytes[1] = ( ( wChar << 2 ) & 0x3f00 | 0x8000) >> 8;
				szBytes[0] = ( ( wChar << 4 ) & 0x3f0000 | 0xe00000) >> 16;
			}

			for( int j = 0; j < nBytes; ++j )
			{
				m_pszBufferU8[ nLen ] = szBytes[ j ];
				nLen++;
			}
		}

		m_pszBufferU8[ nLen ] = '\0';
		DeletePtrA< wchar_t* >( pwszBuffer );

		return m_pszBufferU8;
	}
#endif

	DeletePtrA< wchar_t* >( pwszBuffer );
	return "";
}

CA2U8& CA2U8::operator =( const char* pszStr )
{
	assert( pszStr != NULL );

	DeletePtrA< char* >( m_pszBufferA );
	DeletePtrA< char* >( m_pszBufferU8 );

	CopyAnsiString( &m_pszBufferA, pszStr );

	return *this;
}

CA2U8& CA2U8::operator =( const std::string& str )
{
	DeletePtrA< char* >( m_pszBufferA );
	DeletePtrA< char* >( m_pszBufferU8 );

	CopyAnsiString( &m_pszBufferA, str.c_str() );
	
	return *this;
}

//////////////////////////////////////////////////////////////////////////

CU82A::CU82A( const char* pszStr )
	: m_pszBufferA(NULL), m_pszBufferU8(NULL)
{
	assert( pszStr != NULL );

	CopyAnsiString( &m_pszBufferU8, pszStr );
}

CU82A::CU82A(  const std::string& str )
	: m_pszBufferA(NULL), m_pszBufferU8(NULL)
{
	CopyAnsiString( &m_pszBufferU8, str.c_str() );
}

CU82A::operator const char *()
{
	return c_str();
}

const char* CU82A::c_str()
{
	assert( m_pszBufferU8 != NULL );

	if( m_pszBufferA != NULL )
		return m_pszBufferA;

	wchar_t* pwszBuffer = NULL;

#ifdef _WINDOWS_
	int nReqSize = MultiByteToWideChar( CP_UTF8, 0, m_pszBufferU8, -1, pwszBuffer, 0 );
	pwszBuffer = new wchar_t[ nReqSize ];
	memset( pwszBuffer, '\0', nReqSize * sizeof( wchar_t ) );

	int nRetSize = MultiByteToWideChar( CP_UTF8, 0, m_pszBufferU8, -1, pwszBuffer, nReqSize );
	if( nRetSize > 0 )
	{
		nReqSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, pwszBuffer, -1, m_pszBufferA, 0, NULL, NULL );
		m_pszBufferA = new char[ nReqSize ];
		memset( m_pszBufferA, '\0', nReqSize * sizeof( char ) );

		nRetSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, pwszBuffer, -1, m_pszBufferA, nReqSize, NULL, NULL );
		DeletePtrA< wchar_t* >( pwszBuffer );
		if( nRetSize > 0 )
			return m_pszBufferA;
	}
#else
	// TODO UTF-8 -> UTF-16 -> ANSI 변환코드 필요
#endif

	return "";
}

CU82A& CU82A::operator =( const char* pszStr )
{
	assert( pszStr != NULL );

	CopyAnsiString( &m_pszBufferU8, pszStr );

	return *this;
}

CU82A& CU82A::operator =( const std::string& str )
{
	CopyAnsiString( &m_pszBufferU8, str.c_str() );

	return *this;
}

