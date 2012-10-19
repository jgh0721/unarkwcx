// UnArkWCX_Test.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include "../Library/gtest/gtest.h"

#include "UnArkWCX_Test.h"

#if _MSC_VER == 1600

	#ifdef _WIN64

		#ifdef _DEBUG
			#pragma comment( lib, "gtest-v100-x64-d" )
		#else
			#pragma comment( lib, "gtest-v100-x64-r" )
		#endif

	#else

		#ifdef _DEBUG
			#pragma comment( lib, "gtest-v100-x86-d" )
		#else
			#pragma comment( lib, "gtest-v100-x86-r" )
		#endif

	#endif

#elif _MSC_VER == 1500 

	#ifdef _WIN64

		#ifdef _DEBUG
			#pragma comment( lib, "gtest-v90-x64-d" )
		#else
			#pragma comment( lib, "gtest-v90-x64-r" )
		#endif

	#else

		#ifdef _DEBUG
			#pragma comment( lib, "gtest-v90-x86-d" )
		#else
			#pragma comment( lib, "gtest-v90-x86-r" )
		#endif

	#endif

#endif


int _tmain(int argc, _TCHAR* argv[])
{
	/*!
		�� �Լ����� EXPORT �Ǿ����� Ȯ��
	*/

	testing::InitGoogleTest( &argc, argv );
	testing::AddGlobalTestEnvironment( new CEnvironment );
	
	return RUN_ALL_TESTS();
}


int WINAPI ArchiveTest::processDataProcW( WCHAR *FileName, int Size )
{
	return 1;
}

int WINAPI ArchiveTest::changeVolProcW( WCHAR *ArcName, int Mode )
{
	return 1;
}
