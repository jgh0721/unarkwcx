#include "stdafx.h"

#include <string>
#include <map>

#include "UnArkWCX.h"

#include "commonXMLMapperV2.h"

#include "../uniqueLibs/typeDefs.h"

using namespace nsCommon;
using namespace nsCommon::nsCmnPath;
using namespace nsCommon::nsCmnDateTime;
using namespace nsCommon::nsCmnLogger;
using namespace nsCommon::nsCmnConvert;
using namespace nsCommon::nsCmnFormatter;

#pragma warning( disable: 4996 )
#pragma execution_character_set( "utf-8" )

CArkEvent                           gArkEvent;
SArkCompressorOpt                   gArkCompressorOpt;
WCHAR                               gConfigureINIFullPath[ MAX_PATH ] = { 0, };

//////////////////////////////////////////////////////////////////////////
CArkWCX::CArkWCX()
    : _currentFileIndex( -1 ), _arkCompressor( NULLPTR )
{

}

CArkWCX::~CArkWCX()
{
    if( _arkCompressor != NULLPTR )
        _arkCompressor->Release();

    if( _arkLib.IsCreated() != FALSE )
    {
        _arkLib.Close();
        _arkLib.Destroy();
    }
}

bool CArkWCX::Init( bool isCreateCompressor /* = false */ )
{
    _currentFileIndex = -1;

    ARKERR err = _arkLib.Create( gArkDLLFullPathName );

    if( err == ARKERR_NOERR )
    {
        if( isCreateCompressor == true )
        {
            _arkCompressor = _arkLib.CreateCompressor();
            if( _arkCompressor == NULLPTR )
                err = _arkLib.GetLastError();
        }
    }

    return err == ARKERR_NOERR;
}

IArk* CArkWCX::GetArk()
{
    return _arkLib.GetIArk();
}

IArkCompressor* CArkWCX::GetArkCompressor()
{
    return _arkCompressor;
}

void CArkWCX::SetArkEvent()
{
    IArk* pArk = _arkLib.GetIArk();
    if( pArk != NULLPTR )
        pArk->SetEvent( &gArkEvent );

    if( _arkCompressor != NULLPTR )
        _arkCompressor->SetEvent( &gArkEvent );
}

void CArkWCX::ResetArkEvent()
{
    IArk* pArk = _arkLib.GetIArk();
    if( pArk != NULLPTR )
        pArk->SetEvent( NULL );

    if( _arkCompressor != NULLPTR )
        _arkCompressor->SetEvent( NULL );
}

BOOL32 CArkWCX::OpenW( LPCWSTR pwszFilePath, LPCWSTR password )
{
    BOOL32 isSuccess = FALSE;

    do
    {
        SArkGlobalOpt opt;
        opt.bAzoSupport = TRUE;
        opt.bPrintAssert = TRUE;
        opt.bTreatTBZAsSolidArchive = FALSE;
        opt.bTreatTGZAsSolidArchive = TRUE;
        opt.bUseLongPathName = TRUE;

        _arkLib.SetGlobalOpt( opt );

        isSuccess = _arkLib.Open( pwszFilePath, password );

    } while( false );

    return isSuccess;
}

int CArkWCX::IncrementCurrentFileIndex()
{
    return ++_currentFileIndex;
}

