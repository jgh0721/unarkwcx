#include "stdafx.h"

#include "tst_unArkWCXEnv.h"

#include "ArkLib.h"
#include "../uniqueLibs/typeDefs.h"

using namespace nsCommon;
using namespace nsCommon::nsCmnPath;
using namespace nsCommon::nsCmnDateTime;
using namespace nsCommon::nsCmnLogger;
using namespace nsCommon::nsCmnConvert;
using namespace nsCommon::nsCmnFormatter;

#pragma warning( disable: 4800 )
#pragma warning( disable: 4996 )

#pragma execution_character_set( "utf-8" )

//////////////////////////////////////////////////////////////////////////

TST_GlobalEnvironment::~TST_GlobalEnvironment()
{

}

void TST_GlobalEnvironment::SetUp()
{
    std::wstring currentPath = GetCurrentPath();
    
    ASSERT_TRUE( PathFileExistsW( tsFormat( L"%1\\Ark32.dll", currentPath ).c_str() ) != FALSE );
    ASSERT_TRUE( PathFileExistsW( tsFormat( L"%1\\Ark32lgpl.dll", currentPath ).c_str() ) != FALSE );

#ifdef _WIN64
    ASSERT_TRUE( PathFileExistsW( tsFormat( L"%1\\Ark64.dll", currentPath ).c_str() ) != FALSE );
    ASSERT_TRUE( PathFileExistsW( tsFormat( L"%1\\Ark64lgpl.dll", currentPath ).c_str() ) != FALSE );
#endif

    ASSERT_TRUE( PathFileExistsW( tsFormat( L"%1\\unace32.exe", currentPath ).c_str() ) != FALSE );
    ASSERT_TRUE( PathFileExistsW( tsFormat( L"%1\\unacev2.dll", currentPath ).c_str() ) != FALSE );

}

void TST_GlobalEnvironment::TearDown()
{

}

//////////////////////////////////////////////////////////////////////////

CUnArkMgr::CUnArkMgr() : 
    fnPackSetDefaultParams(NULLPTR),
    fnOpenArchiveW( NULLPTR ),
    fnReadHeaderExW( NULLPTR ),
    fnSetChangeVolProcW( NULLPTR ),
    fnSetProcessDataProcW( NULLPTR ),
    fnCloseArchive( NULLPTR ),
    fnCanYouHandleThisFileW( NULLPTR ),
    fnConfigurePacker( NULLPTR ),
    fnPackFilesW( NULLPTR ),
    fnProcessFileW( NULLPTR )
{

}

CUnArkMgr::~CUnArkMgr()
{

}

bool CUnArkMgr::Init()
{
    hUnArkWCX = NULL;

    fnPackSetDefaultParams = NULLPTR;
    fnOpenArchiveW = NULLPTR;
    fnReadHeaderExW = NULLPTR;
    fnSetChangeVolProcW = NULLPTR;
    fnSetProcessDataProcW = NULLPTR;
    fnCloseArchive = NULLPTR;
    fnCanYouHandleThisFileW = NULLPTR;
    fnConfigurePacker = NULLPTR;
    fnPackFilesW = NULLPTR;
    fnProcessFileW = NULLPTR;

#ifndef _WIN64
    std::wstring wcxFilePath = tsFormat( L"%1\\%2", GetCurrentPath(), "UnArkWCX.wcx" );
#else
    std::wstring wcxFilePath = tsFormat( L"%1\\%2", GetCurrentPath(), "UnArkWCX.wcx64" );
#endif

    hUnArkWCX = LoadLibraryW( wcxFilePath.c_str() );
    return hUnArkWCX != NULL;
}

void CUnArkMgr::Uninit()
{
    fnPackSetDefaultParams = NULLPTR;
    fnOpenArchiveW = NULLPTR;
    fnReadHeaderExW = NULLPTR;
    fnSetChangeVolProcW = NULLPTR;
    fnSetProcessDataProcW = NULLPTR;
    fnCloseArchive = NULLPTR;
    fnCanYouHandleThisFileW = NULLPTR;
    fnConfigurePacker = NULLPTR;
    fnPackFilesW = NULLPTR;
    fnProcessFileW = NULLPTR;

    if( hUnArkWCX != NULL )
        FreeLibrary( hUnArkWCX );
    hUnArkWCX = NULL;
}

