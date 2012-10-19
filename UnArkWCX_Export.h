#pragma once

#include "wcxhead.h"
#include "ArkLib.h"


// 다음 ifdef 블록은 DLL에서 내보내기하는 작업을 쉽게 해 주는 매크로를 만드는 
// 표준 방식입니다. 이 DLL에 들어 있는 파일은 모두 명령줄에 정의된 _EXPORTS 기호로
// 컴파일되며, 동일한 DLL을 사용하는 다른 프로젝트에서는 이 기호를 정의할 수 없습니다.
// 이렇게 하면 소스 파일에 이 파일이 들어 있는 다른 모든 프로젝트에서는 
// UNARKWCX_API 함수를 DLL에서 가져오는 것으로 보고, 이 DLL은
// 이 DLL은 해당 매크로로 정의된 기호가 내보내지는 것으로 봅니다.
#ifdef UNARKWCX_EXPORTS
#define UNARKWCX_API __declspec(dllexport)
#else
#define UNARKWCX_API __declspec(dllimport)
#endif

const wchar_t* const CONFIGURE_INI_FILENAME = L"pkplugin.INI";
const wchar_t* const CONFIGURE_INI_SECNAME = L"UnArkWCX";

const wchar_t* const CONFIGURE_SELECT_EXTENSION = L"SelectArchiveExtension";
const wchar_t* const CONFIGURE_SaveNTFSTimeForZIP = L"SaveNTFSTimeForZIP";
const wchar_t* const CONFIGURE_CompressionMerhod = L"compressionMethod";
const wchar_t* const CONFIGURE_CompressionLevel = L"compressionLevel";

/*!
OpenArchive should perform all necessary operations when an archive is to be opened.

HANDLE __stdcall OpenArchive (tOpenArchiveData *ArchiveData);
Description

OpenArchive should return a unique handle representing the archive. The handle should remain valid until CloseArchive is called. If an error occurs, you should return zero, and specify the error by setting OpenResult member of ArchiveData.

You can use the ArchiveData to query information about the archive being open, and store the information in ArchiveData to some location that can be accessed via the handle.
*/
EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchive( tOpenArchiveData *pArchiveData );
EXTERN_C UNARKWCX_API HANDLE WINAPI OpenArchiveW( tOpenArchiveDataW *pArchiveData );

EXTERN_C UNARKWCX_API int WINAPI ReadHeader( HANDLE hArcData, tHeaderData *pHeaderData );
/*!
Totalcmd calls ReadHeaderEx to find out what files are in the archive. This function is always called instead of ReadHeader if it is present. It only needs to be implemented if the supported archive type may contain files >2 GB. You should implement both ReadHeader and ReadHeaderEx in this case, for compatibility with older versions of Total Commander.

int __stdcall ReadHeaderEx (HANDLE hArcData, tHeaderDataEx *HeaderDataEx);
Description

ReadHeaderEx is called as long as it returns zero (as long as the previous call to this function returned zero). Each time it is called, HeaderDataEx is supposed to provide Totalcmd with information about the next file contained in the archive. When all files in the archive have been returned, ReadHeaderEx should return E_END_ARCHIVE which will prevent ReaderHeaderEx from being called again. If an error occurs, ReadHeaderEx should return one of the error values or 0 for no error.

hArcData contains the handle returned by OpenArchive. The programmer is encouraged to store other information in the location that can be accessed via this handle. For example, you may want to store the position in the archive when returning files information in ReadHeaderEx.
In short, you are supposed to set at least PackSize, PackSizeHigh, UnpSize, UnpSizeHigh, FileTime, and FileName members of tHeaderDataEx. Totalcmd will use this information to display content of the archive when the archive is viewed as a directory.
*/
EXTERN_C UNARKWCX_API int WINAPI ReadHeaderExW( HANDLE hArcData, tHeaderDataExW *pHeaderDataExW );

/*!
ProcessFile should unpack the specified file or test the integrity of the archive.

int __stdcall ProcessFile (HANDLE hArcData, int Operation, char *DestPath, char *DestName);

Description

ProcessFile should return zero on success, or one of the error values otherwise.

hArcData contains the handle previously returned by you in OpenArchive. Using this, you should be able to find out information (such as the archive filename) that you need for extracting files from the archive.

Unlike PackFiles, ProcessFile is passed only one filename. Either DestName contains the full path and file name and DestPath is NULL, or DestName contains only the file name and DestPath the file path. This is done for compatibility with unrar.dll.

When Total Commander first opens an archive, it scans all file names with OpenMode==PK_OM_LIST, so ReadHeader() is called in a loop with calling ProcessFile(...,PK_SKIP,...). When the user has selected some files and started to decompress them, Total Commander again calls ReadHeader() in a loop. For each file which is to be extracted, Total Commander calls ProcessFile() with Operation==PK_EXTRACT immediately after the ReadHeader() call for this file. If the file needs to be skipped, it calls it with Operation==PK_SKIP.

Each time DestName is set to contain the filename to be extracted, tested, or skipped. To find out what operation out of these last three you should apply to the current file within the archive, Operation is set to one of the following:

Constant	Value	Description

PK_SKIP	0	Skip this file
PK_TEST	1	Test file integrity
PK_EXTRACT	2	Extract to disk
*/
EXTERN_C UNARKWCX_API int WINAPI ProcessFile( HANDLE hArcData, int Operation, char *DestPath, char *DestName);
EXTERN_C UNARKWCX_API int WINAPI ProcessFileW( HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName );

