#include "stdafx.h"

#include "cmnConverter.h"

#include "commonXMLMapperV2.h"

CXMLMapperV2& GetXMLMapper()
{
#if _MSC_VER >= 1600
    static CXMLMapperV2* pXMLMapper = nullptr;
    if( pXMLMapper == nullptr )
        pXMLMapper = new CXMLMapperV2;
#else
	static CXMLMapperV2* pXMLMapper = NULL;
	if( pXMLMapper == NULL )
		pXMLMapper = new CXMLMapperV2;
#endif

    return *pXMLMapper;
}

//////////////////////////////////////////////////////////////////////////

CXMLMapperV2::CXMLMapperV2()
{
    InitializeCriticalSectionAndSpinCount( &_cs, 4000 );

    // add a custom declaration node
    pugi::xml_node decl = _xmlDocument.prepend_child( pugi::node_declaration );

    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    decl.append_attribute("standalone") = "no";   

    _xmlSelectNode = _xmlDocument.root();
}

CXMLMapperV2::~CXMLMapperV2()
{
    for( tyXPathToObjectU8::iterator it = _xmlCache.begin(); it != _xmlCache.end(); ++it )
        delete it->second;

    DeleteCriticalSection( &_cs );
}

bool CXMLMapperV2::LoadFile( const char* pszu8FilePath )
{
    bool isSuccess = false;

    do 
    {
        pugi::xml_parse_result result = _xmlDocument.load_file( nsCommon::nsCmnConvert::CU82U(pszu8FilePath).c_str() );
        _xmlFilePath = pszu8FilePath;
        isSuccess = result;
    } while (false);

    return isSuccess;
}

bool CXMLMapperV2::LoadFile( const std::string& u8FilePath )
{
    bool isSuccess = false;

    do 
    {
        pugi::xml_parse_result result = _xmlDocument.load_file( nsCommon::nsCmnConvert::CU82U(u8FilePath).c_str() );
        _xmlFilePath = u8FilePath;
        isSuccess = result;
    } while (false);

    return isSuccess;
}

bool CXMLMapperV2::SaveFile( const char* pszu8FilePath /* = NULL */ )
{
    bool isSuccess = false;

    do 
    {
        EnterCriticalSection( &_cs );
        std::wstring xmlFilePath = pszu8FilePath == NULL ? nsCommon::nsCmnConvert::CU82U( _xmlFilePath ) : nsCommon::nsCmnConvert::CU82U( pszu8FilePath );
        isSuccess = _xmlDocument.save_file( xmlFilePath.c_str() );
        LeaveCriticalSection( &_cs );

    } while (false);

    return isSuccess;
}

bool CXMLMapperV2::SaveFile( const std::string& u8FilePath )
{
    bool isSuccess = false;

    do 
    {
        EnterCriticalSection( &_cs );
        std::wstring xmlFilePath = u8FilePath.empty() == true ? nsCommon::nsCmnConvert::CU82U( _xmlFilePath ) : nsCommon::nsCmnConvert::CU82U( u8FilePath );
        isSuccess = _xmlDocument.save_file( xmlFilePath.c_str() );
        LeaveCriticalSection( &_cs );
        
    } while (false);

    return isSuccess;
}

pugi::xml_node CXMLMapperV2::CreateNode( const std::string& nodeName, pugi::xml_node& nodeAfter /*= pugi::xml_node() */ )
{
    pugi::xml_node node;

    pugi::xml_node parentNode;
    if( nodeAfter.empty() == true )
        parentNode = _xmlSelectNode;
    else
        parentNode = nodeAfter;

    node = parentNode.append_child( nodeName.c_str() );

    return node;
}

pugi::xml_node CXMLMapperV2::CreateNodeRecursively( const std::string& query )
{
    pugi::xml_node node;
    pugi::xpath_variable_set vars;

    std::string::size_type pos = std::string::npos;
    std::string::size_type prevPos = 1;
    std::string nodePath;

    pugi::xml_node parentNode = _xmlDocument.root();

    do 
    {
        if( *query.begin() != '/' )
            break;

        prevPos = query.find( "/" );
        pos = query.find( "/", prevPos + 1 );

        do 
        {
            if( pos != std::string::npos )
                nodePath += query.substr( prevPos, pos - prevPos );
            else
                nodePath += query.substr( prevPos );
            
            if( IsExistNode( nodePath, vars ) == false )
            {
                std::string nodeName = nodePath.substr( nodePath.find_last_of( "/" ) + 1 );
                node = CreateNode( nodeName, parentNode );
                parentNode = node;
            }
            else
            {
                parentNode = GetNodeFromXPath( nodePath );
            }

            prevPos = pos;
            pos = query.find( "/", prevPos + 1 );

        } while ( prevPos != std::string::npos );

    } while( false );

    return node;
}

pugi::xml_node CXMLMapperV2::GetNode( const std::string& nodeName, pugi::xml_node& nodeParent /*= pugi::xml_node() */ )
{
    pugi::xml_node node, parent;

    if( nodeParent.empty() == true )
        parent = _xmlSelectNode;
    else
        parent = nodeParent;

    node = parent.child( nodeName.c_str() );

    return node;
}

pugi::xml_node CXMLMapperV2::GetNodeFromXPath( const std::string& query )
{
    return _xmlDocument.select_single_node( query.c_str() ).node();
}

pugi::xml_node CXMLMapperV2::GetNodeFromXPath( const std::string& query, pugi::xpath_variable_set& vars )
{
    return _xmlDocument.select_single_node( query.c_str(), &vars ).node();
}

pugi::xpath_node_set CXMLMapperV2::GetNodesFromXPath( const std::string& query )
{
	return _xmlDocument.select_nodes( query.c_str() );
}

pugi::xpath_node_set CXMLMapperV2::GetNodesFromXPath( const std::string& query, pugi::xpath_variable_set& vars )
{
	return _xmlDocument.select_nodes( query.c_str(), &vars );
}

bool CXMLMapperV2::SetValue( pugi::xml_node& node, bool rhs )
{
    if( node.empty() == true )
        return false;

    return node.text().set( rhs );
}

bool CXMLMapperV2::SetValue( pugi::xml_node& node, const std::string& rhs )
{
    if( node.empty() == true )
        return false;

    return node.text().set( rhs.c_str() );
}

bool CXMLMapperV2::SetValue( pugi::xml_node& node, int rhs )
{
    if( node.empty() == true )
        return false;

    return node.text().set( rhs );
}

bool CXMLMapperV2::SetValue( pugi::xml_node& node, unsigned int rhs )
{
    if( node.empty() == true )
        return false;

    return node.text().set( rhs );
}

bool CXMLMapperV2::SetValue( pugi::xml_node& node, double rhs )
{
    if( node.empty() == true )
        return false;

    return node.text().set( rhs );
}

void CXMLMapperV2::RefreshXML()
{
    if( _xmlFilePath.empty() == false )
    {
       EnterCriticalSection( &_cs );
		_xmlCache.clear();
//         for( tyXPathToObjectU8::iterator it = _xmlCache.begin(); it != _xmlCache.end(); ++it )
//             delete it->second;
		_xmlDocument.load_file( nsCommon::nsCmnConvert::CU82U( _xmlFilePath ).c_str() );

		LeaveCriticalSection( &_cs );

    }
}

bool CXMLMapperV2::IsExistNode( const std::string& query, pugi::xpath_variable_set& vars )
{
    bool isExist = false;

    do 
    {
        pugi::xpath_node xmlNode = _xmlDocument.select_single_node( query.c_str(), &vars );
        isExist = xmlNode.node().empty() ? false : true;

    } while (false);

    return isExist;
}
