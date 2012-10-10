// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"

#include "ArkLib.h"
#include "UnArkWCX.h"

HINSTANCE g_hInst;
WCHAR gArkDLLFullPathName[ MAX_PATH ] = {0,};
WCHAR gCurrentArchiveExtension[ 32 ] = {0,};
WCHAR gConfigureINIFullPath[ MAX_PATH ] = {0,};
CArkEvent gClsArkEvent;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
			g_hInst = hModule;
			GetModuleFileName( g_hInst, gArkDLLFullPathName, MAX_PATH );
			PathRemoveFileSpec( gArkDLLFullPathName );
			PathAppend( gArkDLLFullPathName, ARK_DLL_RELEASE_FILE_NAME );
			
// #ifndef UNARKWCX_DEPLOY
			LM_INSTANCE()->SetLogLevel( LL_TRACE );
			LM_INSTANCE()->SetLogTransfer( LT_DEBUGGER | LT_STDOUT );
			LM_INSTANCE()->SetLogPrefix( L"[%D-%t, %l, %F-%L] " );
// #endif
			break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
// #ifndef UNARKWCX_DEPLOY
// //		LM_EXIT();
// #endif
		break;
	}
	return TRUE;
}

