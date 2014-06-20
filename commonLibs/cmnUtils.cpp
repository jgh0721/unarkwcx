#include "stdafx.h"

#include <cstdarg>

#include "cmnUtils.h"
#include "cmnConverter.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    std::string format( const char *fmt, ... )
    {
        va_list args;
        va_start( args, fmt );
        std::string s = format_arg_list( fmt, args );
        va_end( args );
        return s;
    }

    /* 유용한 std::string 에 대한 formatting 함수 */
    __inline std::string format_arg_list( const char *fmt, va_list args )
    {
        if( !fmt ) return "";
        int   result = -1, length = 512;
        char *buffer = 0;
        while( result == -1 )    {
            if( buffer )
                delete[] buffer;
            buffer = new char[ length + 1 ];
            memset( buffer, 0, (length + 1) * sizeof(char) );

            // remove deprecate warning
            //result = _vsnprintf(buffer, length, fmt, args);

            result = _vsnprintf_s( buffer, length, _TRUNCATE, fmt, args );
            length *= 2;
        }
        std::string s( buffer );
        delete[] buffer;
        return s;
    }

    std::wstring format( const wchar_t *fmt, ... )
    {
        va_list args;
        va_start( args, fmt );
        std::wstring s = format_arg_list( fmt, args );
        va_end( args );
        return s;
    }

    __inline std::wstring format_arg_list( const wchar_t *fmt, va_list args )
    {
        if( !fmt ) return L"";
        int   result = -1, length = 512;
        wchar_t *buffer = 0;
        while( result == -1 )    {
            if( buffer )
                delete[] buffer;
            buffer = new wchar_t[ length + 1 ];
            memset( buffer, 0, (length + 1) * sizeof(wchar_t) );

            // remove deprecate warning
            //result = _vsnprintf(buffer, length, fmt, args);

            result = _vsnwprintf_s( buffer, length, _TRUNCATE, fmt, args );
            length *= 2;
        }
        std::wstring s( buffer );
        delete[] buffer;
        return s;
    }

    bool endWith( const std::wstring& str, const std::wstring& with )
    {
        if( str.length() >= with.length() )
        {
            return (0 == str.compare( str.length() - with.length(), with.length(), with ));
        }
        else
        {
            return false;
        }
    }

    bool startWith( const std::wstring& str, const std::wstring& with )
    {
        if( str.length() >= with.length() )
        {
            return (0 == str.compare( 0, with.length(), with ));
        }
        else
        {
            return false;
        }
    }

    bool IsAlphabet( const std::string& strString, std::locale loc /*= std::locale("") */ )
    {
        for( std::string::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
        if( isalpha( *iterChar, loc ) == true )
            return true;
        return false;

    }

    bool IsAlphabet( const std::wstring& strString, std::locale loc /*= std::locale( "" ) */ )
    {
        for( std::wstring::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
        if( isalpha( *iterChar, loc ) == true )
            return true;
        return false;

    }

    bool IsNumber( const std::string& strString, std::locale loc /*= std::locale( "" ) */ )
    {
        for( std::string::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
        if( isdigit( *iterChar, loc ) == false )
            return false;

        return true;

    }

    bool IsNumber( const std::wstring& strString, std::locale loc /*= std::locale( "" ) */ )
    {
        for( std::wstring::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
        if( isdigit( *iterChar, loc ) == false )
            return false;
        return true;

    }

    bool isSelectY( const std::string& u8Text )
    {
        if( u8sicmp( u8Text, "Y" ) == 0 )
            return true;

        if( u8sicmp( u8Text, "T" ) == 0 )
            return true;

        if( u8sicmp( u8Text, "Yes" ) == 0 )
            return true;

        if( u8sicmp( u8Text, "True" ) == 0 )
            return true;

        return false;
    }

    bool isSelectY( const std::wstring& text )
    {
        if( _wcsicmp( text, L"Y" ) == 0 )
            return true;

        if( _wcsicmp( text, L"T" ) == 0 )
            return true;

        if( _wcsicmp( text, L"Yes" ) == 0 )
            return true;

        if( _wcsicmp( text, L"True" ) == 0 )
            return true;

        if( _wcsicmp( text, L"1" ) == 0 )
            return true;

        return false;
    }

#ifdef QSTRING_H
    bool isSelectY( const QString& text )
    {
        return isSelectY( text.toStdWString() );
    }
#endif

    size_t textlen( const char* str )
    {
        return strlen( str );
    }

    size_t textlen( const wchar_t* str )
    {
        return wcslen( str );
    }

    std::wstring trim_left( const std::wstring& str )
    {
        size_t n = str.find_first_not_of( L" \t\v\n" );
        return n == std::wstring::npos ? str : str.substr( n, str.length() );
    }

    std::wstring trim_right( const std::wstring& str )
    {
        size_t n = str.find_last_not_of( L" \t\v\n" );
        return n == std::wstring::npos ? str : str.substr( 0, n + 1 );
    }

    std::string trim_left( const std::string& str )
    {
        size_t n = str.find_first_not_of( " \t\v\n" );
        return n == std::string::npos ? str : str.substr( n, str.length() );
    }

    std::string trim_right( const std::string& str )
    {
        size_t n = str.find_last_not_of( " \t\v\n" );
        return n == std::string::npos ? str : str.substr( 0, n + 1 );
    }

    int _wcsicmp( const std::wstring& lhs, const std::wstring& rhs )
    {
        return ::_wcsicmp( lhs.c_str(), rhs.c_str() );
    }

    int _stricmp( const std::string& lhs, const std::string& rhs )
    {
        return ::_stricmp( lhs.c_str(), rhs.c_str() );
    }

#ifdef _AFX
    int _wcsicmp( const CString& lhs, const CString& rhs )
    {
        return ::_wcsicmp( lhs.GetString(), rhs.GetString() );
    }
#endif

    int _wtoi( const std::wstring& lhs )
    {
        return ::_wtoi( lhs.c_str() );
    }

    __int64 _wtoi64( const std::wstring& lhs )
    {
        return ::_wtoi64( lhs.c_str() );
    }

    int u8sicmp( const std::string& lhs, const std::string& rhs )
    {
        return ::_wcsicmp( nsCmnConvert::CU82U( lhs ), nsCmnConvert::CU82U( rhs ) );
    }

    int u8toi( const std::string& lhs )
    {
        return ::_wtoi( nsCmnConvert::CU82U( lhs ) );
    }

    __int64 u8toi64( const std::string& lhs )
    {
        return ::_wtoi64( nsCmnConvert::CU82U( lhs ) );
    }

    std::wstring getMajorVersion( const std::wstring& sVersion, const std::wstring delimiter )
    {
        if( sVersion.find( delimiter ) == -1 )
            return L"";

        std::wstring sMajorVer;
        sMajorVer.clear();

        sMajorVer = sVersion.substr( 0, sVersion.find( delimiter ) );

        return sMajorVer;
    }

    std::wstring GetWordsUsingDelimiter( const std::wstring& strMessage, const std::wstring& delimiter )
    {
        bool isFoundQuote = false;

        size_t delimLength = delimiter.size();

        std::wstring::size_type startPos = strMessage.find( delimiter );
        std::wstring::size_type endPos = std::wstring::npos;

        if( startPos == std::wstring::npos )
            return L"";

        startPos += delimLength;

        if( strMessage[ startPos ] == L'\"' )
        {
            isFoundQuote = true;
            startPos++;
        }

        endPos = strMessage.find( isFoundQuote == false ? L" " : L"\"", startPos + 1 );
        if( endPos == std::wstring::npos )
            return strMessage.substr( startPos );

        return strMessage.substr( startPos, endPos - startPos );
    }

    bool WildcardMatch_straight( const TCHAR* pszString, const TCHAR* pszMatch )
    {
        const TCHAR *mp = NULL;
        const TCHAR *cp = NULL;

        while( *pszString )
        {
            if( *pszMatch == _T( '*' ) )
            {
                if( !*++pszMatch )
                    return true;
                mp = pszMatch;
                cp = pszString + 1;
            }
            else if( *pszMatch == _T( '?' ) || _totupper( *pszMatch ) == _totupper( *pszString ) )
            {
                pszMatch++;
                pszString++;
            }
            else if( !cp )
                return false;
            else
            {
                pszMatch = mp;
                pszString = cp++;
            }
        }

        while( *pszMatch == _T( '*' ) )
            pszMatch++;

        return !*pszMatch;
    }

}