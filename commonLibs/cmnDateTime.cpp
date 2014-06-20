#include "stdafx.h"

#include "cmnDateTime.h"
#include "cmnConverter.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnDateTime
    {
        eDayOfTheWeek GetDayOfTheWeek( __time64_t tmTime )
        {
            eDayOfTheWeek dayOfTheWeek = UNKONWN;
            tm localTime;
            _localtime64_s( &localTime, &tmTime );

            switch( localTime.tm_wday )
            {
                case 0:
                    dayOfTheWeek = SUNDAY;
                    break;
                case 1:
                    dayOfTheWeek = MONDAY;
                    break;
                case 2:
                    dayOfTheWeek = TUESDAY;
                    break;
                case 3:
                    dayOfTheWeek = WEDNESDAY;
                    break;
                case 4:
                    dayOfTheWeek = THURSDAY;
                    break;
                case 5:
                    dayOfTheWeek = FRIDAY;
                    break;
                case 6:
                    dayOfTheWeek = SATURDAY;
                    break;

                default:
                    dayOfTheWeek = UNKONWN;
                    break;
            }

            return dayOfTheWeek;
        }

        std::wstring GetFormattedDateText( __time64_t tmTime, std::wstring fmt /* = L"%Y년 %m월 %d일" */ )
        {
            return GetFormattedDateTimeText( tmTime, fmt );
        }

        std::wstring GetFormattedTimeText( __time64_t tmTime, std::wstring fmt /* = L"%H시 %M분" */ )
        {
            return GetFormattedDateTimeText( tmTime, fmt );
        }

        std::wstring GetFormattedDateTimeText( __time64_t tmTime, std::wstring fmt )
        {
            wchar_t wszBuffer[ 128 ] = { 0, };

            struct tm localTime;
            errno_t errno_ = _localtime64_s( &localTime, &tmTime );
            if( errno_ != 0 )
                return L"";

            if( wcsftime( wszBuffer, _countof( wszBuffer ), fmt.c_str(), &localTime ) == 0 )
                return L"";

            return wszBuffer;
        }


        __time64_t GetTimeValueFromFormat( const std::wstring& dateTimeText, const std::wstring& fmt /*= L"%Y-%m-%d %H:%M:%S" */ )
        {
            if( dateTimeText.empty() == true || fmt.empty() == true )
                return 0;

            int nIdx = 0;
            struct tm tmDate;

            tmDate.tm_year = 0;		// 1900 년 이후 흘러간 연도 수
            tmDate.tm_mon = 0;		// 0 ~ 11 로 나타난 현재 월
            tmDate.tm_mday = 1;		// 1 ~ 31 로 나타난 현재 일

            tmDate.tm_hour = 0;		// 0 ~ 23
            tmDate.tm_min = 0;		// 0 ~ 59
            tmDate.tm_sec = 0;		// 0 ~ 59

            tmDate.tm_wday = 0;		// 0 ~ 6, 일주일 표시 0 = Sun
            tmDate.tm_yday = 0;		// 1월 1일을 0 으로 한 0 ~ 365
            tmDate.tm_isdst = 0;

            for( std::wstring::size_type nPos = 0; nPos < fmt.size(); ++nPos )
            {
                switch( fmt[ nPos ] )
                {
                    case L'%':
                        if( fmt[ nPos + 1 ] == L'Y' )
                        {
                            tmDate.tm_year = _wtoi( dateTimeText.substr( nIdx, 4 ) ) - 1900;
                            nIdx += 3;
                            ++nPos;
                        }
                        if( fmt[ nPos + 1 ] == L'm' )
                        {
                            tmDate.tm_mon = _wtoi( dateTimeText.substr( nIdx, 2 ) ) - 1;
                            nIdx += 1;
                            ++nPos;
                        }
                        if( fmt[ nPos + 1 ] == L'd' )
                        {
                            tmDate.tm_mday = _wtoi( dateTimeText.substr( nIdx, 2 ) );
                            nIdx += 1;
                            ++nPos;
                        }

                        if( fmt[ nPos + 1 ] == L'H' )
                        {
                            tmDate.tm_hour = _wtoi( dateTimeText.substr( nIdx, 2 ) );
                            nIdx += 1;
                            ++nPos;
                        }

                        if( fmt[ nPos + 1 ] == L'p' )
                        {
                            std::wstring strAM, strPM;
                            if( strAM.empty() == true && strPM.empty() == true )
                            {
                                struct tm tmAM, tmPM;

                                tmAM.tm_year = 2012 - 1900;		// 1900 년 이후 흘러간 연도 수
                                tmAM.tm_mon = 1;		// 0 ~ 11 로 나타난 현재 월
                                tmAM.tm_mday = 1;		// 1 ~ 31 로 나타난 현재 일

                                tmAM.tm_hour = 1;		// 0 ~ 23
                                tmAM.tm_min = 1;		// 0 ~ 59
                                tmAM.tm_sec = 1;		// 0 ~ 59

                                tmAM.tm_wday = 0;		// 0 ~ 6, 일주일 표시 0 = Sun
                                tmAM.tm_yday = 0;		// 1월 1일을 0 으로 한 0 ~ 365
                                tmAM.tm_isdst = 0;

                                tmPM = tmAM;

                                tmPM.tm_hour = 13;
                                wchar_t szBuffer[ 64 ] = { 0, };
                                wcsftime( szBuffer, 64, L"%p", &tmAM );
                                strAM = szBuffer;
                                memset( szBuffer, '\0', sizeof(wchar_t)* 64 );

                                wcsftime( szBuffer, 64, L"%p", &tmPM );
                                strPM = szBuffer;
                            }

                            if( nIdx + strAM.size() < dateTimeText.size() && strAM.size() > 0 )
                            {
                                if( _wcsicmp( dateTimeText.substr( nIdx, strAM.size() ), strAM ) == 0 )
                                    tmDate.tm_hour = tmDate.tm_hour;
                            }

                            if( nIdx + strPM.size() < dateTimeText.size() && strPM.size() > 0 )
                            {
                                if( _wcsicmp( dateTimeText.substr( nIdx, strPM.size() ), strPM ) == 0 )
                                    tmDate.tm_hour += 12;
                            }

                            nIdx += 1;
                            ++nPos;
                        }

                        if( fmt[ nPos + 1 ] == L'I' )
                        {
                            tmDate.tm_hour += _wtoi( dateTimeText.substr( nIdx, 2 ) );
                            nIdx += 1;
                            ++nPos;
                        }

                        if( fmt[ nPos + 1 ] == L'M' )
                        {
                            tmDate.tm_min = _wtoi( dateTimeText.substr( nIdx, 2 ) );
                            nIdx += 1;
                            ++nPos;
                        }
                        if( fmt[ nPos + 1 ] == L'S' )
                        {
                            tmDate.tm_sec = _wtoi( dateTimeText.substr( nIdx, 2 ) );
                            nIdx += 1;
                            ++nPos;
                        }
                    default:
                        nIdx++;
                };

            }

            return _mktime64( &tmDate );
        }

        bool IsDate( const std::wstring& strString )
        {
            std::wstring::size_type pos = strString.find( L'-' );
            if( pos == std::wstring::npos )
            {
                if( strString.length() >= wcslen( L"YYYYmmdd" ) )
                {
                    unsigned int nYear = _wtoi( strString.substr( 0, 4 ).c_str() );
                    unsigned int nMonth = _wtoi( strString.substr( 4, 2 ).c_str() );
                    unsigned int nDay = _wtoi( strString.substr( 6, 2 ).c_str() );

                    if( nYear <= 0 || nYear > 3000 )
                        return false;
                    if( nMonth <= 0 || nMonth > 12 )
                        return false;
                    if( nDay <= 0 || nDay > 31 )
                        return false;

                    return true;
                }
            }
            else
            {
                if( strString.length() >= wcslen( L"YYYY-mm-dd" ) )
                {
                    unsigned int nYear = _wtoi( strString.substr( 0, 4 ).c_str() );
                    unsigned int nMonth = _wtoi( strString.substr( 5, 2 ).c_str() );
                    unsigned int nDay = _wtoi( strString.substr( 8, 2 ).c_str() );

                    if( nYear <= 0 || nYear > 3000 )
                        return false;
                    if( nMonth <= 0 || nMonth > 12 )
                        return false;
                    if( nDay <= 0 || nDay > 31 )
                        return false;

                    return true;
                }
            }

            return false;
        }

        std::wstring formatElapsedTime( unsigned int elapsedTimeSec, const std::wstring& fmt /*= L"%H 시간 %M 분 %S 초" */ )
        {
            unsigned int hour = 0;
            unsigned int minute = 0;
            unsigned int sec = 0;

            sec = elapsedTimeSec % 60;
            minute = elapsedTimeSec / 60;
            if( minute >= 60 )
            {
                hour = minute / 60;
                minute = minute % 60;
            }

            std::wstring formatText;

            if( fmt.find( L"%H" ) == std::wstring::npos )
            {
                formatText = nsCmnConvert::string_replace_all( formatText, L"%M", format( L"%d", minute + (hour * 60) ) );
            }
            else
            {
                formatText = nsCmnConvert::string_replace_all( fmt, L"%H", format( L"%d", hour ) );
                formatText = nsCmnConvert::string_replace_all( formatText, L"%M", format( L"%d", minute ) );
            }

            formatText = nsCmnConvert::string_replace_all( formatText, L"%S", format( L"%d", sec ) );

            return formatText;
        }
    }
}