#pragma once

#include <memory.h>

#include "cmnFormatter_Utils.hpp"

#pragma execution_character_set( "utf-8" )

// 렌더러 선언, 렌더러를 이용하여 버퍼에 데이터를 기록함
namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            template< typename CharT >
            class clsRendererBase
            {
            public:
                typedef         CharT               charType;
            };

            template< typename CharT >
            class clsRenderer;

            template< typename CharT >
            class clsRenderer< CharT* > : public clsRendererBase< CharT >
            {
                CharT*&                     textBuffer_;
                size_t&                     bufferSize_;
                size_t                      remainBufferSize_;
            public:
                clsRenderer( CharT*& dest, size_t& bufferSize )
                    : textBuffer_(dest), bufferSize_(bufferSize), remainBufferSize_(bufferSize)
                { };

                clsRenderer( CharT*& dest, size_t& bufferSize, size_t remainSize )
                    : textBuffer_(dest), bufferSize_(bufferSize), remainBufferSize_(remainSize)
                { };
                
                size_t                      getBufferSize() { return bufferSize_; };
                template< typename OtherCharT >
                size_t render( const OtherCharT* rsh, long rshLength, 
                    typename boost::enable_if_c< boost::is_same< CharT, OtherCharT >::value >::type * = 0 )
                {
                    size_t beforeLength = bufferSize_ - remainBufferSize_;

                    size_t nLength = rshLength > 0 ? rshLength : getTextLength(rsh);
                    getBuffer( nLength + 1 /* NULL 문자를 위해 한글자 더 요청함 */ + 2 /* 로거에서 개행문자 삽입을 위해 여분으로 넣음*/, bufferSize_, remainBufferSize_, textBuffer_ );

                    ::memcpy( &textBuffer_[ bufferSize_ - remainBufferSize_ ], rsh, sizeof( OtherCharT ) * nLength );
                    remainBufferSize_ -= nLength;

                    return bufferSize_ - remainBufferSize_ - beforeLength;
                }

                template< typename OtherCharT >
                size_t render( const OtherCharT* rsh, long rshLength, 
                    typename boost::enable_if_c< boost::is_same< CharT, char >::value && boost::is_same< OtherCharT, wchar_t >::value >::type * = 0 )
                {
                    std::auto_ptr< char > apBuffer( convertWideCharToMultiByte( rsh, rshLength ) );
                    return render( apBuffer.get(), (long)getTextLength( apBuffer.get() ) );
                }

                template< typename OtherCharT >
                size_t render( const OtherCharT* rsh, long rshLength, 
                    typename boost::enable_if_c< boost::is_same< CharT, wchar_t >::value && boost::is_same< OtherCharT, char >::value >::type * = 0 )
                {
                    std::auto_ptr< wchar_t > apBuffer( convertMultiByteToWideChar( rsh, rshLength ) );
                    return render( apBuffer.get(), (long)getTextLength( apBuffer.get() ) );
                }
                
            };

            template< typename CharT, typename TraitsT, typename AllocT >
            class clsRenderer< std::basic_string< CharT, TraitsT, AllocT > > : public clsRendererBase< CharT >
            {
                std::basic_string< CharT, TraitsT, AllocT >&             string_;

            public:
                clsRenderer( std::basic_string< CharT, TraitsT, AllocT >& dest )
                    : string_( dest )
                { string_.clear(); };

                template< typename OtherCharT >
                size_t render( const OtherCharT* rsh, long rshLength, 
                    typename boost::enable_if_c< boost::is_same< CharT, OtherCharT >::value >::type * = 0 )
                {
                    size_t nBeforeLength = string_.length();

                    if( rshLength <= 0 )
                        string_.append( (CharT*)rsh );
                    else
                        string_.append( (CharT*)rsh, rshLength );

                    return string_.length() - nBeforeLength;
                }

                template< typename OtherCharT >
                size_t render( const OtherCharT* rsh, long rshLength, 
                    typename boost::enable_if_c< boost::is_same< CharT, char >::value && boost::is_same< OtherCharT, wchar_t >::value >::type * = 0 )
                {
                    size_t nBeforeLength = string_.length();

                    std::auto_ptr< char > apBuffer( convertWideCharToMultiByte( rsh, rshLength ) );

                    string_.append( apBuffer.get() );

                    return string_.length() - nBeforeLength;
                }

                template< typename OtherCharT >
                size_t render( const OtherCharT* rsh, long rshLength, 
                    typename boost::enable_if_c< boost::is_same< CharT, wchar_t >::value && boost::is_same< OtherCharT, char >::value >::type * = 0 )
                {
                    size_t nBeforeLength = string_.length();

                    std::auto_ptr< wchar_t > apBuffer( convertMultiByteToWideChar( rsh, rshLength ) );

                    string_.append( apBuffer.get() );

                    return string_.length() - nBeforeLength;
                }

            };

        }; // namespace nsCmnFmtDetail

    }; // namespace nsCmnFormatter

}; // namespace nsCommon