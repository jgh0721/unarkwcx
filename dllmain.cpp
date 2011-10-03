// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "UnArkWCX.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		LM_INSTANCE()->SetLogTransfer( LT_DEBUGGER | LT_CONSOLE );
		LM_INSTANCE()->SetLogLevel( LL_TRACE );

		LM_TRACE(( L"Ark 라이브러리 %s 불러오기", ARK_DLL_RELEASE_FILE_NAME ));

		switch( g_arkLib.Create(ARK_DLL_RELEASE_FILE_NAME) )
		{
		case ARKERR_NOERR:
			LM_TRACE(( L"Ark 라이브러리 불러오기 성공" ));
			break;
		case ARKERR_UNSUPPORTED_OS:
			LM_ERROR(( L"Ark 라이브러리 불러오기 오류 : %s", GetArkLastErrorText( ARKERR_UNSUPPORTED_OS )  ));
			break;
		case ARKERR_LOADLIBRARY_FAILED:
			LM_ERROR(( L"Ark 라이브러리 불러오기 오류 : %s", GetArkLastErrorText( ARKERR_LOADLIBRARY_FAILED ) ));
			break;
		case ARKERR_GETPROCADDRESS_FAILED:
			LM_ERROR(( L"Ark 라이브러리 불러오기 오류 : %s", GetArkLastErrorText( ARKERR_GETPROCADDRESS_FAILED ) ));
			break;
		case ARKERR_INVALID_VERSION:
			LM_ERROR(( L"Ark 라이브러리 불러오기 오류 : %s", GetArkLastErrorText( ARKERR_INVALID_VERSION ) ));
			break;
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		LM_TRACE(( L"Ark 라이브러리 해제" ));
		// g_arkLib.Release();
		// LM_EXIT();
		break;
	}
	return TRUE;
}

