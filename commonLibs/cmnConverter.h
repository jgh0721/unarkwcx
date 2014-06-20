#pragma once

#include <crtdefs.h>

#include <string>
#include <vector>

#ifdef USE_COM_SUPPORT
#include <comutil.h>
#endif

#include "cmnUtils.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnConvert
    {
        namespace nsDetail
        {
            int                     getActualUTF8Length( const char* szUTF8 );
            int                     convertUTF8toUTF16( const char* pszUTF8, wchar_t* pwszUTF16 );
            int                     getBytesUTF16toUTF8( const wchar_t* pwszUTF16 );
            int                     convertUTF16toUTF8( const wchar_t* pwszUTF16, char* pszUTF8 );

            errno_t                 CopyAnsiString( char** pszDest, const char* pszSrc, size_t srcLength = -1 );
            errno_t                 CopyWideString( wchar_t** pszDest, const wchar_t* pszSrc, size_t srcLength = -1 );
            errno_t                 CopyUTF8String( char** pszDest, const char* pszSrc, int srcLength = -1 );
        }

//         void                        ConvertUpperString( std::string& strText );
//         void                        ConvertUpperString( std::wstring& strText );
#ifdef USE_COM_SUPPORT
        std::string                 ConvertToStringA( const VARIANT& vtItem, const std::string& strSep = "," );
        std::wstring                ConvertToStringW( const VARIANT& vtItem, const std::wstring& strSep = L"," );
#endif
        std::string                 string_replace_all( const std::string& src, const std::string& pattern, const std::string& replace );
        std::wstring                string_replace_all( const std::wstring& src, const std::wstring& pattern, const std::wstring& replace );

        // Double Null-Terminate 로 끝나는 문자열(REG_MULTISZ 등) 목록을 벡터<문자열> 로 변환
        std::vector< std::wstring > convertMultiSZToVector( const wchar_t* pwszText );
        // Double Null-Terminate 로 끝나는 문자열(REG_MULTISZ 등) 목록을 벡터<문자열> 로 변환
        template< typename T >
        std::vector< std::basic_string< T > > SplitMULTI_SZToVector( const T* pwszText )
        {
            const T* pchBuffer = pwszText;
            std::vector< std::basic_string<T> > vecRet;
            size_t bufferLength = 0;

            for( ;; )
            {
                if( pchBuffer == NULLPTR )
                    break;


                bufferLength = textlen( pchBuffer );
                if( bufferLength <= 0 )
                    break;

                vecRet.push_back( pchBuffer );
                pchBuffer += bufferLength;
            }

            return vecRet;
        }

        class CA2U
        {
        public:
            CA2U( const char* szText );
            CA2U( const std::string& strText );
            CA2U( const CA2U& converter );
            ~CA2U();

            CA2U&                   operator=(const char* szText);
            CA2U&                   operator=(const std::string& strText);
            CA2U&                   operator=(const CA2U& converter);

            operator const wchar_t*() const { return _szText != NULLPTR ? _szText : L"(NULL)"; };
            const wchar_t*          c_str() const { return this->operator const wchar_t *(); }
            const std::wstring      toStdString() { return _szText != NULLPTR ? _szText : L"(NULL)"; };

#if _MSC_VER >= 1600
            CA2U( CA2U&& converter );
            CA2U&                   operator=(CA2U&& converter);
#endif

#ifdef QSTRING_H
            CA2U( const QString& strText );
            const QString           toQtString() { return QString::fromUtf16( _szText != NULLPTR ? (const ushort*)_szText : (const ushort*)L"(NULL)" ); };
#endif

#ifdef _AFX
            CA2U( const CStringA& strText );
            const CStringW          toMFCString() { return CStringW( _szText != NULLPTR ? _szText : L"(NULL)" ); };
#endif
        private:
            size_t                  convert( const char* szText );

            wchar_t*                _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CU2A
        {
        public:
            CU2A( const wchar_t* szText );
            CU2A( const std::wstring& strText );
            CU2A( const CU2A& converter );
            ~CU2A();

            CU2A&                   operator=(const wchar_t* szText);
            CU2A&                   operator=(const std::wstring& strText);
            CU2A&                   operator=(const CU2A& converter);

            operator const char*() const { return _szText != NULLPTR ? _szText : "(NULL)"; };
            const char*             c_str() const { return this->operator const char *(); }
            const std::string       toStdString() { return _szText != NULLPTR ? _szText : "(NULL)"; };

#if _MSC_VER >= 1600
            CU2A( CU2A&& converter );
            CU2A&                   operator=(CU2A&& converter);
#endif

#ifdef QSTRING_H
            CU2A( const QString& strText );
            const QString           toQtString() { return QString::fromLocal8Bit( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif

#ifdef _AFX
            CU2A( const CStringW& strText );
            const CStringA          toMFCString() { return CStringA( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif
        private:
            size_t                  convert( const wchar_t* szText );

            char*                   _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CU82U
        {
        public:
            CU82U( const char* szText );
            CU82U( const std::string& strText );
            CU82U( const CU82U& converter );
            ~CU82U();

            CU82U&                  operator=(const char* szText);
            CU82U&                  operator=(const std::string& strText);
            CU82U&                  operator=(const CU82U& converter);

            operator const wchar_t*() const { return _szText != NULLPTR ? _szText : L"(NULL)"; };
            const wchar_t*          c_str() const { return this->operator const wchar_t *(); }
            const std::wstring      toStdString() { return _szText != NULLPTR ? _szText : L"(NULL)"; };

#if _MSC_VER >= 1600
            CU82U( CU82U&& converter );
            CU82U&                  operator=(CU82U&& converter);
#endif

#ifdef QSTRING_H
            CU82U( const QString& strText );
            const QString           toQtString() { return QString::fromUtf16( _szText != NULLPTR ? (const ushort*)_szText : (const ushort*)L"(NULL)" ); };
#endif

#ifdef _AFX
            CU82U( const CStringA& strText );
            const CStringW          toMFCString() { return CStringW( _szText != NULLPTR ? _szText : L"(NULL)" ); };
#endif
        private:
            size_t                  convert( const char* szText );

            wchar_t*                _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CU2U8
        {
        public:
            CU2U8( const wchar_t* szText );
            CU2U8( const std::wstring& strText );
            CU2U8( const CU2U8& converter );
            ~CU2U8();

            CU2U8&                  operator=(const wchar_t* szText);
            CU2U8&                  operator=(const std::wstring& strText);
            CU2U8&                  operator=(const CU2U8& converter);

            operator const char*() const { return _szText != NULLPTR ? _szText : "(NULL)"; };
            const char*             c_str() const { return this->operator const char *(); }
            const std::string       toStdString() { return _szText != NULLPTR ? _szText : "(NULL)"; };

#if _MSC_VER >= 1600
            CU2U8( CU2U8&& converter );
            CU2U8&                  operator=(CU2U8&& converter);
#endif

#ifdef QSTRING_H
            CU2U8( const QString& strText );
            const QString           toQtString() { return QString::fromUtf8( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif

#ifdef _AFX
            CU2U8( const CStringW& strText );
            const CStringA          toMFCString() { return CStringA( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif
        private:
            size_t                  convert( const wchar_t* szText );

            char*                   _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CA2U8
        {
        public:
            CA2U8( const char* szText );
            CA2U8( const std::string& strText );
            CA2U8( const CA2U8& converter );
            ~CA2U8();

            CA2U8&                  operator=(const char* szText);
            CA2U8&                  operator=(const std::string& strText);
            CA2U8&                  operator=(const CA2U8& converter);

            operator const char*() const { return _szText != NULLPTR ? _szText : "(NULL)"; };
            const char*            c_str() const { return this->operator const char *(); }
            const std::string      toStdString() { return _szText != NULLPTR ? _szText : "(NULL)"; };

#if _MSC_VER >= 1600
            CA2U8( CA2U8&& converter );
            CA2U8&                  operator=(CA2U8&& converter);
#endif

#ifdef QSTRING_H
            CA2U8( const QString& strText );
            const QString           toQtString() { return QString::fromUtf8( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif

#ifdef _AFX
            CA2U8( const CStringA& strText );
            const CStringA          toMFCString() { return CStringA( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif
        private:
            size_t                  convert( const char* szText );

            char*                   _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CU82A
        {
        public:
            CU82A( const char* szText );
            CU82A( const std::string& strText );
            CU82A( const CU82A& converter );
            ~CU82A();

            CU82A&                  operator=(const char* szText);
            CU82A&                  operator=(const std::string& strText);
            CU82A&                  operator=(const CU82A& converter);

            operator const char*() const { return _szText != NULLPTR ? _szText : "(NULL)"; };
            const char*            c_str() const { return this->operator const char *(); }
            const std::string      toStdString() { return _szText != NULLPTR ? _szText : "(NULL)"; };

#if _MSC_VER >= 1600
            CU82A( CU82A&& converter );
            CU82A&                  operator=(CU82A&& converter);
#endif

#ifdef QSTRING_H
            CU82A( const QString& strText );
            const QString           toQtString() { return QString::fromUtf8( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif

#ifdef _AFX
            CU82A( const CStringA& strText );
            const CStringA          toMFCString() { return CStringA( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif
        private:
            size_t                  convert( const char* szText );

            char*                   _szText;
        };

    }
}