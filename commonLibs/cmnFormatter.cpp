#include "stdafx.h"

#pragma execution_character_set( "utf-8" )

#include "cmnFormatter.hpp"
#include "cmnFormatter_Utils.hpp"

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            size_t getTextLength( const char* pText ) { return ::strlen( pText ); };
            size_t getTextLength( const wchar_t* pText ) { return ::wcslen( pText ); };

            char* convertWideCharToMultiByte( const wchar_t* pwszSrc, int pwszSrcCount )
            {
                int nReqSize = 0;
                char* pszBuffer = NULL;

                do 
                {
#ifdef _WINDOWS_
                    nReqSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, pwszSrc, pwszSrcCount, pszBuffer, 0, NULL, NULL );
                    pszBuffer = new char[ nReqSize ];
                    if( pszBuffer == NULL )
                        break;
                    memset( pszBuffer, '\0', nReqSize * sizeof(char) );

                    nReqSize = WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, pwszSrc, pwszSrcCount, pszBuffer, nReqSize, NULL, NULL );
#else
                    nReqSize = wcstombs( NULL, pwszSrc, 0 );
                    pszBuffer = new char[ ++nReqSize ];
                    if( pszBuffer == NULL )
                        break;
                    memset( pszBuffer, '\0', nReqSize * sizeof( char ) );

                    nReqSize = wcstombs( pszBuffer, pwszSrc, nReqSize );
#endif
                } while (false);

                return pszBuffer;
            }

            wchar_t* convertMultiByteToWideChar( const char* pszSrc, int pszSrcCount )
            {
                int nReqSize = 0;
                wchar_t* pwszBuffer = NULL;

                do 
                {
#ifdef _WINDOWS_
                    nReqSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszSrc, pszSrcCount, pwszBuffer, 0 );
                    pwszBuffer = new wchar_t[ nReqSize ];
                    if( pwszBuffer == NULL )
                        break;

                    memset( pwszBuffer, '\0', sizeof(wchar_t) * nReqSize );

                    nReqSize = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszSrc, pszSrcCount, pwszBuffer, nReqSize );
#else
                    nReqSize = mbstowcs( NULL, pszSrc, 0 );
                    pwszBuffer = new wchar_t[ ++nReqSize ];
                    if( pwszBuffer == NULL )
                        break;

                    memset( pwszBuffer, '\0', sizeof(wchar_t) * nReqSize );

                    nReqSize = mbstowcs( pwszBuffer, pszSrc, nReqSize );
#endif
                } while (false);

                return pwszBuffer;
            }


        }; // namespace nsCmnFmtDetail
    }; // namespace nsCmnFormatter

}; // namespace nsCommon