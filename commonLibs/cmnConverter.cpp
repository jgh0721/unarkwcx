#include "stdafx.h"

#include <cassert>
#include <string>
#include <algorithm>
#include <locale>

#include "cmnUtils.h"
#include "cmnConverter.h"

#pragma execution_character_set( "utf-8" )

using namespace nsCommon::nsCmnConvert::nsDetail;

namespace nsCommon
{
    namespace nsCmnConvert
    {
        namespace nsDetail
        {
            int getActualUTF8Length( const char* szUTF8 )
            {
                int nLength = 0;
                while( szUTF8 != NULLPTR && *szUTF8 )
                    nLength += (*szUTF8++ & 0xc0) != 0x80;

                return nLength;
            }

            int convertUTF8toUTF16( const char* pszUTF8, wchar_t* pwszUTF16 )
            {
                assert( pszUTF8 != NULLPTR );
                assert( pwszUTF16 != NULLPTR );

                int nRetSize = 0;
                int iIndex = 0;
                wchar_t wChar;

                while( 0 != pszUTF8[ iIndex ] )
                {
                    if( (0xE0 == (pszUTF8[ iIndex ] & 0xE0)) )
                    {
                        wChar = ((pszUTF8[ iIndex ] & 0x0f) << 12) |
                            ((pszUTF8[ iIndex + 1 ] & 0x3F) << 6) |
                            (pszUTF8[ iIndex + 2 ] & 0x3F);

                        iIndex += 3;
                    }
                    else if( 0xC0 == (pszUTF8[ iIndex ] & 0xC0) )
                    {
                        wChar = ((pszUTF8[ iIndex ] & 0x1F) << 6) |
                            (pszUTF8[ iIndex + 1 ] & 0x3F);

                        iIndex += 2;
                    }
                    else
                    {
                        wChar = pszUTF8[ iIndex ] & 0x7F;

                        iIndex++;
                    }

                    pwszUTF16[ nRetSize ] = wChar;

                    nRetSize++;
                }

                pwszUTF16[ nRetSize ] = 0;

                return nRetSize;
            }

            int getBytesUTF16toUTF8( const wchar_t* pwszUTF16 )
            {
                assert( pwszUTF16 != NULLPTR );

                int nBytes = 1; // NULL - Terminating 을 위한 1바이트 미리 잡음
                const wchar_t* wszBuffer = pwszUTF16;

                while( *wszBuffer != L'\0' )
                {
                    if( *wszBuffer < 0x80 )
                        nBytes += 1;
                    else if( *wszBuffer < 0x800 )
                        nBytes += 2;
                    else
                        nBytes += 3;

                    wszBuffer++;
                }

                return nBytes;
            }

            int convertUTF16toUTF8( const wchar_t* pwszUTF16, char* pszUTF8 )
            {
                assert( pwszUTF16 != NULLPTR );
                assert( pszUTF8 != NULLPTR );

                int nTotalBytes = 0;
                int nBytes = -1;
                unsigned char szBytes[ 4 ] = { 0, };
                wchar_t wChar;
                const wchar_t* pwszBuffer = pwszUTF16;

                while( *pwszBuffer != L'\0' )
                {
                    wChar = *pwszBuffer;

                    if( wChar < 0x80 )
                    {
                        nBytes = 1;
                        szBytes[ 0 ] = (unsigned char)wChar;
                    }
                    else if( wChar < 0x800 )
                    {
                        nBytes = 2;
                        szBytes[ 1 ] = (wChar & 0x3f) | 0x80;
                        szBytes[ 0 ] = (((wChar << 2) & 0xcf00) | 0xc000) >> 8;
                    }
                    else
                    {
                        nBytes = 3;
                        szBytes[ 2 ] = (wChar & 0x3f) | 0x80;
                        szBytes[ 1 ] = (((wChar << 2) & 0x3f00) | 0x8000) >> 8;
                        szBytes[ 0 ] = (((wChar << 4) & 0x3f0000) | 0xe00000) >> 16;
                    }

                    for( int j = 0; j < nBytes; ++j )
                        pszUTF8[ nTotalBytes++ ] = szBytes[ j ];

                    pwszBuffer++;
                }

                pszUTF8[ nTotalBytes ] = '\0';

                return nTotalBytes;
            }

            errno_t CopyAnsiString( char** pszDest, const char * pszSrc, size_t srcLength /* = -1 */ )
            {
                assert( pszSrc != NULLPTR );
                assert( *pszDest == NULLPTR );

                size_t nSize = srcLength == -1 ? strlen( pszSrc ) + 1 : srcLength + 1;
                *pszDest = new char[ nSize ];
                memset( *pszDest, '\0', nSize * sizeof(char) );
                return strcpy_s( *pszDest, nSize, pszSrc );
            }

