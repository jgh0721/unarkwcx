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
///	����� �α׸� ��������� ���� ���̺귯��
///	�ۼ��� : �︶
///	���� �ۼ��� : 2009-07-21
///
///	���� �ҽ��ڵ� : ACE Library �� Log_Msg.cpp, Log_Msg.h
///
///	���� OS : ���� NT �迭
///
///	�����ϴ� ���� ���ڿ�
///	�Ű����� �ʿ���� ���� ���ڿ�
///		%% - %����
///		%T - ȣ���� ������ID ( 10���� ��� )
///		%P - ȣ���� ���μ���ID ( 10���� ��� )
///		%N - ȣ���� �ҽ��ڵ尡 ��ġ�� �ҽ����� �̸�
///		%L - ȣ���� �ҽ��ڵ尡 ��ġ�� �ٹ�ȣ
///     %F - ȣ���� �Լ� �̸�
///		%D - ȣ���� ��¥	( ������ �⺻���� �ǰų� ������ ���氡�� )
///		%t - ȣ���� �ð�	( ������ �⺻���� �ǰų� ������ ���氡�� )
///		%l - ȣ���� �α� ���� ( ���ڿ��� ǥ�� )
///	�Ű����� �ʿ��� ���� ���ڿ�
///		%o - 8����
///     %x(X) - 16���� 
///		%d - 32��Ʈ ��ȣ �ִ� ����
///		%u - 32��Ʈ ��ȣ ���� ����
///		%i(I)64d - 64��Ʈ ��ȣ �ִ� ����
///		%i(I)64u - 64��Ʈ ��ȣ ���� ����
///		%f - �ε��Ҽ���
///		%s - ���ڿ�
///		%b(B) - bool ���� �޾Ƽ� TRUE, FALSE ���ڿ��� ���
///		%E - GetLastError() �� ��ȯ�� �����ڵ带 �޾Ƽ� ���ڿ��� ��ȯ�Ͽ� ���
///		%H - HRESULT ������ �����ڵ带 �޾Ƽ� ���ڿ��� ��ȯ�Ͽ� ���
///
///		.2009-07-21		:	�缳�����
///		.2009-07-21		:	�Ϻ� ����ȭ ���ڿ� ����
///		.2009-07-21		:	����̸� ����
///		.2009-07-21		:	ANSI ���ڿ�, UNICODE ���ڿ� ��θ� ���� �� �ֵ��� �ι��� �Լ��� �ۼ�
///		.2009-07-21		:	�α׼����� ������ �� �ֵ��� �߰�
///		.2009-07-21		:	�α׼����� �ǽð����� ������ �� �ֵ��� ��. 
///		.2009-07-21		:	���� ���� ���� ��� �߰�
///		.2009-07-21		:	��Ʈ������ ��� ���� ���� ��Ʈ���̶� �뷮���ѵ��� �� �� ����.
///		.2009-07-21		:	COM ���� �ڵ� �����߰�, %H
///		.2009-07-21		:	�ε��Ҽ��� ���ڸ� ����ȭ�� �� _CVTBUFSIZE �� ����Ͽ� ����ũ�⸦ ��û�ϵ��� ���� 
///		.2009-07-21		:	�α� Ŭ������ �Ѱ��� ��Ʈ��Ŭ������ ������ �α� Ŭ������ �����ϵ��� ����
///		.2009-07-21		:	�α� Ŭ�������� ������ �ݹ��Լ� ���� �߰�
///
///		.2009-07-22		:	�����̸��� ������ �����̸� �ڿ� �α׸� �ۼ��� ��¥�� �������� ��ȣ�� �ڵ����� �ٵ��� ����
///		.2009-07-22		:	�α׷����� ǥ���� �� �ִ� ����ȭ ���ڿ� �߰�
///		.2009-07-22		:	������ ���ҵǾ� ������ ���� ������ ���� ���Ͽ� �̾ ���̵��� ��
///		.2009-07-22		:	�α� ������ �տ� ������ �ٰԵ� ������ ������ �� �ֵ��� ����
///			- �⺻�� = [LEVEL=%I][DateTime=%D %T][PID=%P][TID=%t][File=%N-%L][Func=%F]
///			- ���λ� �κп��� �Ű������� �ʿ���� ����ȭ ���ڿ��� ����.
///			- �⺻�� �߿��� ��� ����ϴ� ���� ����
///		.2009-07-22		:	�α׳��뿡 �α� �������ڿ��� ��Ȯ�� ��µ��� �ʴ� ���� ����
///		.2009-07-22		:	ANSI ���ڿ��� ���� ���� ����
///		.2009-07-22		:	�ܼ�â ��� ��� ����
///		.2009-07-24		:	if �� �ȿ� {} ���� �α� ��ũ���Լ��� ������� �� ��� ��Ÿ���� ���� ����
///			- �α� ��ũ���Լ� ���� �ݵ�� ; �� �ٿ��� ���� ��������
///		.2009-07-25		:	x64 ȯ�濡�� �����Ͻ� �߻��ϴ� ��� ����
///		.2009-08-17		:	�α� ���� �� �Ϸ簡 ������ �ڵ����� ������ ��¥�� ���缭 �������� ����
///
///		.2011-02-XX		:	��¥, �ð� ������ ������ �� �ֵ�����
///						:	�ӵ� ����ȭ�� ���� log_arg_list �� string �� ��ȯ���� ����
///						:	��� ���� ���ڿ� ����
///						:	64��Ʈ ���� ���� �߰�
///						:	�α� ���� ���ڿ��� ���ξ� ���� () 
///						:	���Ϸ� �α� ������ �� ��¥�� ����Ǹ� �α������� �и��Ǵ� �ɼ� �߰�
///						:	�ּ� �߰�
///						:	checkFileExist() �Լ����� _wfindfirst �Լ��� ������ ���ϰ˻� �ڵ��� ���� �ʴ� ���� ����
///						:	�α� ������ �뷮 ������ �������̴��� ��¥�� ������ �α� ������ ���ҵ� �� �ֵ��� ����
///		.2011-07-07		:	�ݹ��Լ��� ȣ���� �� �ݹ��Լ��� ����� FILE* �Ǵ� ofstream �� �����ϵ��� ����
///						:	�ݹ��Լ��� ȣ���� �� prefix �κа� data �κ��� ���ڿ��� �и��Ͽ� �����ϵ��� ����
///						:	�α׸� ����� �� prefix �κа� data �κ����� ������ ����ϵ��� ����
//////////////////////////////////////////////////////////////////////////
/*

	������ �ΰ� ����

	��). �α׸� ����� ����� �����Ѵ�. ( �α� ��� ����� ��Ʈ ������ "OR" �� �̿��Ͽ� �������� ������ �� ���� )
		LM_INSTANCE()->SetLogTransfer()
	��). �α׸� ����� ��� ���� �Ǵ� ��Ʈ���� �����ߴٸ� �ش� ����� �����Ѵ�
		LM_INSTANCE()->SetLogFile()
	��). �ΰ� ����� ������ ������
		LM_EXIT() �� ȣ���Ѵ�
*/

