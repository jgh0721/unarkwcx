// UnArkWCX.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "UnArkWCX.h"

#include <string>
#include <vector>


#include <CommCtrl.h>
#include <time.h>

#pragma warning( disable: 4996 )

EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchive( tOpenArchiveData *pArchiveData )
{
	InitLogger();
	LM_TRACE( L"Here" );

	LM_TRACE( L"압축 파일 열기 시도" );

	tOpenArchiveDataW pArchiveDataW;

	pArchiveDataW.ArcName = _wcsdup( CA2U( pArchiveData->ArcName ).c_str() );
	pArchiveDataW.CmtBuf = NULL;
	pArchiveDataW.CmtBufSize = 0;
	pArchiveDataW.CmtSize = 0;
	pArchiveDataW.CmtState = 0;
	pArchiveDataW.OpenMode = pArchiveData->OpenMode;

	HANDLE hResult = OpenArchiveW( &pArchiveDataW );

	pArchiveData->OpenResult = pArchiveDataW.OpenResult;
	free( pArchiveDataW.ArcName );

	return hResult;
}

EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchiveW( tOpenArchiveDataW *pArchiveDataW )
{
	InitLogger();
	LM_TRACE( L"Here" );

	LM_TRACE( L"압축 파일 열기 시도" );

	CArkInfo* pArkInfo = NULL;

	do
	{
		pArchiveDataW->OpenResult = E_NOT_SUPPORTED;

		if( ( pArchiveDataW->OpenMode != PK_OM_LIST ) && 
			( pArchiveDataW->OpenMode != PK_OM_EXTRACT ) )
			break;

		LM_TRACE( L"압축 파일 이름 : %s", pArchiveDataW->ArcName );

		pArkInfo = new CArkInfo;
		if( pArkInfo == NULL )
		{
			pArchiveDataW->OpenResult = E_NO_MEMORY;
			break;
		}
		
		if( pArkInfo->Open( pArchiveDataW->ArcName, NULL ) == FALSE )
			break;

		LM_TRACE( L"열기 결과 : %d", pArchiveDataW->OpenResult );

		pArchiveDataW->OpenResult = 0;

	} while (false);

	return (HANDLE)pArkInfo;
}

EXTERN_C UNARKWCX_API int WINAPI ReadHeader( HANDLE hArcData, tHeaderData *pHeaderData )
{
	LM_TRACE( L"Here" );

	tHeaderDataExW headerDataW;

	int nResult = ReadHeaderExW( hArcData, &headerDataW );

	strncpy( pHeaderData->ArcName, strdup(CU2A( headerDataW.ArcName ).c_str()), 1024 );
	strncpy( pHeaderData->FileName, strdup(CU2A( headerDataW.FileName ).c_str()), 1024 );

	pHeaderData->FileAttr = headerDataW.FileAttr;
	pHeaderData->FileCRC = headerDataW.FileCRC;
	pHeaderData->FileTime = headerDataW.FileTime;
	pHeaderData->PackSize = headerDataW.PackSize;
	pHeaderData->UnpSize = headerDataW.UnpSize;

	free( pHeaderData->ArcName );
	free( pHeaderData->FileName );

	return nResult;
}

EXTERN_C UNARKWCX_API int WINAPI ReadHeaderExW( HANDLE hArcData, tHeaderDataExW *pHeaderDataExW )
{
	LM_TRACE( L"Here" );

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );

	int nResult = 0;

	do
	{
		if( pArkInfo == INVALID_HANDLE_VALUE )
		{
			nResult = E_EOPEN;
			break;
		}

		int currentFileIndex = pArkInfo->IncrementCurrentFileIndex();

		if( currentFileIndex >= pArkInfo->GetArk()->GetFileItemCount() )
		{
			nResult = E_END_ARCHIVE;
			break;
		}

		const SArkFileItem* pItem = pArkInfo->GetArk()->GetFileItem( currentFileIndex );
		if( pItem == NULL )
		{
			nResult = E_EOPEN;
			break;
		}

		LM_TRACE( L"Arc Name = %s", pArkInfo->GetArk()->GetFilePathName() );
		LM_TRACE( L"File Name = %s", pItem->fileNameW );
		LM_TRACE( L"File Time = %i64d", pItem->fileTime );

		wcsncpy( pHeaderDataExW->ArcName, pArkInfo->GetArk()->GetFilePathName(), 1023 );
		wcsncpy( pHeaderDataExW->FileName, pItem->fileNameW, 1023 );

		pHeaderDataExW->FileAttr		= pItem->attrib;
		struct tm* tmCurrentTime = localtime( &pItem->fileTime );
		if( tmCurrentTime != NULL )
		{
			pHeaderDataExW->FileTime		= 0;
			pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_year + 1900 - 1980) << 25;
			pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_mon + 1) << 21;
			pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_mday << 16 );
			pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_hour << 11 );
			pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_min << 5 );
			pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_sec/2);
		}

		pHeaderDataExW->PackSize		= pItem->compressedSize & 0xffffffff;
		pHeaderDataExW->PackSizeHigh	= pItem->compressedSize >> 32;
		pHeaderDataExW->UnpSize			= pItem->uncompressedSize & 0xffffffff;
		pHeaderDataExW->UnpSizeHigh		= pItem->uncompressedSize >> 32;
		pHeaderDataExW->FileCRC			= pItem->crc32;
		pHeaderDataExW->HostOS			= 0;
		
		memset( pHeaderDataExW->Reserved, '\0', sizeof( char ) * 1024 );

	} while (false);

	return nResult;
}

