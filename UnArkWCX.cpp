// UnArkWCX.cpp : DLL ���� ���α׷��� ���� ������ �Լ��� �����մϴ�.
//

#include "stdafx.h"
#include "UnArkWCX.h"

#include <time.h>
#include <Shlwapi.h>

#pragma comment( lib, "shlwapi" )

#pragma warning( disable: 4996 )

EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchive( tOpenArchiveData *pArchiveData )
{
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
	LM_TRACE( L"���� ���� ���� �õ�" );

	CArkInfo* pArkInfo = NULL;

	do
	{
		pArchiveDataW->OpenResult = E_NOT_SUPPORTED;

		if( ( pArchiveDataW->OpenMode != PK_OM_LIST ) && 
			( pArchiveDataW->OpenMode != PK_OM_EXTRACT ) )
			break;

		LM_TRACE( L"���� ���� �̸� : %s", pArchiveDataW->ArcName );

		pArkInfo = new CArkInfo;
		if( pArkInfo == NULL )
		{
			pArchiveDataW->OpenResult = E_NO_MEMORY;
			break;
		}
		
		if( pArkInfo->Open( pArchiveDataW->ArcName, NULL ) == FALSE )
			break;

		LM_TRACE( L"���� ��� : %d", pArchiveDataW->OpenResult );

		pArchiveDataW->OpenResult = 0;

	} while (false);

	return (HANDLE)pArkInfo;
}

EXTERN_C UNARKWCX_API int WINAPI ReadHeader( HANDLE hArcData, tHeaderData *pHeaderData )
{
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
	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );

	int nResult = 0;

	do
	{
		if( pArkInfo == NULL )
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

		wcsncpy( pHeaderDataExW->ArcName, pArkInfo->GetArk()->GetFilePathName(), 1023 );
		wcsncpy( pHeaderDataExW->FileName, pItem->fileNameW, 1023 );

		struct tm* tmCurrentTime = localtime( &pItem->fileTime );
		pHeaderDataExW->FileAttr		= pItem->attrib;
		pHeaderDataExW->FileTime		= 0;
		pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_year + 1900 - 1980) << 25;
		pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_mon + 1) << 21;
		pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_mday << 16 );
		pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_hour << 11 );
		pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_min << 5 );
		pHeaderDataExW->FileTime		|= (tmCurrentTime->tm_sec/2);

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
					pwszDestPath == NULL �̸� pwszDestName �� ��θ�� ���ϸ��� ��� ����
					pwszDestPath != NULL �̸� pwszDestPath ���� ��ΰ� pwszDestName ���� ���ϸ��� ����
				*/

// 				/*
// 					Ark Library �� ExtractOneAs �޼ҵ忡 ������ ����. 
// 					��ȸå���� pwszDestName �� �Ѿ�� ��θ�� ���ϸ��� �и��س� �� ��θ� �ѱ�
// 					������ pwszDestName ���� ���������� Ǯ ����ο� �������� ���ο����� ��ε��� ��� ���ԵǾ�����. 
// 
// 					��Ż Ŀ�Ǵ����� ������ Ǯ�鼭 �����̸��� �����ϰų� �ϴ� ���� ���ٰ� �Ǵ��Ͽ� ���� ���� �ۼ���
// 				*/

				int ret = 0;

				pArkInfo->GetArk()->SetEvent( &(pArkInfo->GetArkEvent()) );
				
				if( ( pwszDestPath == NULL ) && 
					( pwszDestName != NULL ) )
				{

// 					LM_TRACE(( L"���� ��ü ��� : %s", pwszDestName ));
					const SArkFileItem* pItem = pArkInfo->GetArk()->GetFileItem( pArkInfo->GetCurrentFileIndex() );
// 					LM_TRACE(( L"�ش� ������ �����̸� : %s", pItem->fileNameW ));
// 
					std::wstring wstrDestName( pwszDestName );
					
					std::wstring::size_type nPos = wstrDestName.rfind( pItem->fileNameW );

					if( nPos != std::wstring::npos )
					{
// 						LM_TRACE(( L"��� ��� : %s", wstrDestName.substr( 0, nPos ).c_str() ));

						ret = pArkInfo->GetArk()->ExtractOneTo( pArkInfo->GetCurrentFileIndex(), wstrDestName.substr( 0, nPos ).c_str() );
					}
					else
					{
// 						LM_ERROR(( L"���ΰ� ����ΰ� ��ġ�� ����" ));

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
// 					LM_TRACE(( L"���� Ǯ�� �õ� ��� : %s", pwszDestPath ));
// 					LM_TRACE(( L"���� Ǯ�� �õ� �̸� : %s", pwszDestName ));

					ret = pArkInfo->GetArk()->ExtractOneTo( pArkInfo->GetCurrentFileIndex(), pwszDestPath );
				}

				pArkInfo->GetArk()->SetEvent( NULL );

// 				LM_TRACE(( L"���� Ǯ�� ��� : %d, %s", ret, GetArkLastErrorText( pArkInfo->getArk()->GetLastError() ).c_str() ));

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
	if( hArcData != NULL )
		delete (CArkInfo *)(hArcData);

	return 0;
}
EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProc( HANDLE hArcData, tChangeVolProc pfnChangeVolProc )
{
	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo != NULL )
		pArkInfo->GetArkEvent().pfnChangeVolProc = pfnChangeVolProc;
}

EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc )
{
	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo != NULL )
		pArkInfo->GetArkEvent().pfnChangeVolProcW = pfnChangeVolProc;
}

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProc( HANDLE hArcData, tProcessDataProc pfnProcessDataProc )
{

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo != NULL )
		pArkInfo->GetArkEvent().pfnProcessDataProc = pfnProcessDataProc;
}

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc )
{
	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo != NULL )
		pArkInfo->GetArkEvent().pfnProcessDataProcW = pfnProcessDataProc;
}

EXTERN_C UNARKWCX_API int WINAPI GetPackerCaps()
{
	return PK_CAPS_BY_CONTENT;
}

EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFile( char *FileName )
{
	return CanYouHandleThisFileW( const_cast< WCHAR * >( CA2U( FileName ).c_str() ) );
}

EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFileW( WCHAR *FileName )
{
	BOOL isCanHandle = FALSE;

	do
	{
		CArkLib arkLib;
		if( arkLib.Create( ARK_DLL_RELEASE_FILE_NAME ) != ARKERR_NOERR )
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

BOOL32 CArkInfo::Open( LPCWSTR pwszFilePath, LPCWSTR password )
{
	BOOL32 bSuccess = FALSE;

	do 
	{
		WCHAR wszBuffer[ MAX_PATH ] = {0,};

		GetModuleFileName( g_hInst, wszBuffer, MAX_PATH );
		PathRemoveFileSpec( wszBuffer );
		PathAppend( wszBuffer, ARK_DLL_RELEASE_FILE_NAME );

		ARKERR err = arkLib.Create( wszBuffer );
		
		if( err != ARKERR_NOERR )
		{
			LM_TRACE( "ArkLib Create Failed = %d, %s", err, wszBuffer );
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