// �α� ������ ��Ÿ���� ���ڿ�
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

	// �ݹ� �߰�, ����
	bool			AddCallbackA( pfnLogCallbackA pfnCallback, PVOID pParam );
	bool			AddCallbackW( pfnLogCallbackW pfnCallback, PVOID pParam );
	void			RemoveCallbackA( pfnLogCallbackA pfnCallback );
	void			RemoveCallbackW( pfnLogCallbackW pfnCallback );

	// �α� ��´�� ����
	bool			SetLogFile(  const wchar_t* szLogPath, const wchar_t* szLogName, const wchar_t* szLogExt );
	void			SetFileLimitSize( unsigned int nFileLimitSize = 0, bool IsFileSplit = true, bool IsFileSplitWhenDate = true );
	unsigned int	GetFileLimitSize( void );

	// �α����� ����
	void			SetLogLevel( CLOG_LEVEL logLevel );
	void			SetLogTransfer( DWORD logTransfer);
	void			SetFileName( const char* szFileNameA, const wchar_t* szFileNameW );
	void			SetFuncName( const char* szFuncNameA, const wchar_t* szFuncNameW );
	void			SetLineNumber( unsigned int nLineNumber );
	// �α� ���ڿ��� ������ �� ����� ��¥, �ð� ���� ���ڿ�, NULL �� �����ϸ� �⺻ ���� ����
	void			SetDateFormat( const wchar_t* pwszDateFormat = NULL );
	void			SetTimeFormat( const wchar_t* pwszTimeFormat = NULL );
	// �α� ���ڿ��� ������ �� �׻� �տ� ������ ���ڿ� ����, NULL �� �����ϸ� �⺻�� ����, �������� ������ L"" �� ����
	void			SetLogPrefix( const wchar_t* pwszLogPrefix = NULL );

	// ���� ����ǰ� �ִ� �α� �����̸�( ������ ��� ���� )
	std::wstring	GetFileName( void );