EXTERN_C UNARKWCX_API int WINAPI ProcessFile( HANDLE hArcData, int Operation, char *DestPath, char *DestName)
{
	LM_TRACE( L"Here" );

	int nResult = 0;

	WCHAR* wszDestPath = DestPath == NULL ? NULL : wcsdup( CA2U( DestPath ).c_str() );
	WCHAR* wszDestName = DestName == NULL ? NULL : wcsdup( CA2U( DestName ).c_str() );

	nResult = ProcessFileW( 
		hArcData, 
		Operation, 
		wszDestPath,
		wszDestName );

	free( wszDestName );
	free( wszDestPath );

	return nResult;
}

EXTERN_C UNARKWCX_API int WINAPI ProcessFileW( HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName )
{
	LM_TRACE( L"Here" );

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	int nResult = 0;

	do
	{
		if( pArkInfo == NULL )
		{
			nResult = E_EOPEN;
			break;
		}

		switch( Operation )
		{
		case PK_SKIP:
			break;
		case PK_TEST:
			break;
		case PK_EXTRACT:
			{
				/*
					pwszDestPath == NULL 이면 pwszDestName 에 경로명과 파일명이 모두 있음
					pwszDestPath != NULL 이면 pwszDestPath 에는 경로가 pwszDestName 에는 파일명이 있음
				*/

// 				/*
// 					Ark Library 의 ExtractOneAs 메소드에 오류가 있음. 
// 					우회책으로 pwszDestName 에 넘어온 경로명과 파일명을 분리해낸 후 경로명만 넘김
// 					하지만 pwszDestName 에는 압축파일을 풀 대상경로와 압축파일 내부에서의 경로등이 모두 포함되어있음. 
// 
// 					토탈 커맨더에서 압축을 풀면서 파일이름을 변경하거나 하는 일은 없다고 판단하여 위와 같이 작성함
// 				*/

				int ret = 0;

				pArkInfo->GetArk()->SetEvent( &(pArkInfo->GetArkEvent()) );
				
				if( ( pwszDestPath == NULL ) && 
					( pwszDestName != NULL ) )
				{

// 					LM_TRACE(( L"최초 전체 경로 : %s", pwszDestName ));
					const SArkFileItem* pItem = pArkInfo->GetArk()->GetFileItem( pArkInfo->GetCurrentFileIndex() );
// 					LM_TRACE(( L"해당 색인의 파일이름 : %s", pItem->fileNameW ));
// 
					std::wstring wstrDestName( pwszDestName );
					
					std::wstring::size_type nPos = wstrDestName.rfind( pItem->fileNameW );

					if( nPos != std::wstring::npos )
					{
// 						LM_TRACE(( L"대상 경로 : %s", wstrDestName.substr( 0, nPos ).c_str() ));

						ret = pArkInfo->GetArk()->ExtractOneTo( pArkInfo->GetCurrentFileIndex(), wstrDestName.substr( 0, nPos ).c_str() );
					}
					else
					{
// 						LM_ERROR(( L"색인과 대상경로가 겹치지 않음" ));

						PathRemoveFileSpec( pwszDestName );
						WCHAR* pwszBuffer = wcsdup( pItem->fileNameW );

						PathRemoveFileSpec( pwszBuffer );
						PathAddBackslash( pwszBuffer );
						PathAddBackslash( pwszDestName );

// 						LM_TRACE(( L"%s\n%s", pwszDestName, pItem->fileNameW ));

						pArkInfo->GetArk()->ClearExtractList();
						pArkInfo->GetArk()->AddIndex2ExtractList( pArkInfo->GetCurrentFileIndex() );
						ret = pArkInfo->GetArk()->ExtractMultiFileTo( pwszDestName, pwszBuffer );
						free( pwszBuffer );
					}
				}
				else if( ( pwszDestName != NULL ) && 
					( pwszDestPath != NULL ) )
				{
// 					LM_TRACE(( L"압축 풀기 시도 경로 : %s", pwszDestPath ));
// 					LM_TRACE(( L"압축 풀기 시도 이름 : %s", pwszDestName ));

					ret = pArkInfo->GetArk()->ExtractOneTo( pArkInfo->GetCurrentFileIndex(), pwszDestPath );
				}

				pArkInfo->GetArk()->SetEvent( NULL );

// 				LM_TRACE(( L"압축 풀기 결과 : %d, %s", ret, GetArkLastErrorText( pArkInfo->getArk()->GetLastError() ).c_str() ));

				if( ret == FALSE )
					nResult = E_BAD_ARCHIVE;
			}
			break;
		}


	} while (false);

	return nResult;
}

