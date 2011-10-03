// UnArkWCX.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "UnArkWCX.h"

#pragma warning( disable: 4996 )

CArkLib g_arkLib;

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
	LM_TRACE(( L"압축 파일 열기 시도" ));
	HANDLE nResult = 0;

	do
	{
		pArchiveDataW->OpenResult = E_NOT_SUPPORTED;

		if( ( pArchiveDataW->OpenMode != PK_OM_LIST ) && 
			( pArchiveDataW->OpenMode != PK_OM_EXTRACT ) )
		{
			pArchiveDataW->OpenResult = E_NOT_SUPPORTED;
			break;
		}

		IArk* pArk = g_arkLib.GetIArk();
		if( pArk == NULL )
		{
			pArchiveDataW->OpenResult = E_NOT_SUPPORTED;
			break;
		}

		SArkGlobalOpt opt;
		opt.bAzoSupport = TRUE;
		opt.bPrintAssert = TRUE;
		opt.bTreatTBZAsSolidArchive = FALSE;
		opt.bTreatTGZAsSolidArchive = FALSE;

		pArk->SetGlobalOpt( opt );

		LM_TRACE(( L"압축 파일 이름 : %s", pArchiveDataW->ArcName ));

		// 압축 파일 열기의 목적 구분
		switch( pArchiveDataW->OpenMode )
		{
		case PK_OM_LIST:
			LM_TRACE(( L"압축 파일 열기 모드 : 목록 나열" ));
			break;
		case PK_OM_EXTRACT:
			LM_TRACE(( L"압축 파일 열기 모드 : 압축 해제" ));
			break;
		}

		LM_TRACE(( L"파일 열기 시도" ));

		if( pArk->Open( pArchiveDataW->ArcName, NULL ) == FALSE )
		{
			pArchiveDataW->OpenResult = E_NOT_SUPPORTED;
			break;
		}

		CArkInfo* pArkInfo = new CArkInfo;
		pArkInfo->setArk( pArk );

		pArchiveDataW->OpenResult = 0;
		nResult = (HANDLE)pArkInfo;
	} while (false);

	LM_TRACE(( L"열기 결과 : %d", pArchiveDataW->OpenResult ));


	return nResult;
}

EXTERN_C UNARKWCX_API int WINAPI ReadHeader( HANDLE hArcData, tHeaderData *pHeaderData )
{
	tHeaderDataExW headerDataW;

	int nResult = ReadHeaderExW( hArcData, &headerDataW );

	strncpy( pHeaderData->ArcName, CU2A( headerDataW.ArcName ).c_str(), 1024 );
	strncpy( pHeaderData->FileName, CU2A( headerDataW.FileName ).c_str(), 1024 );

	pHeaderData->FileAttr = headerDataW.FileAttr;
	pHeaderData->FileCRC = headerDataW.FileCRC;
	pHeaderData->FileTime = headerDataW.FileTime;
	pHeaderData->PackSize = headerDataW.PackSize;
	pHeaderData->UnpSize = headerDataW.UnpSize;

	return nResult;
}

EXTERN_C UNARKWCX_API int WINAPI ReadHeaderEx( HANDLE hArcData, tHeaderDataEx *pHeaderDataEx )
{
	tHeaderDataExW headerDataW;

	int nResult = ReadHeaderExW( hArcData, &headerDataW );

	strncpy( pHeaderDataEx->ArcName, CU2A( headerDataW.ArcName ).c_str(), 1024 );
	strncpy( pHeaderDataEx->FileName, CU2A( headerDataW.FileName ).c_str(), 1024 );

	pHeaderDataEx->FileAttr = headerDataW.FileAttr;
	pHeaderDataEx->FileCRC = headerDataW.FileCRC;
	pHeaderDataEx->FileTime = headerDataW.FileTime;
	pHeaderDataEx->PackSize = headerDataW.PackSize;
	pHeaderDataEx->PackSizeHigh = headerDataW.PackSizeHigh;

	pHeaderDataEx->UnpSize = headerDataW.UnpSize;
	pHeaderDataEx->UnpSizeHigh = headerDataW.UnpSizeHigh;

	return nResult;
}


