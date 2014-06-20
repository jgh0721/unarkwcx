#ifndef CMNFORMATTER_STRING_H
#define CMNFORMATTER_STRING_H

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            struct cstring_type_tag
            { };

            template<>
            struct type_tag< char* >
            {
                typedef cstring_type_tag                type;
            };

            template<>
            struct type_tag< wchar_t* >
            {
                typedef cstring_type_tag                type;
            };

            // MFC 의 CString 지원추가
#ifdef _AFX
#ifdef _AFXDLL
            template< typename Renderer, typename Char >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const ATL::CStringT< Char, StrTraitMFC_DLL< Char > >& value )
#else
            template< typename Renderer, typename Char >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const ATL::CStringT< Char, StrTraitMFC< Char > >& value )
#endif
            {
                return renderer.render( value.GetString(), -1 );
            }
#endif

#ifdef QSTRING_H
            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const QString& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, char >::value >::type * = 0 )
            {
                return renderer.render( (const char*)value.toLocal8Bit(), -1 );
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const QString& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, wchar_t >::value >::type * = 0 )
            {
                return renderer.render( (const wchar_t*)value.utf16(), -1 );
            }
#endif

            template< typename Renderer, typename CharT >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, CharT* const& value, cstring_type_tag )
            {
                return renderer.render( value, -1 );
            }

            template< typename Renderer, typename CharT, typename Traits, typename Alloc >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const std::basic_string< CharT, Traits, Alloc >& value )
            {
                return renderer.render( value.c_str(), -1 );
            }

        } // namespace nsCmnFmtDetail

    } // namespace nsCmnFormatter

} // namespace nsCommon

#endif // CMNFORMATTER_STRING_H