private:
	CLog( void );
	CLog& operator =( const CLog& );
	CLog( const CLog& );

	bool			getBufferW( UINT_PTR nRequiredSize, ptrdiff_t nBetween, wchar_t** next_msg );
	bool			getBufferA( UINT_PTR nRequiredSize, ptrdiff_t nBetween, char** next_msg );
	// �������ڸ� �޾� ������ �α� ���Ĺ��ڿ��� ����, ��ȯ���� ������ ���� ��
	size_t			log_arg_list( const wchar_t* fmt, va_list args );
	size_t			log_arg_list( const char* fmt, va_list args );
	const wchar_t*	getLogLevelW();
	const char*		getLogLevelA();
	// ���丮 ���翩�θ� Ȯ���� �� ��������� ���丮 ����
	DWORD			checkDirectory( const wchar_t* szDirectory );
	// ���ҵ� ������ ���翩�θ� �˻��� �� ���� �������� �����Ǿ���� ���� ���Ϲ�ȣ�� ã��
	void			checkFileExist( void );
	// ���� ũ�� ����, false = �Լ� ����, ���� ��� �Ұ�, true = �Լ� ����, ���� ��� ����
	bool			checkFileSizeAndTime( const wchar_t* szFileName );
	bool			createLogFile();

	// ���� ����
	static	CLog*			pClsLog;

	// �ִ� �α� ���� ũ��, MB ����, 0 = ������
	unsigned int			m_nFileLimitSize;
	// �ִ� �α� ���� ũ�⿡ �����ϸ� ������ ������ �� ����, false �̰� �� ���� �� �̻� ��Ͼ���
	bool					m_IsFileSplit;
	// ���ҵ� �α� ���� ��
	unsigned int			m_nSplitCount;
	// ��¥�� �ٲ�� �α� ������ �������� ���� 
	bool					m_IsFileSplitWhenDate;
	// �α������� ������ �ð�
	struct tm				m_tmCreateLogTime;
	// ����� �α����� ���� ������
	wchar_t					m_szLogPath[ MAX_PATH ];
	wchar_t					m_szLogName[ MAX_PATH ];
	wchar_t					m_szLogExt[ MAX_PATH ];
	wchar_t					m_szCurrentLogFile[ MAX_PATH ];	// ���� �αװ� ������� ����
	// ���� ����ü
	FILE*									m_pLogFile;
	std::map<pfnLogCallbackA, PVOID>		m_lstCallbackA;
	std::map<pfnLogCallbackW, PVOID>		m_lstCallbackW;

	// ���۰��� ������
	// �ʱ� ���� ũ�� �� ���� Ȯ�����
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

	// ������ �α� ���ڿ� ����
	char*					m_szNextMsgPtrA;
	wchar_t*				m_szNextMsgPtrW;
	char*					m_szMsgBufferA;
	wchar_t*				m_szMsgBufferW;

	CLOG_LEVEL				m_eLogLevel;
	DWORD					m_eLogTransfer;

	CRITICAL_SECTION		m_cs;

	// �α����� ������ ����
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

// ���� ��ũ�� ����
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
