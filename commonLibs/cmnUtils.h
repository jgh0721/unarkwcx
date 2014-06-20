#pragma once

#include <set>
#include <string>
#include <locale>
#include <unordered_map>

#pragma execution_character_set( "utf-8" )

#if _MSC_VER == 1500 
    #ifndef NULLPTR
        #define NULLPTR NULL
    #endif
#else
    #ifndef NULLPTR
        #define NULLPTR nullptr
    #endif
#endif

#define IF_FALSE_BREAK( var, expr ) if( ( var = (expr) ) == false ) break;
#define IF_SUCCESS_BREAK( var, expr ) if( ( var = (expr) ) == false ) break;

namespace nsCommon
{
    namespace detail
    {

    }

    /// 매개변수들 UTF-8 형태로 관리함
#if _MSC_VER >= 1600
    typedef std::unordered_map< std::string, std::string >              tyMapParams;
#else
    typedef std::tr1::unordered_map< std::string, std::string >         tyMapParams;
#endif

    const std::string		        SELECT_Y = "Y";
    const std::string		        SELECT_N = "N";
    const unsigned int              MAX_TIME_BUFFER_SIZE = 128;
    const unsigned int              MAX_PATH_LENGTH = 260;

    //////////////////////////////////////////////////////////////////////////

    std::string                     format( const char *fmt, ... );
    std::string                     format_arg_list( const char *fmt, va_list args );
    std::wstring                    format( const wchar_t *fmt, ... );
    std::wstring                    format_arg_list( const wchar_t *fmt, va_list args );

    bool                            endWith( const std::wstring& str, const std::wstring& with );
    bool                            startWith( const std::wstring& str, const std::wstring& with );

    bool                            IsAlphabet( const std::string& strString, std::locale loc = std::locale( "" ) );
    bool                            IsAlphabet( const std::wstring& strString, std::locale loc = std::locale( "" ) );

    bool                            IsNumber( const std::string& strString, std::locale loc = std::locale( "" ) );
    bool                            IsNumber( const std::wstring& strString, std::locale loc = std::locale( "" ) );
    
    bool                            isSelectY( const std::string& u8Text );
    bool                            isSelectY( const std::wstring& text );
#ifdef QSTRING_H
    bool                            isSelectY( const QString& text );
#endif

    size_t                          textlen( const char* str );
    size_t                          textlen( const wchar_t* str );

    std::wstring                    trim_left( const std::wstring& str );
    std::wstring                    trim_right( const std::wstring& str );

    std::string                     trim_left( const std::string& str );
    std::string                     trim_right( const std::string& str );

    int                             _wcsicmp( const std::wstring& lhs, const std::wstring& rhs );
    int                             _stricmp( const std::string& lhs, const std::string& rhs );
#ifdef _AFX
    int                             _wcsicmp( const CString& lhs, const CString& rhs );
#endif

    int                             _wtoi( const std::wstring& lhs );
    __int64                         _wtoi64( const std::wstring& lhs );

    int                             u8sicmp( const std::string& lhs, const std::string& rhs );
    int                             u8toi( const std::string& lhs );
    __int64                         u8toi64( const std::string& lhs );

    std::wstring                    getMajorVersion( const std::wstring& sVersion, const std::wstring delimiter = L"." );
    // 문자열에서 공백으로 구분된 후 특정 delmiter 를 제거한 단어를 반환한다. 
    // /MessageText:"fsdfsdfds" /MessageCode:232 
    // delimiter = /MessageText: 라면 "fsdfsdfds" 반환
    // delimiter = /MessageCode: 라면 "232" 반환
    std::wstring                    GetWordsUsingDelimiter( const std::wstring& strMessage, const std::wstring& delimiter );

    // http://www.codeproject.com/Articles/188256/A-Simple-Wildcard-Matching-Function
    bool                            WildcardMatch_straight( const TCHAR* pszString, const TCHAR* pszMatch );

    //////////////////////////////////////////////////////////////////////////

    template <typename T> 
    inline void DeletePtr( T& ptr )
    {
        if( ptr != NULLPTR )
        {
            delete ptr;
            ptr = NULLPTR;
        }
    }

    template <typename T> 
    inline void DeletePtrA( T& ptr )
    {
        if( ptr != NULLPTR )
        {
            delete[] ptr;
            ptr = NULLPTR;
        }
    }

    template <typename T> 
    inline void ReleasePtr( T& ptr )
    {
        if( ptr != NULLPTR )
        {
            ptr->Release();
            delete ptr;
            ptr = NULLPTR;
        }
    }

    template< typename T > 
    inline void DeleteMapContainerPointerValue( T& container )
    {
        for( typename T::iterator it = container.begin(); it != container.end(); ++it )
            delete (*it).second;
    }

    class eq_nocaseW
        : public std::binary_function< const std::wstring, const std::wstring, bool >
    {
    public:
        bool operator()( const std::wstring& lsh, const std::wstring& rsh ) const
        {
            return _wcsicmp( lsh, rsh ) == 0;
        }
    };

    class lt_nocaseW
        : public std::binary_function< const std::wstring, const std::wstring, bool>
    {
    public:
        bool operator()( const std::wstring& x, const std::wstring& y ) const
        {
            return _wcsicmp( x, y ) < 0;
        }
    };

    class lt_nocaseA
        : public std::binary_function< char*, char*, bool>
    {
    public:
        bool operator()( std::string x, std::string y ) const
        {
            return _stricmp( x.c_str(), y.c_str() ) < 0;
        }
    };

    // 구글 프로토콜 버퍼의 키-값 쌍의 목록을 tyMapParams 로 변환
    template< typename T >
    nsCommon::tyMapParams convertKVPairListToMap( const T& kvPairList )
    {
        nsCommon::tyMapParams mapParams;
        for( auto& it = kvPairList.begin( ); it != kvPairList.end( ); ++it )
            mapParams[ it->key( ) ] = it->value( );

        return mapParams;
    }

    template< typename T, typename R >
    R convertKVPairListToHash( const T& kvPairList )
    {
        R mapParams;
        for( auto& it = kvPairList.begin(); it != kvPairList.end(); ++it )
        {
            mapParams[ it->key() ] = it->value();
        }

        return mapParams;
    }

#if defined(QT_QTCORE_MODULE_H) || defined(QHASH_H)
    template< typename T >
    QHash< QString, QString > convertKVPairListToHash( const T& kvPairList )
    {
        QHash< QString, QString > mapParams;
        for( auto& it = kvPairList.begin(); it != kvPairList.end(); ++it )
        {
            mapParams[ QString::fromUtf8(it->key().c_str()) ] = QString::fromUtf8( it->value().c_str() );
        }

        return mapParams;
    }
#endif

    // 구글 프로토콜 버퍼의 메시지에 키-값 쌍 데이터를 설정
    // 구글 프로토콜 버퍼의 메시지에서는 key, value 가 선언되어 있어야함
    template< typename T, typename K, typename V >
    void setKVPairWithValue( T kvPair, const K& key, const V& value )
    {
        kvPair->set_key( key );
        kvPair->set_value( value );
    }

    typedef std::set< std::wstring, nsCommon::lt_nocaseW >              TySetTextNoCase;

}