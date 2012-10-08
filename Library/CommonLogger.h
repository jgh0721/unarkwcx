#pragma once

#include <cassert>
#include <string>
#include <iostream>
#include <map>

#include <io.h>
#include <time.h>
#include <stdarg.h>

#include <vector>
#include <queue>

#include "CommonUtil.h"
#include "ConvertStr.h"

/*!
    @file CommonLogger.h

    @page hdrCmnLoggerLibrary ���� �α� ���̺귯��

    ����� �α׸� ��������� ���� ���̺귯��
    �ۼ��� : �︶
    ���� �ۼ��� : 2009-07-21

    ���� �ҽ��ڵ� : ACE Library �� Log_Msg.cpp, Log_Msg.h

    ���� OS : ���� NT �迭

    �����ϴ� ���� ���ڿ� \n
    �Ű����� �ʿ���� ���� ���ڿ� \n
    %% - %���� \n
    %T - ȣ���� ������ID ( 10���� ��� ) \n
    %P - ȣ���� ���μ���ID ( 10���� ��� )
    %N - ȣ���� �ҽ��ڵ尡 ��ġ�� �ҽ����� �̸�
    %L - ȣ���� �ҽ��ڵ尡 ��ġ�� �ٹ�ȣ
    %F - ȣ���� �Լ� �̸�
    %D - ȣ���� ��¥    ( ������ �⺻���� �ǰų� ������ ���氡�� )
    %t - ȣ���� �ð�    ( ������ �⺻���� �ǰų� ������ ���氡�� )
    %l - ȣ���� �α� ���� ( ���ڿ��� ǥ�� )
    �Ű����� �ʿ��� ���� ���ڿ�
    %o - 8����
    %x(X) - 16���� 
    %d - 32��Ʈ ��ȣ �ִ� ����
    %u - 32��Ʈ ��ȣ ���� ����
    %i(I)64d - 64��Ʈ ��ȣ �ִ� ����
    %i(I)64u - 64��Ʈ ��ȣ ���� ����
    %f - �ε��Ҽ���
    %s - ���ڿ�
    %b(B) - bool ���� �޾Ƽ� TRUE, FALSE ���ڿ��� ���
    %E - GetLastError() �� ��ȯ�� �����ڵ带 �޾Ƽ� ���ڿ��� ��ȯ�Ͽ� ���
    %H - HRESULT ������ �����ڵ带 �޾Ƽ� ���ڿ��� ��ȯ�Ͽ� ���

    .2009-07-21        :    �缳�����
    .2009-07-21        :    �Ϻ� ����ȭ ���ڿ� ����
    .2009-07-21        :    ����̸� ����
    .2009-07-21        :    ANSI ���ڿ�, UNICODE ���ڿ� ��θ� ���� �� �ֵ��� �ι��� �Լ��� �ۼ�
    .2009-07-21        :    �α׼����� ������ �� �ֵ��� �߰�
    .2009-07-21        :    �α׼����� �ǽð����� ������ �� �ֵ��� ��. 
    .2009-07-21        :    ���� ���� ���� ��� �߰�
    .2009-07-21        :    ��Ʈ������ ��� ���� ���� ��Ʈ���̶� �뷮���ѵ��� �� �� ����.
    .2009-07-21        :    COM ���� �ڵ� �����߰�, %H
    .2009-07-21        :    �ε��Ҽ��� ���ڸ� ����ȭ�� �� _CVTBUFSIZE �� ����Ͽ� ����ũ�⸦ ��û�ϵ��� ���� 
    .2009-07-21        :    �α� Ŭ������ �Ѱ��� ��Ʈ��Ŭ������ ������ �α� Ŭ������ �����ϵ��� ����
    .2009-07-21        :    �α� Ŭ�������� ������ �ݹ��Լ� ���� �߰�

    .2009-07-22        :    �����̸��� ������ �����̸� �ڿ� �α׸� �ۼ��� ��¥�� �������� ��ȣ�� �ڵ����� �ٵ��� ����
    .2009-07-22        :    �α׷����� ǥ���� �� �ִ� ����ȭ ���ڿ� �߰�
    .2009-07-22        :    ������ ���ҵǾ� ������ ���� ������ ���� ���Ͽ� �̾ ���̵��� ��
    .2009-07-22        :    �α� ������ �տ� ������ �ٰԵ� ������ ������ �� �ֵ��� ����
    - �⺻�� = [LEVEL=%I][DateTime=%D %T][PID=%P][TID=%t][File=%N-%L][Func=%F]
    - ���λ� �κп��� �Ű������� �ʿ���� ����ȭ ���ڿ��� ����.
    - �⺻�� �߿��� ��� ����ϴ� ���� ����
    .2009-07-22        :    �α׳��뿡 �α� �������ڿ��� ��Ȯ�� ��µ��� �ʴ� ���� ����
    .2009-07-22        :    ANSI ���ڿ��� ���� ���� ����
    .2009-07-22        :    �ܼ�â ��� ��� ����
    .2009-07-24        :    if �� �ȿ� {} ���� �α� ��ũ���Լ��� ������� �� ��� ��Ÿ���� ���� ����
    - �α� ��ũ���Լ� ���� �ݵ�� ; �� �ٿ��� ���� ��������
    .2009-07-25        :    x64 ȯ�濡�� �����Ͻ� �߻��ϴ� ��� ����
    .2009-08-17        :    �α� ���� �� �Ϸ簡 ������ �ڵ����� ������ ��¥�� ���缭 �������� ����

    .2011-02-XX        :    ��¥, �ð� ������ ������ �� �ֵ�����
    :    �ӵ� ����ȭ�� ���� log_arg_list �� string �� ��ȯ���� ����
    :    ��� ���� ���ڿ� ����
    :    64��Ʈ ���� ���� �߰�
    :    �α� ���� ���ڿ��� ���ξ� ���� () 
    :    ���Ϸ� �α� ������ �� ��¥�� ����Ǹ� �α������� �и��Ǵ� �ɼ� �߰�
    :    �ּ� �߰�
    :    checkFileExist() �Լ����� _wfindfirst �Լ��� ������ ���ϰ˻� �ڵ��� ���� �ʴ� ���� ����
    :    �α� ������ �뷮 ������ �������̴��� ��¥�� ������ �α� ������ ���ҵ� �� �ֵ��� ����
    .2011-07-07        :    �ݹ��Լ��� ȣ���� �� �ݹ��Լ��� ����� FILE* �Ǵ� ofstream �� �����ϵ��� ����
    :    �ݹ��Լ��� ȣ���� �� prefix �κа� data �κ��� ���ڿ��� �и��Ͽ� �����ϵ��� ����
    :    �α׸� ����� �� prefix �κа� data �κ����� ������ ����ϵ��� ����
    .2012-01-11        :    LT_STDOUT �� �߰��Ͽ� ���� �ܼ������ ����ϴ� �ɼ� �߰�

    .2012-09-28        :    �ΰ� ��¹� ����� ����, ��ȣ�� 2�� ������� �ʰ� 1���� ����� �� �ֵ��� ����. �����Ϸ� VS 2008 �̻�!!
*/

