#include "stdafx.h"

#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <vector>

#ifdef USE_FILEVER_CERTINFO_SUPPORT
    #include <Winver.h>
    #pragma comment(lib, "version")
    #include <WinCrypt.h>
    #include <SoftPub.h>
    #include <WinTrust.h>
    #pragma comment( lib, "WinTrust" )
    #include <WinCrypt.h>
    #pragma comment( lib, "crypt32" )
#endif

#include "cmnUtils.h"
#include "cmnPath.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnPath
    {
#ifdef USE_SHELL_SUPPORT
        std::wstring CanonicalizePath( const std::wstring& strPath )
        {
            std::vector< wchar_t > vecBuffer;
            vecBuffer.resize( strPath.size() + 1 );

            if( ::PathCanonicalizeW( &vecBuffer[ 0 ], strPath.c_str() ) == FALSE )
                return strPath;

            return &vecBuffer[ 0 ];
        }

        std::wstring GetCurrentPath( HMODULE hModule /* = NULL */ )
        {
            wchar_t wszBuffer[ MAX_PATH ] = { 0, };

            ::GetModuleFileNameW( hModule, wszBuffer, MAX_PATH );
            ::PathRemoveFileSpecW( wszBuffer );

            return CanonicalizePath( wszBuffer );
        }

        std::wstring GetShellFolders( int nFolder /* = CSIDL_LOCAL_APPDATA */ )
        {
            WCHAR wszFolderPath[ MAX_PATH ] = { 0, };

            LPITEMIDLIST pidl = NULLPTR;

            do
            {
                if( FAILED( ::SHGetSpecialFolderLocation( NULL, nFolder, &pidl ) ) )
                    break;

                if( ::SHGetPathFromIDListW( pidl, wszFolderPath ) == FALSE )
                    break;

            } while( false );

            if( pidl != NULLPTR )
            {
                LPMALLOC pMalloc = NULLPTR;

                SHGetMalloc( &pMalloc );
                if( pMalloc != NULLPTR )
                {
                    pMalloc->Free( pidl );
                    pMalloc->Release();
                }
            }

            return wszFolderPath;
        }

        std::wstring GetDefaultBrowserPath()
        {
            WCHAR szPath[ MAX_PATH ] = { 0, };
            HFILE hFile = _lcreat( "dummy.htm", 0 );

            do 
            {
                if( hFile == HFILE_ERROR )
                    break;

                if( reinterpret_cast<DWORD>(FindExecutableW( L"dummy.htm", NULL, szPath )) <= 32 )
                    break;

            } while (false);


            if( hFile != HFILE_ERROR )
            {
                _lclose( hFile );
                DeleteFileW( L"dummy.htm" );
            }

            return std::wstring( szPath );
        }    

        std::wstring SelectBrowseFolder( const std::wstring& titleName /* = L"" */ )
        {
            std::wstring retVal;
            TCHAR pszFolderName[ MAX_PATH ] = { 0, };

            BROWSEINFO bi;
            ZeroMemory( &bi, sizeof(BROWSEINFO) );

            bi.hwndOwner = NULL;
            bi.pidlRoot = NULL;
            bi.pszDisplayName = pszFolderName;
            bi.lpszTitle = titleName.c_str();
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_EDITBOX;

            PIDLIST_ABSOLUTE pidl = SHBrowseForFolder( &bi );

            ZeroMemory( pszFolderName, sizeof(TCHAR)* MAX_PATH );

            if( SHGetPathFromIDList( pidl, pszFolderName ) != FALSE )
                retVal = pszFolderName;

            return retVal;
        }

#endif

#ifdef USE_FILEVER_CERTINFO_SUPPORT
        std::wstring GetFileInfomation( const std::wstring& filePath, const tagInformationType selectInfo )
        {
            if( filePath.empty() == true )
                return L"";

            std::wstring fileInfo;

            detail::LANGCODEPAGE* lpLang = NULL;

            UINT uiTranslate = 0;
            UINT dwResultLenth = 0;

            WCHAR* pszResult = NULL;
            WCHAR* pszVer = NULL;
            WCHAR  szSub[ MAX_PATH ];

            INT iVerSize = ::GetFileVersionInfoSizeW( filePath.c_str(), NULL );

            if( iVerSize == 0 )
                return fileInfo;

            pszVer = (WCHAR*)malloc( sizeof(WCHAR)* iVerSize );
            memset( pszVer, 0, sizeof(WCHAR)* iVerSize );

            try
            {
                if( ::GetFileVersionInfoW( filePath.c_str(), 0, iVerSize, OUT pszVer ) == FALSE )
                {
                    free( pszVer );
                    return fileInfo;
                }

                if( ::VerQueryValueW( pszVer, L"\\VarFileInfo\\Translation", OUT( LPVOID * )&lpLang, OUT &uiTranslate ) == FALSE )
                {
                    free( pszVer );
                    return fileInfo;
                }

                switch( selectInfo )
                {
                    case COMMENTS:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\Comments", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  INTERNALNAME:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\InternalName", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  PRODUCTNAME:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\ProductName", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  COMPANYNAME:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\CompanyName", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  LEGALCOPYRIGHT:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\LegalCopyright", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  PRODUCTVERSION:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\ProductVersion", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  FILEDESCRIPTION:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\FileDescription", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  LEGALTRADEMARKS:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\LegalTrademarks", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  PRIVATEBUILD:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\PrivateBuild", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  FILEVERSION:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\FileVersion", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  ORIGINALFILENAME:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\OriginalFilename", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    case  SPECIALBUILD:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\SpecialBuild", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                    default:
                        wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\ProductName", lpLang[ 0 ].wLanguage, lpLang[ 0 ].wCodePage );
                        break;
                }

                if( ::VerQueryValueW( pszVer, szSub, OUT( LPVOID* )&pszResult, OUT &dwResultLenth ) == FALSE )
                {
                    free( pszVer );
                    return fileInfo;
                }

                if( pszResult != NULL )
                    fileInfo = pszResult;

                free( pszVer );
            }
            catch( ... )
            {
                free( pszVer );
                return fileInfo;
            }

            return trim_right( fileInfo );
        }

    #ifndef QT_QTCORE_MODULE_H
        CCertInformation::CCertInformation( )
        {
        }

        CCertInformation::CCertInformation( const std::wstring& filePath )
        {
            sFilePath = filePath;
        }

        CCertInformation::~CCertInformation( )
        {
        }

        bool CCertInformation::HasDigitalSignature( )
        {
            bool isHas = false;
            LONG lStatus = 0;
            DWORD dwLastError = 0;

            GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
            WINTRUST_DATA WinTrustData;

            do
            {
                if( sFilePath.empty( ) == true )
                    break;

                // MSDN 의 WinVerifyTrust 함수 예제코드에서 발췌
                WINTRUST_FILE_INFO wfi;
                memset( &wfi, '\0', sizeof(wfi) );
                wfi.cbStruct = sizeof(WINTRUST_FILE_INFO);
                wfi.pcwszFilePath = sFilePath.c_str( );
                wfi.hFile = NULL;
                wfi.pgKnownSubject = NULL;

                /*
                WVTPolicyGUID specifies the policy to apply on the file
                WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:

                1) The certificate used to sign the file chains up to a root
                certificate located in the trusted root certificate store. This
                implies that the identity of the publisher has been verified by
                a certification authority.

                2) In cases where user interface is displayed (which this example
                does not do), WinVerifyTrust will check for whether the
                end entity certificate is stored in the trusted publisher store,
                implying that the user trusts content from this publisher.

                3) The end entity certificate has sufficient permission to sign
                code, as indicated by the presence of a code signing EKU or no
                EKU.
                */

                // Initialize the WinVerifyTrust input data structure.

                // Default all fields to 0.
                memset( &WinTrustData, 0, sizeof(WinTrustData) );
                WinTrustData.cbStruct = sizeof(WinTrustData);

                WinTrustData.pPolicyCallbackData = NULL;
                WinTrustData.pSIPClientData = NULL;
                WinTrustData.dwUIChoice = WTD_UI_NONE;
                WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
                WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
                WinTrustData.dwStateAction = WTD_STATEACTION_IGNORE;
                WinTrustData.hWVTStateData = NULL;
                // Not used.
                WinTrustData.pwszURLReference = NULL;

                // This is not applicable if there is no UI because it changes 
                // the UI to accommodate running applications instead of 
                // installing applications.
                WinTrustData.dwUIContext = WTD_UICONTEXT_EXECUTE;

                // Set pFile.
                WinTrustData.pFile = &wfi;

                // WinVerifyTrust verifies signatures as specified by the GUID 
                // and Wintrust_Data.
                lStatus = WinVerifyTrust( NULL, &WVTPolicyGUID, &WinTrustData );
                switch( lStatus )
                {
                    case ERROR_SUCCESS:
                    case TRUST_E_EXPLICIT_DISTRUST:
                    case TRUST_E_SUBJECT_NOT_TRUSTED:
                    case CERT_E_EXPIRED:
                        isHas = true;
                        break;
                    case TRUST_E_NOSIGNATURE:
                    case TRUST_E_SUBJECT_FORM_UNKNOWN:
                    case TRUST_E_PROVIDER_UNKNOWN:
                        isHas = false;
                        break;
                }

            } while( false );

            WinTrustData.dwUIChoice = WTD_UI_NONE;
            WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
            WinVerifyTrust( (HWND)INVALID_HANDLE_VALUE, &WVTPolicyGUID, &WinTrustData );

            return isHas;
        }

        bool CCertInformation::HasValidDigitalSignature( CERT_INFORMATION& certInfo, SYSTEMTIME& certDateTime )
        {
            bool isHasValidSignature = false;

            GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
            WINTRUST_FILE_INFO  sWintrustFileInfo;
            WINTRUST_DATA       sWintrustData;
            HRESULT hr;

            do
            {
                memset( (VOID*)&sWintrustFileInfo, '\0', sizeof(WINTRUST_FILE_INFO) );
                memset( (VOID*)&sWintrustData, '\0', sizeof(WINTRUST_DATA) );

                sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
                sWintrustFileInfo.pcwszFilePath = sFilePath.c_str( );
                sWintrustFileInfo.hFile = NULL;

                sWintrustData.cbStruct = sizeof(WINTRUST_DATA);
                sWintrustData.dwUIChoice = WTD_UI_NONE;
                sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
                sWintrustData.dwUnionChoice = WTD_CHOICE_FILE;
                sWintrustData.pFile = &sWintrustFileInfo;
                sWintrustData.dwStateAction = WTD_STATEACTION_VERIFY;

                hr = WinVerifyTrust( (HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData );

                if( hr == TRUST_E_NOSIGNATURE ||
                    hr == TRUST_E_BAD_DIGEST ||
                    hr == TRUST_E_PROVIDER_UNKNOWN ||
                    (hr != CERT_E_EXPIRED && hr != S_OK) )
                {
                    break;
                }

                CRYPT_PROVIDER_SGNR*        psProvSigner = NULL;
                CRYPT_PROVIDER_CERT*        psProvCert = NULL;
                CRYPT_PROVIDER_DATA const*  psProvData = WTHelperProvDataFromStateData( sWintrustData.hWVTStateData );
                if( psProvData == NULLPTR )
                    break;

                psProvSigner = WTHelperGetProvSignerFromChain( (PCRYPT_PROVIDER_DATA)psProvData, 0, FALSE, 0 );
                if( psProvSigner == NULLPTR )
                    break;

                if( psProvSigner->dwSignerType == SGNR_TYPE_TIMESTAMP ) // Type 이 해당 타입이 아니면 현재 시간이 들어있음
                {
                    FILETIME            localFt;
                    SYSTEMTIME          sysTime;

                    FileTimeToLocalFileTime( &psProvSigner->sftVerifyAsOf, &localFt );
                    FileTimeToSystemTime( &localFt, &sysTime );

                    certDateTime = sysTime;
                }

                psProvCert = WTHelperGetProvCertFromChain( psProvSigner, 0 );
                if( psProvCert == NULLPTR )
                    break;

                certInfo = GetCertificateDescription( psProvCert->pCert );

                if( hr == S_OK )
                    isHasValidSignature = true;
                else
                    isHasValidSignature = false;
            } while( false );

            sWintrustData.dwUIChoice = WTD_UI_NONE;
            sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
            WinVerifyTrust( (HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData );

            return isHasValidSignature;
        }

        CERT_INFORMATION CCertInformation::GetCertificateDescription( PCCERT_CONTEXT pCertCtx )
        {
            CERT_INFORMATION certInfo;
            std::string sAlgorithm;

            certInfo.sigVersion = pCertCtx->pCertInfo->dwVersion;
            sAlgorithm = pCertCtx->pCertInfo->SignatureAlgorithm.pszObjId;
            certInfo.signatureAlgorithm.assign( sAlgorithm.begin( ), sAlgorithm.end( ) );
            certInfo.serialNo = 1;

            {
                FILETIME            localFt;
                SYSTEMTIME          sysTime;
                FileTimeToLocalFileTime( &pCertCtx->pCertInfo->NotBefore, &localFt );
                FileTimeToSystemTime( &localFt, &sysTime );

                certInfo.notbefore = sysTime;
            }

            {
                FILETIME            localFt;
                SYSTEMTIME          sysTime;
                FileTimeToLocalFileTime( &pCertCtx->pCertInfo->NotAfter, &localFt );
                FileTimeToSystemTime( &localFt, &sysTime );

                certInfo.notafter = sysTime;
            }

            TCHAR wszBuffer[ 512 ] = { 0, };
            CertNameToStr( X509_ASN_ENCODING, &pCertCtx->pCertInfo->Issuer, CERT_X500_NAME_STR, wszBuffer, 512 );
            certInfo.issuer = wszBuffer;

            ZeroMemory( wszBuffer, sizeof(TCHAR)* 512 );
            CertNameToStr( X509_ASN_ENCODING, &pCertCtx->pCertInfo->Subject, CERT_X500_NAME_STR, wszBuffer, 512 );
            certInfo.subject = wszBuffer;

            return certInfo;
        }
    #endif

#endif

        bool IsFileExists( const std::wstring& filePath )
        {
            _wfinddatai64_t c_file;
            intptr_t hFile;
            bool result = false;

            if( (hFile = _wfindfirsti64( filePath.c_str(), &c_file )) == -1L )
                result = false; // 파일 없으면 거짓 반환
            else
            if( c_file.attrib & _A_SUBDIR )
                result = false; // 있어도 디렉토리면 거짓
            else
                result = true; // 그밖의 경우는 "존재하는 파일"이기에 참

            _findclose( hFile );

            return result;
        }

        bool IsDirExists( const std::wstring& dirPath )
        {
            _wfinddatai64_t c_file;
            intptr_t hFile;
            bool result = false;

            hFile = _wfindfirsti64( dirPath.c_str(), &c_file );
            if( hFile == -1L )
                return result;
            if( c_file.attrib & _A_SUBDIR )
                result = true;
            _findclose( hFile );

            return result;
        }

        bool CreateDirectoryRecursively( const wchar_t* wszDirectory )
        {
            bool isSuccess = false;
            const unsigned int MAX_FOLDER_DEPTH = 122;
            const size_t WIN_SHELL_MAX_PATH = 260;

            do
            {
                size_t nLen = wcslen( wszDirectory );
                if( nLen < (_MAX_DRIVE + 1) || nLen > ( WIN_SHELL_MAX_PATH - 1 ) )
                    break;

                wchar_t szPath[ WIN_SHELL_MAX_PATH ] = { 0, };
                intptr_t offset = _MAX_DRIVE;

                for( int i = 0; i < MAX_FOLDER_DEPTH; i++ )
                {
                    if( (uintptr_t)offset >= nLen )
                        break;

                    const wchar_t * pos = wszDirectory + offset;
                    const wchar_t * dst = wcschr( pos, L'\\' );

                    if( dst == NULL )
                    {
                        wcsncpy_s( szPath, WIN_SHELL_MAX_PATH, wszDirectory, nLen );
                        i = MAX_FOLDER_DEPTH;
                    }
                    else
                    {
                        intptr_t cnt = dst - wszDirectory;
                        wcsncpy_s( szPath, WIN_SHELL_MAX_PATH, wszDirectory, cnt );

                        offset = cnt + 1;
                    }
                    isSuccess = _wmkdir( szPath ) != 0 ? true : false;
                    if( isSuccess == false )
                        isSuccess = IsDirExists( szPath );
                }

            } while( false );

            return isSuccess;
        }

        std::wstring getFileExtension( std::wstring sFilePath )
        {
            wchar_t szFileExt[ nsCommon::MAX_PATH_LENGTH ] = { 0, };
            _wsplitpath_s( sFilePath.c_str(), NULL, 0, NULL, 0, NULL, 0, szFileExt, nsCommon::MAX_PATH_LENGTH - 1 );
            return szFileExt;
        }

        __int64 getFileSize( const std::wstring& sFilePath )
        {
            __int64 retFileSize = 0;
            do 
            {
#ifdef _WINDOWS_
                WIN32_FILE_ATTRIBUTE_DATA fileData;
                if( GetFileAttributesExW( sFilePath.c_str(), GetFileExInfoStandard, &fileData ) == 0 )
                    break;

                retFileSize = fileData.nFileSizeLow | ((__int64)fileData.nFileSizeHigh << 32);
#else
                _stat64 stat;
                if( _wstat64( sFilePath.c_str(), &stat ) < 0 )
                    break;

                retFileSize = stat.st_size;
#endif
            } while( false );

            return retFileSize;
        }

        std::wstring ConvertSizeToString( size_t size )
        {
            WCHAR wszBuf[ 30 ] = { 0, };

            static const WCHAR* SIZES[] = { L"B", L"KB", L"MB", L"GB", L"TB", L"PB", L"EB" };

            size_t div = 0;
            size_t rem = 0;

            while( size >= 1024 && div < ( sizeof SIZES / sizeof *SIZES ) )
            {
                rem = ( size % 1024 );
                div++;
                size /= 1024;
            }

            swprintf( wszBuf, 30, L"%0.2f %s", ( float )size + ( float )rem / 1024.0, SIZES[ div ] );

            return std::wstring( wszBuf );
        }

    } // nsCmnPath

} // nsCommon
