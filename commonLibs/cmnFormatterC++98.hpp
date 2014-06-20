#pragma once


#include <string>
#include <crtdefs.h>

#include <stdexcept>
#include <iosfwd>
#include <iomanip>
#include <sstream>
#include <locale>
#include <algorithm>
#include <iterator>
#include <xutility>
#include <utility>

#define FORMAT_MAX_ARGS                 20
#ifdef BOOST_MPL_LIMIT_VECTOR_SIZE
#undef BOOST_MPL_LIMIT_VECTOR_SIZE
#define BOOST_MPL_LIMIT_VECTOR_SIZE                 FORMAT_MAX_ARGS
#endif

#ifdef BOOST_MPL_LIMIT_LIST_SIZE
#undef BOOST_MPL_LIMIT_LIST_SIZE
#define BOOST_MPL_LIMIT_LIST_SIZE                   FORMAT_MAX_ARGS
#endif

#include <boost/variant.hpp>
#include <boost/type_traits.hpp>
#include <boost/call_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/integer.hpp>
#include <boost/concept_check.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/tuple/tuple.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/inserter.hpp>
#include <boost/mpl/sort.hpp>
#include <boost/mpl/unique.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/front_inserter.hpp>

#include <boost/preprocessor.hpp>

#pragma execution_character_set( "utf-8" )

#include "cmnFormatter_Utils.hpp"
#include "cmnFormatter_Parse.hpp"
#include "cmnFormatter_Renderder.hpp"
#include "cmnFormatter_Format.hpp"

#include "cmnFormatter_Integer.hpp"
#include "cmnFormatter_Float.hpp"
#include "cmnFormatter_String.hpp"
#include "cmnFormatter_Converter.hpp"
#include "cmnFormatter_Logger.hpp"

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
        }; // namespace nsCmnFmtDetail

    }; // namespace nsCmnFormatter

}; // namespace nsCommon

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {

#define FORMAT_EXPAND(...)          __VA_ARGS__
#define FORMAT_TYPE(z, n, unused)   BOOST_PP_COMMA_IF(n) typename boost::call_traits<T ## n>::param_type
#define FORMAT_DETAIL_FUNC(z, n, unused)    \
            template< typename Dest BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
            size_t format_internal( nsCmnFmtDetail::clsRenderer<Dest>& dest, const typename nsCmnFmtDetail::clsRenderer<Dest>::charType* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, T, arg ) ) \
            {                               \
                using namespace ::boost::mpl;   \
                typename nsCmnFmtDetail::clsRenderer<Dest>::charType fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE ] = {0,}; \
                BOOST_PP_EXPAND( FORMAT_EXPAND BOOST_PP_IF( n, (typedef vector ## n<BOOST_PP_REPEAT_ ## z(n, FORMAT_TYPE, 0)> template_types), (typedef vector1<int> template_types ) ) ); \
                typedef typename boost::mpl::sort< template_types, type_less, boost::mpl::front_inserter< vector0<> > >::type sorted_types;                                               \
                typedef typename boost::mpl::unique< sorted_types, boost::is_same< boost::mpl::_1, boost::mpl::_2 >, boost::mpl::front_inserter< boost::mpl::vector0<> > >::type unique_types;                                              \
                typedef boost::make_variant_over< typename unique_types::type > variant;                                                                                                                            \
                typename variant::type args[n] = { BOOST_PP_ENUM_PARAMS_Z(z, n, arg) };                                                                                                                             \
                return doFormat( dest, format, fmtBuffer, args, n ); \
            }

            BOOST_PP_REPEAT( BOOST_PP_ADD( FORMAT_MAX_ARGS, 1 ), FORMAT_DETAIL_FUNC, 0 )

#undef FORMAT_DETAIL_FUNC

#define FORMAT_DETAIL_FUNC(z, n, unused)    \
            template< typename Dest BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) >     \
            size_t format_internal_logger( nsCmnFmtDetail::clsRenderer<Dest>& dest, const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, const typename nsCmnFmtDetail::clsRenderer<Dest>::charType* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, T, arg ) )  \
            {                               \
                using namespace ::boost::mpl;   \
                typename nsCmnFmtDetail::clsRenderer<Dest>::charType fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE ] = {0,}; \
                BOOST_PP_EXPAND( FORMAT_EXPAND BOOST_PP_IF( n, (typedef boost::mpl::vector<BOOST_PP_REPEAT_ ## z(n,FORMAT_TYPE, 0), typename boost::call_traits< nsCmnFmtTypes::TyTpLoggerType >::param_type, typename boost::call_traits< const unsigned int >::param_type > template_types), (typedef vector1<int> template_types ) ) ); \
                typedef typename boost::mpl::sort< template_types, type_less, boost::mpl::front_inserter< vector0<> > >::type sorted_types;                                               \
                typedef typename boost::mpl::unique< sorted_types, boost::is_same< boost::mpl::_1, boost::mpl::_2 >, boost::mpl::front_inserter< boost::mpl::vector0<> > >::type unique_types;                                              \
                typedef boost::make_variant_over< typename unique_types::type > variant;                                                                                                                            \
                typename variant::type args[ n + 1 ] = { BOOST_PP_ENUM_PARAMS_Z(z, n, arg), tpLoggerType };   \
                return doFormat( dest, format, fmtBuffer, args, n + 1 ); \
            }

            BOOST_PP_REPEAT( BOOST_PP_ADD( FORMAT_MAX_ARGS, 1 ), FORMAT_DETAIL_FUNC, 0 )

#undef FORMAT_EXPAND
#undef FORMAT_TYPE
#undef FORMAT_DETAIL_FUNC

        }; // namespace nsCmnFmtDetail

        // 외부에서 사용할 함수들 선언
#define FORMAT_FUNCS(z, n, unused) \
        template< typename Dest, typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        size_t tsFormatBuffer( Dest& dest, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ) ) \
        {                       \
            nsCmnFmtDetail::clsRenderer<Dest> out( dest );  \
            return format_internal( out, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) ); \
        }   \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        size_t tsFormatBuffer( CharT*& buffer, size_t& bufferSize, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ) ) \
        {                       \
            nsCmnFmtDetail::clsRenderer< CharT* > out( buffer, bufferSize );              \
            return format_internal( out, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) ); \
        }                       \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        std::basic_string< CharT > tsFormat( const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg) ) \
        {                       \
            std::basic_string< CharT > s;    \
            tsFormatBuffer( s, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
            return s;           \
        }                       \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        size_t tsFormatBuffer( const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, CharT*& buffer, size_t& bufferSize, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ) ) \
        {                       \
            nsCmnFmtDetail::clsRenderer< CharT* > out( buffer, bufferSize );             \
            return format_internal_logger( out, tpLoggerType, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
        }                       \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        std::basic_string< CharT > tsFormat( const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg) ) \
        {                       \
            std::basic_string< CharT > s;    \
            nsCmnFmtDetail::clsRenderer< std::basic_string< CharT > > out(s);                                       \
            format_internal_logger( out, tpLoggerType, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
            return s;           \
        }                       

        BOOST_PP_REPEAT( BOOST_PP_ADD( FORMAT_MAX_ARGS, 1 ), FORMAT_FUNCS, 0 )

#undef FORMAT_FUNCS

    }; // namespace nsCmnFormatter

}; // namespace nsCommon