/*

    ������ �ΰ� ����

    ��). �α׸� ����� ����� �����Ѵ�. ( �α� ��� ����� ��Ʈ ������ "OR" �� �̿��Ͽ� �������� ������ �� ���� )
        LM_INSTANCE()->SetLogTransfer()
    ��). �α׸� ����� ��� ���� �Ǵ� ��Ʈ���� �����ߴٸ� �ش� ����� �����Ѵ�
		LM_INSTANCE()->SetLogFile()
	��). �ΰ� ����� ������ ������
		LM_EXIT() �� ȣ���Ѵ�
*/

/// �α� ������ ��Ÿ���� ������ ���
enum CLOG_LEVEL
{
    LL_NONE = 0,
    LL_CRITICAL,
    LL_ERROR,
    LL_WARNING,
    LL_DEBUG,
    LL_TRACE
};

/// �α� ��� ����� ��Ÿ���� ������ ���
enum CLOG_TRANSFER
{
    LT_NONE = 0,
    LT_STDERR = 1,
    LT_STREAM = 2,
    LT_FILE = 4,
    LT_EVENTLOG = 8,
    LT_CONSOLE = 16,
    LT_DEBUGGER = 32,
    LT_CALLBACK = 64,
    LT_STDOUT = 128
};

// #pragma pack(push)
// #pragma pack(MEMORY_ALLOCATION_ALIGNMENT)
typedef struct tag_logdata
{
    bool        isUnicode;
    char*       szLogData;
    wchar_t*    wszLogData;
    char*       szNextMsgPtrA;
    wchar_t*    szNextMsgPtrW;

    DWORD       dwBufferSizeA;
    DWORD       dwBufferSizeW;

    DWORD       dwRemainBufferSizeA;
    DWORD       dwRemainBufferSizeW;

    volatile LONG        isUse;

    tag_logdata() : isUse( 0 ), isUnicode( true ), szLogData( NULL ), wszLogData( NULL ), dwBufferSizeA( 0 ), dwBufferSizeW( 0 ),
    dwRemainBufferSizeA( 0 ), dwRemainBufferSizeW( 0 ), szNextMsgPtrA( NULL ), szNextMsgPtrW( NULL ) {};

} LOGDATA, *PLOGDATA;
// #pragma pack(pop)

