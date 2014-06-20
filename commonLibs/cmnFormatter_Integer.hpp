#ifndef CMNFORMATTER_INTEGER_H
#define CMNFORMATTER_INTEGER_H

#include <math.h>

#include "cmnFormatter_Utils.hpp"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            struct integer_type_tag
            { };

            template< typename Renderer, typename T >
            size_t formatNumberInternal( typename Renderer::charType*& fmtBuffer, const formatSpec& fmtSpec, bool isNegative, T value )
            {
                size_t numberCount = 0;

                if( value == 0 )
                {
                    *--fmtBuffer = '0';
                    ++numberCount;
                }
                else if ( isNegative == true )
                {
                    __int64 absValue = _abs64( value );

                    for( ; absValue; absValue /= 10 )
                    {
                        *--fmtBuffer = '0' + ( typename Renderer::charType ) ( absValue % 10 );
                        ++numberCount;
                    }
                }
                else
                {
                    for( ; value; value /= 10 )
                    {
                        *--fmtBuffer = '0' + ( typename Renderer::charType ) ( value % 10 );
                        ++numberCount;
                    }
                }

                if( fmtSpec.plus == true || isNegative == true )
                {
                    *--fmtBuffer = isNegative == false ? '+' : '-';
                    ++numberCount;
                }

                return numberCount;
            }

            template< typename Renderer, typename T >
            size_t formatNumber( typename Renderer::charType*& fmtBuffer, const formatSpec& fmtSpec, T value,
                typename boost::enable_if_c< boost::is_signed< T >::value >::type * = 0 )
            {
                ::memset( fmtBuffer, '\0', sizeof( typename Renderer::charType ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );

                typename Renderer::charType* destBuffer = &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - 1 ];

                return formatNumberInternal< Renderer >( destBuffer, fmtSpec, value < 0, value );
            }

            template< typename Renderer, typename T >
            size_t formatNumber( typename Renderer::charType*& fmtBuffer, const formatSpec& fmtSpec, T value,
                typename boost::enable_if_c< boost::is_unsigned< T >::value >::type * = 0 )
            {
                ::memset( fmtBuffer, '\0', sizeof( typename Renderer::charType ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );

                typename Renderer::charType* destBuffer = &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - 1 ];

                return formatNumberInternal< Renderer >( destBuffer, fmtSpec, false, value );
            }

            //////////////////////////////////////////////////////////////////////////

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, short value )
            {
                size_t numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, unsigned short value )
            {
                size_t numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, int value )
            {
                if( fmtSpec.datetime == true )
                {
                    formatDateTime<typename Renderer::charType>( fmtBuffer, fmtSpec, value );
                    return renderer.render( fmtBuffer, -1 );
                }
                else
                {
                    size_t numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, long value )
            {
                if( fmtSpec.datetime == true )
                {
                    formatDateTime<typename Renderer::charType>( fmtBuffer, fmtSpec, value );
                    return renderer.render( fmtBuffer, -1 );
                }
                else if( fmtSpec.errorHRESULT == true )
                {
                    size_t numberCount = formatErrorText< Renderer >( fmtBuffer, fmtSpec, value );
                    return renderer.render( fmtBuffer, -1 );
                }
                else
                {
                    size_t numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, unsigned int value )
            {
                if( fmtSpec.errorText == false )
                {
                    size_t numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }
                else
                {
                    size_t numberCount = formatErrorText< Renderer >( fmtBuffer, fmtSpec, value );
                    return renderer.render( fmtBuffer, -1 );
                }
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, unsigned long value )
            {
                if( fmtSpec.errorText == false )
                {
                    size_t numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }
                else
                {
                    size_t numberCount = formatErrorText< Renderer >( fmtBuffer, fmtSpec, value );
                    return renderer.render( fmtBuffer, -1 );
                }
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, long long value )
            {
                if( fmtSpec.datetime == true )
                {
                    formatDateTime<typename Renderer::charType>( fmtBuffer, fmtSpec, value );
                    return renderer.render( fmtBuffer, -1 );
                }
                else
                {
                    size_t numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, unsigned long long value )
            {
                size_t numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, bool value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, char >::value >::type * = 0 )
            {
                if( fmtSpec.upperLetter == true )
                    return renderer.render( value == true ? "TRUE" : "FALSE", -1 );

                return renderer.render( value == true ? "true" : "false", -1 );
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, bool value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, wchar_t >::value >::type * = 0 )
            {
                if( fmtSpec.upperLetter == true )
                    return renderer.render( value == true ? L"TRUE" : L"FALSE", -1 );

                return renderer.render( value == true ? L"true" : L"false", -1 );
            }

        } // namespace nsCmnFmtDetail

    } // namespace nsCmnFormatter

} // namespace nsCommon
#endif // CMNFORMATTER_INTEGER_H