/*!
	파일을 압축하는 방법

	PackFiles specifies what should happen when a user creates, or adds files to the archive.

	int __stdcall PackFiles (char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags);

	Description

	PackFiles should return zero on success, or one of the error values otherwise.

	PackedFile refers to the archive that is to be created or modified. The string contains the full path.

	SubPath is either NULL, when the files should be packed with the paths given with the file names, or not NULL when they should be placed below the given subdirectory within the archive. Example:

	SubPath="subdirectory"Name in AddList="subdir2\filename.ext"-> File should be packed as "subdirectory\subdir2\filename.ext"

	SrcPath contains path to the files in AddList. SrcPath and AddList together specify files that are to be packed into PackedFile. Each string in AddList is zero-delimited (ends in zero), and the AddList string ends with an extra zero byte, i.e. there are two zero bytes at the end of AddList.

	Flags can contain a combination of the following values reflecting the user choice from within Totalcmd:

	Constant	Value	Description

	PK_PACK_MOVE_FILES	1	Delete original after packing
	PK_PACK_SAVE_PATHS	2	Save path names of files
	PK_PACK_ENCRYPT	4	Ask user for password, then encrypt file with that password
*/
EXTERN_C UNARKWCX_API int WINAPI PackFiles( char* PackedFile, char* SubPath, char* SrcPath, char* AddList, int Flags );
EXTERN_C UNARKWCX_API int WINAPI PackFilesW( wchar_t* PackedFile, wchar_t* SubPath, wchar_t* SrcPath, wchar_t* AddList, int Flags );

EXTERN_C UNARKWCX_API int WINAPI DeleteFiles( char *PackedFile, char *DeleteList );
EXTERN_C UNARKWCX_API int WINAPI DeleteFilesW( wchar_t *PackedFile, wchar_t *DeleteList );

EXTERN_C UNARKWCX_API void WINAPI ConfigurePacker( HWND Parent, HINSTANCE DllInstance );

EXTERN_C UNARKWCX_API int WINAPI CloseArchive( HANDLE hArcData );

EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProc( HANDLE hArcData, tChangeVolProc pfnChangeVolProc );
EXTERN_C UNARKWCX_API void WINAPI SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc );

EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProc( HANDLE hArcData, tProcessDataProc pfnProcessDataProc );
EXTERN_C UNARKWCX_API void WINAPI SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc );

EXTERN_C UNARKWCX_API int WINAPI GetPackerCaps();
EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFile( char *FileName );
EXTERN_C UNARKWCX_API BOOL WINAPI CanYouHandleThisFileW( WCHAR *FileName );

EXTERN_C UNARKWCX_API void WINAPI PackSetDefaultParams(PackDefaultParamStruct* dps);

typedef HANDLE (WINAPI *pFnOpenArchive)( tOpenArchiveData *pArchiveData );
typedef HANDLE (WINAPI *pFnOpenArchiveW)( tOpenArchiveDataW *pArchiveData );

typedef int (WINAPI *pFnReadHeader)( HANDLE hArcData, tHeaderData *pHeaderData );
typedef int (WINAPI *pFnReadHeaderExW)( HANDLE hArcData, tHeaderDataExW *pArchiveData );

typedef int (WINAPI *pFnProcessFile)( HANDLE hArcData, int Operation, char *DestPath, char *DestName);
typedef int (WINAPI *pFnProcessFileW)( HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName );

typedef int (WINAPI *pFnPackFiles)( char* PackedFile, char* SubPath, char* SrcPath, char* AddList, int Flags );
typedef int (WINAPI *pFnPackFilesW)( wchar_t* PackedFile, wchar_t* SubPath, wchar_t* SrcPath, wchar_t* AddList, int Flags );

typedef int (WINAPI *pFnDeleteFiles)( char *PackedFile, char *DeleteList );
typedef int (WINAPI *pFnDeleteFilesW)( wchar_t *PackedFile, wchar_t *DeleteList );

typedef void (WINAPI *pFnConfigurePacker)( HWND Parent, HINSTANCE DllInstance );

typedef int (WINAPI *pFnCloseArchive)( HANDLE hArcData );

typedef void (WINAPI *pFnSetChangeVolProc)( HANDLE hArcData, tChangeVolProc pfnChangeVolProc );
typedef void (WINAPI *pFnSetChangeVolProcW)( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc );

typedef void (WINAPI *pFnSetProcessDataProc)( HANDLE hArcData, tProcessDataProc pfnProcessDataProc );
typedef void (WINAPI *pFnSetProcessDataProcW)( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc );

typedef int (WINAPI *pFnGetPackerCaps)();
typedef BOOL (WINAPI *pFnCanYouHandleThisFile)( char *FileName );
typedef BOOL (WINAPI *pFnCanYouHandleThisFileW)( WCHAR *FileName );

typedef void (WINAPI *pFnPackSetDefaultParams)(PackDefaultParamStruct* dps);

