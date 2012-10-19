#pragma once

#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <Windows.h>

#include <vector>
#include <string>

#include <Shlwapi.h>
#pragma comment( lib, "shlwapi" )

#include "../Library/gtest/gtest.h"
#include "../UnArkWCX_Export.h"
#include "UnArkWCX_Test_Util.h"

#ifdef _WIN64
const wchar_t* const LIB_UNARKWCX_NAME = L"UnArkWCX.WCX64";
#else
const wchar_t* const LIB_UNARKWCX_NAME = L"UnArkWCX.WCX";
#endif

#pragma warning( disable: 4996 )

HMODULE hMod;

class CEnvironment : public ::testing::Environment
{
public:
	virtual ~CEnvironment() {}
	// Override this to define how to set up the environment.
	virtual void SetUp() 
	{
		std::cout << "fdsfds";

		hMod = LoadLibrary( LIB_UNARKWCX_NAME );
		ASSERT_EQ( true, hMod != NULL );
	}
	// Override this to define how to tear down the environment.
	virtual void TearDown() 
	{
		if( hMod != NULL )
			FreeLibrary( hMod );
		hMod = NULL;
	}
};

TEST( ExportTest, OpenArchive )
{
	EXPECT_EQ( true,
		GetProcAddress(	hMod, 
			::testing::UnitTest::GetInstance()->current_test_info()->name() 
			) != NULL 
		);
}

TEST( ExportTest, OpenArchiveW )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
			::testing::UnitTest::GetInstance()->current_test_info()->name() 
			) != NULL 
		);
}

TEST( ExportTest, ReadHeader )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, ReadHeaderExW )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, ProcessFile )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, ProcessFileW )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, PackFiles )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, PackFilesW )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, DeleteFiles )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, DeleteFilesW )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, ConfigurePacker )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, CloseArchive )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}


TEST( ExportTest, SetChangeVolProc )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, SetChangeVolProcW )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}


TEST( ExportTest, SetProcessDataProc )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, SetProcessDataProcW )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, GetPackerCaps )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, CanYouHandleThisFile )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, CanYouHandleThisFileW )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

TEST( ExportTest, PackSetDefaultParams )
{
	EXPECT_EQ( true,
		GetProcAddress( hMod, 
		::testing::UnitTest::GetInstance()->current_test_info()->name() 
		) != NULL 
		);
}

class ArchiveTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		pwszAddList = NULL;
		vecAddFile.clear();
		ZeroMemory( wszSrcPath, sizeof( wchar_t ) * MAX_PATH );

		pfnOpenArchiveW = (pFnOpenArchiveW)GetProcAddress( hMod, "OpenArchiveW" );

		pfnReadHeaderExW = (pFnReadHeaderExW)GetProcAddress( hMod, "ReadHeaderExW" );

		pfnProcessFileW = (pFnProcessFileW)GetProcAddress( hMod, "ProcessFileW" );

		pfnPackFilesW = (pFnPackFilesW)GetProcAddress( hMod, "PackFilesW" );

		pfnDeleteFilesW = (pFnDeleteFilesW)GetProcAddress( hMod, "DeleteFilesW" );

		pfnSetChangeVolProcW = (pFnSetChangeVolProcW)GetProcAddress( hMod, "SetChangeVolProcW" );
		pfnSetProcessDataProcW = (pFnSetProcessDataProcW)GetProcAddress( hMod, "SetProcessDataProcW" );

		pfnCanYouHandleThisFileW = (pFnCanYouHandleThisFileW)GetProcAddress( hMod, "CanYouHandleThisFileW" );

		pfnPackSetDefaultParams = (pFnPackSetDefaultParams)GetProcAddress( hMod, "PackSetDefaultParams" );

		pfnSetChangeVolProcW( INVALID_HANDLE_VALUE, &ArchiveTest::changeVolProcW );
		pfnSetProcessDataProcW( INVALID_HANDLE_VALUE, &ArchiveTest::processDataProcW );

	}

	virtual void TearDown()
	{
		Sleep(500);
		if( pwszAddList != NULL )
			delete [] pwszAddList;
		pwszAddList = NULL;

	}

	static int WINAPI processDataProcW( WCHAR *FileName, int Size );
	static int WINAPI changeVolProcW( WCHAR *ArcName, int Mode );

	pFnOpenArchive						pfnOpenArchive;
	pFnOpenArchiveW						pfnOpenArchiveW;

	pFnReadHeader						pfnReadHeader;
	pFnReadHeaderExW					pfnReadHeaderExW;

	pFnProcessFile						pfnProcessFile;
	pFnProcessFileW						pfnProcessFileW;

	pFnPackFiles						pfnPackFiles;
	pFnPackFilesW						pfnPackFilesW;

	pFnDeleteFiles						pfnDeleteFiles;
	pFnDeleteFilesW						pfnDeleteFilesW;

	pFnConfigurePacker					pfnConfigurePacker;

	pFnCloseArchive						pfnCloseArchive;

	pFnSetChangeVolProcW				pfnSetChangeVolProcW;
	pFnSetProcessDataProcW				pfnSetProcessDataProcW;

	pFnGetPackerCaps					pfnGetPackerCaps;
	pFnCanYouHandleThisFileW			pfnCanYouHandleThisFileW;

	pFnPackSetDefaultParams				pfnPackSetDefaultParams;

	wchar_t*							pwszAddList;
	wchar_t								wszSrcPath[ MAX_PATH ];
	std::vector< std::wstring >			vecAddFile;
};