/// �α� ����� ��������� �Լ��� �ޱ� ���� �ݹ� �Լ� ���� ( �����ڵ���� )
typedef void (*pfnLogCallbackW)
    ( 
    PVOID pParam,
    CLOG_LEVEL prevLevel,
    CLOG_LEVEL nextLevel,
    const wchar_t* wszData
    );

/// �α� ����� ��������� �Լ��� �ޱ� ���� �ݹ� �Լ� ���� ( ��Ƽ����Ʈ���� )
typedef void (*pfnLogCallbackA)
    ( 
    PVOID pParam,
    CLOG_LEVEL prevLevel,
    CLOG_LEVEL nextLevel,
    const char* szData
    );

class CLog
{
public:
    ~CLog();
    static CLog*	GetInstance( void );
    static void		CloseInstance( void );

    bool			IsLogging( CLOG_LEVEL logLevel );

    size_t CLog::Log( const char* szFileName, const char* szFuncName, const wchar_t* wszFileName, const wchar_t* wszFuncName, unsigned int nLineNumber, const char* fmt, ... );
    size_t CLog::Log( const char* szFileName, const char* szFuncName, const wchar_t* wszFileName, const wchar_t* wszFuncName, unsigned int nLineNumber, const wchar_t* fmt, ... );

    // �ݹ� �߰�, ����
    bool			AddCallbackA( pfnLogCallbackA pfnCallback, PVOID pParam );
    bool			AddCallbackW( pfnLogCallbackW pfnCallback, PVOID pParam );
    void			RemoveCallbackA( pfnLogCallbackA pfnCallback );
    void			RemoveCallbackW( pfnLogCallbackW pfnCallback );

    // �α����� ����
    void			SetLogLevel( CLOG_LEVEL logLevel );
    void			SetLogTransfer( DWORD logTransfer);
    // �α� ���ڿ��� ������ �� �׻� �տ� ������ ���ڿ� ����, NULL �� �����ϸ� �⺻�� ����, �������� ������ L"" �� ����
    void			SetLogPrefix( const wchar_t* pwszLogPrefix = NULL );
    // �α� ���ڿ��� ������ �� ����� ��¥, �ð� ���� ���ڿ�, NULL �� �����ϸ� �⺻ ���� ����
    void			SetDateFormat( const wchar_t* pwszDateFormat = NULL );
    void			SetTimeFormat( const wchar_t* pwszTimeFormat = NULL );

    // �α� ��´�� ����
    bool			SetLogFile(  const wchar_t* szLogPath, const wchar_t* szLogName, const wchar_t* szLogExt );
    void			SetFileLimitSize( unsigned int nFileLimitSize = 0, bool IsFileSplit = true, bool IsFileSplitWhenDate = true );
    unsigned int	GetFileLimitSize( void );

    // ���� ����ǰ� �ִ� �α� �����̸�( ������ ��� ���� )
    std::wstring	GetFileName( void );

private:
    CLog();
    CLog& operator =( const CLog& );
    CLog( const CLog& );

    void			Lock();
    void			Unlock();

    // �������ڸ� �޾� ������ �α� ���Ĺ��ڿ��� ����, ��ȯ���� ������ ���� ��
    size_t			log_arg_list( const char* szFileName, const char* szFuncName, unsigned int nLineNumber, const char* fmt, bool isAppendNewLine, LOGDATA* pLOGDATA, va_list args );
    size_t			log_arg_list( const wchar_t* wszFileName, const wchar_t* wszFuncName, unsigned int nLineNumber, const wchar_t* fmt, bool isAppendNewLine, LOGDATA* pLOGDATA, va_list args );

    bool			getBufferA( UINT_PTR nRequiredSize, LOGDATA* pLOGDATA );
    bool			getBufferW( UINT_PTR nRequiredSize, LOGDATA* pLOGDATA );
    const char*		getLogLevelA();
    const wchar_t*	getLogLevelW();

    // ���丮 ���翩�θ� Ȯ���� �� ��������� ���丮 ����
    DWORD			checkDirectory( const wchar_t* szDirectory );
    // ���ҵ� ������ ���翩�θ� �˻��� �� ���� �������� �����Ǿ���� ���� ���Ϲ�ȣ�� ã��
    void			checkFileExist( void );
    // ���� ũ�� ����, false = �Լ� ����, ���� ��� �Ұ�, true = �Լ� ����, ���� ��� ����
    bool			checkFileSizeAndTime( const wchar_t* szFileName );
    bool			createLogFile();