            errno_t CopyWideString( wchar_t** pszDest, const wchar_t * pszSrc, size_t srcLength /* = -1 */ )
            {
                assert( pszSrc != NULLPTR );
                assert( *pszDest == NULLPTR );

                size_t nSize = srcLength == -1 ? wcslen( pszSrc ) + 1 : srcLength + 1;
                *pszDest = new wchar_t[ nSize ];
                memset( *pszDest, '\0', nSize * sizeof(wchar_t) );
                return wcscpy_s( *pszDest, nSize, pszSrc );
            }

            errno_t CopyUTF8String( char** pszDest, const char* pszSrc, int srcLength /* = -1 */ )
            {
                assert( pszSrc != NULLPTR );
                assert( pszDest == NULLPTR );

                // TODO: 완성 필요
                return srcLength == -1 ? getActualUTF8Length( pszSrc ) : srcLength + 1;
            }

        } // nsCmnConvertDetail

//         void ConvertUpperString( std::string& strText )
//         {
//             std::transform( strText.begin(), strText.end(), strText.begin(), (int( *)(int))std::toupper );
//         }
// 
//         void ConvertUpperString( std::wstring& strText )
//         {
//             std::transform( strText.begin(), strText.end(), strText.begin(), (int( *)(int))std::toupper );
//         }

#ifdef USE_COM_SUPPORT
        std::string ConvertToStringA( const VARIANT& vtItem, const std::string& strSep /* = "," */ )
        {
            switch( vtItem.vt )
            {
                case VT_EMPTY:
                    return "";
                case VT_NULL:
                    return "(null)";
                    break;
                case VT_I2:
                    return format( "%d", vtItem.iVal );
                    break;
                case VT_I2 | VT_BYREF:
                    return format( "%d", *(vtItem.piVal) );
                    break;
                case VT_I4:
                    return format( "%d", vtItem.lVal );
                    break;
                case VT_I4 | VT_BYREF:
                    return format( "%d", *(vtItem.plVal) );
                    break;
                case VT_R4:
                    return format( "%f", vtItem.fltVal );
                    break;
                case VT_R4 | VT_BYREF:
                    return format( "%f", *(vtItem.pfltVal) );
                    break;
                case VT_R8:
                    return format( ("%f"), vtItem.dblVal );
                    break;
                case VT_R8 | VT_BYREF:
                    return format( ("%f"), *(vtItem.pdblVal) );
                    break;
                case VT_CY:
                case VT_CY | VT_BYREF:
                {
#ifdef _AFX
                    return CW2A( COleCurrency( vtItem ).Format().GetString() ).m_psz;
#else
                    _variant_t vtData( vtItem );
                    vtData.ChangeType( VT_BSTR, NULL );
                    return _com_util::ConvertBSTRToString( vtData.bstrVal );
#endif
                    break;
                }
                case VT_DATE:
                case VT_DATE | VT_BYREF:
                {
#ifdef _AFX
                    return CW2A( COleDateTime( vtItem ).Format( _T( "%Y-%m-%d %H-%M-%S" ) ).GetString() ).m_psz;
#else
                    _variant_t vtData( vtItem );
                    vtData.ChangeType( VT_BSTR, NULL );
                    return _com_util::ConvertBSTRToString( vtData.bstrVal );
#endif
                }

                    break;
                case VT_BSTR:
                {
                                LPSTR pszTemp = NULL;
                                std::string strResult;
                                pszTemp = _com_util::ConvertBSTRToString( V_BSTR( &vtItem ) );
                                strResult = pszTemp;
                                delete[] pszTemp;
                                return strResult;
                }
                    break;
                case VT_BSTR | VT_BYREF:
                {
                                           LPSTR pszTemp = NULL;
                                           std::string strResult;
                                           pszTemp = _com_util::ConvertBSTRToString( *(V_BSTRREF( &vtItem )) );
                                           strResult = pszTemp;
                                           delete[] pszTemp;
                                           return strResult;
                }
                    break;
                case VT_BSTR | VT_ARRAY:
                {
                                           long lUpper = 0;
                                           long lLower = 0;
                                           HRESULT hr = S_OK;
                                           hr = ::SafeArrayGetUBound( V_ARRAY( &vtItem ), 1, &lUpper );
                                           if( FAILED( hr ) )
                                               return "";
                                           hr = ::SafeArrayGetLBound( V_ARRAY( &vtItem ), 1, &lLower );
                                           if( FAILED( hr ) )
                                               return "";
                                           long lNumOfElems = lUpper - lLower + 1;
                                           BSTR HUGEP *pbstrArray = NULL;
                                           std::string strResult;

                                           hr = ::SafeArrayAccessData( V_ARRAY( &vtItem ), (void **)&pbstrArray );
                                           if( FAILED( hr ) )
                                               return "";

                                           LPSTR pszTemp = NULL;
                                           for( int nIDX = 0; nIDX < lNumOfElems - 1; ++nIDX )
                                           {
                                               pszTemp = _com_util::ConvertBSTRToString( pbstrArray[ nIDX ] );
                                               strResult += pszTemp;
                                               strResult += strSep;
                                               delete[] pszTemp;
                                               pszTemp = NULL;
                                           }

                                           if( lNumOfElems >= 1 )
                                           {
                                               pszTemp = _com_util::ConvertBSTRToString( pbstrArray[ lNumOfElems - 1 ] );
                                               strResult += pszTemp;
                                               delete[] pszTemp;
                                               pszTemp = NULL;
                                           }

                                           ::SafeArrayUnaccessData( V_ARRAY( &vtItem ) );

                                           return strResult;
                }
                    break;
                case VT_BOOL:
                    return vtItem.boolVal == VARIANT_TRUE ? "True" : "False";
                    break;
                case VT_BOOL | VT_BYREF:
                    return *(vtItem.pboolVal) == VARIANT_TRUE ? "True" : "False";
                    break;
                case VT_DECIMAL:
                    break;
                case VT_I1:
                    return format( "%c", vtItem.cVal );
                    break;
                case VT_I1 | VT_BYREF:
                    return format( "%c", *(vtItem.pcVal) );
                    break;
                case VT_UI1:
                    return format( "%c", vtItem.bVal );
                    break;
                case VT_UI1 | VT_BYREF:
                    return format( "%c", *(vtItem.pbVal) );
                    break;
                case VT_UI2:
                    return format( "%u", vtItem.uiVal );
                    break;
                case VT_UI2 | VT_BYREF:
                    return format( "%u", *(vtItem.puiVal) );
                    break;
                case VT_UI4:
                    return format( "%u", vtItem.ulVal );
                    break;
                case VT_UI4 | VT_BYREF:
                    return format( "%u", *(vtItem.pulVal) );
                    break;
                case VT_UI8:
                    return format( "%I64u", vtItem.ullVal );
                    break;
                case VT_UI8 | VT_BYREF:
                    return format( "%I64u", *(vtItem.pullVal) );
                    break;
                case VT_I8:
                    return format( "%I64d", vtItem.llVal );
                    break;
                case VT_I8 | VT_BYREF:
                    return format( "%I64d", *(vtItem.pllVal) );
                    break;
                case VT_INT:
                    return format( "%d", vtItem.intVal );
                    break;
                case VT_INT | VT_BYREF:
                    return format( "%d", *(vtItem.pintVal) );
                    break;
                case VT_UINT:
                    return format( "%u", vtItem.uintVal );
                    break;
                case VT_UINT | VT_BYREF:
                    return format( "%u", *(vtItem.puintVal) );
                    break;
                case VT_SAFEARRAY:
                case VT_LPSTR:
                case VT_LPWSTR:
                case VT_INT_PTR:
                case VT_UINT_PTR:
                case VT_ARRAY:
                default:
                    return "";
            }

            return "(null)";
        }

