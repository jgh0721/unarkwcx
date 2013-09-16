#pragma once

#include "UnArkWCX_Export.h"

#include "resource.h"

/*
	토탈커맨더 플러그인인 WCX 의 수행에 있어서 필수적인 함수들 선언

*/
extern HINSTANCE g_hInst;
extern CArkLib gArkLib;
extern WCHAR gArkDLLFullPathName[ MAX_PATH ];
extern WCHAR gCurrentArchiveExtension[ 32 ];
extern WCHAR gConfigureINIFullPath[ MAX_PATH ];
extern BOOL isInitLogger;
extern SArkCompressorOpt gArkCompressorOpt;

BOOL CALLBACK ConfigurePackerDlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam );

class CArkEvent : public IArkEvent
{
public:
	CArkEvent() : pfnChangeVolProc( NULL ), pfnChangeVolProcW( NULL ), pfnProcessDataProc( NULL ), pfnProcessDataProcW( NULL ) {};

	ARKMETHOD(void)	OnOpening(const SArkFileItem* pFileItem, float progress, BOOL& bStop) {}

	ARKMETHOD(void) OnStartFile(const SArkFileItem* pFileItem, BOOL& bStopCurrent, BOOL& bStopAll, int index ) {}
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


void InitLogger();
void SetCompressorOptFromINI();