int CArkWCX::GetCurrentFileIndex()
{
    return _currentFileIndex;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// 플러그인 설치
//////////////////////////////////////////////////////////////////////////

EXTERN_C UNARKWCX_API int WINAPI GetPackerCaps()
{
    int retVal = PK_CAPS_NEW |
        PK_CAPS_MODIFY |
        PK_CAPS_MULTIPLE |
        PK_CAPS_DELETE |
        PK_CAPS_OPTIONS |
        PK_CAPS_BY_CONTENT;

    return retVal;
}

EXTERN_C UNARKWCX_API int WINAPI GetBackgroundFlags( void )
{
    return BACKGROUND_PACK | BACKGROUND_UNPACK;
}

//////////////////////////////////////////////////////////////////////////
/// 플러그인 로딩
//////////////////////////////////////////////////////////////////////////

EXTERN_C UNARKWCX_API void WINAPI PackSetDefaultParams( PackDefaultParamStruct* dps )
{
    if( dps != NULLPTR )
        wcsncpy_s( gConfigureINIFullPath, MAX_PATH - 1, CA2U( dps->DefaultIniName ).c_str(), _TRUNCATE );

    CXMLMapperV2& xmlMapper = GetXMLMapper();
    gArkCompressorOpt.Init();

    bool isSuccessLoad = false;

    if( (isSuccessLoad = xmlMapper.LoadFile( CU2U8( GetCurrentPath( g_hInst ) + L"\\" + CONFIGURE_XML_FILENAME + L".sample" ) )) == false )
        isSuccessLoad = xmlMapper.LoadFile( CU2U8( GetCurrentPath( g_hInst ) + L"\\" + CONFIGURE_XML_FILENAME ) );

    if( isSuccessLoad == true )
    {
        std::string compressionFormat = xmlMapper.GetDataFromItem( OPT_COMPRESSION_FORMAT );
        for( size_t idx = 0; idx < VecCompressionFormat.size(); ++idx )
        {
            if( u8sicmp( VecCompressionFormat[ idx ].second, compressionFormat ) == 0 )
                gArkCompressorOpt.ff = VecCompressionFormat[ idx ].first;
        }

        std::string compressionMethod = xmlMapper.GetDataFromItem( OPT_COMPRESSION_METHOD );
        if( u8sicmp(compressionMethod, "STORE") == 0 )
            gArkCompressorOpt.compressionMethod = ARK_COMPRESSION_METHOD_STORE;
        else if( u8sicmp( compressionMethod, "DEFLATE" ) == 0 )
            gArkCompressorOpt.compressionMethod = ARK_COMPRESSION_METHOD_DEFLATE;
        else if( u8sicmp( compressionMethod, "LZMA" ) == 0 )
            gArkCompressorOpt.compressionMethod = ARK_COMPRESSION_METHOD_LZMA;
        else if( u8sicmp( compressionMethod, "LZMA2" ) == 0 )
            gArkCompressorOpt.compressionMethod = ARK_COMPRESSION_METHOD_LZMA2;
        else if( u8sicmp( compressionMethod, "LH6" ) == 0 )
            gArkCompressorOpt.compressionMethod = ARK_COMPRESSION_METHOD_LH6;
        else if( u8sicmp( compressionMethod, "LH7" ) == 0 )
            gArkCompressorOpt.compressionMethod = ARK_COMPRESSION_METHOD_LH7;

        gArkCompressorOpt.compressionLevel = xmlMapper.GetDataFromItem( OPT_COMPRESSION_LEVEL );

        gArkCompressorOpt.encryptionMethod = ARK_ENCRYPTION_METHOD_NONE;
        std::string encryptionMethod = xmlMapper.GetDataFromItem( OPT_ENCRYPTION_METHOD );
        if( u8sicmp( encryptionMethod, "ZIP" ) == 0 )
            gArkCompressorOpt.encryptionMethod = ARK_ENCRYPTION_METHOD_ZIP;
        else if( u8sicmp(encryptionMethod, "AES256") == 0 )
            gArkCompressorOpt.encryptionMethod = ARK_ENCRYPTION_METHOD_AES256;

        bool isUseSplitArchive = xmlMapper.GetDataFromItem( OPT_SPLIT_ARCHIVE );
        if( isUseSplitArchive == true )
        {
            int splitSizeManual = xmlMapper.GetDataFromItem( OPT_SPLIT_ARCHIVE_MANUAL );
            if( splitSizeManual > 0 )
            {
                gArkCompressorOpt.splitSize = static_cast<__int64>(splitSizeManual) * 1024 * 1024;
            }
            else
            {
                eSplitArchiveSize splitArchiveEnum = static_cast< eSplitArchiveSize >( xmlMapper.GetDataFromItem( OPT_SPLIT_ARCHIVE_ENUM ) );
                switch( splitArchiveEnum )
                {
                    case SPLIT_ARCHIVE_NONE:
                        gArkCompressorOpt.splitSize = 0;
                        break;
                    case SPLIT_ARCHIVE_1MB:
                        gArkCompressorOpt.splitSize = 1024 * 1024;
                        break;
                    case SPLIT_ARCHIVE_10MB:
                        gArkCompressorOpt.splitSize = 1024 * 1024 * 10;
                        break;
                    case SPLIT_ARCHIVE_100MB:
                        gArkCompressorOpt.splitSize = 1024 * 1024 * 100;
                        break;
                    case SPLIT_ARCHIVE_1GB:
                        gArkCompressorOpt.splitSize = 1024 * 1024 * 1024;
                        break;
                    case SPLIT_ARCHIVE_2GB:
                        gArkCompressorOpt.splitSize = 1024LL * 1024LL * 1024LL * 2LL;
                        break;
                    case SPLIT_ARCHIVE_4GB:
                        gArkCompressorOpt.splitSize = 1024LL * 1024LL * 1024LL * 4LL;
                        break;
                }
            }
        }
        else
        {
            gArkCompressorOpt.splitSize = 0;
        }
        // TODO: 
//        xmlMapper.GetDataFromItem( OPT_MULTI_THREAD_SUPPORT );
//        xmlMapper.GetDataFromItem( OPT_EXTRACT_CODEPAGE );
    }
}

//////////////////////////////////////////////////////////////////////////
/// 공통
//////////////////////////////////////////////////////////////////////////

EXTERN_C UNARKWCX_API HANDLE __stdcall OpenArchive( tOpenArchiveData *ArchiveData )
{
    HANDLE hResult = 0;

    do
    {
        tOpenArchiveDataW ArchiveDataW;

        ArchiveDataW.ArcName = _wcsdup( CA2U( ArchiveData->ArcName ).c_str() );
        ArchiveDataW.CmtBuf = NULL;
        ArchiveDataW.CmtBufSize = 0;
        ArchiveDataW.CmtSize = 0;
        ArchiveDataW.CmtState = 0;
        ArchiveDataW.OpenMode = ArchiveData->OpenMode;

        hResult = OpenArchiveW( &ArchiveDataW );

        ArchiveData->OpenResult = ArchiveDataW.OpenResult;

        free( ArchiveDataW.ArcName );

    } while( false );

    return hResult;
}

EXTERN_C UNARKWCX_API HANDLE __stdcall OpenArchiveW( tOpenArchiveDataW *ArchiveDataW )
{
    CArkWCX* pArkInfo = 0;

    do
    {
        ArchiveDataW->OpenResult = E_NOT_SUPPORTED;

        if( (ArchiveDataW->OpenMode != PK_OM_LIST) &&
            (ArchiveDataW->OpenMode != PK_OM_EXTRACT) )
            break;

        pArkInfo = new CArkWCX;
        if( pArkInfo == NULL )
        {
            ArchiveDataW->OpenResult = E_NO_MEMORY;
            break;
        }

        if( pArkInfo->Init() == false )
        {
            delete pArkInfo;
            pArkInfo = NULL;
            ArchiveDataW->OpenResult = E_EOPEN;
            break;
        }

        if( pArkInfo->OpenW( ArchiveDataW->ArcName, NULL ) == FALSE )
        {
            delete pArkInfo;
            pArkInfo = NULL;
            ArchiveDataW->OpenResult = E_NO_FILES;

            break;
        }

        ArchiveDataW->OpenResult = 0;

    } while( false );

    return (HANDLE)pArkInfo;
}

EXTERN_C UNARKWCX_API int __stdcall ReadHeader( HANDLE hArcData, tHeaderData *HeaderData )
{
    int nResult = 0;
    tHeaderDataExW headerDataW;

    do
    {
        if( hArcData == NULLPTR || HeaderData == NULLPTR )
            break;

        memset( HeaderData, '\0', sizeof( tHeaderData ) );
        memset( &headerDataW, '\0', sizeof( tHeaderDataExW ) );

        nResult = ReadHeaderExW( hArcData, &headerDataW );
        if( nResult != 0 )
            break;

        strncpy_s( HeaderData->ArcName, WCX_ANSI_MAX_PATH - 1, CU2A( headerDataW.ArcName ).c_str(), _TRUNCATE );
        strncpy_s( HeaderData->FileName, WCX_ANSI_MAX_PATH - 1, CU2A( headerDataW.FileName ).c_str(), _TRUNCATE );

        HeaderData->FileAttr = headerDataW.FileAttr;
        HeaderData->FileCRC = headerDataW.FileCRC;
        HeaderData->FileTime = headerDataW.FileTime;
        HeaderData->PackSize = headerDataW.PackSize;
        HeaderData->UnpSize = headerDataW.UnpSize;

    } while( false );

    return nResult;
}

EXTERN_C UNARKWCX_API int __stdcall ReadHeaderEx( HANDLE hArcData, tHeaderDataEx *HeaderDataEx )
{
    int nResult = 0;
    tHeaderDataExW headerDataW;

    do
    {
        if( hArcData == NULLPTR || HeaderDataEx == NULLPTR )
            break;

        memset( HeaderDataEx, '\0', sizeof( tHeaderDataEx ) );
        memset( &headerDataW, '\0', sizeof( tHeaderDataExW ) );

        nResult = ReadHeaderExW( hArcData, &headerDataW );
        if( nResult != 0 )
            break;

        strncpy_s( HeaderDataEx->ArcName, WCX_WIDE_MAX_PATH - 1, CU2A( headerDataW.ArcName ).c_str(), WCX_WIDE_MAX_PATH - 1 );
        strncpy_s( HeaderDataEx->FileName, WCX_WIDE_MAX_PATH - 1, CU2A( headerDataW.FileName ).c_str(), WCX_WIDE_MAX_PATH - 1 );

        HeaderDataEx->FileAttr = headerDataW.FileAttr;
        HeaderDataEx->FileCRC = headerDataW.FileCRC;
        HeaderDataEx->FileTime = headerDataW.FileTime;
        HeaderDataEx->PackSize = headerDataW.PackSize;
        HeaderDataEx->UnpSize = headerDataW.UnpSize;

    } while( false );

    return nResult;
}

EXTERN_C UNARKWCX_API int __stdcall ReadHeaderExW( HANDLE hArcData, tHeaderDataExW *HeaderDataExW )
{
    int nResult = E_EOPEN;

    CArkWCX* pArkWCX = NULLPTR;

    do
    {
        if( hArcData == NULLPTR || HeaderDataExW == NULLPTR )
            break;

        memset( HeaderDataExW, '\0', sizeof( tHeaderDataExW ) );
        pArkWCX = reinterpret_cast<CArkWCX*>(hArcData);

        int currentFileIndex = pArkWCX->IncrementCurrentFileIndex();

        if( currentFileIndex >= pArkWCX->GetArk()->GetFileItemCount() )
        {
            nResult = E_END_ARCHIVE;
            break;
        }

        const SArkFileItem* pItem = pArkWCX->GetArk()->GetFileItem( currentFileIndex );
        if( pItem == NULL )
        {
            nResult = E_EOPEN;
            break;
        }

        wcsncpy_s( HeaderDataExW->ArcName, WCX_WIDE_MAX_PATH - 1, pArkWCX->GetArk()->GetFilePathName(), _TRUNCATE );
        wcsncpy_s( HeaderDataExW->FileName, WCX_WIDE_MAX_PATH - 1, pItem->fileNameW, _TRUNCATE );

        if( pItem->attrib & ARK_FILEATTR_DIRECTORY )
            HeaderDataExW->FileAttr |= 0x10;
        if( pItem->attrib & ARK_FILEATTR_FILE )
            HeaderDataExW->FileAttr |= 0x20;
        if( pItem->attrib & ARK_FILEATTR_HIDDEN )
            HeaderDataExW->FileAttr |= 0x2;
        if( pItem->attrib & ARK_FILEATTR_READONLY )
            HeaderDataExW->FileAttr |= 0x1;
        if( (pItem->attrib & ARK_FILEATTR_FILE) == 0 &&
            (pItem->attrib & ARK_FILEATTR_DIRECTORY) == 0 )
            HeaderDataExW->FileAttr = 0x20;

        struct tm* tmCurrentTime = _localtime64( &pItem->fileTime );
        if( tmCurrentTime != NULL )
        {
            HeaderDataExW->FileTime = 0;
            HeaderDataExW->FileTime |= (tmCurrentTime->tm_year + 1900 - 1980) << 25;
            HeaderDataExW->FileTime |= (tmCurrentTime->tm_mon + 1) << 21;
            HeaderDataExW->FileTime |= (tmCurrentTime->tm_mday << 16);
            HeaderDataExW->FileTime |= (tmCurrentTime->tm_hour << 11);
            HeaderDataExW->FileTime |= (tmCurrentTime->tm_min << 5);
            HeaderDataExW->FileTime |= (tmCurrentTime->tm_sec / 2);
        }

        HeaderDataExW->PackSize = pItem->compressedSize & 0xffffffff;
        HeaderDataExW->PackSizeHigh = pItem->compressedSize >> 32;
        if( pItem->uncompressedSize != ARK_FILESIZE_UNKNOWN )
        {
            HeaderDataExW->UnpSize = pItem->uncompressedSize & 0xffffffff;
            HeaderDataExW->UnpSizeHigh = pItem->uncompressedSize >> 32;
        }
        HeaderDataExW->FileCRC = pItem->crc32;
        HeaderDataExW->HostOS = 0;

        nResult = 0;

    } while( false );

    return nResult;
}


EXTERN_C UNARKWCX_API void __stdcall SetChangeVolProc( HANDLE hArcData, tChangeVolProc pChangeVolProc )
{
    gArkEvent.pfnChangeVolProc = pChangeVolProc;
}

EXTERN_C UNARKWCX_API void __stdcall SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProcW )
{
    gArkEvent.pfnChangeVolProcW = pfnChangeVolProcW;
}