        std::wstring ConvertToStringW( const VARIANT& vtItem, const std::wstring& strSep /* = L"," */ )
        {
            switch( vtItem.vt )
            {
                case VT_EMPTY:
                    return L"";
                case VT_NULL:
                    return L"(null)";
                    break;
                case VT_I2:
                    return format( L"%d", vtItem.iVal );
                    break;
                case VT_I2 | VT_BYREF:
                    return format( L"%d", *(vtItem.piVal) );
                    break;
                case VT_I4:
                    return format( L"%d", vtItem.lVal );
                    break;
                case VT_I4 | VT_BYREF:
                    return format( L"%d", *(vtItem.plVal) );
                    break;
                case VT_R4:
                    return format( L"%f", vtItem.fltVal );
                    break;
                case VT_R4 | VT_BYREF:
                    return format( L"%f", *(vtItem.pfltVal) );
                    break;
                case VT_R8:
                    return format( (L"%f"), vtItem.dblVal );
                    break;
                case VT_R8 | VT_BYREF:
                    return format( (L"%f"), *(vtItem.pdblVal) );
                    break;
                case VT_CY:
                case VT_CY | VT_BYREF:
                {
#ifdef _AFX
                    return COleCurrency( vtItem ).Format().GetString();
#else
                    _variant_t vtData( vtItem );
                    vtData.ChangeType( VT_BSTR, NULL );
                    return vtData.bstrVal;
#endif

                }
                    break;
                case VT_DATE:
                case VT_DATE | VT_BYREF:
                {
#ifdef _AFX
                    return COleDateTime( vtItem ).Format( _T( "%Y-%m-%d %H-%M-%S" ) ).GetString();
#else
                    _variant_t vtData( vtItem );
                    vtData.ChangeType( VT_BSTR, NULL );
                    return vtData.bstrVal;
#endif
                }
                    break;
                case VT_BSTR:
                    return V_BSTR( &vtItem );
                    break;
                case VT_BSTR | VT_BYREF:
                    return *(V_BSTRREF( &vtItem ));
                    break;
                case VT_BSTR | VT_ARRAY:
                {
                                           long lUpper = 0;
                                           long lLower = 0;
                                           HRESULT hr = S_OK;
                                           hr = ::SafeArrayGetUBound( V_ARRAY( &vtItem ), 1, &lUpper );
                                           if( FAILED( hr ) )
                                               return L"";
                                           hr = ::SafeArrayGetLBound( V_ARRAY( &vtItem ), 1, &lLower );
                                           if( FAILED( hr ) )
                                               return L"";
                                           long lNumOfElems = lUpper - lLower + 1;
                                           BSTR HUGEP *pbstrArray = NULL;
                                           std::wstring strResult;

                                           hr = ::SafeArrayAccessData( V_ARRAY( &vtItem ), (void **)&pbstrArray );
                                           if( FAILED( hr ) )
                                               return L"";

                                           for( int nIDX = 0; nIDX < lNumOfElems - 1; ++nIDX )
                                           {
                                               strResult += pbstrArray[ nIDX ];
                                               strResult += strSep;
                                           }

                                           if( lNumOfElems >= 1 )
                                               strResult += pbstrArray[ lNumOfElems - 1 ];

                                           ::SafeArrayUnaccessData( V_ARRAY( &vtItem ) );

                                           return strResult;
                }
                    break;
                case VT_BOOL:
                    return vtItem.boolVal == VARIANT_TRUE ? L"True" : L"False";
                    break;
                case VT_BOOL | VT_BYREF:
                    return *(vtItem.pboolVal) == VARIANT_TRUE ? L"True" : L"False";
                    break;
                case VT_DECIMAL:
                    break;
                case VT_I1:
                    return format( L"%c", vtItem.cVal );
                    break;
                case VT_I1 | VT_BYREF:
                    return format( L"%c", *(vtItem.pcVal) );
                    break;
                case VT_UI1:
                    return format( L"%c", vtItem.bVal );
                    break;
                case VT_UI1 | VT_BYREF:
                    return format( L"%c", *(vtItem.pbVal) );
                    break;
                case VT_UI2:
                    return format( L"%u", vtItem.uiVal );
                    break;
                case VT_UI2 | VT_BYREF:
                    return format( L"%u", *(vtItem.puiVal) );
                    break;
                case VT_UI4:
                    return format( L"%u", vtItem.ulVal );
                    break;
                case VT_UI4 | VT_BYREF:
                    return format( L"%u", *(vtItem.pulVal) );
                    break;
                case VT_UI8:
                    return format( L"%I64u", vtItem.ullVal );
                    break;
                case VT_UI8 | VT_BYREF:
                    return format( L"%I64u", *(vtItem.pullVal) );
                    break;
                case VT_I8:
                    return format( L"%I64d", vtItem.llVal );
                    break;
                case VT_I8 | VT_BYREF:
                    return format( L"%I64d", *(vtItem.pllVal) );
                    break;
                case VT_INT:
                    return format( L"%d", vtItem.intVal );
                    break;
                case VT_INT | VT_BYREF:
                    return format( L"%d", *(vtItem.pintVal) );
                    break;
                case VT_UINT:
                    return format( L"%u", vtItem.uintVal );
                    break;
                case VT_UINT | VT_BYREF:
                    return format( L"%u", *(vtItem.puintVal) );
                    break;
                case VT_SAFEARRAY:
                case VT_LPSTR:
                case VT_LPWSTR:
                case VT_INT_PTR:
                case VT_UINT_PTR:
                case VT_ARRAY:
                default:
                    return L"";
            }

            return L"(null)";
        }
#endif
        std::string string_replace_all( const std::string& src, const std::string& pattern, const std::string& replace )
        {
            std::string result = src;
            std::string::size_type pos = 0;
            std::string::size_type offset = 0;
            std::string::size_type pattern_len = pattern.size();
            std::string::size_type replace_len = replace.size();

            while( (pos = result.find( pattern, offset )) != std::string::npos )
            {
                result.replace( result.begin() + pos,
                                result.begin() + pos + pattern_len,
                                replace );
                offset = pos + replace_len;
            }
            return result;
        }

