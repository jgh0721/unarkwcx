#pragma once

#include <unordered_map>
#include <boost/lexical_cast.hpp>

#include "pugixml.hpp"

/*!
    XPath �� XML �� ���ϰ� �̿��ϱ� ���� XML ���� Ŭ����

*/

template< typename T = std::string, typename T2 = std::string >
struct tagXMLItemV2
{
    T2                      xmlQuery;
    T                       xmlDefaultValue;

    operator const T2() const { return xmlQuery; };
};

class CXMLMapperV2
{
public:
    CXMLMapperV2();
    ~CXMLMapperV2();

    bool                LoadFile( const char* pszu8FilePath );
    bool                LoadFile( const std::string& u8FilePath );
    
    bool                SaveFile( const char* pszu8FilePath = NULL );
    bool                SaveFile( const std::string& u8FilePath );

    std::string         GetFilePathU8() { return _xmlFilePath; }

    void                RefreshXML();

    bool                IsExistNode( const std::string& query, pugi::xpath_variable_set& vars );

    // nodeAfter �ؿ� �ڽ� ��带 �����Ѵ�. nodeAfter �� �������� ������ ���� ���õ� ��忡 �ڽ� ��带 �����Ѵ�.
    pugi::xml_node      CreateNode( const std::string& nodeName, pugi::xml_node& nodeAfter = pugi::xml_node() );
    // /A/B/C/ �������� ������ ��带 ���������� �����Ѵ�. 
    pugi::xml_node      CreateNodeRecursively( const std::string& query );
    // �ش��̸��� ���� �ڽĳ�带 �˻���. ���� �̸��� ���� �ڽĳ�尡 ������ ��� ù��° �˻��� �ڽĳ�带 ��ȯ
    pugi::xml_node      GetNode( const std::string& nodeName, pugi::xml_node& nodeParent = pugi::xml_node() );
    pugi::xml_node      GetNodeFromXPath( const std::string& query );
    pugi::xml_node      GetNodeFromXPath( const std::string& query, pugi::xpath_variable_set& vars );
	pugi::xpath_node_set	GetNodesFromXPath( const std::string& query );
	pugi::xpath_node_set	GetNodesFromXPath( const std::string& query, pugi::xpath_variable_set& vars );
	
    bool                SetValue( pugi::xml_node& node, bool rhs );
    bool                SetValue( pugi::xml_node& node, const std::string& rhs );
    bool                SetValue( pugi::xml_node& node, int rhs );
    bool                SetValue( pugi::xml_node& node, unsigned int rhs );
    bool                SetValue( pugi::xml_node& node, double rhs );

    pugi::xml_document& GetDocument() { return _xmlDocument; };

    template< typename T >
    T                   GetDataFromItem( const tagXMLItemV2<T>& item )
    {
        T retValue = item.xmlDefaultValue;

        EnterCriticalSection( &_cs );
        
        do 
        {
            if( item.xmlQuery.empty() == true )
                break;

            pugi::xpath_node xmlNode = _xmlDocument.select_single_node( item.xmlQuery.c_str() );
            if( xmlNode.node().empty() == true )
                break;

            try
            {
                retValue = boost::lexical_cast<T>( ( const char* )xmlNode.node().text().as_string() );
            }
            catch( boost::bad_lexical_cast& e )
            {
                std::cerr << "CXMLMapper::GetDataFromItem " << e.what();
                UNREFERENCED_PARAMETER(e);
                break;
            }
            catch( boost::bad_numeric_cast& e )
            {
                std::cerr << "CXMLMapper::GetDataFromItem " << e.what();
                UNREFERENCED_PARAMETER(e);
                break;
            }

        } while (false);

        LeaveCriticalSection( &_cs );

        return retValue;
    };

    template<>
    bool                   GetDataFromItem( const tagXMLItemV2<bool>& item )
    {
        try
        {
            pugi::xpath_query* pQuery = NULL;

            EnterCriticalSection( &_cs );

            if( _xmlCache.count( item.xmlQuery ) <= 0 )
            {
                pQuery = new pugi::xpath_query( item.xmlQuery.c_str() );
                _xmlCache.insert( std::make_pair( item.xmlQuery, pQuery ) );
            }
            pQuery = _xmlCache[ item.xmlQuery ];

            pugi::string_t result = pQuery->evaluate_string( _xmlDocument.root() );

            LeaveCriticalSection( &_cs );

            if( _stricmp( result.c_str(), "true" ) == 0 )
                return true;
            else if( _stricmp( result.c_str(), "false" ) == 0 )
                return false;
            else
                return item.xmlDefaultValue;
        }
        catch( const pugi::xpath_exception& e )
        {
            UNREFERENCED_PARAMETER(e);
            LeaveCriticalSection( &_cs );
            return item.xmlDefaultValue;
        }
    };

    template< typename T >
    void                SetDataFromItem( const tagXMLItemV2<T>& item, const T& value )
    {
        pugi::xpath_query* pQuery = NULL;
        pugi::xpath_variable_set vars;

        EnterCriticalSection( &_cs );

        if( _xmlCache.count( item.xmlQuery ) <= 0 )
        {
            pQuery = new pugi::xpath_query( item.xmlQuery.c_str() );
            _xmlCache.insert( std::make_pair( item.xmlQuery, pQuery ) );
        }
        pQuery = _xmlCache[ item.xmlQuery ];

        if( IsExistNode( item.xmlQuery, vars ) == false )
            CreateNodeRecursively( item.xmlQuery );

        pugi::xpath_node_set nodeSet = pQuery->evaluate_node_set( _xmlDocument.root() );
        for( size_t idx = 0; idx < nodeSet.size(); ++idx )
        {
            if( nodeSet[ idx ].node().empty() == false )
                nodeSet[ idx ].node().text().set( value );
        }

        LeaveCriticalSection( &_cs );
    }

    template< typename T >
    void                SetDataFromItemWithVars( const tagXMLItemV2<T>& item, const T& value, pugi::xpath_variable_set& vars )
    {
        pugi::xpath_query query( item.xmlQuery.c_str(), &vars );
        pugi::xpath_variable_set vars;

        if( IsExistNode( item.xmlQuery, vars ) == false )
            CreateNodeRecursively( item.xmlQuery );

        pugi::xpath_node_set xmlNodeSet = query.evaluate_node_set( _xmlDocument.root() );

        for( size_t idx = 0; idx < xmlNodeSet.size(); ++idx )
        {
            if( xmlNodeSet[ idx ].node().empty() == false )
                xmlNodeSet[ idx ].node().text().set( value );
        }
    }

private:
#if _MSC_VER >= 1600 
	typedef std::unordered_map< std::string, pugi::xpath_query* >       tyXPathToObjectU8;
	typedef std::unordered_map< std::wstring, pugi::xpath_query* >      tyXPathToObjectU16;
#else
    typedef std::tr1::unordered_map< std::string, pugi::xpath_query* >       tyXPathToObjectU8;
    typedef std::tr1::unordered_map< std::wstring, pugi::xpath_query* >      tyXPathToObjectU16;
#endif

    CRITICAL_SECTION        _cs;
    tyXPathToObjectU8       _xmlCache;
    pugi::xml_document      _xmlDocument;
    pugi::xml_node          _xmlSelectNode;
    std::string             _xmlFilePath;
};

CXMLMapperV2& GetXMLMapper();
