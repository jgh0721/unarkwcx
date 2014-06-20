#ifndef CMNFORMATTER_FLOAT_H
#define CMNFORMATTER_FLOAT_H

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            template< typename Renderer, typename T >
            long formatNumber( typename Renderer::charType*& fmtBuffer, const formatSpec& fmtSpec, T value,
                typename boost::enable_if_c< boost::is_floating_point< T >::value && boost::is_same< typename Renderer::charType, char >::value >::type * = 0 )
            {
                memset( fmtBuffer, '\0', sizeof( typename Renderer::charType ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );

                return sprintf_s( fmtBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - 1, "%f", value );
            }

            template< typename Renderer, typename T >
            long formatNumber( typename Renderer::charType*& fmtBuffer, const formatSpec& fmtSpec, T value,
                typename boost::enable_if_c< boost::is_floating_point< T >::value && boost::is_same< typename Renderer::charType, wchar_t >::value >::type * = 0 )
            {
                memset( fmtBuffer, '\0', sizeof( typename Renderer::charType ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );

                return swprintf_s( fmtBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - 1, L"%f", value );
            }

            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, float value )
            {
                long numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                return (long)renderer.render( fmtBuffer, -1 );
            }

            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, double value )
            {
                long numberCount = formatNumber< Renderer>( fmtBuffer, fmtSpec, value );
                return (long)renderer.render( fmtBuffer, -1 );
            }

        } // namespace nsCmnFmtDetail

    } // namespace nsCmnFormatter

} // namespace nsCommon

#endif // CMNFORMATTER_FLOAT_H