EXTERN_C UNARKWCX_API void __stdcall SetProcessDataProc( HANDLE hArcData, tProcessDataProc pProcessDataProc )
{
    gArkEvent.pfnProcessDataProc = pProcessDataProc;
}

EXTERN_C UNARKWCX_API void __stdcall SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProcW )
{
    gArkEvent.pfnProcessDataProcW = pfnProcessDataProcW;
}

EXTERN_C UNARKWCX_API int __stdcall CloseArchive( HANDLE hArcData )
{
    if( hArcData == NULLPTR )
        return E_ECLOSE;

    delete reinterpret_cast<CArkWCX*>(hArcData);

    return 0;
}

EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFile( char *FileName )
{
    if( FileName == NULLPTR )
        return 0;

    return CanYouHandleThisFileW( const_cast<WCHAR *>(CA2U( FileName ).c_str()) );
}

EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFileW( WCHAR *FileName )
{
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

        if( (fileFormat >= ARK_FF_UNSUPPORTED_FIRST) &&
            (fileFormat <= ARK_FF_UNSUPPORTED_LAST) )
            break;

        if( (fileFormat >= ARK_FF_NOTARCHIVE_FIRST) &&
            (fileFormat <= ARK_FF_NOTARCHIVE_LAST) )
            break;

        isCanHandle = TRUE;
    } while( false );

    return isCanHandle;
}

