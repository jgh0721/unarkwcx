#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the UNARKWCX_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// UNARKWCX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef UNARKWCX_EXPORTS
#define UNARKWCX_API __declspec(dllexport)
#else
#define UNARKWCX_API __declspec(dllimport)
#endif

/*
	제작자 : 헬마
	최종 수정 일 : 2011-10-01

	참조 : Kipple 님의 Unarkwcx

	0.1 ( 2011-10-01 )
	----
		wcxtest 유틸리티의 시험 통과 ( -f, -l, -x )
	
	0.2 ( 2011-10-01 )
	----
		Ark Library 의 ExtractOneAs 메소드의 우회책 작성
	
	0.3 ( 2011-10-02 )
	----
		GetPackerCaps 구현
		CanYouHandleThisFile 구현

	0.4 ( 2011-10-02 )
	----
		x64 지원 추가

	0.5 ( 2011-10-03 )
	----
		해당 플러그인을 통해서 작업한 파일이 계속 LOCK 를 소유하는 문제 수정
*/

/*
	토탈커맨더 플러그인인 WCX 의 수행에 있어서 필수적인 함수들 선언

*/


/*
	압축 파일을 연다. 
	
	성공 : 해당 압축파일에 대한 핸들 반환
	실패 : 0
*/


EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchive( tOpenArchiveData *pArchiveData );
EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchiveW( tOpenArchiveDataW *pArchiveData );

EXTERN_C UNARKWCX_API int WINAPI ReadHeader( HANDLE hArcData, tHeaderData *pHeaderData );
EXTERN_C UNARKWCX_API int WINAPI ReadHeaderEx( HANDLE hArcData, tHeaderDataEx *pHeaderDataEx );
EXTERN_C UNARKWCX_API int WINAPI ReadHeaderExW( HANDLE hArcData, tHeaderDataExW *pHeaderDataExW );

EXTERN_C UNARKWCX_API int WINAPI ProcessFile( HANDLE hArcData, int Operation, char *DestPath, char *DestName);
EXTERN_C UNARKWCX_API int WINAPI ProcessFileW( HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName );

EXTERN_C UNARKWCX_API int WINAPI CloseArchive( HANDLE hArcData );

EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProc( HANDLE hArcData, tChangeVolProc pfnChangeVolProc );
EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc );

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProc( HANDLE hArcData, tProcessDataProc pfnProcessDataProc );
EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc );

EXTERN_C UNARKWCX_API int WINAPI GetPackerCaps();
EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFile( char *FileName );
EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFileW( WCHAR *FileName );


class CArkEvent : public IArkEvent
{
public:
	CArkEvent();

	ARKMETHOD(void)	OnOpening(const SArkFileItem* pFileItem, float progress, BOOL& bStop) {}

	ARKMETHOD(void) OnStartFile(const SArkFileItem* pFileItem, BOOL& bStopCurrent, BOOL& bStopAll) {}
	ARKMETHOD(void)	OnProgressFile(const SArkProgressInfo* pProgressInfo, BOOL& bStopCurrent, BOOL& bStopAll)
	{
		if( pfnProcessDataProc != NULL )
		{
			pfnProcessDataProc( NULL, pProgressInfo->processed );
		}
		else if( pfnProcessDataProcW != NULL )
		{
			pfnProcessDataProcW( NULL, pProgressInfo->processed );
		}
	}

	ARKMETHOD(void)	OnCompleteFile(const SArkProgressInfo* pProgressInfo, ARKERR nErr) {}

	ARKMETHOD(void)	OnError(ARKERR nErr, const SArkFileItem* pFileItem, BOOL bIsWarning, BOOL& bStopAll) 
	{
		LM_ERROR(( L"압축파일 작업 중 오류발생, %d", nErr ));
		LM_ERROR(( L"파일 이름 : %s", pFileItem->fileNameW ));


	}
	ARKMETHOD(void)	OnMultiVolumeFileChanged(LPCWSTR szPathFileName)
	{
		if( pfnChangeVolProc != NULL )
		{
			char* pFilePath = _strdup( CU2A(szPathFileName).c_str() );

			pfnChangeVolProc( pFilePath, PK_VOL_NOTIFY );

			free( pFilePath );
		}
		else if( pfnChangeVolProcW != NULL )
		{
			WCHAR* pFilePath = _wcsdup( szPathFileName );

			pfnChangeVolProcW( pFilePath, PK_VOL_NOTIFY );

			free( pFilePath );
		}
	}
	ARKMETHOD(void)	OnAskOverwrite(const SArkFileItem* pFileItem, LPCWSTR szLocalPathName, ARK_OVERWRITE_MODE& overwrite, WCHAR pathName2Rename[ARK_MAX_PATH]) {}
 	ARKMETHOD(void) OnAskPassword(const SArkFileItem* pFileItem, ARK_PASSWORD_ASKTYPE askType, ARK_PASSWORD_RET& ret, WCHAR passwordW[ARK_MAX_PASS]) {}

	tProcessDataProcW	pfnProcessDataProcW;
	tProcessDataProc	pfnProcessDataProc;

	tChangeVolProcW		pfnChangeVolProcW;
	tChangeVolProc		pfnChangeVolProc;
};

class CArkInfo
{
public:
	CArkInfo();
	~CArkInfo();


	IArk*	getArk() { return pArk; };
	void	setArk( IArk* pArk ) { this->pArk = pArk; };

	int		getCurrentFileIndex() { return currentFileIndex; };
	void	incrementCurrentFileIndex() { ++currentFileIndex; };
	void	decrementCurrentFileIndex() { --currentFileIndex; };

	CArkEvent	eEvent;

private:

	IArk*		pArk;
	int			currentFileIndex;

};

std::wstring GetArkLastErrorText( ARKERR lastError );

extern CArkLib g_arkLib;
