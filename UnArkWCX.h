#pragma once

#include "wcxhead.h"
#include "ArkLib.h"
#include "resource.h"

// 다음 ifdef 블록은 DLL에서 내보내기하는 작업을 쉽게 해 주는 매크로를 만드는 
// 표준 방식입니다. 이 DLL에 들어 있는 파일은 모두 명령줄에 정의된 _EXPORTS 기호로
// 컴파일되며, 동일한 DLL을 사용하는 다른 프로젝트에서는 이 기호를 정의할 수 없습니다.
// 이렇게 하면 소스 파일에 이 파일이 들어 있는 다른 모든 프로젝트에서는 
// UNARKWCX_API 함수를 DLL에서 가져오는 것으로 보고, 이 DLL은
// 이 DLL은 해당 매크로로 정의된 기호가 내보내지는 것으로 봅니다.
#ifdef UNARKWCX_EXPORTS
#define UNARKWCX_API __declspec(dllexport)
#else
#define UNARKWCX_API __declspec(dllimport)
#endif

/*
	토탈커맨더 플러그인인 WCX 의 수행에 있어서 필수적인 함수들 선언

*/
extern HINSTANCE g_hInst;
extern CArkLib gArkLib;
extern WCHAR gArkDLLFullPathName[ MAX_PATH ];
extern WCHAR gCurrentArchiveExtension[ 32 ];
extern WCHAR gConfigureINIFullPath[ MAX_PATH ];

const LPCWSTR CONFIGURE_INI_FILENAME = L"UnArkWCX.INI";

EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchive( tOpenArchiveData *pArchiveData );
EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchiveW( tOpenArchiveDataW *pArchiveData );

EXTERN_C UNARKWCX_API int WINAPI ReadHeader( HANDLE hArcData, tHeaderData *pHeaderData );
EXTERN_C UNARKWCX_API int WINAPI ReadHeaderExW( HANDLE hArcData, tHeaderDataExW *pHeaderDataExW );

EXTERN_C UNARKWCX_API int WINAPI ProcessFile( HANDLE hArcData, int Operation, char *DestPath, char *DestName);
EXTERN_C UNARKWCX_API int WINAPI ProcessFileW( HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName );

EXTERN_C UNARKWCX_API int WINAPI PackFiles( char* PackedFile, char* SubPath, char* SrcPath, char* AddList, int Flags );
EXTERN_C UNARKWCX_API int WINAPI PackFilesW( wchar_t* PackedFile, wchar_t* SubPath, wchar_t* SrcPath, wchar_t* AddList, int Flags );

EXTERN_C UNARKWCX_API int WINAPI DeleteFiles( char *PackedFile, char *DeleteList );
EXTERN_C UNARKWCX_API int WINAPI DeleteFilesW( wchar_t *PackedFile, wchar_t *DeleteList );

EXTERN_C UNARKWCX_API void WINAPI ConfigurePacker( HWND Parent, HINSTANCE DllInstance );
BOOL CALLBACK ConfigurePackerDlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam );

EXTERN_C UNARKWCX_API int WINAPI CloseArchive( HANDLE hArcData );

EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProc( HANDLE hArcData, tChangeVolProc pfnChangeVolProc );
EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc );

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProc( HANDLE hArcData, tProcessDataProc pfnProcessDataProc );
EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc );

EXTERN_C UNARKWCX_API int WINAPI GetPackerCaps();
EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFile( char *FileName );
EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFileW( WCHAR *FileName );

EXTERN_C UNARKWCX_API void WINAPI PackSetDefaultParams(PackDefaultParamStruct* dps);

class CArkEvent : public IArkEvent
{
public:
	CArkEvent() : pfnChangeVolProc( NULL ), pfnChangeVolProcW( NULL ), pfnProcessDataProc( NULL ), pfnProcessDataProcW( NULL ) {};

	ARKMETHOD(void)	OnOpening(const SArkFileItem* pFileItem, float progress, BOOL& bStop) {}

	ARKMETHOD(void) OnStartFile(const SArkFileItem* pFileItem, BOOL& bStopCurrent, BOOL& bStopAll) {}
	ARKMETHOD(void)	OnProgressFile(const SArkProgressInfo* pProgressInfo, BOOL& bStopCurrent, BOOL& bStopAll)
	{
		int bContinue = TRUE;

		if( pfnProcessDataProc != NULL )
		{
			bContinue = pfnProcessDataProc( NULL, pProgressInfo->_processed );
		}
		else if( pfnProcessDataProcW != NULL )
		{
			bContinue = pfnProcessDataProcW( NULL, pProgressInfo->_processed );
		}

		if( bContinue == FALSE )
		{
			bStopAll = TRUE;
			bStopCurrent = TRUE;
		}

	}

	ARKMETHOD(void)	OnCompleteFile(const SArkProgressInfo* pProgressInfo, ARKERR nErr) {}

	ARKMETHOD(void)	OnError(ARKERR nErr, const SArkFileItem* pFileItem, BOOL bIsWarning, BOOL& bStopAll) 
	{
// 		LM_ERROR(( L"압축파일 작업 중 오류발생, %d", nErr ));
// 		LM_ERROR(( L"파일 이름 : %s", pFileItem->fileNameW ));


	}
	ARKMETHOD(void)	OnMultiVolumeFileChanged(LPCWSTR szPathFileName)
	{
// 		if( pfnChangeVolProc != NULL )
// 		{
// 			char* pFilePath = _strdup( CU2A(szPathFileName).c_str() );
// 
// 			pfnChangeVolProc( pFilePath, PK_VOL_NOTIFY );
// 
// 			free( pFilePath );
// 		}
// 		else if( pfnChangeVolProcW != NULL )
// 		{
// 			WCHAR* pFilePath = _wcsdup( szPathFileName );
// 
// 			pfnChangeVolProcW( pFilePath, PK_VOL_NOTIFY );
// 
// 			free( pFilePath );
// 		}
	}
	ARKMETHOD(void)	OnAskOverwrite(const SArkFileItem* pFileItem, LPCWSTR szLocalPathName, ARK_OVERWRITE_MODE& overwrite, WCHAR pathName2Rename[ARK_MAX_PATH]) {}
	ARKMETHOD(void) OnAskPassword(const SArkFileItem* pFileItem, ARK_PASSWORD_ASKTYPE askType, ARK_PASSWORD_RET& ret, WCHAR passwordW[ARK_MAX_PASS]) {}

	tProcessDataProcW	pfnProcessDataProcW;
	tProcessDataProc	pfnProcessDataProc;

	tChangeVolProcW		pfnChangeVolProcW;
	tChangeVolProc		pfnChangeVolProc;
};

extern CArkEvent gClsArkEvent;

class CArkInfo
{
public:
	CArkInfo() : pArkLib( NULL ), currentFileIndex( -1 ) {} ;
	~CArkInfo() { arkLib.Close(); arkLib.Destroy(); } ;

	BOOL32		Open( LPCWSTR pwszFilePath, LPCWSTR password );
	int			IncrementCurrentFileIndex() { return ++currentFileIndex; };
	int			GetCurrentFileIndex() { return currentFileIndex; };

	CArkLib*	GetArk() { return &arkLib; };

	CArkEvent&	GetArkEvent() { return eEvent; };

private:
	CArkLib		arkLib;
	IArk*		pArkLib;
	CArkEvent	eEvent;
	int			currentFileIndex;
};