EXTERN_C UNARKWCX_API void WINAPI ConfigurePacker( HWND Parent, HINSTANCE DllInstance )
{
    std::wstring configurePath = tsFormat( L"%1\\%2", GetCurrentPath( g_hInst ), CONFIG_EXENAME );
    if( PathFileExistsW( configurePath.c_str() ) == FALSE )
        return;

    boost::thread t( [=]()
    {
        SHELLEXECUTEINFOW ShExecInfo;

        ShExecInfo.cbSize = sizeof( SHELLEXECUTEINFOW );
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
        ShExecInfo.hwnd = NULL;
        ShExecInfo.lpVerb = NULL;
        ShExecInfo.lpFile = configurePath.c_str();
        ShExecInfo.lpParameters = NULL;
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_NORMAL;
        ShExecInfo.hInstApp = NULL;

        if( ShellExecuteExW( &ShExecInfo ) != FALSE )
        {
            if( ShExecInfo.hProcess != NULL )
            {
                WaitForSingleObject( ShExecInfo.hProcess, INFINITE );
                CloseHandle( ShExecInfo.hProcess );
            }
        }
    } );
}

//////////////////////////////////////////////////////////////////////////
/// 압축
//////////////////////////////////////////////////////////////////////////

EXTERN_C UNARKWCX_API int __stdcall PackFiles( char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags )
{
    int nResult = 0;
    std::vector< wchar_t > vecAddListW;
    for( char* pszCurrent = AddList; *pszCurrent != NULL; pszCurrent += strlen( pszCurrent ) + 1 )
    {
        CA2U wszAddFile( pszCurrent );
        const wchar_t* pwszAddFile = wszAddFile.c_str();
        while( pwszAddFile++ != NULL )
            vecAddListW.push_back( *pwszAddFile );
    }

    vecAddListW.push_back( L'\0' );

    return PackFilesW( const_cast<wchar_t*>(CA2U( PackedFile ).c_str()),
                       SubPath != NULLPTR ? const_cast<wchar_t*>(CA2U( SubPath ).c_str()) : NULLPTR,
                       const_cast<wchar_t*>(CA2U( SrcPath ).c_str()),
                       &vecAddListW[ 0 ],
                       Flags );
}