EXTERN_C UNARKWCX_API int WINAPI CloseArchive( HANDLE hArcData )
{
	LM_TRACE( L"Here" );

	if( hArcData != INVALID_HANDLE_VALUE )
		delete (CArkInfo *)(hArcData);
	else
	{
		gClsArkEvent.pfnProcessDataProc = NULL;
		gClsArkEvent.pfnProcessDataProcW = NULL;
		gClsArkEvent.pfnChangeVolProc = NULL;
		gClsArkEvent.pfnChangeVolProcW = NULL;
	}

	return 0;
}
EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProc( HANDLE hArcData, tChangeVolProc pfnChangeVolProc )
{
	InitLogger();
	LM_TRACE( L"Here" );

// 	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
// 	if( pArkInfo != NULL )
// 		pArkInfo->GetArkEvent().pfnChangeVolProc = pfnChangeVolProc;
}

EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc )
{
	InitLogger();
	LM_TRACE( L"Here" );

// 	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
// 	if( pArkInfo != NULL )
// 		pArkInfo->GetArkEvent().pfnChangeVolProcW = pfnChangeVolProc;
}

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProc( HANDLE hArcData, tProcessDataProc pfnProcessDataProc )
{
	InitLogger();
	LM_TRACE( L"Here" );

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo != INVALID_HANDLE_VALUE )
		pArkInfo->GetArkEvent().pfnProcessDataProc = pfnProcessDataProc;
	else
		gClsArkEvent.pfnProcessDataProc = pfnProcessDataProc;
}

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc )
{
	InitLogger();
	LM_TRACE( L"Here" );

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo != INVALID_HANDLE_VALUE )
		pArkInfo->GetArkEvent().pfnProcessDataProcW = pfnProcessDataProc;
	else
		gClsArkEvent.pfnProcessDataProcW = pfnProcessDataProc;
}

EXTERN_C UNARKWCX_API int WINAPI GetPackerCaps()
{
	InitLogger();

	LM_TRACE( L"Here" );

	return 
		PK_CAPS_NEW |
		PK_CAPS_MODIFY |
		PK_CAPS_MULTIPLE |
		PK_CAPS_DELETE |
		PK_CAPS_OPTIONS |
		PK_CAPS_BY_CONTENT;
}

EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFile( char *FileName )
{
	InitLogger();
	LM_TRACE( L"Here" );
	return CanYouHandleThisFileW( const_cast< WCHAR * >( CA2U( FileName ).c_str() ) );
}

EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFileW( WCHAR *FileName )
{
	InitLogger();
	LM_TRACE( L"Here" );
	BOOL isCanHandle = FALSE;

	do
	{
		CArkLib arkLib;
		if( arkLib.Create( gArkDLLFullPathName ) != ARKERR_NOERR )
			break;

		ARK_FF fileFormat = arkLib.CheckFormat( FileName );

		arkLib.Release();
		arkLib.Destroy();

		if( fileFormat == ARK_FF_UNKNOWN )
			break;

		if( ( fileFormat >= ARK_FF_UNSUPPORTED_FIRST ) && 
			( fileFormat <= ARK_FF_UNSUPPORTED_LAST ) )
			break;

		if( ( fileFormat >= ARK_FF_NOTARCHIVE_FIRST ) &&
			( fileFormat <= ARK_FF_NOTARCHIVE_LAST ) )
			break;

		isCanHandle = TRUE;
	} while (false);

	return isCanHandle;
}

