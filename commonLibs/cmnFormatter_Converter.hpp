#pragma once

#include "cmnConverter.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            struct encoding_converter_type_tag
            { };

            template<>
            struct type_tag< nsCommon::nsCmnConvert::CA2U >
            {
                typedef encoding_converter_type_tag             type;
            };

            template<>
            struct type_tag< nsCommon::nsCmnConvert::CU2A >
            {
                typedef encoding_converter_type_tag             type;
            };

            template<>
            struct type_tag< nsCommon::nsCmnConvert::CU82U >
            {
                typedef encoding_converter_type_tag             type;
            };

            template<>
            struct type_tag< nsCommon::nsCmnConvert::CU82A >
            {
                typedef encoding_converter_type_tag             type;
            };

            template< typename Renderer, typename CharT >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, CharT const& value, encoding_converter_type_tag )
            {
                return (long)renderer.render( value.c_str(), -1 );
            }

            // 기본적으로 렌더러와 포메터는 UTF-8 문자열을 지원하지 않기 때문에 ANSI 또는 UNICODE 형태로 변환시켜서 출력한다.
            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCommon::nsCmnConvert::CA2U8& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, char >::value >::type * = 0 )
            {
                return (long)renderer.render( nsCommon::nsCmnConvert::CU82A( value.c_str() ).c_str(), -1 );
            }

            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCommon::nsCmnConvert::CA2U8& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, wchar_t >::value >::type * = 0 )
            {
                return (long)renderer.render( nsCommon::nsCmnConvert::CU82U( value.c_str() ).c_str(), -1 );
            }

            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCommon::nsCmnConvert::CU2U8& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, char >::value >::type * = 0 )
            {
                return (long)renderer.render( nsCommon::nsCmnConvert::CU82A( value.c_str() ).c_str(), -1 );
            }

            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCommon::nsCmnConvert::CU2U8& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, wchar_t >::value >::type * = 0 )
            {
                return (long)renderer.render( nsCommon::nsCmnConvert::CU82U( value.c_str() ).c_str(), -1 );
            }

        } // nsCmnFmtDetail
    } // nsCmnFormatter
} // nsCommon
