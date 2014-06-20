#ifndef CMNFORMATTER_PARSE_H
#define CMNFORMATTER_PARSE_H

#include <stdlib.h>
#include <process.h>

#if _MSC_VER < 1800
#include <boost/variant.hpp>
#endif

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
            static inline 
                unsigned long strtoul( const char* __restrict str, const char* __restrict * endptr, int base )
            {
                return ::strtoul( str, const_cast<char **>( endptr ), base );
            }

            static inline
                unsigned long strtoul( const wchar_t* __restrict str, const wchar_t* __restrict * endptr, int base )
            {
                return ::wcstoul( str, const_cast< wchar_t** >( endptr ), base );
            }

            static inline 
                errno_t itoa( int Val, char* DstBuf, size_t BufferSize )
            {
                return ::_itoa_s( Val, DstBuf, BufferSize, 10 );
            }

            static inline
                errno_t itoa( int Val, wchar_t* DstBuf, size_t BufferSize )
            {
                return ::_itow_s( Val, DstBuf, BufferSize, 10 );
            }

#if _MSC_VER >= 1800
            template< typename CharT >
            void fmtParse( const CharT* __restrict& format, formatSpec& fmtSpec, CharT* fmtBuffer, const CharT*& prefix, long& prefixLength, long argCount )
#else
            template< typename CharT, typename Variant >
            void fmtParse( const CharT* __restrict& format, formatSpec& fmtSpec, CharT*& fmtBuffer, const CharT*& prefix, long& prefixLength, Variant* args, long argCount )