        std::wstring string_replace_all( const std::wstring& src, const std::wstring& pattern, const std::wstring& replace )
        {
            std::wstring result = src;
            std::wstring::size_type pos = 0;
            std::wstring::size_type offset = 0;
            std::wstring::size_type pattern_len = pattern.size();
            std::wstring::size_type replace_len = replace.size();

            while( (pos = result.find( pattern, offset )) != std::wstring::npos )
            {
                result.replace( result.begin() + pos,
                                result.begin() + pos + pattern_len,
                                replace );
                offset = pos + replace_len;
            }
            return result;

        }

        std::vector< std::wstring > convertMultiSZToVector( const wchar_t* pwszText )
        {
            std::vector< std::wstring > vecRet;

            size_t index = 0;
            size_t len = wcslen( pwszText );

            while( len > 0 )
            {
                vecRet.push_back( &pwszText[ index ] );
                index += len + 1;
                len = wcslen( &pwszText[ index ] );
            }

            return vecRet;
        }

        //////////////////////////////////////////////////////////////////////////

        CA2U::CA2U( const char* szText )
            : _szText( NULLPTR )
        {
            assert( szText != NULLPTR );
            convert( szText );
        }

        CA2U::CA2U( const std::string& strText )
            : _szText( NULLPTR )
        {
            convert( strText.c_str() );
        }