bool CUnArkMgr::SetCompressionFileFormats( ARK_FF fileFormat )
{
    return false;
}

bool CUnArkMgr::CallPackSetDefaultParams()
{
    bool isSuccess = false;

    do 
    {
        if( fnPackSetDefaultParams == NULLPTR )
            fnPackSetDefaultParams = (pfnPackSetDefaultParams)GetProcAddress( hUnArkWCX, "PackSetDefaultParams" );

        if( fnPackSetDefaultParams == NULLPTR )
            break;

        std::string defaultINIPath = tsFormat( "%1\\%2", GetCurrentPath(), "pkplugin.ini" );

        PackDefaultParamStruct dps;
        memset( &dps, '\0', sizeof( dps ) );
        
        dps.size = sizeof( dps );
        dps.PluginInterfaceVersionHi = 2;
        dps.PluginInterfaceVersionLow = 20;
        strcpy_s( dps.DefaultIniName, defaultINIPath.c_str() );

        fnPackSetDefaultParams( &dps );

        isSuccess = true;

    } while( false );

    return isSuccess;
}

bool CUnArkMgr::CallPackFiles( const std::wstring& packedFile, const std::wstring& subPath, const std::wstring& srcPath, const std::vector< std::wstring >& addList, int Flags )
{
    bool isSuccess = false;

    wchar_t* wszPackedFile = NULLPTR;
    wchar_t* wszSubPath = NULLPTR;
    wchar_t* wszSrcPath = NULLPTR;
    wchar_t* wszAddList = NULLPTR;

    do 
    {
        if( packedFile.empty() == true ||
            addList.empty() == true )
            break;

        wszPackedFile = _wcsdup( packedFile.c_str() );

        if( subPath.empty() == false )
            wszSubPath = _wcsdup( subPath.c_str() );

        if( srcPath.empty() == false )
            wszSrcPath = _wcsdup( srcPath.c_str() );

        wszAddList = convertVectorToAddList( addList );

        if( fnPackFilesW == NULLPTR )
            fnPackFilesW = (pfnPackFilesW)GetProcAddress( hUnArkWCX, "PackFilesW" );

        if( fnPackFilesW == NULLPTR )
            break;

        isSuccess = fnPackFilesW( wszPackedFile, wszSubPath, wszSrcPath, wszAddList, Flags ) == 0 ? true : false;
        
    } while (false);

    if( wszPackedFile != NULLPTR )
        free( wszPackedFile );

    if( wszSubPath != NULLPTR )
        free( wszSubPath );

    if( wszSrcPath != NULLPTR )
        free( wszSrcPath );

    if( wszAddList != NULLPTR )
        free( wszAddList );

    return isSuccess;
}

bool CUnArkMgr::ListFilesInArchive( const std::wstring& packedFile, std::vector< tHeaderDataExW >& vecHeaderItem )
{
    bool isSuccess = false;
    tOpenArchiveDataW archiveData;

    do 
    {
        if( fnOpenArchiveW == NULLPTR )
            fnOpenArchiveW = (pfnOpenArchiveW)GetProcAddress( hUnArkWCX, "OpenArchiveW" );

        if( fnOpenArchiveW == NULLPTR )
            break;

        if( fnReadHeaderExW == NULLPTR )
            fnReadHeaderExW = (pfnReadHeaderExW)GetProcAddress( hUnArkWCX, "ReadHeaderExW" );

        if( fnReadHeaderExW == NULLPTR )
            break;

        if( fnCloseArchive == NULLPTR )
            fnCloseArchive = (pfnCloseArchive)GetProcAddress( hUnArkWCX, "CloseArchive" );

        if( fnCloseArchive == NULLPTR )
            break;

        archiveData.ArcName = wcsdup(packedFile.c_str() );
        archiveData.CmtBuf = NULL;
        archiveData.CmtBufSize = 0;
        archiveData.CmtSize = 0;
        archiveData.CmtState = 0;
        archiveData.OpenMode = PK_OM_LIST;
        archiveData.OpenResult = 0;

        HANDLE hArchive = fnOpenArchiveW( &archiveData );
        if( hArchive == NULL )
            break;

        tHeaderDataExW headerDataExW;
        int nRet = 0;
        while( (( nRet = fnReadHeaderExW( hArchive, &headerDataExW ) ) == 0) && (nRet != E_END_ARCHIVE) )
        {
            vecHeaderItem.push_back( headerDataExW );
            memset( &headerDataExW, '\0', sizeof( tHeaderDataExW ) );
        }

        fnCloseArchive( hArchive );
        hArchive = NULL;

        isSuccess = true;
    } while (false);

    if( archiveData.ArcName != NULLPTR )
        free( archiveData.ArcName );

    return isSuccess;
}

