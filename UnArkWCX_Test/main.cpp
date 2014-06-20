#include "stdafx.h"

#include <cstdio>

#include "gtest/gtest.h"

#include "tst_unArkWCX.h"
#include "tst_unArkWCXEnv.h"

#pragma execution_character_set( "utf-8" )

int wmain( int argc, wchar_t* argv[], wchar_t *envp[] )
{
    ::testing::InitGoogleTest( &argc, argv );

    ::testing::AddGlobalTestEnvironment( new TST_GlobalEnvironment );

    return RUN_ALL_TESTS();
}