        CA2U::CA2U( const CA2U& converter )
            : _szText( NULLPTR )
        {
            if( converter._szText != NULLPTR )
                CopyWideString( &_szText, converter._szText );
        }

        CA2U::~CA2U()
        {
            DeletePtrA< wchar_t* >( _szText );
        }

        CA2U& CA2U::operator=(const char* szText)
        {
            assert( szText != NULLPTR );

            convert( szText );

            return *this;
        }

        CA2U& CA2U::operator=(const std::string& strText)
        {
            convert( strText.c_str() );

            return *this;
        }

        CA2U& CA2U::operator=(const CA2U& converter)
        {
            if( converter._szText != NULLPTR )
                CopyWideString( &_szText, converter._szText );
            return *this;
        }

#if _MSC_VER >= 1600
        CA2U::CA2U( CA2U&& converter )
            : _szText( converter._szText )
        {
            converter._szText = NULLPTR;
        }

        CA2U& CA2U::operator=(CA2U&& converter)
        {
            _szText = converter._szText;

            converter._szText = NULLPTR;
            return *this;
        }
#endif

#ifdef QSTRING_H
        CA2U::CA2U( const QString& strText )
            : _szText( NULLPTR )
        {
            CopyWideString( &_szText, (const wchar_t*)strText.utf16() );
        }
#endif

#ifdef _AFX
        CA2U::CA2U( const CStringA& strText )
            : _szText( NULLPTR )
        {
            convert( strText.GetString() );
        }
#endif

        size_t CA2U::convert( const char* szText )
        {
            size_t nRetSize = 0;
            DeletePtrA< wchar_t* >( _szText );

            do
            {
                if( szText == NULLPTR )
                    break;
#ifdef _WINDOWS_
                int nReqSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szText, -1, _szText, 0 );
                _szText = new wchar_t[ nReqSize ];
                memset( _szText, '\0', sizeof(wchar_t)* nReqSize );

                nRetSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szText, -1, _szText, nReqSize );
                if( nRetSize == 0 )
                    DeletePtrA< wchar_t* >( _szText );
#else
                errno_t err = mbstowcs_s( &nRetSize, NULL, 0, szText, _TRUNCATE );
                if( err != 0 )
                    break;

                _szText = new wchar_t[ nRetSize ];
                memset( _szText, '\0', sizeof(wchar_t)* nRetSize );
                err = mbstowcs_s( &nRetSize, _szText, nRetSize, szText, _TRUNCATE );
                if( err != 0 )
                    DeletePtrA< wchar_t* >( _szText );
#endif
            } while( false );

            return nRetSize;
        }

        //////////////////////////////////////////////////////////////////////////

        CU2A::CU2A( const wchar_t* szText )
            : _szText( NULLPTR )
        {
            assert( szText != NULLPTR );

            convert( szText );
        }

        CU2A::CU2A( const std::wstring& strText )
            : _szText( NULLPTR )
        {
            convert( strText.c_str() );
        }

        CU2A::CU2A( const CU2A& converter )
            : _szText( NULLPTR )
        {
            if( converter._szText != NULLPTR )
                CopyAnsiString( &_szText, converter._szText );
        }

        CU2A::~CU2A()
        {
            DeletePtrA< char* >( _szText );
        }

        CU2A& CU2A::operator=(const wchar_t* szText)
        {
            assert( szText != NULLPTR );

            convert( szText );

            return *this;
        }

        CU2A& CU2A::operator=(const std::wstring& strText)
        {
            convert( strText.c_str() );

            return *this;
        }

        CU2A& CU2A::operator=(const CU2A& converter)
        {
            if( converter._szText != NULLPTR )
                CopyAnsiString( &_szText, converter._szText );
            return *this;
        }

#if _MSC_VER >= 1600
        CU2A::CU2A( CU2A&& converter )
            : _szText( converter._szText )
        {
            converter._szText = NULLPTR;
        }

        CU2A& CU2A::operator=(CU2A&& converter)
        {
            _szText = converter._szText;

            converter._szText = NULLPTR;
            return *this;
        }
#endif

#ifdef QSTRING_H
        CU2A::CU2A( const QString& strText )
            : _szText( NULLPTR )
        {
            CopyAnsiString( &_szText, strText.toLocal8Bit() );
        }
#endif

#ifdef _AFX
        CU2A::CU2A( const CStringW& strText )
            : _szText( NULLPTR )
        {
            convert( strText.GetString() );
        }
