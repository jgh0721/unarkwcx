#include "stdafx.h"

#include "tst_unArkWCX.h"

using namespace nsCommon;
using namespace nsCommon::nsCmnPath;
using namespace nsCommon::nsCmnDateTime;
using namespace nsCommon::nsCmnLogger;
using namespace nsCommon::nsCmnConvert;
using namespace nsCommon::nsCmnFormatter;

#pragma warning( disable: 4996 )

#pragma execution_character_set( "utf-8" )

TEST_F( TST_Pack, PackSingleFileToZIP )
{
    const std::wstring testFilePath = tsFormat( L"%1\\test.jpg", GetCurrentPath() );

    ASSERT_TRUE( PathFileExistsW( testFilePath.c_str() ) != FALSE );
    SetPackerToZIP();
    ASSERT_TRUE( CallPackSetDefaultParams() == true );

    const ::testing::TestInfo* const test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();

    std::wstring packedFile = tsFormat( L"%1\\test_%2.zip", GetCurrentPath(), test_info->name() );
    std::vector< std::wstring > vecAddList;
    vecAddList.push_back( tsFormat( L"test.jpg" ) );

    ASSERT_TRUE( CallPackFiles( packedFile, L"", GetCurrentPath(), vecAddList, 0 ) == true );
    ASSERT_TRUE( PathFileExistsW( packedFile.c_str() ) != FALSE );

    std::vector< tHeaderDataExW > vecHeaderDataItem;
    ASSERT_TRUE( ListFilesInArchive( packedFile, vecHeaderDataItem ) == true );
    for( size_t idx = 0; idx < vecHeaderDataItem.size(); ++idx )
    {
        auto& headerDataEx = vecHeaderDataItem[ idx ];
        PrintHeaderItem( headerDataEx );
    }

    DeleteFileW( packedFile.c_str() );
    ASSERT_TRUE( PathFileExistsW( packedFile.c_str() ) == FALSE );
}

TEST_F( TST_Pack, PackSingleFileToISO )
{
    const std::wstring testFilePath = tsFormat( L"%1\\test.jpg", GetCurrentPath() );

    ASSERT_TRUE( PathFileExistsW( testFilePath.c_str() ) != FALSE );
    SetPackerToISO();
    ASSERT_TRUE( CallPackSetDefaultParams() == true );

    const ::testing::TestInfo* const test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();

    std::wstring packedFile = tsFormat( L"%1\\test_%2.iso", GetCurrentPath(), test_info->name() );
    std::vector< std::wstring > vecAddList;
    vecAddList.push_back( tsFormat( L"test.jpg" ) );

    ASSERT_TRUE( CallPackFiles( packedFile, L"", GetCurrentPath(), vecAddList, 0 ) == true );
    ASSERT_TRUE( PathFileExistsW( packedFile.c_str() ) != FALSE );
    DeleteFileW( packedFile.c_str() );
    ASSERT_TRUE( PathFileExistsW( packedFile.c_str() ) == FALSE );
}

TEST_F( TST_Pack, PackMultiFileToISO )
{

}

TEST_F( TST_Extract, Extract )
{

}

TEST( CreateInstaller, MakeInstaller )
{
    const std::wstring currentPath = GetCurrentPath();

    ASSERT_TRUE( PathFileExistsW( tsFormat( L"%1\\UnArkWCX.wcx", currentPath ).c_str() ) != FALSE );
    ASSERT_TRUE( PathFileExistsW( tsFormat( L"%1\\UnArkWCX.wcx64", currentPath ).c_str( ) ) != FALSE );

    DeleteFileW( tsFormat( L"%1\\pluginst.inf", currentPath ).c_str() );
    FILE* fd = fopen( tsFormat( "%1\\pluginst.inf", currentPath ).c_str(), "wt" );
    ASSERT_TRUE( fd != NULL );
    fputs( "[plugininstall]\r\n", fd );
    fputs( "description = \r\n", fd );
    fputs( "type = wcx\r\n", fd );
    fputs( "file = UnArkWCX.wcx\r\n", fd );
    fputs( "defaultdir = UnArkWCX\r\n", fd );
    fputs( "defaultextension = ALZ, EGG, ISO, 7Z, WIM, TAR, BH, UDF, CAB, XZ, Z, LZMA, ARJ, GZ, TGZ, BZ2, ARKWCX\r\n", fd );
    fclose( fd );
    fd = NULL;

}
