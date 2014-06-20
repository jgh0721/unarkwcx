#include "stdafx.h"

#include "tst_unArkWCX.h"

using namespace nsCommon;
using namespace nsCommon::nsCmnPath;
using namespace nsCommon::nsCmnDateTime;
using namespace nsCommon::nsCmnLogger;
using namespace nsCommon::nsCmnConvert;
using namespace nsCommon::nsCmnFormatter;

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

TEST_F( TST_Extract, Extract )
{

}
