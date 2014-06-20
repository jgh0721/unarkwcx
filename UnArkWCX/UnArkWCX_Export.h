#pragma once

#ifdef UNARKWCX_EXPORTS
#define UNARKWCX_API __declspec(dllexport)
#else
#define UNARKWCX_API __declspec(dllimport)
#endif

#pragma execution_character_set( "utf-8" )


/*!
    토탈 커맨더 압축 플러그인

    옵션 함수들

        해당 플러그인이 백그라운드 작업을 지원하는지 여부
        int __stdcall GetBackgroundFlags(void);
            반환값 : 아래 값들의 조합을 반환
                BACKGROUND_UNPACK   1   Calls to OpenArchive, ReadHeader(Ex), ProcessFile and CloseArchive are thread-safe (unpack in background)
                BACKGROUND_PACK     2   Calls to PackFiles are thread-safe (pack in background)
                BACKGROUND_MEMPACK  4   Calls to StartMemPack, PackToMem and DoneMemPack are thread-safe

    Totalcmd calls ReadHeaderEx to find out what files are in the archive. This function is always called instead of ReadHeader if it is present. It only needs to be implemented if the supported archive type may contain files >2 GB. You should implement both ReadHeader and ReadHeaderEx in this case, for compatibility with older versions of Total Commander.
    int __stdcall ReadHeaderEx (HANDLE hArcData, tHeaderDataEx *HeaderDataEx);
    int __stdcall ReadHeaderExW( HANDLE hArcData, tHeaderDataExW *pHeaderDataExW );
    ReadHeaderEx is called as long as it returns zero (as long as the previous call to this function returned zero). Each time it is called, HeaderDataEx is supposed to provide Totalcmd with information about the next file contained in the archive. When all files in the archive have been returned, ReadHeaderEx should return E_END_ARCHIVE which will prevent ReaderHeaderEx from being called again. If an error occurs, ReadHeaderEx should return one of the error values or 0 for no error.
    hArcData contains the handle returned by OpenArchive. The programmer is encouraged to store other information in the location that can be accessed via this handle. For example, you may want to store the position in the archive when returning files information in ReadHeaderEx.
    In short, you are supposed to set at least PackSize, PackSizeHigh, UnpSize, UnpSizeHigh, FileTime, and FileName members of tHeaderDataEx. Totalcmd will use this information to display content of the archive when the archive is viewed as a directory.

    GetPackerCaps tells Totalcmd what features your packer plugin supports.
    int __stdcall GetPackerCaps();
    Description

        Implement GetPackerCaps to return a combination of the following values:

        Constant	Value	Description

        PK_CAPS_NEW	1	Can create new archives
        PK_CAPS_MODIFY	2	Can modify existing archives
        PK_CAPS_MULTIPLE	4	Archive can contain multiple files
        PK_CAPS_DELETE	8	Can delete files
        PK_CAPS_OPTIONS	16	Has options dialog

        PK_CAPS_MEMPACK	32	Supports packing in memory
        PK_CAPS_BY_CONTENT	64	Detect archive type by content
        PK_CAPS_SEARCHTEXT	128	Allow searching for text in archives created with this plugin
        PK_CAPS_HIDE	256	Don't show packer icon, don't open with Enter but with Ctrl+PgDn
        PK_CAPS_ENCRYPT	512	Plugin supports encryption.

        Omitting PK_CAPS_NEW and PK_CAPS_MODIFY means PackFiles will never be called and so you don뭪 have to implement PackFiles.
        Omitting PK_CAPS_MULTIPLE means PackFiles will be supplied with just one file.
        Leaving out PK_CAPS_DELETE means DeleteFiles will never be called;
        leaving out PK_CAPS_OPTIONS means ConfigurePacker will not be called.
        PK_CAPS_MEMPACK enables the functions StartMemPack, PackToMem and DoneMemPack.
        If PK_CAPS_BY_CONTENT is returned, Totalcmd calls the function CanYouHandleThisFile when the user presses Ctrl+PageDown on an unknown archive type.
        Finally, if PK_CAPS_SEARCHTEXT is returned, Total Commander will search for text inside files packed with this plugin. This may not be a good idea for certain plugins like the diskdir plugin, where file contents may not be available. If PK_CAPS_HIDE is set, the plugin will not show the file type as a packer. This is useful for plugins which are mainly used for creating files, e.g. to create batch files, avi files etc. The file needs to be opened with Ctrl+PgDn in this case, because Enter will launch the associated application.

    Important note:
        If you change the return values of this function, e.g. add packing support, you need to reinstall the packer plugin in Total Commander, otherwise it will not detect the new capabilities.

    CanYouHandleThisFile allows the plugin to handle files with different extensions than the one defined in Total Commander. It is called when the plugin defines PK_CAPS_BY_CONTENT, and the user tries to open an archive with Ctrl+PageDown.
    BOOL __stdcall CanYouHandleThisFile (char *FileName);
    BOOL __stdcall CanYouHandleThisFileW( WCHAR *FileName );

    Description

        CanYouHandleThisFile should return true (nonzero) if the plugin recognizes the file as an archive which it can handle. The detection must be by contents, NOT by extension. If this function is not implemented, Totalcmd assumes that only files with a given extension can be handled by the plugin.
        Filename contains the fully qualified name (path+name) of the file to be checked.


        PackSetDefaultParams is called immediately after loading the DLL, before any other function. This function is new in version 2.1. It requires Total Commander >=5.51, but is ignored by older versions.
        void __stdcall PackSetDefaultParams(PackDefaultParamStruct* dps);
        Description of parameters:
        dps	This structure of type PackDefaultParamStruct currently contains the version number of the plugin interface, and the suggested location for the settings file (ini file). It is recommended to store any plugin-specific information either directly in that file, or in that directory under a different name. Make sure to use a unique header when storing data in this file, because it is shared by other file system plugins! If your plugin needs more than 1kbyte of data, you should use your own ini file because ini files are limited to 64k.

    Return value:
        The function has no return value:

    Important note:
        This function is only called in Total Commander 5.51 and later. The plugin version will be >= 2.1.

    DeleteFiles should delete the specified files from the archive
    int __stdcall DeleteFiles (char *PackedFile, char *DeleteList);
    int __stdcall DeleteFilesW( wchar_t *PackedFile, wchar_t *DeleteList );

    ConfigurePacker gets called when the user clicks the Configure button from within "Pack files..." dialog box in Totalcmd.
    void __stdcall ConfigurePacker (HWND Parent, HINSTANCE DllInstance);

    Description

        Usually, you provide a user with a dialog box specifying a method and/or its parameters that should be applied in the packing process. Or, you just want to display a message box about what your plugin is, just like Christian Ghisler뭩 DiskDir does.
        In order to help you with a feedback, you can use a window handle of Totalcmd process, Parent. That is, you make your dialog box a child of Parent.
        When creating a window, you may also need handle of the DLL (your DLL) that creates your dialog box, DllInstance.

        You may decide not to implement this function. Then, make sure you omit PK_CAPS_OPTIONS from return values of GetPackerCaps.

    공통적으로 필요한 함수들

    OpenArchive should perform all necessary operations when an archive is to be opened.
    HANDLE __stdcall OpenArchive (tOpenArchiveData *ArchiveData);
    HANDLE __stdcall OpenArchiveW( tOpenArchiveDataW *pArchiveData );
        반환값 :   
            성공 = 해당 압축파일을 나타내는 고유 핸들값, CloseArchive 를 호출할 때 까지 유효해야함
            실패 = 0 을 반환한 후, ArchiveData 의 openResult 멤버에 오류코드를 설정

    Totalcmd calls ReadHeader to find out what files are in the archive.
    int __stdcall ReadHeader (HANDLE hArcData, tHeaderData *HeaderData);
        반환값 :   
            성공 = 압축파일내에 읽을 데이터가 있는 동안, 0 을 반환하고 더이상 데이터가 없다면 E_END_ARCHIVE 반환
            실패 = 오류 코드 설정

    Value	Description

        0x1	Read-only file
        0x2	Hidden file
        0x4	System file
        0x8	Volume ID file
        0x10	Directory
        0x20	Archive file
        0x3F	Any file

    ProcessFile should unpack the specified file or test the integrity of the archive.
    int __stdcall ProcessFile (HANDLE hArcData, int Operation, char *DestPath, char *DestName);
    int __stdcall ProcessFileW( HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName );
        반환값 :   성공 = 0
        실패 = 오류코드

        ※ DestPath 가 NULL 이라면 DestName 에 경로명을 포함한 완전 경로가 전달됨
            PK_SKIP     0	Skip this file
            PK_TEST     1	Test file integrity
            PK_EXTRACT  2	Extract to disk

    CloseArchive should perform all necessary operations when an archive is about to be closed.
    int __stdcall CloseArchive (HANDLE hArcData);
        반환값 :   성공 = 0
        실패 = 오류코드

    This function allows you to notify user about changing a volume when packing files.
    void __stdcall SetChangeVolProc (HANDLE hArcData, tChangeVolProc pChangeVolProc1);
    void __stdcall SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProc );
    Description

    pChangeVolProc1 contains a pointer to a function that you may want to call when notifying user to change volume (e.g. insterting another diskette). You need to store the value at some place if you want to use it; you can use hArcData that you have returned by OpenArchive to identify that place.

    This function allows you to notify user about the progress when you un/pack files.
    void __stdcall SetProcessDataProc (HANDLE hArcData, tProcessDataProc pProcessDataProc);
    void __stdcall SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProc );
    Description

    pProcessDataProc contains a pointer to a function that you may want to call when notifying user about the progress being made when you pack or extract files from an archive. You need to store the value at some place if you want to use it; you can use hArcData that you have returned by OpenArchive to identify that place.

    압축 전용 함수들

        PackFiles specifies what should happen when a user creates, or adds files to the archive.
        int __stdcall PackFiles (char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags);
        int __stdcall PackFilesW( wchar_t* PackedFile, wchar_t* SubPath, wchar_t* SrcPath, wchar_t* AddList, int Flags );
        반환값 :   성공 = 0
        실패 = 오류코드

        PackedFile refers to the archive that is to be created or modified. The string contains the full path.
        SubPath is either NULL, when the files should be packed with the paths given with the file names, or not NULL when they should be placed below the given subdirectory within the archive. Example:
        SubPath="subdirectory"
        Name in AddList="subdir2\filename.ext"->
        File should be packed as "subdirectory\subdir2\filename.ext"
        SrcPath contains path to the files in AddList. SrcPath and AddList together specify files that are to be packed into PackedFile. Each string in AddList is zero-delimited (ends in zero), and the AddList string ends with an extra zero byte, i.e. there are two zero bytes at the end of AddList.
        Flags can contain a combination of the following values reflecting the user choice from within Totalcmd:

        PK_PACK_MOVE_FILES	1	Delete original after packing
        PK_PACK_SAVE_PATHS	2	Save path names of files
        PK_PACK_ENCRYPT	4	Ask user for password, then encrypt file with that password

    압축해제 전용 함수들

    플러그인 로딩
        구현되어 있다면, PackSetDefaultParams 호출

    플러그인 설치
        구현되어 있다면, GetPackerCaps 호출
        구현되어 있다면, GetBackgroundFlags 호출

    개략적인 압축 흐름
        PackFiles

    개략적인 압축해제 흐름
        OpenArchive
        ReadHeader 또는 ReadHeaderEx
        ProcessFile
        CloseArchive

    개략적인 리스트 흐름
        OpenArchive
        ReadHeaderEx
        CloseArchive

    개략적인 설정 흐름
        구현되어 있다면, ConfigurePacker 호출

    개략적인 삭제 흐름
        DeleteFiles    
*/


