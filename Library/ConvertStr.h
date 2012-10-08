#pragma once

#include <cassert>
#include <string>

#include "CommonUtil.h"
//////////////////////////////////////////////////////////////////////////
/// ���ڿ� ��ȯ�� ���� ������ Ŭ���� ����
/// �ۼ��� : �︶
///
/// 2012-02-04 : CA2U8 Ŭ������ c_str() �Լ����� ���� ����� ������� ���� �� �����ϵ��� �ʴ� ���� ����
/// 2012-02-04 : CU2U8 Ŭ������ c_str() �Լ����� ���� ����� ������� ���� �� ���ܰ� �߻��ϴ� ���� ���� 
/// 2012-02-04 : CU82A Ŭ������ c_str() �Լ����� ���� ����� ������� ���� �� �ڵ� �ϼ�
//////////////////////////////////////////////////////////////////////////

#if defined(TEST_EXE_INCLUDE)
#define IMON_DLL_API
#else
#define IMON_DLL_API AFX_EXT_CLASS
#endif

// MBCS To Unicode
class IMON_DLL_API CA2U
{
public:
	CA2U( const char* pszStr );
	CA2U( const std::string& str );
	~CA2U()	{ DeletePtrA< char* >( m_pszBuffer ); DeletePtrA< wchar_t* >( m_pwszBuffer ); }

	operator const wchar_t* ();
#ifdef _AFX
	operator const CStringW ();
#endif
	CA2U& operator =( const char* pszStr );
	CA2U& operator =( const std::string& str );

	const wchar_t* c_str();
private:
	char*		m_pszBuffer;
	wchar_t*	m_pwszBuffer;

};

// Unicode To MBCS
class IMON_DLL_API CU2A
{
public:
	CU2A( const wchar_t* pwszStr );
	CU2A( const std::wstring& str );
#ifdef _AFX
	CU2A( const CStringW& str );
#endif
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
class IMON_DLL_API CU82U
{
public:
	CU82U( const char* pszStr );
	CU82U( const std::string& str );
	~CU82U()	{ DeletePtrA< char* >( m_pszBuffer ); DeletePtrA< wchar_t* >( m_pwszBuffer ); }

// 	operator const wchar_t* ();
	operator const std::wstring() { return c_str(); };

#ifdef _AFX
	operator const CStringW ();
#endif

	CU82U& operator =( const char* pszStr );
	CU82U& operator =( const std::string& str );

	const wchar_t* c_str();
private:
	char*		m_pszBuffer;
	wchar_t*	m_pwszBuffer;
};

// Unicode To UTF-8
class IMON_DLL_API CU2U8
{
public:
	CU2U8( const wchar_t* pwszStr );
	CU2U8( const std::wstring& str );
#ifdef _AFX
	CU2U8( const CString& str );
#endif
	~CU2U8()	{ DeletePtrA< char* >( m_pszBuffer ); DeletePtrA< wchar_t* >( m_pwszBuffer ); }

// 	operator const char* ();
	operator const std::string() { return c_str(); };

	CU2U8& operator =( const wchar_t* pwszStr );
	CU2U8& operator =( const std::wstring& str );

	const char* c_str();
private:
	char*			m_pszBuffer;
	wchar_t*		m_pwszBuffer;
};

// MBCS to UTF-8
class IMON_DLL_API CA2U8
{
public:
	CA2U8( const char* pszStr );
	CA2U8( const std::string& str );
	~CA2U8()	{ DeletePtrA< char* >( m_pszBufferA ); DeletePtrA< char* >( m_pszBufferU8 ); }

	// UTF-8 ���ڿ��� ��ȯ��
	operator const char* ();

	CA2U8& operator =( const char* pszStr );
	CA2U8& operator =( const std::string& str );

	const char* c_str();
private:
	char*			m_pszBufferA;
	char*			m_pszBufferU8;

};

// UTF-8 to MBCS
class IMON_DLL_API CU82A
{
public:
	CU82A( const char* pszStr );
	CU82A( const std::string& str );
	~CU82A()	{ DeletePtrA< char* >( m_pszBufferA ); DeletePtrA< char* >( m_pszBufferU8 ); }

	// UTF-8 ���ڿ��� ��ȯ��
	operator const char* ();

	CU82A& operator =( const char* pszStr );
	CU82A& operator =( const std::string& str );

	const char* c_str();
private:
	char*			m_pszBufferA;
	char*			m_pszBufferU8;

};


