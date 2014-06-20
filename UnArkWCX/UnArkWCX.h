#pragma once

#include <boost/atomic.hpp>

#include "wcxhead.h"
#include "ArkLib.h"
#include "UnArkWCX_Export.h"

#pragma execution_character_set( "utf-8" )

#define WCX_ANSI_MAX_PATH 260 
#define WCX_WIDE_MAX_PATH 1024

extern HINSTANCE g_hInst;
extern WCHAR gArkDLLFullPathName[ MAX_PATH ];
extern WCHAR gConfigureINIFullPath[ MAX_PATH ];

const wchar_t* const CONFIGURE_INI_FILENAME = L"pkplugin.INI";
const wchar_t* const CONFIGURE_XML_FILENAME = L"UnArkWCX.xml";

// const wchar_t* const CONFIGURE_INI_SECNAME = L"UnArkWCX";

/*
    토탈커맨더 플러그인인 WCX 의 수행에 있어서 필수적인 함수들 선언
*/

extern SArkCompressorOpt                gArkCompressorOpt;

// extern WCHAR gCurrentArchiveExtension[ 32 ];            // 압축파일을 생성할 때 사용할 확장자

// void InitLogger();
// void SetCompressorOptFromINI();

class CArkWCX
{
public:
    CArkWCX();
    ~CArkWCX();

    bool                                    Init( bool isCreateCompressor = false );

    IArk*                                   GetArk();
    IArkCompressor*                         GetArkCompressor();

    void                                    SetArkEvent();
    void                                    ResetArkEvent();

    //////////////////////////////////////////////////////////////////////////
    /// DeCompress, List 
    //////////////////////////////////////////////////////////////////////////
    BOOL32                                  OpenW( LPCWSTR pwszFilePath, LPCWSTR password );
    int                                     IncrementCurrentFileIndex();
    int                                     GetCurrentFileIndex();

    //////////////////////////////////////////////////////////////////////////
    /// Compress
    //////////////////////////////////////////////////////////////////////////

private:
    CArkLib                                 _arkLib;
    IArkCompressor*                         _arkCompressor;
    boost::atomic_int                       _currentFileIndex;
};

class CArkEvent : public IArkEvent
{
public:
    CArkEvent() : pfnChangeVolProc( NULL ), pfnChangeVolProcW( NULL ), pfnProcessDataProc( NULL ), pfnProcessDataProcW( NULL ) {};

    ARKMETHOD( void )   OnOpening( const SArkFileItem* pFileItem, float progress, BOOL& bStop ) {}

    ARKMETHOD( void )   OnStartFile( const SArkFileItem* pFileItem, BOOL& bStopCurrent, BOOL& bStopAll, int index ) {}
    ARKMETHOD( void )   OnProgressFile( const SArkProgressInfo* pProgressInfo, BOOL& bStopCurrent, BOOL& bStopAll )
    {
        int bContinue = TRUE;

        if( pfnProcessDataProcW != NULL )
        {
            bContinue = pfnProcessDataProcW( NULL, pProgressInfo->_processed );
        }
        else if( pfnProcessDataProc != NULL )
        {
            bContinue = pfnProcessDataProc( NULL, pProgressInfo->_processed );
        }

        if( bContinue == FALSE )
        {
            bStopAll = TRUE;
            bStopCurrent = TRUE;
        }

    }

    ARKMETHOD( void )   OnCompleteFile( const SArkProgressInfo* pProgressInfo, ARKERR nErr ) {}

    ARKMETHOD( void )   OnError( ARKERR nErr, const SArkFileItem* pFileItem, BOOL bIsWarning, BOOL& bStopAll )
    {

    }

    ARKMETHOD( void )   OnMultiVolumeFileChanged( LPCWSTR szPathFileName )
    {
        if( pfnChangeVolProcW != NULL )
        {
            WCHAR* pFilePath = _wcsdup( szPathFileName );

            pfnChangeVolProcW( pFilePath, PK_VOL_NOTIFY );

            free( pFilePath );
        }
        else if( pfnChangeVolProc != NULL )
        {
            char* pFilePath = _strdup( nsCommon::nsCmnConvert::CU2A( szPathFileName ).c_str() );

            pfnChangeVolProc( pFilePath, PK_VOL_NOTIFY );

            free( pFilePath );
        }
    }

    ARKMETHOD( void )   OnAskOverwrite( const SArkFileItem* pFileItem, LPCWSTR szLocalPathName, ARK_OVERWRITE_MODE& overwrite, WCHAR pathName2Rename[ ARK_MAX_PATH ] ) {}
    ARKMETHOD( void )   OnAskPassword( const SArkFileItem* pFileItem, ARK_PASSWORD_ASKTYPE askType, ARK_PASSWORD_RET& ret, WCHAR passwordW[ ARK_MAX_PASS ] ) {}

    tProcessDataProcW   pfnProcessDataProcW;
    tProcessDataProc    pfnProcessDataProc;

    tChangeVolProcW     pfnChangeVolProcW;
    tChangeVolProc      pfnChangeVolProc;
};

extern CArkEvent                               gArkEvent;