    static unsigned int __stdcall processLOGThread( PVOID pParam );
    // ���� ����
    static	CLog*			pClsLog;
    static	volatile bool	isExit;
    static  const unsigned int      MAX_MESSAGE_COUNT   = 8192;
    static  const unsigned int      MSG_BUFFER_SIZE     = 2048;
    static  const unsigned int		MAX_TIME_BUFFER_SIZE = 128;

    volatile LONG                   m_lCurrentIndex;
    CLOG_LEVEL				        m_eLogLevel;
    CLOG_LEVEL				        m_eCurrentLogLevel;
    DWORD					        m_eLogTransfer;
    std::string			        	m_strPreFixA;
    std::wstring			        m_strPreFixW;
    char					        m_szDateFormatA[64];
    wchar_t					        m_szDateFormatW[64];
    char					        m_szTimeFormatA[64];
    wchar_t					        m_szTimeFormatW[64];

    // ���ҵ� �α� ���� ��
    unsigned int			        m_nSplitCount;
    // �ִ� �α� ���� ũ��, MB ����, 0 = ������
    unsigned int			        m_nFileLimitSize;
    // �ִ� �α� ���� ũ�⿡ �����ϸ� ������ ������ �� ����, false �̰� �� ���� �� �̻� ��Ͼ���
    bool					        m_IsFileSplit;
    // ��¥�� �ٲ�� �α� ������ �������� ���� 
    bool			        		m_IsFileSplitWhenDate;

    // �α������� ������ �ð�
    struct tm				        m_tmCreateLogTime;

    // ����� �α����� ���� ������
    // ���� ����ü
    FILE*							m_pLogFile;
    wchar_t			        		m_szLogPath[ MAX_PATH ];
    wchar_t			        		m_szLogName[ MAX_PATH ];
    wchar_t			        		m_szLogExt[ MAX_PATH ];
    wchar_t			        		m_szCurrentLogFile[ MAX_PATH ];	// ���� �αװ� ������� ����
    std::map<pfnLogCallbackA, PVOID>		m_lstCallbackA;
    std::map<pfnLogCallbackW, PVOID>		m_lstCallbackW;

    std::vector< PLOGDATA >         m_vecLOGData;
    std::queue< int >               m_queueLOGData;
    CRITICAL_SECTION		        m_cs;
    HANDLE                          m_hProcessLOGThread;
};

// ���� ��ũ�� ����
#define LM_INSTANCE() CLog::GetInstance()

#define LM_TRACE(...) \
    do { \
    if( LM_INSTANCE()->IsLogging( LL_TRACE ) ) \
{	\
    LM_INSTANCE()->Log( __FILE__, __FUNCTION__, __FILEW__, __FUNCTIONW__, __LINE__, __VA_ARGS__ ); \
}	\
    } while(0)

#define LM_DEBUG(...) \
    do { \
    if( LM_INSTANCE()->IsLogging( LL_DEBUG ) ) \
{	\
    LM_INSTANCE()->Log( __FILE__, __FUNCTION__, __FILEW__, __FUNCTIONW__, __LINE__, __VA_ARGS__ ); \
}	\
    } while(0)

#define LM_ERROR(...) \
    do { \
    if( LM_INSTANCE()->IsLogging( LL_ERROR ) ) \
{	\
    LM_INSTANCE()->Log( __FILE__, __FUNCTION__, __FILEW__, __FUNCTIONW__, __LINE__, __VA_ARGS__ ); \
}	\
    } while(0)

#define LM_WARNING(...) \
    do { \
    if( LM_INSTANCE()->IsLogging( LL_WARNING ) ) \
{	\
    LM_INSTANCE()->Log( __FILE__, __FUNCTION__, __FILEW__, __FUNCTIONW__, __LINE__, __VA_ARGS__ ); \
}	\
    } while(0)

#define LM_CRITICAL(...) \
    do { \
    if( LM_INSTANCE()->IsLogging( LL_CRITICAL ) ) \
{	\
    LM_INSTANCE()->Log( __FILE__, __FUNCTION__, __FILEW__, __FUNCTIONW__, __LINE__, __VA_ARGS__ ); \
}	\
    } while(0)

#define LM_EXIT() \
    do { \
    CLog::CloseInstance(); \
    } while( 0 )