EXTERN_C UNARKWCX_API int __stdcall PackFilesW( wchar_t* PackedFile, wchar_t* SubPath, wchar_t* SrcPath, wchar_t* AddList, int Flags )
{
    int nRetValue = E_NOT_SUPPORTED;		// 0 == SUCCESS
    std::vector< std::wstring > vecAddFile, vecSrcFile;

    for( wchar_t* pwszCurrent = AddList; *pwszCurrent != NULL; pwszCurrent += wcslen( pwszCurrent ) + 1 )
    {
        vecAddFile.push_back( pwszCurrent );
    }

    PackSetDefaultParams( NULL );

    CArkWCX arkWCX;
    IArkCompressor* pCompressor = NULL;
    std::wstring strSrcPath, strDstPath;

    do
    {
        if( Flags == PK_PACK_ENCRYPT )
        {
            nRetValue = E_NOT_SUPPORTED;
            break;
        }

        if( arkWCX.Init( true ) == false )
        {
            nRetValue = E_NOT_SUPPORTED;
            break;
        }
        
        arkWCX.SetArkEvent();
        pCompressor = arkWCX.GetArkCompressor();
        if( pCompressor == NULLPTR )
        {
            nRetValue = E_NOT_SUPPORTED;
            break;
        }

        pCompressor->Init();
        if( pCompressor->SetOption( gArkCompressorOpt, NULL, 0 ) == FALSE )
        {
            nRetValue = E_NOT_SUPPORTED;
            break;
        }

        BOOL32 bSuccess = FALSE;
        for( size_t idx = 0; idx < vecAddFile.size(); ++idx )
        {
            strSrcPath = SrcPath;
            if( *strSrcPath.rbegin() != L'\\' )
                strSrcPath += L"\\" + vecAddFile[ idx ];
            else
                strSrcPath += vecAddFile[ idx ];

            if( SubPath != NULL && SubPath[ wcslen( SubPath ) - 1 ] != L'\\' )
                strDstPath = SubPath + vecAddFile[ idx ];
            else if( SubPath != NULL && SubPath[ wcslen( SubPath ) - 1 ] == L'\\' )
                strDstPath = SubPath + std::wstring( L"\\" ) + vecAddFile[ idx ];
            else
                strDstPath = vecAddFile[ idx ];

            vecSrcFile.push_back( strSrcPath );
            if( (bSuccess = pCompressor->AddFileItem( strSrcPath.c_str(), strDstPath.c_str(), FALSE )) == FALSE )
            {
                nRetValue = E_NO_FILES;
                break;
            }
        }

        if( bSuccess == FALSE )
            break;

        std::wstring packedFilePath( PackedFile );
        std::wstring packerExtension;
        for( size_t idx = 0; idx < VecCompressionFormat.size(); ++idx )
        {
            if( gArkCompressorOpt.ff == VecCompressionFormat[ idx ].first )
            {
                packerExtension = CA2U( VecCompressionFormat[ idx ].second );
                break;
            }
        }   

        wchar_t* extension = PathFindExtensionW( PackedFile );
        if( wcsicmp( extension, packerExtension.c_str() ) != 0 )
        {
            if( endWith( packedFilePath, PACKER_EXTENSION ) == true )
            {
                packedFilePath = string_replace_all( packedFilePath, L"." + PACKER_EXTENSION, L"." + packerExtension );
            }
            else
            {
                packedFilePath += L"." + packerExtension;
            }
        }

        if( pCompressor->CreateArchive( packedFilePath.c_str() ) == FALSE )
        {
            nRetValue = E_ECREATE;
            break;
        }
        else
        {
            if( Flags == PK_PACK_MOVE_FILES )
            {
                for( size_t idx = 0; idx < vecSrcFile.size(); ++idx )
                    DeleteFileW( vecSrcFile[ idx ].c_str() );
            }
        }

        nRetValue = 0;

    } while( false );

    return nRetValue;
}