EXTERN_C UNARKWCX_API int WINAPI ReadHeaderExW( HANDLE hArcData, tHeaderDataExW *pHeaderDataExW )
{
	LM_TRACE(( L"Called ReadHeaderExW" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );

	int nResult = 0;

	do
	{
		if( pArkInfo == NULL )
		{
			LM_ERROR(( L"유효하지 않은 핸들값 전달됨" ));
			nResult = E_NOT_SUPPORTED;
			break;
		}

		pArkInfo->incrementCurrentFileIndex();

		if( pArkInfo->getCurrentFileIndex() >= pArkInfo->getArk()->GetFileItemCount() )
		{
			LM_TRACE(( L"파일의 끝에 도달" ));
			nResult = E_END_ARCHIVE;
			break;
		}

		const SArkFileItem* pItem = pArkInfo->getArk()->GetFileItem( pArkInfo->getCurrentFileIndex() );
		if( pItem == NULL )
		{
			LM_TRACE(( L"파일의 끝에 도달 __" ));
			nResult = E_END_ARCHIVE;
			break;
		}

		wcsncpy( pHeaderDataExW->ArcName, pArkInfo->getArk()->GetFilePathName(), 1024 );
		wcsncpy( pHeaderDataExW->FileName, pItem->fileNameW, 1024 );

		pHeaderDataExW->FileAttr		= pItem->attrib;
		pHeaderDataExW->FileTime		= (int)pItem->fileTime;
		pHeaderDataExW->PackSize		= pItem->compressedSize & 0xffffffff;
		pHeaderDataExW->PackSizeHigh	= pItem->compressedSize >> 32;
		pHeaderDataExW->UnpSize			= pItem->uncompressedSize & 0xffffffff;
		pHeaderDataExW->UnpSizeHigh		= pItem->uncompressedSize >> 32;
		pHeaderDataExW->FileCRC			= pItem->crc32;
		pHeaderDataExW->HostOS			= 0;

		LM_TRACE(( L"현재 파일 색인번호 : %d", pArkInfo->getCurrentFileIndex() ));
		LM_TRACE(( L"현재 압축파일 이름 : %s", pHeaderDataExW->ArcName ));
		LM_TRACE(( L"색인번호의 파일이름 : %s", pHeaderDataExW->FileName ));


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
	LM_TRACE(( L"Called ProcessFileW" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	int nResult = 0;

	do
	{
		if( pArkInfo == NULL )
		{
			LM_ERROR(( L"유효하지 않은 핸들값 전달됨" ));
			nResult = E_NOT_SUPPORTED;
			break;
		}

		LM_TRACE(( L"현재 색인 번호 : %d", pArkInfo->getCurrentFileIndex() ));

		switch( Operation )
		{
		case PK_SKIP:
			LM_TRACE(( L"작동 모드 : PK_SKIP" ));
			break;
		case PK_TEST:
			LM_TRACE(( L"작동 모드 : PK_TEST" ));
			break;
		case PK_EXTRACT:
			{
				LM_TRACE(( L"작동 모드 : PK_EXTRACT" ));
				/*
					pwszDestPath == NULL 이면 pwszDestName 에 경로명과 파일명이 모두 있음
					pwszDestPath != NULL 이면 pwszDestPath 에는 경로가 pwszDestName 에는 파일명이 있음
				*/

				/*
					Ark Library 의 ExtractOneAs 메소드에 오류가 있음. 
					우회책으로 pwszDestName 에 넘어온 경로명과 파일명을 분리해낸 후 경로명만 넘김
					하지만 pwszDestName 에는 압축파일을 풀 대상경로와 압축파일 내부에서의 경로등이 모두 포함되어있음. 

					토탈 커맨더에서 압축을 풀면서 파일이름을 변경하거나 하는 일은 없다고 판단하여 위와 같이 작성함
				*/

				int ret = 0;

				pArkInfo->getArk()->SetEvent( &pArkInfo->eEvent );

				if( ( pwszDestPath == NULL ) && 
					( pwszDestName != NULL ) )
				{

					LM_TRACE(( L"최초 전체 경로 : %s", pwszDestName ));
					const SArkFileItem* pItem = pArkInfo->getArk()->GetFileItem( pArkInfo->getCurrentFileIndex() );
					LM_TRACE(( L"해당 색인의 파일이름 : %s", pItem->fileNameW ));

					std::wstring wstrDestName( pwszDestName );
					
					std::wstring::size_type nPos = wstrDestName.rfind( pItem->fileNameW );

					if( nPos != std::wstring::npos )
					{
						LM_TRACE(( L"대상 경로 : %s", wstrDestName.substr( 0, nPos ).c_str() ));

						ret = pArkInfo->getArk()->ExtractOneTo( pArkInfo->getCurrentFileIndex(), wstrDestName.substr( 0, nPos ).c_str() );
					}
					else
					{
						LM_ERROR(( L"색인과 대상경로가 겹치지 않음" ));

						PathRemoveFileSpec( pwszDestName );
						WCHAR* pwszBuffer = wcsdup( pItem->fileNameW );

						PathRemoveFileSpec( pwszBuffer );
						PathAddBackslash( pwszBuffer );
						PathAddBackslash( pwszDestName );

						LM_TRACE(( L"%s\n%s", pwszDestName, pItem->fileNameW ));

						pArkInfo->getArk()->ClearExtractList();
						pArkInfo->getArk()->AddIndex2ExtractList( pArkInfo->getCurrentFileIndex() );
						ret = pArkInfo->getArk()->ExtractMultiFileTo( pwszDestName, pwszBuffer );
						free( pwszBuffer );
					}
				}
				else if( ( pwszDestName != NULL ) && 
					( pwszDestPath != NULL ) )
				{
					LM_TRACE(( L"압축 풀기 시도 경로 : %s", pwszDestPath ));
					LM_TRACE(( L"압축 풀기 시도 이름 : %s", pwszDestName ));

					ret = pArkInfo->getArk()->ExtractOneTo( pArkInfo->getCurrentFileIndex(), pwszDestPath );
				}

				pArkInfo->getArk()->SetEvent( NULL );

				LM_TRACE(( L"압축 풀기 결과 : %d, %s", ret, GetArkLastErrorText( pArkInfo->getArk()->GetLastError() ).c_str() ));

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
	LM_TRACE(( L"[CloseArchive]" ));

	if( hArcData != NULL )
		delete (CArkInfo *)(hArcData);

	return 0;
}

EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProc( HANDLE hArcData, tChangeVolProc pfnChangeVolProc )
{
	LM_TRACE(( L"[SetChangeVolProc]" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo == NULL )
	{
		LM_ERROR(( L"[SetChangeVolProc] 유효하지 않은 핸들값 전달됨" ));
	}
	
	pArkInfo->eEvent.pfnChangeVolProc = pfnChangeVolProc;
	
}

EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc )
{
	LM_TRACE(( L"[SetChangeVolProcW]" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo == NULL )
	{
		LM_ERROR(( L"[SetChangeVolProcW] 유효하지 않은 핸들값 전달됨" ));
	}

	pArkInfo->eEvent.pfnChangeVolProcW = pfnChangeVolProc;
}

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProc( HANDLE hArcData, tProcessDataProc pfnProcessDataProc )
{
	LM_TRACE(( L"[SetProcessDataProc]" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo == NULL )
	{
		LM_ERROR(( L"[SetProcessDataProc] 유효하지 않은 핸들값 전달됨" ));
	}
	
	pArkInfo->eEvent.pfnProcessDataProc = pfnProcessDataProc;
}


EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc )
{
	LM_TRACE(( L"[SetProcessDataProcW]" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo == NULL )
	{
		LM_ERROR(( L"[SetProcessDataProcW] 유효하지 않은 핸들값 전달됨" ));
	}
	
	pArkInfo->eEvent.pfnProcessDataProcW = pfnProcessDataProc;
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
		LM_TRACE(( L"[CanYouHandleThisFileW]" ));

		IArk* pArk = g_arkLib.GetIArk();
		if( pArk == NULL )
			break;

		LM_TRACE(( L"파일 이름 : %s 확인 시도", FileName ));

		ARK_FF fileFormat = pArk->CheckFormat( FileName );
		LM_TRACE(( L"파일의 형식 = %d,%x", fileFormat, fileFormat ));

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

///
CArkEvent::CArkEvent()
: pfnChangeVolProc( NULL ), 
pfnChangeVolProcW( NULL ),
pfnProcessDataProc( NULL ),
pfnProcessDataProcW( NULL )
{

}

///
CArkInfo::CArkInfo()
: currentFileIndex( -1 ), pArk( NULL )
{

}

CArkInfo::~CArkInfo()
{

}

///

std::wstring GetArkLastErrorText( ARKERR lastError )
{
	switch( lastError )
	{
	case ARKERR_NOERR:
		return L"오류 없음";
	case 	ARKERR_CANT_OPEN_FILE:
		return L" 파일 열기 실패";
	case 	ARKERR_CANT_READ_SIG								:
		return L" signature 읽기 실패";
	case 	ARKERR_AT_READ_CONTAINER_HEADER						:
		return L" 컨테이너 헤더가 손상되었음";
	case 	ARKERR_INVALID_FILENAME_LENGTH						:
		return L" 파일명 길이에 문제";
	case 	ARKERR_READ_FILE_NAME_FAILED						:
		return L" 파일이름 읽기 실패";
	case 	ARKERR_INVALID_EXTRAFIELD_LENGTH					:
		return L" extra field 읽기";
	case 	ARKERR_READ_EXTRAFILED_FAILED						:
		return L" extra field 읽기 실패";
	case 	ARKERR_CANT_READ_CENTRAL_DIRECTORY_STRUCTURE		:
		return L" (zip) Central Directory 정보를 읽는데 실패하였음";
	case 	ARKERR_INVALID_FILENAME_SIZE						:
		return L" 파일명 길이 정보가 잘못되었음";
	case 	ARKERR_INVALID_EXTRAFIELD_SIZE						:
		return L" (zip) ExtraField 정보 길이가 잘못되었음";
	case 	ARKERR_INVALID_FILECOMMENT_SIZE						:
		return L" Comment 정보 길이가 잘못되었음";
	case 	ARKERR_CANT_READ_CONTAINER_HEADER					:
		return L" 컨테이너의 헤더에 문제가 있음";
	case 	ARKERR_MEM_ALLOC_FAILED								:
		return L" 메모리 할당 실패";
	case 	ARKERR_CANT_READ_DATA								:
		return L" 압축 데이타 읽기 실패";
	case 	ARKERR_INFLATE_FAILED								:
		return L" Inflate 함수 호출중 에러 발생";
	case 	ARKERR_USER_ABORTED									:
		return L" 사용자 중지";
	case 	ARKERR_INVALID_FILE_CRC								:
		return L" 압축 해제후 CRC 에러 발생";
	case 	ARKERR_UNKNOWN_COMPRESSION_METHOD					:
		return L" 모르는(혹은 지원하지 않는) 압축방식";
	case 	ARKERR_PASSWD_NOT_SET								:
		return L" 암호걸린 파일인데 암호가 지정되지 않았음";
	case 	ARKERR_INVALID_PASSWD								:
		return L" 암호가 틀렸음";
	case 	ARKERR_WRITE_FAIL									:
		return L" 파일 쓰다가 실패";
	case 	ARKERR_CANT_OPEN_DEST_FILE							:
		return L" 대상 파일을 만들 수 없음";
	case 	ARKERR_BZIP2_ERROR									:
		return L" BZIP2 압축해제중 에러 발생";
	case 	ARKERR_INVALID_DEST_PATH							:
		return L" 경로명에 ../ 이 포함된 경우, 대상 경로에 접근이 불가능한 경우";
	case 	ARKERR_CANT_CREATE_FOLDER							:
		return L" 경로 생성 실패";
	case 	ARKERR_DATA_CORRUPTED								:
		return L" 압축푸는데 데이타가 손상됨 + RAR 분할압축 파일의 뒷부분이 없음";
	case 	ARKERR_CANT_OPEN_FILE_TO_WRITE						:
		return L" 쓰기용으로 파일 열기 실패";
	case 	ARKERR_INVALID_INDEX								:
		return L" 압축풀 대상의 index 파라메터가 잘못됨";
	case 	ARKERR_CANT_READ_CODEC_HEADER						:
		return L" 압축 코덱의 헤더를 읽는데 에러";
	case 	ARKERR_CANT_INITIALIZE_CODEC						:
		return L" 코덱 초기화 실패";
	case 	ARKERR_LZMA_ERROR									:
		return L" LZMA 압축 해제중 에러 발생";
	case 	ARKERR_PPMD_ERROR									:
		return L" ppmd 에러";
	case 	ARKERR_CANT_SET_OUT_FILE_SIZE						:
		return L" 출력파일의 SetSize() 실패";
	case 	ARKERR_NOT_MATCH_FILE_SIZE							:
		return L" 압축을 푼 파일 크기가 맞지 않음";
	case 	ARKERR_NOT_A_FIRST_VOLUME_FILE						:
		return L" 분할 압축 파일중 첫번째 파일이 아님";
	case 	ARKERR_NOT_OPENED									:
		return L" 파일이 열려있지 않음";
	case 	ARKERR_NOT_SUPPORTED_ENCRYPTION_METHOD				:
		return L" 지원하지 않는 암호 방식";
	case 	ARKERR_INTERNAL										:
		return L" 내부 에러";
	case 	ARKERR_NOT_SUPPORTED_FILEFORMAT						:
		return L" 지원하지 않는 파일 포맷";
	case 	ARKERR_UNKNOWN_FILEFORMAT							:
		return L" 압축파일이 아님";
	case 	ARKERR_FILENAME_EXCED_RANGE							:
		return L" 경로명이 너무 길어서 파일이나 폴더를 만들 수 없음";
	case 	ARKERR_LZ_ERROR										:
		return L" lz 에러";
	case 	ARKERR_NOTIMPL										:
		return L" not implemented";
	case 	ARKERR_DISK_FULL									:
		return L" 파일 쓰다가 실패";
	case 	ARKERR_FILE_TRUNCATED								:
		return L" 파일의 뒷부분이 잘렸음";
	case 	ARKERR_CANT_DO_THAT_WHILE_WORKING					:
		return L" 압축 해제 작업중에는 파일을 열거나 닫을 수 없음";
	case 	ARKERR_CANNOT_FIND_NEXT_VOLUME						:
		return L" 분할 압축된 파일의 다음 파일을 찾을 수 없음";
	case 	ARKERR_NOT_ARCHIVE_FILE								:
		return L" 압축파일이 아님 (Open() 호출시 명백히 압축파일이 아닌 경우 발생)";
	case 	ARKERR_USER_SKIP									:
		return L" 사용자가 건너띄기 했음.";
	case 	ARKERR_INVALID_PASSWD_OR_BROKEN_ARCHIVE				:
		return L" 암호가 틀리거나 파일이 손상되었음 (rar 포맷)";
	case 	ARKERR_ZIP_LAST_VOL_ONLY							:
		return L" 분할 zip 인데 마지막 zip 파일만 열려고 했음";
	case 	ARKERR_ACCESS_DENIED_TO_DEST_PATH					:
		return L" 대상 폴더에 대해서 쓰기 권한이 없음";

	case 	ARKERR_CORRUPTED_FILE								:
		return L" 파일이 손상되었음";
	case 	ARKERR_INVALID_FILE									:
		return L" 포맷이 다르다";
	case 	ARKERR_CANT_READ_FILE								:
		return L" 파일을 읽을 수 없음";

	case 	ARKERR_INVALID_VERSION								:
		return L" 헤더파일과 dll 의 버전이 맞지 않음";
	case 	ARKERR_ENCRYPTED_BOND_FILE							:
		return L" 압축 해제 불가(암호화된 bond 파일임)";

	case 	ARKERR_7ZERR_BROKEN_ARCHIVE							:
		return L" 7z.dll 으로 열때 에러가 발생(깨진파일)";
	case 	ARKERR_LOAD_7Z_DLL_FAILED							:
		return L" 7z.dll 열다가 에러 발생";

	case 	ARKERR_CANT_CREATE_FILE								:
		return L" 파일을 쓰기용으로 생성하지 못함";
	case 	ARKERR_INIT_NOT_CALLED								:
		return L" Init() 함수가 호출되지 않았음";
	case 	ARKERR_INVALID_PARAM								:
		return L" 잘못된 파라메터로 호출하였음";
	case 	ARKERR_CANT_OPEN_INPUT_SFX							:
		return L" SFX 파일을 열지 못함";
	case 	ARKERR_SFX_SIZE_OVER_4GB							:
		return L" SFX 파일의 크기가 4GB를 넘었음";

	case 	ARKERR_ALREADY_DLL_CREATED							:
		return L" (CArkLib) 이미 ARK DLL 파일을 로드하였음";
	case 	ARKERR_LOADLIBRARY_FAILED							:
		return L" (CArkLib) LoadLibrary() 호출 실패";
	case 	ARKERR_GETPROCADDRESS_FAILED						:
		return L" (CArkLib) GetProcAddress() 호출 실패";
	case 	ARKERR_UNSUPPORTED_OS								:
		return L" (CArkLib) 지원하지 않는 os";
	case 	ARKERR_LIBRARY_NOT_LOADED							:
		return L" (CArkLib) 라이브러리를 로드하지 않았거나 로드하는데 실패하였음";
	}

	return L"알 수 없는 오류";
}