TEST_F( ArchiveTest, SingleFileAddToZIP )
{
	do 
	{
		SetDefaultPackerExtension( L"ZIP" );
		SetINIFilePath( pfnPackSetDefaultParams);

		GetModuleFileName( NULL, wszSrcPath, MAX_PATH - 1 );
		PathRemoveFileSpec( wszSrcPath );

		vecAddFile.push_back( L"Ark64.dll" );
		pwszAddList = GetAddList( vecAddFile, L"" );

		ASSERT_EQ( 0, pfnPackFilesW( L"ArchiveTest_SingleFileAddToZIP.ZIP", NULL, wszSrcPath, pwszAddList, PK_PACK_SAVE_PATHS ) );
		
	} while (false);
}

TEST_F( ArchiveTest, SingleFileMoveAndExtractToZIP )
{
	do 
	{
		SetDefaultPackerExtension( L"ZIP" );
		SetINIFilePath( pfnPackSetDefaultParams);

		GetModuleFileName( NULL, wszSrcPath, MAX_PATH - 1 );
		PathRemoveFileSpec( wszSrcPath );

		vecAddFile.push_back( L"Ark64.dll" );
		pwszAddList = GetAddList( vecAddFile, L"" );

		ASSERT_EQ( 0, pfnPackFilesW( L"ArchiveTest_SingleFileMoveAndExtractToZIP.ZIP", NULL, wszSrcPath, pwszAddList, PK_PACK_MOVE_FILES ) );

		ASSERT_EQ( FALSE, PathFileExists( L"Ark64.dll" ) );

	} while (false);
}

TEST_F( ArchiveTest, ListAllZIPFiles )
{
	intptr_t hFind = -1;
	tOpenArchiveDataW archiveData;
	tHeaderDataExW headerDataExW;
	
	archiveData.ArcName				= NULL;
	
	do 
	{
		_wfinddata_t fd;
		archiveData.ArcName				= new wchar_t[ MAX_PATH ];
		if( archiveData.ArcName == NULL )
			break;

		hFind = _wfindfirst( L"*.zip", &fd );
		if( hFind == -1 )
			break;

		do 
		{
			if( StrStrIW( fd.name, L".zip" ) == NULL )
				continue;

			wcscpy( archiveData.ArcName, fd.name );

			archiveData.CmtBuf				= NULL;
			archiveData.CmtBufSize			= NULL;
			archiveData.CmtSize				= NULL;
			archiveData.CmtState			= NULL;
			archiveData.OpenMode			= PK_OM_EXTRACT;
			archiveData.OpenResult			= 0;

			HANDLE hArchive = pfnOpenArchiveW( &archiveData );
			EXPECT_NE( (HANDLE)0, hArchive );
			if( hArchive == NULL )
				break;

			int nResult = pfnReadHeaderExW( hArchive, &headerDataExW );

			

		} while ( _wfindnext( hFind, &fd ) == 0 );

// 		while( pfnReadHeaderExW( hArchive, &headerDataExW ) == 0 )
// 		{
// 
// 		}
// 
	} while (false);

	if( hFind != -1 )
		_findclose( hFind );

	if( archiveData.ArcName != NULL )
		delete [] archiveData.ArcName;

}
// 
// TEST_F( ArchiveTest, SingleFileAddToISO )
// {
// 	do 
// 	{
// 		SetDefaultPackerExtension( L"ISO" );
// 		SetINIFilePath( pfnPackSetDefaultParams);
// 
// 		GetModuleFileName( NULL, wszSrcPath, MAX_PATH - 1 );
// 		PathRemoveFileSpec( wszSrcPath );
// 
// 		vecAddFile.push_back( L"Ark64.dll" );
// 		pwszAddList = GetAddList( vecAddFile, L"" );
// 
// 		ASSERT_EQ( 0, pfnPackFilesW( L"ArchiveTest_Method1.ISO", NULL, wszSrcPath, pwszAddList, PK_PACK_SAVE_PATHS ) );
// 
// 	} while (false);
// }
// 
// 
// TEST_F( ArchiveTest, SingleFileAddTo7Z )
// {
// 	do 
// 	{
// 		SetDefaultPackerExtension( L"7Z" );
// 		SetINIFilePath( pfnPackSetDefaultParams);
// 
// 		GetModuleFileName( NULL, wszSrcPath, MAX_PATH - 1 );
// 		PathRemoveFileSpec( wszSrcPath );
// 
// 		vecAddFile.push_back( L"Ark64.dll" );
// 		pwszAddList = GetAddList( vecAddFile, L"" );
// 
// 		ASSERT_EQ( 0, pfnPackFilesW( L"ArchiveTest_Method1.7Z", NULL, wszSrcPath, pwszAddList, PK_PACK_SAVE_PATHS ) );
// 
// 	} while (false);
// }
