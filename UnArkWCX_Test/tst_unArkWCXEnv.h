#pragma once

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "Ark.h"
#include "wcxhead.h"

#pragma execution_character_set( "utf-8" )

EXTERN_C typedef void( __stdcall *pfnPackSetDefaultParams )(PackDefaultParamStruct* dps);
EXTERN_C typedef HANDLE( __stdcall *pfnOpenArchiveW )(tOpenArchiveDataW* ArchiveDataW);
EXTERN_C typedef int( __stdcall *pfnReadHeaderExW )(HANDLE hArcData, tHeaderDataExW* HeaderDataExW);
EXTERN_C typedef void( __stdcall *pfnSetChangeVolProcW )(HANDLE hArcData, tChangeVolProcW pfnChangeVolProcW);
EXTERN_C typedef void( __stdcall *pfnSetProcessDataProcW )(HANDLE hArcData, tProcessDataProcW pfnProcessDataProcW);
EXTERN_C typedef void( __stdcall *pfnCloseArchive )(HANDLE hArcData);
EXTERN_C typedef BOOL( __stdcall *pfnCanYouHandleThisFileW )(WCHAR* FileName);
EXTERN_C typedef void( __stdcall *pfnConfigurePacker )(HWND Parent, HINSTANCE DllInstance);
EXTERN_C typedef int( __stdcall *pfnPackFilesW )(wchar_t* PackedFile, wchar_t* SubPath, wchar_t* SrcPath, wchar_t* AddList, int Flags);
EXTERN_C typedef int( __stdcall *pfnProcessFileW )(HANDLE hArcData, int Operation, WCHAR* pwszDestPath, WCHAR* pwszDestName);

class TST_GlobalEnvironment : 
    public ::testing::Environment
{
public:
    virtual ~TST_GlobalEnvironment();
    virtual void SetUp();
    virtual void TearDown();
};

class CUnArkMgr
{
public:
    CUnArkMgr();
    virtual ~CUnArkMgr();

    bool                Init();
    void                Uninit();

    bool                SetCompressionFileFormats( ARK_FF fileFormat );
    bool                CallPackSetDefaultParams();
    bool                CallPackFiles( const std::wstring& packedFile, const std::wstring& subPath, const std::wstring& srcPath, const std::vector< std::wstring >& addList, int Flags );

private:
    wchar_t*                                convertVectorToAddList( const std::vector< std::wstring >& vecAddList );

    HMODULE                                 hUnArkWCX;
    pfnPackSetDefaultParams                 fnPackSetDefaultParams;
    pfnOpenArchiveW                         fnOpenArchiveW;
    pfnReadHeaderExW                        fnReadHeaderExW;
    pfnSetChangeVolProcW                    fnSetChangeVolProcW;
    pfnSetProcessDataProcW                  fnSetProcessDataProcW;
    pfnCloseArchive                         fnCloseArchive;
    pfnCanYouHandleThisFileW                fnCanYouHandleThisFileW;
    pfnConfigurePacker                      fnConfigurePacker;
    pfnPackFilesW                           fnPackFilesW;
    pfnProcessFileW                         fnProcessFileW;
};

class TST_Extract : 
    public ::testing::Test, public CUnArkMgr
{
public:
    virtual void SetUp();
    virtual void TearDown();

private:
};

class TST_Pack :
    public ::testing::Test, public CUnArkMgr
{
public:
    virtual void SetUp();
    virtual void TearDown();

    void                SetPackerToZIP();
    void                SetPackerToISO();
    void                SetPackerToLZH();
    void                SetPackerTo7Z();

private:

};