#endif
            {
                /*!
                    %로 시작되는 형식 문자열에 속하지 않는, 대상으로 그대로 복사되어야할 문자열들
                    qwerty%1fddsdfsd 라면 qwerty 가 prefix에 fmtSpec 에는 %1 이 들어간다. 
                    다음번 분석에는 fddsdfsd 가 prefix 가 들어간다. 
                */

                fmtSpec.flags           = 0;
                fmtSpec.argIndex        = 0;

                prefix = format;
                prefixLength = 0;
                CharT* extraBuffer = fmtBuffer;

                // 인자가 나오기 전까지 prefix 로 포함시킴
                while( *format && *format != '%' )
                    ++prefixLength, ++format;

                do 
                {
                    if( *format == 0 )
                    {
                        fmtSpec.argIndex = -1;
                        break;
                    }

                    ++format;

                    // %% 를 입력했으면 % 문자를 넣고 끝내도록 함
                    if( *format == '%' )
                    {
                        fmtSpec.argIndex = 0;
                        ++prefixLength, ++format;
                        break;
                    }
                    
                    // % 다음에 숫자( 매개변수 색인 ) 로 시작하는지 점검
                    if( ::isdigit( *format ) == false )
                        break;

                    fmtSpec.argIndex = strtoul( format, &format, 10 );
                    if( format[0] != '{' )
                        break;

                    ++format;   // { 문자 건너뛰기

                    if( fmtSpec.argIndex == 0 )
                    {
						SleepEx( 0, TRUE );	// 릴리즈시 prifix가 정상적으로 적용 되지 않는 문제 임시 조치

                        // %0 특수 문법에 대한 처리
                        if( format[0] == 'T' && format[1] == 'I' && format[2] == 'D' )
                        {
                            itoa( ::GetCurrentThreadId(), extraBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );
                        }
                        else if( format[0] == 'P' && format[1] == 'I' && format[2] == 'D' )
                        {
                            itoa( _getpid(), extraBuffer, nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );
                        }
                        else if( format[0] == 'F' && format[1] == 'I' && format[2] == 'L' && format[3] == 'E' )
                        {
                            fmtSpec.loggerFileName = 1;
                            fmtSpec.argIndex = argCount;
                        }
                        else if( format[0] == 'F' && format[1] == 'U' && format[2] == 'N' && format[3] == 'C' )
                        {
                            fmtSpec.loggerFuncName = 1;
                            fmtSpec.argIndex = argCount;
                        }
                        else if( format[0] == 'L' && format[1] == 'I' && format[2] == 'N' && format[3] == 'E' )
                        {
                            fmtSpec.loggerLineNumber = 1;
                            fmtSpec.argIndex = argCount;
                        }
                        else if( format[0] == 'D' && format[1] == 'A' && format[2] == 'T' && format[3] == 'E' )
                        {
                            fmtSpec.datetime = true;

                            if( sizeof( CharT ) == sizeof(char) )
                                ::memcpy( fmtBuffer, nsCmnFmtTypes::DEFAULT_DATE_FORMAT_A, sizeof( CharT ) * strlen( nsCmnFmtTypes::DEFAULT_DATE_FORMAT_A ) );
                            else
                                ::memcpy( fmtBuffer, nsCmnFmtTypes::DEFAULT_DATE_FORMAT_W, sizeof( CharT ) * wcslen( nsCmnFmtTypes::DEFAULT_DATE_FORMAT_W ) );

                            formatDateTime( fmtBuffer, fmtSpec, ::time(NULL) );
                        }
                        else if( format[0] == 'T' && format[1] == 'I' && format[2] == 'M' && format[3] == 'E' )
                        {
                            fmtSpec.datetime = true;

                            if( sizeof( CharT ) == sizeof(char) )
                                ::memcpy( fmtBuffer, nsCmnFmtTypes::DEFAULT_TIME_FORMAT_A, sizeof( CharT ) * strlen( nsCmnFmtTypes::DEFAULT_TIME_FORMAT_A ) );
                            else
                                ::memcpy( fmtBuffer, nsCmnFmtTypes::DEFAULT_TIME_FORMAT_W, sizeof( CharT ) * wcslen( nsCmnFmtTypes::DEFAULT_TIME_FORMAT_W ) );

                            formatDateTime( fmtBuffer, fmtSpec, ::time(NULL) );
                        }
                        else if( format[0] == 'L' && format[1] == 'O' && format[2] == 'G' && format[3] == 'G' && format[4] == 'E' && format[5] == 'R' )
                        {
                            fmtSpec.loggerName = 1;
                            fmtSpec.argIndex = argCount;
                        }
                        else if( format[0] == 'L' && format[1] == 'O' && format[2] == 'G' && format[3] == 'L' && format[4] == 'E' && format[5] == 'V' && format[6] == 'E' && format[7] == 'L' )
                        {
                            fmtSpec.loggerLogLevel = 1;
                            fmtSpec.argIndex = argCount;
                        }
                    }
                    else
                    {
                        // 인자 색인 번호에 따른 if 문
                        // 일반 형식 지정자 처리
                        if( *format == 'U' )
                        {
                            fmtSpec.upperLetter         = true;
                        }
                        else if( *format == '+' )
                        {
                            fmtSpec.plus                = true;
                        }
                        else if( format[0] == 'D' && format[1] == 'T' )
                        {
                            format = format + 2;
                            fmtSpec.datetime            = true;

                            if( format[0] == ':' )
                            {
                                while( (*format) && (*++format != '}') )
                                    *extraBuffer++ = *format;
                            }
                        }
                        else if( format[0] == 'E' && format[1] == 'R' && format[2] == 'R' )
                        {
                            fmtSpec.errorText           = true;

                            format = format + 3;
                        }
                        else if( format[0] == 'H' && format[1] == 'E' && format[2] == 'R' && format[3] == 'R' )
                        {
                            fmtSpec.errorHRESULT        = true;

                            format = format + 4;
                        }
                    }  

                    while( (*format) && (*format != '}') )
                        ++format;

                    if( *format && *format == '}' )
                        ++format;

                } while (false);

            } // function fmtParse

        } // namespace nsCmnFmtDetail

    } // namespace nsFormatter

} // namespace nsCommon

#endif // CMNFORMATTER_PARSE_H