EXTERN_C UNARKWCX_API int WINAPI PackFiles( char* PackedFile, char* SubPath, char* SrcPath, char* AddList, int Flags )
{
	LM_TRACE( L"Here" );

	LM_TRACE( "Packed Files = %s", PackedFile );
	if( SubPath != NULL )
	{
		LM_TRACE( "Sub Path = %s", SubPath );
	}
	LM_TRACE( "Src Path = %s", SrcPath );

	for( char* pszCurrent = AddList; *pszCurrent != 0; pszCurrent += strlen( pszCurrent ) + 1)
	{
		LM_TRACE( L"Add List = %s", pszCurrent );
	}

	LM_TRACE( "Flags = %d", Flags );

	return 0;
}

EXTERN_C UNARKWCX_API int WINAPI PackFilesW( wchar_t* PackedFile, wchar_t* SubPath, wchar_t* SrcPath, wchar_t* AddList, int Flags )
{
	int nRetValue = E_NOT_SUPPORTED;		// 0 == SUCCESS

	CArkLib arkLib;
	IArkCompressor* pCompressor = NULL;
	std::wstring strSrcPath, strDstPath;
	std::vector< std::wstring > vecAddFile, vecSrcFile;
	
	LM_TRACE( L"Parameter, Packed Files = %s||Sub Path = %s||Src Path = %s", PackedFile, SubPath, SrcPath );

	for( wchar_t* pwszCurrent = AddList; *pwszCurrent != 0; pwszCurrent += wcslen( pwszCurrent ) + 1)
	{
		vecAddFile.push_back( pwszCurrent );
		LM_TRACE( L"Parameter, Add List = %s", pwszCurrent );
	}

	LM_TRACE( L"Parameter, Flags = %d", Flags );

	do 
	{
		if( arkLib.Create( gArkDLLFullPathName ) != ARKERR_NOERR )
		{
			LM_ERROR( L"Ark Library Create Instance Failed = %d, %x, SYS ERROR = %E", arkLib.GetLastError(), arkLib.GetLastError(), arkLib.GetLastSystemError() );
			nRetValue = E_NOT_SUPPORTED;
			break;
		}

		pCompressor = arkLib.CreateCompressor();
		if( pCompressor == NULL )
		{
			LM_ERROR( L"Ark Library Create Compressor Instance Failed" );
			nRetValue = E_NOT_SUPPORTED;
			break;
		}

		pCompressor->Init();
		pCompressor->SetEvent( &gClsArkEvent );
		if( pCompressor->SetOption( gArkCompressorOpt, NULL, 0 ) == FALSE )
		{
			LM_ERROR( L"Ark Library Set Option Failed = %d, %x, SYS ERROR = %E", arkLib.GetLastError(), arkLib.GetLastError(), arkLib.GetLastSystemError() );
			nRetValue = E_NOT_SUPPORTED;
			break;
		}

		for( size_t idx = 0; idx < vecAddFile.size(); ++idx )
		{
			strSrcPath = SrcPath;
			if( *strSrcPath.rbegin() != L'\\' )
				strSrcPath += L"\\" + vecAddFile[ idx ];
			else
				strSrcPath += vecAddFile[ idx ];

			if( SubPath != NULL && SubPath[ wcslen( SubPath ) - 1 ] != L'\\' )
				strDstPath = SubPath + vecAddFile[idx];
			else if( SubPath != NULL && SubPath[ wcslen( SubPath ) - 1 ] == L'\\' )
				strDstPath = SubPath + std::wstring( L"\\" ) + vecAddFile[idx];
			else
				strDstPath = vecAddFile[idx];

			vecSrcFile.push_back( strSrcPath );
			pCompressor->AddFileItem( strSrcPath.c_str(), strDstPath.c_str(), FALSE );
		}

		if( pCompressor->CreateArchive( PackedFile ) == FALSE )
		{
			LM_ERROR( L"ERROR = %d, %x", pCompressor->GetLastError(), pCompressor->GetLastError() );
			nRetValue = E_ECREATE;
			break;
		}
		else
		{
			if( Flags == PK_PACK_MOVE_FILES )
			{
				for( size_t idx = 0; idx < vecSrcFile.size(); ++idx )
					DeleteFile( vecSrcFile[idx].c_str() );
			}
		}

		nRetValue = 0;

	} while (false);

	if( pCompressor != NULL )
		pCompressor->Release();

	if( arkLib.IsCreated() != FALSE )
	{
		arkLib.Close();
		arkLib.Destroy();
	}

	return nRetValue;
}