/*!
    플러그인 설치
*/
EXTERN_C UNARKWCX_API int WINAPI GetPackerCaps();
EXTERN_C UNARKWCX_API int WINAPI GetBackgroundFlags( void );

/*!
    플러그인 로딩
*/
EXTERN_C UNARKWCX_API void __stdcall PackSetDefaultParams( PackDefaultParamStruct* dps );

/*!
    공통
*/
EXTERN_C UNARKWCX_API HANDLE __stdcall OpenArchive( tOpenArchiveData *ArchiveData );
EXTERN_C UNARKWCX_API HANDLE __stdcall OpenArchiveW( tOpenArchiveDataW *ArchiveDataW );
EXTERN_C UNARKWCX_API int __stdcall ReadHeader( HANDLE hArcData, tHeaderData *HeaderData );
EXTERN_C UNARKWCX_API int __stdcall ReadHeaderEx( HANDLE hArcData, tHeaderDataEx *HeaderDataEx );
EXTERN_C UNARKWCX_API int __stdcall ReadHeaderExW( HANDLE hArcData, tHeaderDataExW *HeaderDataExW );

EXTERN_C UNARKWCX_API void __stdcall SetChangeVolProc( HANDLE hArcData, tChangeVolProc pChangeVolProc );
EXTERN_C UNARKWCX_API void __stdcall SetChangeVolProcW( HANDLE hArcData, tChangeVolProcW pfnChangeVolProcW );
EXTERN_C UNARKWCX_API void __stdcall SetProcessDataProc( HANDLE hArcData, tProcessDataProc pProcessDataProc );
EXTERN_C UNARKWCX_API void __stdcall SetProcessDataProcW( HANDLE hArcData, tProcessDataProcW pfnProcessDataProcW );