#endif

        size_t CU2A::convert( const wchar_t* szText )
        {
            size_t nRetSize = 0;
            DeletePtrA< char* >( _szText );

            do
            {
                if( szText == NULLPTR )
                    break;
#ifdef _WINDOWS_
                int nReqSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, szText, -1, _szText, 0, NULL, NULL );
                _szText = new char[ nReqSize ];
                memset( _szText, '\0', nReqSize * sizeof(char) );

                int nRetSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, szText, -1, _szText, nReqSize, NULL, NULL );
                if( nRetSize == 0 )
                    DeletePtrA< char* >( _szText );
#else
                errno_t err = wcstombs_s( &nRetSize, NULL, 0, szText, _TRUNCATE );
                if( err != 0 )
                    break;

                _szText = new char[ nRetSize ];
                memset( _szText, '\0', sizeof(char)* nRetSize );
                err = wcstombs_s( &nRetSize, _szText, nRetSize, szText, _TRUNCATE );
                if( err != 0 )
                    DeletePtrA< char* >( _szText );
#endif
            } while( false );

            return nRetSize;
        }

        //////////////////////////////////////////////////////////////////////////

        CU82U::CU82U( const char* szText )
            : _szText( NULLPTR )
        {
            assert( szText != NULLPTR );
            convert( szText );
        }

        CU82U::CU82U( const std::string& strText )
            : _szText( NULLPTR )
        {
            convert( strText.c_str() );
        }

        CU82U::CU82U( const CU82U& converter )
            : _szText( NULLPTR )
        {
            if( converter._szText != NULLPTR )
                CopyWideString( &_szText, converter._szText );
        }

        CU82U::~CU82U()
        {
            DeletePtrA< wchar_t* >( _szText );
        }

        CU82U& CU82U::operator=(const char* szText)
        {
            assert( szText != NULLPTR );

            convert( szText );

            return *this;
        }

        CU82U& CU82U::operator=(const std::string& strText)
        {
            convert( strText.c_str() );

            return *this;
        }

        CU82U& CU82U::operator=(const CU82U& converter)
        {
            if( converter._szText != NULLPTR )
                CopyWideString( &_szText, converter._szText );
            return *this;
        }

#if _MSC_VER >= 1600
        CU82U::CU82U( CU82U&& converter )
            : _szText( converter._szText )
        {
            converter._szText = NULLPTR;
        }

        CU82U& CU82U::operator=(CU82U&& converter)
        {
            _szText = converter._szText;

            converter._szText = NULLPTR;
            return *this;
        }
#endif

#ifdef QSTRING_H
        CU82U::CU82U( const QString& strText )
            : _szText( NULLPTR )
        {
            CopyWideString( &_szText, (const wchar_t*)strText.utf16() );
        }
#endif

#ifdef _AFX
        CU82U::CU82U( const CStringA& strText )
            : _szText( NULLPTR )
        {
            convert( strText.GetString() );
        }
#endif

        size_t CU82U::convert( const char* szText )
        {
            size_t nRetSize = 0;
            DeletePtrA< wchar_t* >( _szText );

            do
            {
                if( szText == NULLPTR )
                    break;
#ifdef _WINDOWS_
                int nReqSize = MultiByteToWideChar( CP_UTF8, 0, szText, -1, _szText, 0 );
                _szText = new wchar_t[ nReqSize ];
                memset( _szText, '\0', sizeof(wchar_t)* nReqSize );

                nRetSize = MultiByteToWideChar( CP_UTF8, 0, szText, -1, _szText, nReqSize );
                if( nRetSize == 0 )
                    DeletePtrA< wchar_t* >( _szText );
#else
                nRetSize = getActualUTF8Length( szText );

                if( nRetSize <= 0 )
                    break;

                _szText = new wchar_t[ nRetSize + 1 ];
                memset( _szText, '\0', sizeof(wchar_t)* nRetSize );

                nRetSize = convertUTF8toUTF16( szText, _szText );
                if( nRetSize <= 0 )
                    DeletePtrA< wchar_t* >( _szText );
#endif
            } while( false );

            return nRetSize;
        }

        //////////////////////////////////////////////////////////////////////////

        CU2U8::CU2U8( const wchar_t* szText )
            : _szText( NULLPTR )
        {
            assert( szText != NULLPTR );

            convert( szText );
        }

        CU2U8::CU2U8( const std::wstring& strText )
            : _szText( NULLPTR )
        {
            convert( strText.c_str() );
        }

        CU2U8::CU2U8( const CU2U8& converter )
            : _szText( NULLPTR )
        {
            if( converter._szText != NULLPTR )
                CopyAnsiString( &_szText, converter._szText );
        }

        CU2U8::~CU2U8()
        {
            DeletePtrA< char* >( _szText );
        }

        CU2U8& CU2U8::operator=(const wchar_t* szText)
        {
            assert( szText != NULLPTR );

            convert( szText );

            return *this;
        }

        CU2U8& CU2U8::operator=(const std::wstring& strText)
        {
            convert( strText.c_str() );

            return *this;
        }

        CU2U8& CU2U8::operator=(const CU2U8& converter)
        {
            if( converter._szText != NULLPTR )
                CopyAnsiString( &_szText, converter._szText );
            return *this;
        }

