#include "stdafx.h"

#include "ArkLib.h"
#include "UnArkWCX.h"

#pragma execution_character_set( "utf-8" )

//////////////////////////////////////////////////////////////////////////

HINSTANCE g_hInst;
WCHAR gArkDLLFullPathName[ MAX_PATH ] = { 0, };

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        GetModuleFileNameW( g_hInst, gArkDLLFullPathName, MAX_PATH );
        PathRemoveFileSpecW( gArkDLLFullPathName );
        PathAppendW( gArkDLLFullPathName, ARK_DLL_RELEASE_FILE_NAME );
        DisableThreadLibraryCalls( hModule );
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