//////////////////////////////////////////////////////////////////////////
/// 해제 / 리스트
//////////////////////////////////////////////////////////////////////////

EXTERN_C UNARKWCX_API int __stdcall ProcessFile( HANDLE hArcData, int Operation, char *DestPath, char *DestName )
{
    int nResult = E_EOPEN;

    do
    {
        if( hArcData == NULLPTR )
            break;

        if( DestPath == NULLPTR && DestName == NULLPTR )
        {
            nResult = E_NO_FILES;
            break;
        }

        WCHAR* wszDestPath = DestPath == NULL ? NULL : wcsdup( CA2U( DestPath ).c_str() );
        WCHAR* wszDestName = DestName == NULL ? NULL : wcsdup( CA2U( DestName ).c_str() );

        nResult = ProcessFileW(
            hArcData,
            Operation,
            wszDestPath,
            wszDestName );

        if( wszDestPath != NULLPTR )
            free( wszDestPath );
        if( wszDestName != NULLPTR )
            free( wszDestName );

    } while( false );

    return nResult;

}

EXTERN_C UNARKWCX_API int __stdcall ProcessFileW( HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName )
{
    int nResult = E_EOPEN;
    CArkWCX* pArkWCX = NULLPTR;

    do
    {
        if( hArcData == NULLPTR )
            break;

        pArkWCX = reinterpret_cast<CArkWCX*>(hArcData);

        switch( Operation )
        {
            case PK_SKIP:
                nResult = 0;
                break;
            case PK_TEST:
                nResult = 0;
                break;
            case PK_EXTRACT:
            {
                /*
                    pwszDestPath == NULL 이면 pwszDestName 에 경로명과 파일명이 모두 있음
                    pwszDestPath != NULL 이면 pwszDestPath 에는 경로가 pwszDestName 에는 파일명이 있음
                */
                int ret = FALSE;

                pArkWCX->SetArkEvent();

                if( (pwszDestPath == NULL) &&
                    (pwszDestName != NULL) )
                {
                    const SArkFileItem* pItem = pArkWCX->GetArk()->GetFileItem( pArkWCX->GetCurrentFileIndex() );
                    if( pItem == NULLPTR )
                        break;

                    std::wstring wstrDestName( pwszDestName );

                    std::wstring::size_type nPos = wstrDestName.rfind( pItem->fileNameW );

                    if( nPos != std::wstring::npos )
                    {
                        ret = pArkWCX->GetArk()->ExtractOneTo( pArkWCX->GetCurrentFileIndex(), wstrDestName.substr( 0, nPos ).c_str() );
                    }
                    else
                    {
                        PathRemoveFileSpec( pwszDestName );
                        WCHAR* pwszBuffer = wcsdup( pItem->fileNameW );

                        PathRemoveFileSpec( pwszBuffer );
                        PathAddBackslash( pwszBuffer );
                        PathAddBackslash( pwszDestName );

                        pArkWCX->GetArk()->ClearExtractList();
                        pArkWCX->GetArk()->AddIndex2ExtractList( pArkWCX->GetCurrentFileIndex() );
                        ret = pArkWCX->GetArk()->ExtractMultiFileTo( pwszDestName, pwszBuffer );
                        free( pwszBuffer );
                    }
                }
                else if( (pwszDestName != NULL) &&
                         (pwszDestPath != NULL) )
                {
                    ret = pArkWCX->GetArk()->ExtractOneTo( pArkWCX->GetCurrentFileIndex(), pwszDestPath );
                }

                pArkWCX->ResetArkEvent();

                if( ret == FALSE )
                    nResult = E_BAD_ARCHIVE;
                else
                    nResult = 0;

            } // for PK_EXTRACT
                break;
        } // switch operation

    } while( false );

    return nResult;
}

