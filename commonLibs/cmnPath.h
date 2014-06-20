#pragma once

#include <string>

#ifdef USE_FILEVER_CERTINFO_SUPPORT
#include <WinCrypt.h>
#endif

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnPath
    {
        namespace detail
        {
#ifdef USE_FILEVER_CERTINFO_SUPPORT
            struct LANGCODEPAGE
            {
                WORD wLanguage;
                WORD wCodePage;
            };
#endif
        }

#ifdef USE_SHELL_SUPPORT
        std::wstring                CanonicalizePath( const std::wstring& strPath );
        std::wstring                GetCurrentPath( HMODULE hModule = NULL );
        std::wstring                GetShellFolders( int nFolder = 0x001c /*CSIDL_LOCAL_APPDATA*/ );
        std::wstring                GetDefaultBrowserPath();
        /*!
            찾아보기 대화상자를 출력하여 특정 폴더를 선택 후 선택된 폴더 경로를 반환한다.

            @return 선택된 폴더 경로
            @return 빈 문자열, 사용자가 대화상자를 취소했을 때
        */
        std::wstring                SelectBrowseFolder( const std::wstring& titleName = L"" );
#endif

#ifdef USE_FILEVER_CERTINFO_SUPPORT
        // 파일의 버전 정보 및 회사 정보를 구할수 있다.
        enum tagInformationType 
        { COMMENTS, INTERNALNAME, PRODUCTNAME,
        COMPANYNAME,LEGALCOPYRIGHT, PRODUCTVERSION,
        FILEDESCRIPTION, LEGALTRADEMARKS, PRIVATEBUILD,
        FILEVERSION, ORIGINALFILENAME, SPECIALBUILD };

        std::wstring                GetFileInfomation( const std::wstring& filePath, const tagInformationType selectInfo = PRODUCTVERSION );

#ifndef QT_QTCORE_MODULE_H
        typedef struct tagCertInfo
        {
            DWORD			sigVersion;
            DWORD			serialNo;
            std::wstring	signatureAlgorithm;
            std::wstring	issuer;         // 인증서를 발급한 기관/사람
            SYSTEMTIME		notbefore;
            SYSTEMTIME		notafter;
            std::wstring	subject;        // 인증서를 발급받은 기관/사람

        } CERT_INFORMATION, *PCERT_INFORMATION;

        class CCertInformation
        {
        public:
            CCertInformation();
            CCertInformation( const std::wstring& filePath );
            ~CCertInformation();

            bool HasDigitalSignature();
            bool HasValidDigitalSignature( CERT_INFORMATION& certInfo, SYSTEMTIME& certDateTime );

        private:
            CERT_INFORMATION GetCertificateDescription( PCCERT_CONTEXT pCertCtx );
            std::wstring	sFilePath;
        };
#endif

#endif

        bool                        IsFileExists( const std::wstring& filePath );
        bool                        IsDirExists( const std::wstring& dirPath );
        bool                        CreateDirectoryRecursively( const wchar_t* wszDirectory );
        std::wstring                getFileExtension( std::wstring sFilePath );
        __int64                     getFileSize( const std::wstring& sFilePath );
        std::wstring                ConvertSizeToString( size_t size );
    }
}