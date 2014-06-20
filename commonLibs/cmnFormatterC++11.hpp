#pragma once

#pragma execution_character_set( "utf-8" )

#include <string>

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

#include "tek/tek.h"

#include "cmnFormatter_Utils.hpp"
#include "cmnFormatter_Parse.hpp"
#include "cmnFormatter_Renderder.hpp"
#include "cmnFormatter_Format.hpp"

#include "cmnFormatter_Integer.hpp"
#include "cmnFormatter_Float.hpp"
#include "cmnFormatter_String.hpp"
#include "cmnFormatter_Converter.hpp"
#include "cmnFormatter_Logger.hpp"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            template< typename Renderer, typename T >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const T& value, unknown_type_tag )
            {
                return renderer.render( "Unknown Type", -1 );
            }

            template< typename Renderer, typename CharT, typename ... Args >
            size_t doFormatEx( Renderer& renderer, const CharT* __restrict fmt, CharT* fmtBuffer, Args ... args )
            {
                tek::record< Args ... > argsList( args... );

                nsCmnFmtDetail::formatSpec fmtSpec;
                const CharT* prefix = nullptr;
                long prefixLength = 0;
                size_t nRetFormattedCount = 0;

                while( fmt )
                {
                    ::memset( fmtBuffer, '\0', sizeof( CharT ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );

                    fmtParse( fmt, fmtSpec, fmtBuffer, prefix, prefixLength, argsList.size );

                    if( (fmtSpec.argIndex > 0) && (fmtSpec.argIndex > argsList.size) )
                    {
                        nRetFormattedCount = -1;
                        break;
                    }

                    if( prefixLength > 0 )
                        nRetFormattedCount += renderer.render( prefix, prefixLength );

                    // %0 특수문법에 대해 기록
                    if( *fmtBuffer && fmtSpec.argIndex == 0 )
                        nRetFormattedCount += renderer.render( fmtBuffer, -1 );

                    if( fmtSpec.argIndex == 0 )
                        continue;

                    if( fmtSpec.argIndex < 0 )
                        break;

                    if( argsList.is_valid_index( fmtSpec.argIndex - 1 ) )
                        nRetFormattedCount += argsList.apply( clsFormatterEx<Renderer>( renderer, fmtSpec, fmtBuffer ), fmtSpec.argIndex - 1 );
                    else
                        nRetFormattedCount += renderer.render( "Invalid Index", -1 );
                }

                return nRetFormattedCount;
            }

            template< typename Dest, typename CharT, typename ... Args >
            size_t tsFormatBuffer( Dest& dest, const CharT* fmt, Args ... args )
            {
                nsCmnFmtDetail::clsRenderer< Dest > out( dest );
                typename clsRenderer<Dest>::charType fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE ] = { 0, };

                return doFormatEx( out, fmt, fmtBuffer, args... );
            }

            template< typename Dest, typename ... Args >
            size_t tsFormatLoggerBuffer( nsCmnFmtDetail::clsRenderer<Dest>& dest, const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, const typename nsCmnFmtDetail::clsRenderer<Dest>::charType* fmt, Args ... args )
            {
                typename clsRenderer<Dest>::charType fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE ] = { 0, };

                return doFormatEx( dest, fmt, fmtBuffer, args..., tpLoggerType );
            }
        }

        template< typename CharT, typename ... Args >
        std::basic_string< CharT > tsFormat( const CharT* fmt, Args ... args )
        {
            std::basic_string< CharT > s;
            nsCmnFmtDetail::tsFormatBuffer( s, fmt, args... );
            return s;
        }
        
    } // namespace nsCmnFormatter
} // namespace nsCommon
