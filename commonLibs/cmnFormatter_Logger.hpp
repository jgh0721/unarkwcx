#pragma once

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            struct logger_type_tag
            { };

            template<>
            struct type_tag < nsCmnFmtTypes::TyTpLoggerType >
            {
                typedef logger_type_tag                type;
            };

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, logger_type_tag,
                                typename boost::enable_if< boost::is_same< typename Renderer::charType, char > >::type * = 0 )
            {
                // FileNameA, FileNameW, FuncNameA, FuncNameW, LoggerNameA, LoggerNameW, LogLevelA, LogLevelW, lineNumber
                if( fmtSpec.loggerFileName )
                    return renderer.render( tpLoggerType.get<0>(), -1 );
                else if( fmtSpec.loggerFuncName )
                    return renderer.render( tpLoggerType.get<2>(), -1 );
                else if( fmtSpec.loggerName )
                    return renderer.render( tpLoggerType.get<4>(), -1 );
                else if( fmtSpec.loggerLogLevel )
                    return renderer.render( tpLoggerType.get<6>(), -1 );
                else if( fmtSpec.loggerLineNumber )
                {
                    memset( fmtBuffer, '\0', sizeof( char ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );
                    size_t numberCount = formatNumber< Renderer, unsigned int >( fmtBuffer, fmtSpec, tpLoggerType.get<8>() );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }

                return renderer.render( "Unknown Logger Type", -1 );
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, logger_type_tag,
                                typename boost::enable_if< boost::is_same< typename Renderer::charType, wchar_t > >::type * = 0 )
            {
                // FileNameA, FileNameW, FuncNameA, FuncNameW, LoggerNameA, LoggerNameW, LogLevelA, LogLevelW, lineNumber
                if( fmtSpec.loggerFileName )
                    return renderer.render( tpLoggerType.get<1>(), -1 );
                else if( fmtSpec.loggerFuncName )
                    return renderer.render( tpLoggerType.get<3>(), -1 );
                else if( fmtSpec.loggerName )
                    return renderer.render( tpLoggerType.get<5>(), -1 );
                else if( fmtSpec.loggerLogLevel )
                    return renderer.render( tpLoggerType.get<7>(), -1 );
                else if( fmtSpec.loggerLineNumber )
                {
                    memset( fmtBuffer, '\0', sizeof( wchar_t ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );
                    size_t numberCount = formatNumber< Renderer, unsigned int >( fmtBuffer, fmtSpec, tpLoggerType.get<8>() );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }

                return renderer.render( "Unknown Logger Type", -1 );
            }
        }
    }
}