void CUnArkMgr::PrintHeaderItem( tHeaderDataExW& headerDataEx )
{
    std::wcout << headerDataEx.FileName << std::endl;
}

//////////////////////////////////////////////////////////////////////////

wchar_t* CUnArkMgr::convertVectorToAddList( const std::vector< std::wstring >& vecAddList )
{
    wchar_t* pwszBuffer = NULL;
    wchar_t* pwszAddList = NULL;

    do
    {
        size_t nAddListLength = 0;
        for( size_t idx = 0; idx < vecAddList.size(); ++idx )
        {
            nAddListLength += vecAddList[ idx ].size();
            nAddListLength += 1;	// NULL 문자
        }
        nAddListLength += 1;		// 문자열 목록 종료를 알리기 위한 널 문자 추가

        pwszAddList = (wchar_t*)malloc( sizeof( wchar_t ) * nAddListLength );
        if( pwszAddList == NULL )
            break;

        ZeroMemory( pwszAddList, sizeof( wchar_t ) * nAddListLength );
        pwszBuffer = pwszAddList;
        for( size_t idx = 0; idx < vecAddList.size(); ++idx )
        {
            wcscpy( pwszBuffer, vecAddList[ idx ].c_str() );
            pwszBuffer = pwszBuffer + wcslen( pwszBuffer ) + 1;
        }

    } while( false );

    return pwszAddList;
}

//////////////////////////////////////////////////////////////////////////

void TST_Extract::SetUp()
{
    ASSERT_TRUE( Init() == true );
}

void TST_Extract::TearDown()
{
    Uninit();
}

//////////////////////////////////////////////////////////////////////////

void TST_Pack::SetUp()
{
    ASSERT_TRUE( Init() == true );
}

void TST_Pack::TearDown()
{
    Uninit();
}

void TST_Pack::SetPackerToZIP()
{
    DeleteFileW( tsFormat( L"%1\\%2", GetCurrentPath(), CONFIG_FILENAME ).c_str() );
    DeleteFileW( tsFormat( L"%1\\%2.sample", GetCurrentPath(), CONFIG_FILENAME ).c_str() );

    FILE* fd = _wfopen( tsFormat( L"%1\\%2", GetCurrentPath(), CONFIG_FILENAME ).c_str(), L"wt,ccs=UTF-8" );
    if( fd == NULL )
        return;

    fputws( L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n", fd );

    fputws( L"<UnArkWCX>\r\n", fd );
    fputws( L"<compressionFormat>ZIP</compressionFormat>\r\n", fd );
    fputws( L"<compressionLevel>-1</compressionLevel>\r\n", fd );
    fputws( L"<compressionMethod>DEFLATE</compressionMethod>\r\n", fd );
    fputws( L"</UnArkWCX>", fd );

    fclose( fd );
    fd = NULL;
}

void TST_Pack::SetPackerToISO()
{
    DeleteFileW( tsFormat( L"%1\\%2", GetCurrentPath(), CONFIG_FILENAME ).c_str() );
    DeleteFileW( tsFormat( L"%1\\%2.sample", GetCurrentPath(), CONFIG_FILENAME ).c_str() );

    FILE* fd = _wfopen( tsFormat( L"%1\\%2", GetCurrentPath(), CONFIG_FILENAME ).c_str(), L"wt,ccs=UTF-8" );
    if( fd == NULL )
        return;

    fputws( L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n", fd );

    fputws( L"<UnArkWCX>\r\n", fd );
    fputws( L"<compressionFormat>ISO</compressionFormat>\r\n", fd );
    fputws( L"<compressionLevel>-1</compressionLevel>\r\n", fd );
    fputws( L"<compressionMethod>STORE</compressionMethod>\r\n", fd );
    fputws( L"</UnArkWCX>", fd );

    fclose( fd );
    fd = NULL;
}

void TST_Pack::SetPackerToLZH()
{

}

void TST_Pack::SetPackerTo7Z()
{

}