#if _MSC_VER >= 1600
        CU2U8::CU2U8( CU2U8&& converter )
            : _szText( converter._szText )
        {
            converter._szText = NULLPTR;
        }

        CU2U8& CU2U8::operator=(CU2U8&& converter)
        {
            _szText = converter._szText;

            converter._szText = NULLPTR;
            return *this;
        }
#endif

#ifdef QSTRING_H
        CU2U8::CU2U8( const QString& strText )
            : _szText( NULLPTR )
        {
            CopyAnsiString( &_szText, strText.toUtf8() );
        }
#endif

#ifdef _AFX
        CU2U8::CU2U8( const CStringW& strText )
            : _szText( NULLPTR )
        {
            convert( strText.GetString() );
        }
#endif

        size_t CU2U8::convert( const wchar_t* szText )
        {
            size_t nRetSize = 0;
            DeletePtrA< char* >( _szText );

            do
            {
                if( szText == NULLPTR )
                    break;
#ifdef _WINDOWS_
                int nReqSize = WideCharToMultiByte( CP_UTF8, 0, szText, -1, _szText, 0, NULL, NULL );
                _szText = new char[ nReqSize ];
                memset( _szText, '\0', nReqSize * sizeof(char) );

                int nRetSize = WideCharToMultiByte( CP_UTF8, 0, szText, -1, _szText, nReqSize, NULL, NULL );
                if( nRetSize == 0 )
                    DeletePtrA< char* >( _szText );
#else
                nRetSize = getBytesUTF16toUTF8( szText );
                if( nRetSize <= 0 )
                    break;

                _szText = new char[ nRetSize + 1 ];
                memset( _szText, '\0', nRetSize * sizeof(char) );
                nRetSize = convertUTF16toUTF8( szText, _szText );
                if( nRetSize <= 0 )
                    DeletePtrA< char* >( _szText );
#endif
            } while( false );

            return nRetSize;
        }

        //////////////////////////////////////////////////////////////////////////

        CA2U8::CA2U8( const char* szText )
            : _szText( NULLPTR )
        {
            assert( szText != NULLPTR );
            convert( szText );
        }

        CA2U8::CA2U8( const std::string& strText )
            : _szText( NULLPTR )
        {
            convert( strText.c_str() );
        }

        CA2U8::CA2U8( const CA2U8& converter )
            : _szText( NULLPTR )
        {
            if( converter._szText != NULLPTR )
                CopyAnsiString( &_szText, converter._szText );
        }

        CA2U8::~CA2U8()
        {
            DeletePtrA< char* >( _szText );
        }

        CA2U8& CA2U8::operator=(const char* szText)
        {
            assert( szText != NULLPTR );

            convert( szText );

            return *this;
        }

        CA2U8& CA2U8::operator=(const std::string& strText)
        {
            convert( strText.c_str() );

            return *this;
        }

        CA2U8& CA2U8::operator=(const CA2U8& converter)
        {
            if( converter._szText != NULLPTR )
                CopyAnsiString( &_szText, converter._szText );
            return *this;
        }

#if _MSC_VER >= 1600
        CA2U8::CA2U8( CA2U8&& converter )
            : _szText( converter._szText )
        {
            converter._szText = NULLPTR;
        }

        CA2U8& CA2U8::operator=(CA2U8&& converter)
        {
            _szText = converter._szText;

            converter._szText = NULLPTR;
            return *this;
        }
#endif

#ifdef QSTRING_H
        CA2U8::CA2U8( const QString& strText )
            : _szText( NULLPTR )
        {
            CopyAnsiString( &_szText, (const char*)strText.toUtf8() );
        }
#endif

#ifdef _AFX
        CA2U8::CA2U8( const CStringA& strText )
            : _szText( NULLPTR )
        {
            convert( strText.GetString() );
        }
#endif

        size_t CA2U8::convert( const char* szText )
        {
            size_t nRetSize = 0;
            wchar_t* pwszBuffer = NULLPTR;
            DeletePtrA< char* >( _szText );

            do
            {
                if( szText == NULLPTR )
                    break;

#ifdef _WINDOWS_
                int nReqSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szText, -1, pwszBuffer, 0 );
                pwszBuffer = new wchar_t[ nReqSize ];
                memset( pwszBuffer, '\0', sizeof(wchar_t)* nReqSize );

                int nRetSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szText, -1, pwszBuffer, nReqSize );
                if( nRetSize > 0 )
                {
                    nReqSize = WideCharToMultiByte( CP_UTF8, 0, pwszBuffer, -1, _szText, 0, NULL, NULL );
                    _szText = new char[ nReqSize ];
                    memset( _szText, '\0', nReqSize * sizeof(char) );

                    nRetSize = WideCharToMultiByte( CP_UTF8, 0, pwszBuffer, -1, _szText, nReqSize, NULL, NULL );
                    if( nRetSize <= 0 )
                        DeletePtrA< char* >( _szText );
                }
