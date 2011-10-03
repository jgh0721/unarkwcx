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
	LM_TRACE(( L"���� ���� ���� �õ�" ));
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

		LM_TRACE(( L"���� ���� �̸� : %s", pArchiveDataW->ArcName ));

		// ���� ���� ������ ���� ����
		switch( pArchiveDataW->OpenMode )
		{
		case PK_OM_LIST:
			LM_TRACE(( L"���� ���� ���� ��� : ��� ����" ));
			break;
		case PK_OM_EXTRACT:
			LM_TRACE(( L"���� ���� ���� ��� : ���� ����" ));
			break;
		}

		LM_TRACE(( L"���� ���� �õ�" ));

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

	LM_TRACE(( L"���� ��� : %d", pArchiveDataW->OpenResult ));


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
			LM_ERROR(( L"��ȿ���� ���� �ڵ鰪 ���޵�" ));
			nResult = E_NOT_SUPPORTED;
			break;
		}

		pArkInfo->incrementCurrentFileIndex();

		if( pArkInfo->getCurrentFileIndex() >= pArkInfo->getArk()->GetFileItemCount() )
		{
			LM_TRACE(( L"������ ���� ����" ));
			nResult = E_END_ARCHIVE;
			break;
		}

		const SArkFileItem* pItem = pArkInfo->getArk()->GetFileItem( pArkInfo->getCurrentFileIndex() );
		if( pItem == NULL )
		{
			LM_TRACE(( L"������ ���� ���� __" ));
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

		LM_TRACE(( L"���� ���� ���ι�ȣ : %d", pArkInfo->getCurrentFileIndex() ));
		LM_TRACE(( L"���� �������� �̸� : %s", pHeaderDataExW->ArcName ));
		LM_TRACE(( L"���ι�ȣ�� �����̸� : %s", pHeaderDataExW->FileName ));


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
			LM_ERROR(( L"��ȿ���� ���� �ڵ鰪 ���޵�" ));
			nResult = E_NOT_SUPPORTED;
			break;
		}

		LM_TRACE(( L"���� ���� ��ȣ : %d", pArkInfo->getCurrentFileIndex() ));

		switch( Operation )
		{
		case PK_SKIP:
			LM_TRACE(( L"�۵� ��� : PK_SKIP" ));
			break;
		case PK_TEST:
			LM_TRACE(( L"�۵� ��� : PK_TEST" ));
			break;
		case PK_EXTRACT:
			{
				LM_TRACE(( L"�۵� ��� : PK_EXTRACT" ));
				/*
					pwszDestPath == NULL �̸� pwszDestName �� ��θ�� ���ϸ��� ��� ����
					pwszDestPath != NULL �̸� pwszDestPath ���� ��ΰ� pwszDestName ���� ���ϸ��� ����
				*/

				/*
					Ark Library �� ExtractOneAs �޼ҵ忡 ������ ����. 
					��ȸå���� pwszDestName �� �Ѿ�� ��θ�� ���ϸ��� �и��س� �� ��θ� �ѱ�
					������ pwszDestName ���� ���������� Ǯ ����ο� �������� ���ο����� ��ε��� ��� ���ԵǾ�����. 

					��Ż Ŀ�Ǵ����� ������ Ǯ�鼭 �����̸��� �����ϰų� �ϴ� ���� ���ٰ� �Ǵ��Ͽ� ���� ���� �ۼ���
				*/

				int ret = 0;

				pArkInfo->getArk()->SetEvent( &pArkInfo->eEvent );

				if( ( pwszDestPath == NULL ) && 
					( pwszDestName != NULL ) )
				{

					LM_TRACE(( L"���� ��ü ��� : %s", pwszDestName ));
					const SArkFileItem* pItem = pArkInfo->getArk()->GetFileItem( pArkInfo->getCurrentFileIndex() );
					LM_TRACE(( L"�ش� ������ �����̸� : %s", pItem->fileNameW ));

					std::wstring wstrDestName( pwszDestName );
					
					std::wstring::size_type nPos = wstrDestName.rfind( pItem->fileNameW );

					if( nPos != std::wstring::npos )
					{
						LM_TRACE(( L"��� ��� : %s", wstrDestName.substr( 0, nPos ).c_str() ));

						ret = pArkInfo->getArk()->ExtractOneTo( pArkInfo->getCurrentFileIndex(), wstrDestName.substr( 0, nPos ).c_str() );
					}
					else
					{
						LM_ERROR(( L"���ΰ� ����ΰ� ��ġ�� ����" ));

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
					LM_TRACE(( L"���� Ǯ�� �õ� ��� : %s", pwszDestPath ));
					LM_TRACE(( L"���� Ǯ�� �õ� �̸� : %s", pwszDestName ));

					ret = pArkInfo->getArk()->ExtractOneTo( pArkInfo->getCurrentFileIndex(), pwszDestPath );
				}

				pArkInfo->getArk()->SetEvent( NULL );

				LM_TRACE(( L"���� Ǯ�� ��� : %d, %s", ret, GetArkLastErrorText( pArkInfo->getArk()->GetLastError() ).c_str() ));

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
		LM_ERROR(( L"[SetChangeVolProc] ��ȿ���� ���� �ڵ鰪 ���޵�" ));
	}
	
	pArkInfo->eEvent.pfnChangeVolProc = pfnChangeVolProc;
	
}

EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc )
{
	LM_TRACE(( L"[SetChangeVolProcW]" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo == NULL )
	{
		LM_ERROR(( L"[SetChangeVolProcW] ��ȿ���� ���� �ڵ鰪 ���޵�" ));
	}

	pArkInfo->eEvent.pfnChangeVolProcW = pfnChangeVolProc;
}

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProc( HANDLE hArcData, tProcessDataProc pfnProcessDataProc )
{
	LM_TRACE(( L"[SetProcessDataProc]" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo == NULL )
	{
		LM_ERROR(( L"[SetProcessDataProc] ��ȿ���� ���� �ڵ鰪 ���޵�" ));
	}
	
	pArkInfo->eEvent.pfnProcessDataProc = pfnProcessDataProc;
}


EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc )
{
	LM_TRACE(( L"[SetProcessDataProcW]" ));

	CArkInfo* pArkInfo = reinterpret_cast< CArkInfo* >( hArcData );
	if( pArkInfo == NULL )
	{
		LM_ERROR(( L"[SetProcessDataProcW] ��ȿ���� ���� �ڵ鰪 ���޵�" ));
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

		LM_TRACE(( L"���� �̸� : %s Ȯ�� �õ�", FileName ));

		ARK_FF fileFormat = pArk->CheckFormat( FileName );
		LM_TRACE(( L"������ ���� = %d,%x", fileFormat, fileFormat ));

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
		return L"���� ����";
	case 	ARKERR_CANT_OPEN_FILE:
		return L" ���� ���� ����";
	case 	ARKERR_CANT_READ_SIG								:
		return L" signature �б� ����";
	case 	ARKERR_AT_READ_CONTAINER_HEADER						:
		return L" �����̳� ����� �ջ�Ǿ���";
	case 	ARKERR_INVALID_FILENAME_LENGTH						:
		return L" ���ϸ� ���̿� ����";
	case 	ARKERR_READ_FILE_NAME_FAILED						:
		return L" �����̸� �б� ����";
	case 	ARKERR_INVALID_EXTRAFIELD_LENGTH					:
		return L" extra field �б�";
	case 	ARKERR_READ_EXTRAFILED_FAILED						:
		return L" extra field �б� ����";
	case 	ARKERR_CANT_READ_CENTRAL_DIRECTORY_STRUCTURE		:
		return L" (zip) Central Directory ������ �дµ� �����Ͽ���";
	case 	ARKERR_INVALID_FILENAME_SIZE						:
		return L" ���ϸ� ���� ������ �߸��Ǿ���";
	case 	ARKERR_INVALID_EXTRAFIELD_SIZE						:
		return L" (zip) ExtraField ���� ���̰� �߸��Ǿ���";
	case 	ARKERR_INVALID_FILECOMMENT_SIZE						:
		return L" Comment ���� ���̰� �߸��Ǿ���";
	case 	ARKERR_CANT_READ_CONTAINER_HEADER					:
		return L" �����̳��� ����� ������ ����";
	case 	ARKERR_MEM_ALLOC_FAILED								:
		return L" �޸� �Ҵ� ����";
	case 	ARKERR_CANT_READ_DATA								:
		return L" ���� ����Ÿ �б� ����";
	case 	ARKERR_INFLATE_FAILED								:
		return L" Inflate �Լ� ȣ���� ���� �߻�";
	case 	ARKERR_USER_ABORTED									:
		return L" ����� ����";
	case 	ARKERR_INVALID_FILE_CRC								:
		return L" ���� ������ CRC ���� �߻�";
	case 	ARKERR_UNKNOWN_COMPRESSION_METHOD					:
		return L" �𸣴�(Ȥ�� �������� �ʴ�) ������";
	case 	ARKERR_PASSWD_NOT_SET								:
		return L" ��ȣ�ɸ� �����ε� ��ȣ�� �������� �ʾ���";
	case 	ARKERR_INVALID_PASSWD								:
		return L" ��ȣ�� Ʋ����";
	case 	ARKERR_WRITE_FAIL									:
		return L" ���� ���ٰ� ����";
	case 	ARKERR_CANT_OPEN_DEST_FILE							:
		return L" ��� ������ ���� �� ����";
	case 	ARKERR_BZIP2_ERROR									:
		return L" BZIP2 ���������� ���� �߻�";
	case 	ARKERR_INVALID_DEST_PATH							:
		return L" ��θ� ../ �� ���Ե� ���, ��� ��ο� ������ �Ұ����� ���";
	case 	ARKERR_CANT_CREATE_FOLDER							:
		return L" ��� ���� ����";
	case 	ARKERR_DATA_CORRUPTED								:
		return L" ����Ǫ�µ� ����Ÿ�� �ջ�� + RAR ���Ҿ��� ������ �޺κ��� ����";
	case 	ARKERR_CANT_OPEN_FILE_TO_WRITE						:
		return L" ��������� ���� ���� ����";
	case 	ARKERR_INVALID_INDEX								:
		return L" ����Ǯ ����� index �Ķ���Ͱ� �߸���";
	case 	ARKERR_CANT_READ_CODEC_HEADER						:
		return L" ���� �ڵ��� ����� �дµ� ����";
	case 	ARKERR_CANT_INITIALIZE_CODEC						:
		return L" �ڵ� �ʱ�ȭ ����";
	case 	ARKERR_LZMA_ERROR									:
		return L" LZMA ���� ������ ���� �߻�";
	case 	ARKERR_PPMD_ERROR									:
		return L" ppmd ����";
	case 	ARKERR_CANT_SET_OUT_FILE_SIZE						:
		return L" ��������� SetSize() ����";
	case 	ARKERR_NOT_MATCH_FILE_SIZE							:
		return L" ������ Ǭ ���� ũ�Ⱑ ���� ����";
	case 	ARKERR_NOT_A_FIRST_VOLUME_FILE						:
		return L" ���� ���� ������ ù��° ������ �ƴ�";
	case 	ARKERR_NOT_OPENED									:
		return L" ������ �������� ����";
	case 	ARKERR_NOT_SUPPORTED_ENCRYPTION_METHOD				:
		return L" �������� �ʴ� ��ȣ ���";
	case 	ARKERR_INTERNAL										:
		return L" ���� ����";
	case 	ARKERR_NOT_SUPPORTED_FILEFORMAT						:
		return L" �������� �ʴ� ���� ����";
	case 	ARKERR_UNKNOWN_FILEFORMAT							:
		return L" ���������� �ƴ�";
	case 	ARKERR_FILENAME_EXCED_RANGE							:
		return L" ��θ��� �ʹ� �� �����̳� ������ ���� �� ����";
	case 	ARKERR_LZ_ERROR										:
		return L" lz ����";
	case 	ARKERR_NOTIMPL										:
		return L" not implemented";
	case 	ARKERR_DISK_FULL									:
		return L" ���� ���ٰ� ����";
	case 	ARKERR_FILE_TRUNCATED								:
		return L" ������ �޺κ��� �߷���";
	case 	ARKERR_CANT_DO_THAT_WHILE_WORKING					:
		return L" ���� ���� �۾��߿��� ������ ���ų� ���� �� ����";
	case 	ARKERR_CANNOT_FIND_NEXT_VOLUME						:
		return L" ���� ����� ������ ���� ������ ã�� �� ����";
	case 	ARKERR_NOT_ARCHIVE_FILE								:
		return L" ���������� �ƴ� (Open() ȣ��� ����� ���������� �ƴ� ��� �߻�)";
	case 	ARKERR_USER_SKIP									:
		return L" ����ڰ� �ǳʶ�� ����.";
	case 	ARKERR_INVALID_PASSWD_OR_BROKEN_ARCHIVE				:
		return L" ��ȣ�� Ʋ���ų� ������ �ջ�Ǿ��� (rar ����)";
	case 	ARKERR_ZIP_LAST_VOL_ONLY							:
		return L" ���� zip �ε� ������ zip ���ϸ� ������ ����";
	case 	ARKERR_ACCESS_DENIED_TO_DEST_PATH					:
		return L" ��� ������ ���ؼ� ���� ������ ����";

	case 	ARKERR_CORRUPTED_FILE								:
		return L" ������ �ջ�Ǿ���";
	case 	ARKERR_INVALID_FILE									:
		return L" ������ �ٸ���";
	case 	ARKERR_CANT_READ_FILE								:
		return L" ������ ���� �� ����";

	case 	ARKERR_INVALID_VERSION								:
		return L" ������ϰ� dll �� ������ ���� ����";
	case 	ARKERR_ENCRYPTED_BOND_FILE							:
		return L" ���� ���� �Ұ�(��ȣȭ�� bond ������)";

	case 	ARKERR_7ZERR_BROKEN_ARCHIVE							:
		return L" 7z.dll ���� ���� ������ �߻�(��������)";
	case 	ARKERR_LOAD_7Z_DLL_FAILED							:
		return L" 7z.dll ���ٰ� ���� �߻�";

	case 	ARKERR_CANT_CREATE_FILE								:
		return L" ������ ��������� �������� ����";
	case 	ARKERR_INIT_NOT_CALLED								:
		return L" Init() �Լ��� ȣ����� �ʾ���";
	case 	ARKERR_INVALID_PARAM								:
		return L" �߸��� �Ķ���ͷ� ȣ���Ͽ���";
	case 	ARKERR_CANT_OPEN_INPUT_SFX							:
		return L" SFX ������ ���� ����";
	case 	ARKERR_SFX_SIZE_OVER_4GB							:
		return L" SFX ������ ũ�Ⱑ 4GB�� �Ѿ���";

	case 	ARKERR_ALREADY_DLL_CREATED							:
		return L" (CArkLib) �̹� ARK DLL ������ �ε��Ͽ���";
	case 	ARKERR_LOADLIBRARY_FAILED							:
		return L" (CArkLib) LoadLibrary() ȣ�� ����";
	case 	ARKERR_GETPROCADDRESS_FAILED						:
		return L" (CArkLib) GetProcAddress() ȣ�� ����";
	case 	ARKERR_UNSUPPORTED_OS								:
		return L" (CArkLib) �������� �ʴ� os";
	case 	ARKERR_LIBRARY_NOT_LOADED							:
		return L" (CArkLib) ���̺귯���� �ε����� �ʾҰų� �ε��ϴµ� �����Ͽ���";
	}

	return L"�� �� ���� ����";
}