EXTERN_C UNARKWCX_API int __stdcall CloseArchive( HANDLE hArcData );
EXTERN_C UNARKWCX_API BOOL __stdcall CanYouHandleThisFile( char *FileName );
EXTERN_C UNARKWCX_API BOOL __stdcall CanYouHandleThisFileW( WCHAR *FileName );

EXTERN_C UNARKWCX_API void __stdcall ConfigurePacker( HWND Parent, HINSTANCE DllInstance );
/*!
    압축
*/
EXTERN_C UNARKWCX_API int __stdcall PackFiles( char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags );
EXTERN_C UNARKWCX_API int __stdcall PackFilesW( wchar_t* PackedFile, wchar_t* SubPath, wchar_t* SrcPath, wchar_t* AddList, int Flags );

/*!
    압축해제, 리스트
*/
EXTERN_C UNARKWCX_API int __stdcall ProcessFile( HANDLE hArcData, int Operation, char *DestPath, char *DestName );
EXTERN_C UNARKWCX_API int __stdcall ProcessFileW( HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName );

/*!
    압축삭제
*/
EXTERN_C UNARKWCX_API int __stdcall DeleteFiles( char* PackedFile, char* DeleteList );
EXTERN_C UNARKWCX_API int __stdcall DeleteFilesW( WCHAR* PackedFile, WCHAR* DeleteList );
