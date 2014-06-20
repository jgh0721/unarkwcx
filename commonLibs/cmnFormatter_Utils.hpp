#ifndef CMNFORMATTER_UTILS_H
#define CMNFORMATTER_UTILS_H

#include <time.h>
#include <memory>

#ifndef _WINDOWS_
#include <Windows.h>
#endif

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            size_t            getTextLength( const char* pText );
            size_t            getTextLength( const wchar_t* pText );

            char* convertWideCharToMultiByte( const wchar_t* pwszSrc, int pwszSrcCount );
            wchar_t* convertMultiByteToWideChar( const char* pszSrc, int pszSrcCount );

            template< typename CharT >
            bool getBuffer( size_t nRequiredSize, size_t& totalBufferSize, size_t& nRemainBufferSize, CharT*& pBuffer )
            {
                bool isSuccess = true;

                do 
                {
                    if( nRemainBufferSize > nRequiredSize )
                        break;

                    do 
                    {
                        totalBufferSize += nsCmnFmtTypes::DEFAULT_BUFFER_SIZE;
                        nRemainBufferSize += nsCmnFmtTypes::DEFAULT_BUFFER_SIZE;

                    } while ( nRemainBufferSize < nRequiredSize );

                    CharT* pCharBuffer = new CharT[ totalBufferSize ];
                    if( pCharBuffer == NULL )
                    {
                        isSuccess = false;
                        break;
                    }

                    ::memset( pCharBuffer, '\0', totalBufferSize * sizeof( CharT ) );
                    if( pBuffer != NULL )
                    {
                        ::memcpy( pCharBuffer, pBuffer, sizeof( CharT ) * getTextLength( pBuffer ) );

                        delete [] pBuffer;
                        pBuffer = NULL;
                    }

                    pBuffer = pCharBuffer;

                    // ptrdiff_t nBetween = 
                } while (false);

                return isSuccess;
            }

            template< typename CharT, typename T >
            size_t formatDateTime( CharT*& fmtBuffer, const formatSpec& fmtSpec, T value,
                                 typename boost::enable_if< boost::is_same< CharT, char > >::type * = 0 )
            {
                struct tm tmConversion;
                localtime_s( &tmConversion, (time_t*)(&value) );

                if( !*fmtBuffer )
                {
                    // 기본 형식 문자열 사용
                    return strftime( fmtBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE, nsCmnFmtTypes::DEFAULT_DATE_TIME_FORMAT_A, &tmConversion );
                }
                else
                {
                    // 별도 버퍼 필요
                    CharT datetimeText[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE ] = {0,};

                    size_t nRet = strftime( datetimeText, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE, fmtBuffer, &tmConversion );
                    ::memset( fmtBuffer, '\0', sizeof( CharT ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );
                    strncpy_s( fmtBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - 1, datetimeText, nRet );
                    return nRet;
                }
            }

            template< typename CharT, typename T >
            size_t formatDateTime( CharT*& fmtBuffer, const formatSpec& fmtSpec, T value,
                typename boost::enable_if< boost::is_same< CharT, wchar_t > >::type * = 0 )
            {
                struct tm tmConversion;
                localtime_s( &tmConversion, (time_t*)(&value) );

                if( !*fmtBuffer )
                {
                    // 기본 형식 문자열 사용
                    return wcsftime( fmtBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE, nsCmnFmtTypes::DEFAULT_DATE_TIME_FORMAT_W, &tmConversion );
                }
                else
                {
                    // 별도 버퍼 필요
                    CharT datetimeText[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE ] = {0,};

                    size_t nRet = wcsftime( datetimeText, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE, fmtBuffer, &tmConversion );
                    ::memset( fmtBuffer, '\0', sizeof( CharT ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );
                    wcsncpy_s( fmtBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - 1, datetimeText, nRet );
                    return nRet;
                }
            }

            template< typename Renderer, typename T >
            long formatErrorText( typename Renderer::charType*& fmtBuffer, const formatSpec& fmtSpec, T value,
                typename boost::enable_if< boost::is_same< typename Renderer::charType, char > >::type * = 0 )
            {
                return ::FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                    NULL, 
                    value,
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), 
                    (LPSTR) fmtBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE, NULL );
            }

            template< typename Renderer, typename T >
            long formatErrorText( typename Renderer::charType*& fmtBuffer, const formatSpec& fmtSpec, T value,
                typename boost::enable_if< boost::is_same< typename Renderer::charType, wchar_t > >::type * = 0 )
            {
                return ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                    NULL, 
                    value,
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), 
                    (LPWSTR) fmtBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE, NULL );
            }


        }; // namespace nsCmnFmtDetail

    }; // namespace nsCmnFormatter

}; // namespace nsCommon