EXTERN_C UNARKWCX_API int WINAPI DeleteFiles( char *PackedFile, char *DeleteList )
{
	LM_TRACE( L"Here" );

	return 0;
}

EXTERN_C UNARKWCX_API int WINAPI DeleteFilesW( wchar_t *PackedFile, wchar_t *DeleteList )
{
	LM_TRACE( L"Here" );
	return 0;
}

EXTERN_C UNARKWCX_API void WINAPI ConfigurePacker( HWND Parent, HINSTANCE DllInstance )
{
	InitLogger();
	LM_TRACE( L"Here" );

	DialogBox( DllInstance, MAKEINTRESOURCE(IDD_DLG_OPTIONS), Parent, ConfigurePackerDlgProc );
}

BOOL CALLBACK ConfigurePackerDlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{ 
	case WM_INITDIALOG: 
		{
			wchar_t szBuffer[ 128 ] = {0,};
			GetPrivateProfileString( CONFIGURE_INI_SECNAME, CONFIGURE_SELECT_EXTENSION, L"ZIP", szBuffer, 128, gConfigureINIFullPath );
			wcscpy( gCurrentArchiveExtension, szBuffer );

			HWND hCBX_ARCHIVE_TYPE = GetDlgItem( hwndDlg, IDC_CBX_ARCHIVE_TYPE );
			if( hCBX_ARCHIVE_TYPE != NULL )
			{
				SendMessage( hCBX_ARCHIVE_TYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(L"7Z") );
				SendMessage( hCBX_ARCHIVE_TYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(L"ZIP") );
				SendMessage( hCBX_ARCHIVE_TYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)(L"ISO") );

				SendMessage( hCBX_ARCHIVE_TYPE, CB_SELECTSTRING, (WPARAM)0, (LPARAM)(LPCTSTR)gCurrentArchiveExtension );
			}

			HWND hTab = GetDlgItem( hwndDlg, IDC_TAB );
			if( hTab != NULL )
			{
				// TabCtrl_SetExtendedStyle( hTab, );
				TCITEM tabItem;

				tabItem.mask = TCIF_TEXT;
				tabItem.iImage = -1;
				tabItem.lParam = NULL;
				tabItem.pszText = L"7Z";
				tabItem.cchTextMax = wcslen( L"7Z" );
				TabCtrl_InsertItem( hTab, 0, &tabItem );

				tabItem.pszText = L"ZIP";
				tabItem.cchTextMax = wcslen( L"ZIP" );
				TabCtrl_InsertItem( hTab, 1, &tabItem );

				tabItem.pszText = L"ISO";
				tabItem.cchTextMax = wcslen( L"ISO" );
				TabCtrl_InsertItem( hTab, 2, &tabItem );
			}
		}

		return TRUE;
		break;

	case WM_COMMAND: 
		{
			switch (LOWORD(wParam)) 
			{ 
			case IDOK: 
				WritePrivateProfileString( L"UnArkWCX", L"SelectArchiveExtension", gCurrentArchiveExtension, gConfigureINIFullPath );
				
				// 			if (!GetDlgItemText(hwndDlg, ID_ITEMNAME, szItemName, 80)) 
				// 				*szItemName=0; 

				// Fall through. 

			case IDCANCEL: 
				EndDialog(hwndDlg, wParam); 
				return TRUE; 
			} 
		}
	} 
	return FALSE; 
}

EXTERN_C UNARKWCX_API void WINAPI PackSetDefaultParams( PackDefaultParamStruct* dps )
{
	InitLogger();

	LM_TRACE( L"Here" );

	wcscpy( gConfigureINIFullPath, CA2U( dps->DefaultIniName ).c_str() );

	SetCompressorOptFromINI();

}

