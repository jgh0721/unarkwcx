#pragma once

#include <cassert>
#include <string>

#include "CommonUtil.h"
//////////////////////////////////////////////////////////////////////////
/// 문자열 변환을 위한 간편한 클래스 모음
/// 작성자 : 헬마
//////////////////////////////////////////////////////////////////////////

// MBCS To Unicode
class CA2U
{
public:
	CA2U( const char* pszStr );
	CA2U( const std::string& str );
	~CA2U()	{ DeletePtrA< char* >( m_pszBuffer ); DeletePtrA< wchar_t* >( m_pwszBuffer ); }

	operator const wchar_t* ();
	CA2U& operator =( const char* pszStr );
	CA2U& operator =( const std::string& str );

	const wchar_t* c_str();
private:
	char*		m_pszBuffer;
	wchar_t*	m_pwszBuffer;

};

// Unicode To MBCS
class CU2A
{
public:
	CU2A( const wchar_t* pwszStr );
	CU2A( const std::wstring& str );
	~CU2A()	{ DeletePtrA< char* >( m_pszBuffer ); DeletePtrA< wchar_t* >( m_pwszBuffer ); }

	operator const char* ();
	CU2A& operator =( const wchar_t* pwszStr );
	CU2A& operator =( const std::wstring& pwszStr );

	const char* c_str();
private:
	char*		m_pszBuffer;
	wchar_t*	m_pwszBuffer;
};

// UTF-8 To Unicode
class CU82U
{
public:
	CU82U( const char* pszStr );
	CU82U( const std::string& str );
	~CU82U()	{ DeletePtrA< char* >( m_pszBuffer ); DeletePtrA< wchar_t* >( m_pwszBuffer ); }

	operator const wchar_t* ();
	CU82U& operator =( const char* pszStr );
	CU82U& operator =( const std::string& str );

	const wchar_t* c_str();
private:
	char*		m_pszBuffer;
	wchar_t*	m_pwszBuffer;
};

// Unicode To UTF-8
class CU2U8
{
public:
	CU2U8( const wchar_t* pwszStr );
	CU2U8( const std::wstring& str );
	~CU2U8()	{ DeletePtrA< char* >( m_pszBuffer ); DeletePtrA< wchar_t* >( m_pwszBuffer ); }

	operator const char* ();
	CU2U8& operator =( const wchar_t* pwszStr );
	CU2U8& operator =( const std::wstring& str );

	const char* c_str();
private:
	char*			m_pszBuffer;
	wchar_t*		m_pwszBuffer;
};

// MBCS to UTF-8
class CA2U8
{
public:
	CA2U8( const char* pszStr );
	CA2U8( const std::string& str );
	~CA2U8()	{ DeletePtrA< char* >( m_pszBufferA ); DeletePtrA< char* >( m_pszBufferU8 ); }

	// UTF-8 문자열을 반환함
	operator const char* ();
	CA2U8& operator =( const char* pszStr );
	CA2U8& operator =( const std::string& str );

	const char* c_str();
private:
	char*			m_pszBufferA;
	char*			m_pszBufferU8;

};

// UTF-8 to MBCS
class CU82A
{
public:
	CU82A( const char* pszStr );
	CU82A( const std::string& str );
	~CU82A()	{ DeletePtrA< char* >( m_pszBufferA ); DeletePtrA< char* >( m_pszBufferU8 ); }

	// UTF-8 문자열을 반환함
	operator const char* ();
	CU82A& operator =( const char* pszStr );
	CU82A& operator =( const std::string& str );

	const char* c_str();
private:
	char*			m_pszBufferA;
	char*			m_pszBufferU8;

};