#else
                errno_t err = mbstowcs_s( &nRetSize, NULL, 0, szText, 0 );
                if( err != 0 )
                    break;

                pwszBuffer = new wchar_t[ nRetSize + 1 ];
                memset( pwszBuffer, '\0', sizeof(wchar_t)* nRetSize );

                err = mbstowcs_s( &nRetSize, pwszBuffer, nRetSize, szText, _TRUNCATE );
                if( err != 0 )
                    break;

                nRetSize = convertUTF16toUTF8( pwszBuffer, _szText );
                if( nRetSize <= 0 )
                    DeletePtrA< char* >( _szText );
#endif
            } while( false );

            DeletePtrA< wchar_t *>( pwszBuffer );
            return nRetSize;
        }

        //////////////////////////////////////////////////////////////////////////

        CU82A::CU82A( const char* szText )
            : _szText( NULLPTR )
        {
            assert( szText != NULLPTR );
            convert( szText );
        }

        CU82A::CU82A( const std::string& strText )
            : _szText( NULLPTR )
        {
            convert( strText.c_str() );
        }

        CU82A::CU82A( const CU82A& converter )
            : _szText( NULLPTR )
        {
            if( converter._szText != NULLPTR )
                CopyAnsiString( &_szText, converter._szText );
        }

        CU82A::~CU82A()
        {
            DeletePtrA< char* >( _szText );
        }

        CU82A& CU82A::operator=(const char* szText)
        {
            assert( szText != NULLPTR );

            convert( szText );

            return *this;
        }

        CU82A& CU82A::operator=(const std::string& strText)
        {
            convert( strText.c_str() );

            return *this;
        }

        CU82A& CU82A::operator=(const CU82A& converter)
        {
            if( converter._szText != NULLPTR )
                CopyAnsiString( &_szText, converter._szText );
            return *this;
        }

#if _MSC_VER >= 1600
        CU82A::CU82A( CU82A&& converter )
            : _szText( converter._szText )
        {
            converter._szText = NULLPTR;
        }

        CU82A& CU82A::operator=(CU82A&& converter)
        {
            _szText = converter._szText;

            converter._szText = NULLPTR;
            return *this;
        }
#endif

#ifdef QSTRING_H
        CU82A::CU82A( const QString& strText )
            : _szText( NULLPTR )
        {
            CopyAnsiString( &_szText, (const char*)strText.toUtf8() );
        }
#endif

#ifdef _AFX
        CU82A::CU82A( const CStringA& strText )
            : _szText( NULLPTR )
        {
            convert( strText.GetString() );
        }
#endif

        size_t CU82A::convert( const char* szText )
        {
            size_t nRetSize = 0;
            wchar_t* pwszBuffer = NULL;
            DeletePtrA< char* >( _szText );

            do
            {
                if( szText == NULLPTR )
                    break;
#ifdef _WINDOWS_
                int nReqSize = MultiByteToWideChar( CP_UTF8, 0, szText, -1, pwszBuffer, 0 );
                pwszBuffer = new wchar_t[ nReqSize ];
                memset( pwszBuffer, '\0', nReqSize * sizeof(wchar_t) );

                int nRetSize = MultiByteToWideChar( CP_UTF8, 0, szText, -1, pwszBuffer, nReqSize );
                if( nRetSize <= 0 )
                    break;

                nReqSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, pwszBuffer, -1, _szText, 0, NULL, NULL );
                _szText = new char[ nReqSize ];
                memset( _szText, '\0', nReqSize * sizeof(char) );

                nRetSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, pwszBuffer, -1, _szText, nReqSize, NULL, NULL );
                if( nRetSize <= 0 )
                    DeletePtrA< char* >( _szText );
#else
                int nReqSize = getActualUTF8Length( szText );
                pwszBuffer = new wchar_t[ nReqSize + 1 ];
                memset( pwszBuffer, '\0', (nReqSize + 1 ) * sizeof(wchar_t) );

                convertUTF8toUTF16( szText, pwszBuffer );

                errno_t err = wcstombs_s( &nRetSize, NULL, 0, pwszBuffer, 0 );
                if( err != 0 )
                    break;

                _szText = new char[ nRetSize + 1 ];
                memset( _szText, '\0', ( nRetSize + 1) * sizeof(char) );

                err = wcstombs_s( &nRetSize, _szText, nRetSize, pwszBuffer, _TRUNCATE );
                if( err != 0 )
                    DeletePtrA< char* >( _szText );
#endif
            } while( false );

            DeletePtrA< wchar_t* >( pwszBuffer );
            return nRetSize;
        }

    } // nsCmnConvert
}