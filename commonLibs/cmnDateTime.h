#pragma once

#include <string>
#include <time.h>

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnDateTime
    {
        enum eDayOfTheWeek
        {
            UNKONWN = 0,
            SUNDAY = 1,
            MONDAY = 2,
            TUESDAY = 4,
            WEDNESDAY = 8,
            THURSDAY = 16,
            FRIDAY = 32,
            SATURDAY = 64
        };

        const char* const QT_DEFAULT_SYNC_DATE_FORMAT = "yyyy-MM-dd";
        const char* const QT_DEFAULT_SYNC_TIME_FORMAT = "HH:mm:ss";
        const char* const QT_DEFAULT_SYNC_DATETIME_FORMAT = "yyyy-MM-dd HH:mm:ss";

        eDayOfTheWeek GetDayOfTheWeek( __time64_t tmTime );

        std::wstring GetFormattedDateTimeText( __time64_t tmTime, std::wstring fmt = L"%Y-%m-%d %H:%M:%S" );

        std::wstring GetFormattedDateText( __time64_t tmTime, std::wstring fmt = L"%Y년 %m월 %d일" );
        std::wstring GetFormattedTimeText( __time64_t tmTime, std::wstring fmt = L"%H시 %M분" );

        __time64_t	GetTimeValueFromFormat( const std::wstring& dateTimeText, const std::wstring& fmt = L"%Y-%m-%d %H:%M:%S" );

        /*
        전송받은 문자열이 날짜형식인지 점검
        점검 문자열 형식 "%Y-%m-%d", "%Y%m%d"
        */
        bool IsDate( const std::wstring& strString );

        // 초단위의 숫자를 넘겨서 경과시간을 %H %M %S  를 사용한 문자열로 서식화, 
        std::wstring formatElapsedTime( unsigned int elapsedTimeSec, const std::wstring& fmt = L"%H 시간 %M 분 %S 초" );
    }
}