void SetCompressorOptFromINI()
{
	gArkCompressorOpt.Init();

	wchar_t szBuffer[ 128 ] = {0,};
	ARK_COMPRESSION_METHOD defaultMethod;

	// 압축 형식 설정
	GetPrivateProfileString( CONFIGURE_INI_SECNAME, CONFIGURE_SELECT_EXTENSION, L"ZIP", szBuffer, 128, gConfigureINIFullPath );
	wcscpy( gCurrentArchiveExtension, szBuffer );
	
	if( wcsicmp( gCurrentArchiveExtension, L"ZIP" ) == 0 )
	{
		gArkCompressorOpt.ff = ARK_FF_ZIP;
		defaultMethod = ARK_COMPRESSION_METHOD_DEFLATE;
	}
	else if( wcsicmp( gCurrentArchiveExtension, L"ISO" ) == 0 )
	{
		gArkCompressorOpt.ff = ARK_FF_ISO;
	}
	else if( wcsicmp( gCurrentArchiveExtension, L"7Z" ) == 0 )
	{
		gArkCompressorOpt.ff = ARK_FF_7Z;
		defaultMethod = ARK_COMPRESSION_METHOD_LZMA2;
	}
	else if( wcsicmp( gCurrentArchiveExtension, L"LZH" ) == 0 )
	{
		gArkCompressorOpt.ff = ARK_FF_7Z;
		defaultMethod = ARK_COMPRESSION_METHOD_LH6;
	}

	gArkCompressorOpt.saveNTFSTime				= GetPrivateProfileInt( CONFIGURE_INI_SECNAME, CONFIGURE_SaveNTFSTimeForZIP, 0, gConfigureINIFullPath );
	gArkCompressorOpt.streamOutput				= FALSE;

	gArkCompressorOpt.compressionMethod			= (ARK_COMPRESSION_METHOD)GetPrivateProfileInt( CONFIGURE_INI_SECNAME, CONFIGURE_CompressionMerhod, defaultMethod, gConfigureINIFullPath );
	gArkCompressorOpt.encryptionMethod			= ARK_ENCRYPTION_METHOD_ZIP;			

	gArkCompressorOpt.compressionLevel			= GetPrivateProfileInt( CONFIGURE_INI_SECNAME, CONFIGURE_CompressionLevel, -1, gConfigureINIFullPath );			
	if( gArkCompressorOpt.compressionLevel < -1 || gArkCompressorOpt.compressionLevel > 9 )
		gArkCompressorOpt.compressionLevel = -1;

	gArkCompressorOpt.splitSize					= 0;					
	gArkCompressorOpt.forceZip64				= FALSE;	
	gArkCompressorOpt.useDosTime2PasswordCheck	= FALSE;	
	gArkCompressorOpt.sfxPathName				= NULL;				
	gArkCompressorOpt.forceUtf8FileName			= TRUE;			
	gArkCompressorOpt.forceUtf8Comment			= FALSE;			
	gArkCompressorOpt.utf8FileNameIfNeeded		= TRUE;		
	gArkCompressorOpt.bypassWhenUncompressible	= TRUE;	
	gArkCompressorOpt.lzmaEncodeThreadCount		= 2;		
	gArkCompressorOpt.enableMultithreadDeflate	= TRUE;	
	gArkCompressorOpt.deflateEncodeThreadCount	= 0;	

	gArkCompressorOpt._7zCompressHeader			= FALSE;			
	gArkCompressorOpt._7zEncryptHeader			= FALSE;			
	gArkCompressorOpt.lzma2NumBlockThreads		= 2;		
	gArkCompressorOpt.threadPriority			= THREAD_PRIORITY_NORMAL;				
}

//////////////////////////////////////////////////////////////////////////

BOOL32 CArkInfo::Open( LPCWSTR pwszFilePath, LPCWSTR password )
{
	BOOL32 bSuccess = FALSE;

	do 
	{
		ARKERR err = arkLib.Create( gArkDLLFullPathName );
		
		if( err != ARKERR_NOERR )
		{
			LM_TRACE( "ArkLib Create Failed = %d, %s", err, gArkDLLFullPathName );
			break;
		}

		SArkGlobalOpt opt;
		opt.bAzoSupport					= TRUE;
		opt.bPrintAssert				= TRUE;
		opt.bTreatTBZAsSolidArchive		= FALSE;
		opt.bTreatTGZAsSolidArchive		= FALSE;
		opt.bUseLongPathName			= TRUE;
		
		arkLib.SetGlobalOpt( opt );

		bSuccess = arkLib.Open( pwszFilePath, password );
		
		LM_TRACE( L"File Open Error Code = %d", arkLib.GetLastError() );

	} while (false);

	return bSuccess;
}