#if _MSC_VER < 1800
namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {

        /** A predicate for sorting a list of types.
         *
         * Compares two types and returns true if the first one
         * is "less than" the second.  The goal is to generate
         * the same variant for calls to outf() that pass the
         * same set of types of arguments, regardless of the
         * number or order of the arguments.  This serves several
         * purposes:
         *
         * -   Minimizes the number of different instantiations
         *     of do_format().
         *
         * -   Allows the boost::mpl::unique<> metafunction to
         *     remove duplicate types (it only removes duplicates
         *     if they're adjacent).
         *
         * If two different types appear the same to the
         * type_less metafunction, their order in the list will
         * depend on the order they are passed to outf(), and
         * duplicates may not be removed from the variant's type
         * list.  That means extra instantiations of do_format(),
         * but
         */
        template<typename T1>
        struct sort_order
        {
            BOOST_STATIC_CONSTANT(int, value = 500);
        };

        template<>
        struct sort_order<int>
        {
            BOOST_STATIC_CONSTANT(int, value = 0);
        };

        template<>
        struct sort_order<unsigned int>
        {
            BOOST_STATIC_CONSTANT(int, value = 1);
        };

        template<>
        struct sort_order<long>
        {
            BOOST_STATIC_CONSTANT(int, value = 2);
        };

        template<>
        struct sort_order<unsigned long>
        {
            BOOST_STATIC_CONSTANT(int, value = 3);
        };

        template<>
        struct sort_order<long long>
        {
            BOOST_STATIC_CONSTANT(int, value = 4);
        };

        template<>
        struct sort_order<unsigned long long>
        {
            BOOST_STATIC_CONSTANT(int, value = 5);
        };

        template<>
        struct sort_order<float>
        {
            BOOST_STATIC_CONSTANT(int, value = 6);
        };

        template<>
        struct sort_order<double>
        {
            BOOST_STATIC_CONSTANT(int, value = 7);
        };

        template<>
        struct sort_order<long double>
        {
            BOOST_STATIC_CONSTANT(int, value = 8);
        };

        struct type_less
        {
            template<typename T1, typename T2>
            struct apply_impl :
                ::boost::mpl::bool_<
                  sort_order<T1>::value < sort_order<T2>::value
              >
            {
            };

            template<typename T1, typename T2>
            struct apply : apply_impl<typename boost::remove_cv<T1>::type,
                                        typename boost::remove_cv<T2>::type>
            {
            };
        };

        namespace type_traits
        {
            template<typename T>
            struct is_character_pointer
            {
                typedef boost::false_type type;
                BOOST_STATIC_CONSTANT(bool, value = false);
            };

            template<>
            struct is_character_pointer<char*>
            {
                typedef boost::true_type type;
                BOOST_STATIC_CONSTANT(bool, value = true);
            };

            template<>
            struct is_character_pointer<wchar_t*>
            {
                typedef boost::true_type type;
                BOOST_STATIC_CONSTANT(bool, value = true);
            };
        } // namespace type_traits

        } // namespace nsCmnFmtDetail

    } // namespace nsCmnFormatter

} // namespace nsCommon
#endif

#endif // CMNFORMATTER_UTILS_H
