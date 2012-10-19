#pragma once

#include <string>
#include <vector>

#include "../UnArkWCX_Export.h"

std::wstring GetCurrentPath();

void SetDefaultPackerExtension( const wchar_t* wszExtension );
void SetINIFilePath( pFnPackSetDefaultParams pfnPackSetDefaultParams );
wchar_t* GetAddList( const std::vector< std::wstring >& vecAddedFile, const std::wstring& strSubPath );
