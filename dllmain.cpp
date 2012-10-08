// dllmain.cpp : DLL ���� ���α׷��� �������� �����մϴ�.
#include "stdafx.h"

#include "ArkLib.h"

HINSTANCE g_hInst;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
			g_hInst = hModule;
			LM_INSTANCE()->SetLogLevel( LL_TRACE );
			LM_INSTANCE()->SetLogTransfer( LT_DEBUGGER | LT_STDOUT );
			LM_INSTANCE()->SetLogPrefix( L"[%D-%t, %l, %F-%L] " );
			break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

