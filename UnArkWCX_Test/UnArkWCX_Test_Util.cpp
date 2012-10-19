#include "stdafx.h"

#include <iostream>

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi" )

#include "UnArkWCX_Test_Util.h"
#include "../UnArkWCX_Export.h"

#pragma warning( disable: 4996 )

std::wstring GetCurrentPath()
{
	wchar_t wszBuffer[ MAX_PATH ] = {0,};
	GetModuleFileName( NULL, wszBuffer, MAX_PATH - 1 );
	PathRemoveFileSpec( wszBuffer );

	return wszBuffer;
}

void SetDefaultPackerExtension( const wchar_t* wszExtension )
{
	WritePrivateProfileString( CONFIGURE_INI_SECNAME, CONFIGURE_SELECT_EXTENSION, wszExtension, (GetCurrentPath() + L"\\" + CONFIGURE_INI_FILENAME).c_str() );
	WritePrivateProfileString( NULL, NULL, NULL, NULL );
}

void SetINIFilePath( pFnPackSetDefaultParams pfnPackSetDefaultParams )
{
	PackDefaultParamStruct dps;
	memset( dps.DefaultIniName, '\0', sizeof( char ) * MAX_PATH );
	dps.size = sizeof( dps );

	strcpy( dps.DefaultIniName, CU2A( GetCurrentPath() + L"\\" + CONFIGURE_INI_FILENAME ).c_str() );

	pfnPackSetDefaultParams( &dps );
}

wchar_t* GetAddList( const std::vector< std::wstring >& vecAddedFile, const std::wstring& strSubPath )
{
	wchar_t* pwszBuffer = NULL;
	wchar_t* pwszAddList = NULL;

	do
	{
		int nAddListLength = 0;
		for( size_t idx = 0; idx < vecAddedFile.size(); ++idx )
		{
			nAddListLength += vecAddedFile[ idx ].size();
			nAddListLength += strSubPath.size();
			nAddListLength += 1;	// NULL 문자
		}
		nAddListLength += 1;		// 문자열 목록 종료를 알리기 위한 널 문자 추가

		pwszAddList = new wchar_t[ nAddListLength ];
		if( pwszAddList == NULL )
			break;

		ZeroMemory( pwszAddList, sizeof( wchar_t ) * nAddListLength );
		pwszBuffer = pwszAddList;
		for( size_t idx = 0; idx < vecAddedFile.size(); ++idx )
		{
			if( strSubPath.empty() == false )
				wcscpy( pwszBuffer, strSubPath.c_str() );
			wcscpy( pwszBuffer, vecAddedFile[ idx ].c_str() );
			pwszBuffer = pwszBuffer + wcslen( pwszBuffer ) + 1;
		}

	} while (false);

	return pwszAddList;
}