EXTERN_C UNARKWCX_API int __stdcall DeleteFiles( char* PackedFile, char* DeleteList )
{
    wchar_t* wszPackedFile = NULLPTR;

    if( PackedFile == NULLPTR || DeleteList == NULLPTR )
        return E_NO_FILES;

    std::vector< wchar_t > vecDeleteListW;
    wszPackedFile = wcsdup( CA2U( PackedFile ) );

    for( char* pszCurrent = DeleteList; *pszCurrent != NULL; pszCurrent += strlen( pszCurrent ) + 1 )
    {
        CA2U wszDeleteFile( pszCurrent );
        const wchar_t* pwszDeleteFile = wszDeleteFile.c_str();
        while( pwszDeleteFile++ != NULL )
            vecDeleteListW.push_back( *pwszDeleteFile );
    }

    vecDeleteListW.push_back( L'\0' );

    int nRet = DeleteFilesW( wszPackedFile,
                             &vecDeleteListW[ 0 ] );

    if( wszPackedFile != NULLPTR )
        free( wszPackedFile );

    return nRet;
}

EXTERN_C UNARKWCX_API int __stdcall DeleteFilesW( WCHAR* PackedFile, WCHAR* DeleteList )
{
    int nRetValue = E_NOT_SUPPORTED;		// 0 == SUCCESS

    if( PackedFile == NULLPTR || DeleteList == NULLPTR )
        return E_NO_FILES;

    std::vector< std::wstring > vecDeleteFile;
    for( wchar_t* pwszCurrent = DeleteList; *pwszCurrent != NULL; pwszCurrent += wcslen( pwszCurrent ) + 1 )
        vecDeleteFile.push_back( pwszCurrent );

    CArkWCX arkWCX;
    IArk* pArk = NULL;
    IArkCompressor* pCompressor = NULL;

    do 
    {
        if( arkWCX.Init( true ) == false )
            break;

        arkWCX.SetArkEvent();
        pCompressor = arkWCX.GetArkCompressor();
        if( pCompressor == NULLPTR )
            break;

        if( arkWCX.OpenW( PackedFile, NULL ) == FALSE )
            break;

        pArk = arkWCX.GetArk();
        if( pArk == NULLPTR )
            break;

        if( pCompressor->SetArchiveFile( pArk ) == FALSE )
            break;

        if( vecDeleteFile.empty() == true )
        {
            nRetValue = E_NO_FILES;
            break;
        }

        std::map< std::wstring, int, nsCommon::lt_nocaseW > mapFileNameToIndex;

        for( int idx = 0; idx < pArk->GetFileItemCount(); ++idx )
        {
            auto sArkFileItem = pArk->GetFileItem( idx );
            if( sArkFileItem == NULLPTR )
                continue;

            mapFileNameToIndex[ sArkFileItem->fileNameW ] = idx;
        }

        for( size_t idx = 0; idx < vecDeleteFile.size(); ++idx )
        {
            if( mapFileNameToIndex.count(vecDeleteFile[idx]) <= 0 )
                continue;

            if( pCompressor->DeleteItem( mapFileNameToIndex[ vecDeleteFile[ idx ] ] ) == FALSE )
            {
                nRetValue = E_BAD_ARCHIVE;
                return nRetValue;
            }
        }

        nRetValue = pCompressor->CreateArchive( PackedFile ) != FALSE ? 0 : E_BAD_ARCHIVE;

    } while( false );

    return nRetValue;
}
