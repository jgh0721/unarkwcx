#ifndef CMNFORMATTER_FORMAT_H
#define CMNFORMATTER_FORMAT_H

#include "cmnFormatter_Parse.hpp"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            struct unknown_type_tag
            { };

            template< typename T, typename Enabled = void >
            struct type_tag
            {
                typedef unknown_type_tag            type;
            };

#if _MSC_VER >= 1800
            template< typename Renderer >
            class clsFormatterEx : public tek::return_type < size_t >
            {
            public:
                Renderer&                            renderer_;
                const nsCmnFmtDetail::formatSpec&    fmtSpec_;
                typename Renderer::charType*         fmtBuffer_;

                clsFormatterEx( Renderer&renderer, const nsCmnFmtDetail::formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer )
                    : renderer_(renderer), fmtSpec_(fmtSpec), fmtBuffer_( fmtBuffer )
                {}

                template< typename T >
                size_t operator()( T& value ) const
                {
                    return formatValue( renderer_, fmtSpec_, fmtBuffer_, value );
                }
            };
#else
            /*!
                Boost::Variant 에 담겨진 인자들을 형식에 맞게 순회하기 위해 Visitor 패턴을 사용한다. 

                Boost::apply_visitor 라는 함수를 통해 호출됨, static_visitor 의 long 매개변수는 args 의 색인번호
            */
            template< typename Renderer >
            class clsFormatter : public boost::static_visitor< size_t >
            {
                Renderer&                               renderer_;
                const formatSpec&                       fmtSpec_;
                typename Renderer::charType*            fmtBuffer_;

            public:
                clsFormatter( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer )
                    : renderer_( renderer ), fmtSpec_( fmtSpec ), fmtBuffer_( fmtBuffer )
                { }

                template< typename T >
                size_t operator()( const T& value ) const 
                {
                    return formatValue( renderer_, fmtSpec_, fmtBuffer_, value );
                }
            };
#endif
            //
            // Remove const and volatile qualifiers from the type, to
            // minimize the number of specializations of type_tag
            // required (e.g. char*, const char*, volatilie char*, etc).
            // We can't just use boost::remove_cv<> here, because that
            // doesn't do what we want for pointers.  It removes the
            // const and volatile qualifiers from the pointer, but not
            // from the pointed-to type.
            // 
            template<typename T>
            struct basic_type
            {
            private:
                typedef BOOST_DEDUCED_TYPENAME boost::remove_reference<T>::type Ty;

            public:
                typedef BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<
                    boost::is_pointer<Ty>,
                    boost::add_pointer<
                        BOOST_DEDUCED_TYPENAME boost::remove_cv<
                            BOOST_DEDUCED_TYPENAME boost::remove_pointer<Ty>::type
                        >::type
                    >,
                    boost::mpl::identity<Ty>
                >::type type;
            };

            //
            // To provide a format_value overload for multiple types
            // using a template, we use tag-based dispatching.  The
            // type_tag struct provides the tag, and the format_value
            // template function takes the tag as an extra argument
            // so the compiler can find it.
            //
            // To provide a format_value() overload for a single type,
            // we just use a non-template function (or, rather, a
            // template for the output type, but not for the value
            // type).
            //
            template<typename Renderer, typename T, typename Tag>
            size_t formatValue( Renderer& renderer, const formatSpec& fspec, typename Renderer::charType* fmtBuffer, const T& value, Tag );

            template<typename Renderer, typename T>
            size_t formatValue( Renderer& renderer, const formatSpec& fspec, typename Renderer::charType* fmtBuffer, const T& value )
            {
                typename type_tag< typename basic_type<T>::type >::type tag;

                return formatValue(renderer, fspec, fmtBuffer, value, tag);
            }

#if _MSC_VER < 1800
            template< typename Renderer, typename CharT, typename Variant >
            size_t doFormat( Renderer& renderer, const CharT* __restrict format, CharT* fmtBuffer, Variant* args, long argCount )
            {
                formatSpec fmtSpec;
                const CharT* prefix = NULL;
                long prefixLength = 0;
                size_t nRetFormattedCount = 0;

                while( format )
                {
                    ::memset( fmtBuffer, '\0', sizeof( CharT ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );

                    fmtParse( format, fmtSpec, fmtBuffer, prefix, prefixLength, args, argCount );

                    if( fmtSpec.argIndex > 0 && fmtSpec.argIndex > argCount )
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

                    /*!
                        clsTsFormatter 의 operator() 를 호출
                        formatValue 를 호출하며 해당 함수는 각 형식에 맞게 오버로딩 된 함수가 있다면 해당함수를 호출하고 그렇지 않으면 
                        long formatValue( Renderer& renderer, const formatSpec& fspec, const T& value ) 를 호출한 후 type_tag 에 따라 dispatch 됨
                    */
                    nRetFormattedCount += boost::apply_visitor( clsFormatter< Renderer >( renderer, fmtSpec, fmtBuffer ), args[ fmtSpec.argIndex - 1 ] );
                }

                return nRetFormattedCount;
            }
#endif
        } // namespace nsCmnFmtDetail

    } // namespace nsCmnFormatter

} // namespace nsCommon

#endif // CMNFORMATTER_FORMAT_H
