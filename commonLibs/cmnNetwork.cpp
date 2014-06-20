#include "stdafx.h"

#include <string>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#ifdef USE_NETWORK_SUPPORT
#include <IPHlpApi.h>
#pragma comment(lib, "iphlpapi" )
#endif

#include "cmnUtils.h"
#include "cmnNetwork.h"
#include "cmnConverter.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnNetwork
    {
        unsigned long getULONGFromIPAddress( unsigned char lsb, unsigned char lsbBy1, unsigned char lsbBy2, unsigned char lsbBy3 )
        {
            unsigned long ipaddress = 0;

            ipaddress = lsb | (lsbBy1 << 8);
            ipaddress = ipaddress | (lsbBy2 << 16);
            ipaddress = ipaddress | (lsbBy3 << 24);

            return ipaddress;
        }

        std::string getIPAddressFromULONG( unsigned long ulIPaddress )
        {
            unsigned short a = 0, b = 0, c = 0, d = 0;

            a = (ulIPaddress & (0xff << 24)) >> 24;
            b = (ulIPaddress & (0xff << 16)) >> 16;
            c = (ulIPaddress & (0xff << 8)) >> 8;
            d = ulIPaddress & 0xff;

            return nsCommon::format( "%hu.%hu.%hu.%hu", d, c, b, a );
        }

        std::wstring getMACFromHDR( const unsigned char* pEtherHder )
        {
            return format( L"%.2X%.2X%.2X%.2X%.2X%.2X", pEtherHder[ 0 ], pEtherHder[ 1 ], pEtherHder[ 2 ], pEtherHder[ 3 ], pEtherHder[ 4 ], pEtherHder[ 5 ] );
        }

        unsigned long getULONGFromIPV4( const std::string& ip )
        {
            if( ip.empty() == true )
                return 0;

            unsigned int a = 0, b = 0, c = 0, d = 0;
            if( sscanf_s( ip.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d ) < 4 )
                return 0;

            return (unsigned long)((unsigned char)d << 24 | (unsigned char)c << 16 | (unsigned char)b << 8 | (unsigned char)a);
        }

        unsigned long getULONGFromIPV4( const std::wstring& ip )
        {
            if( ip.empty() == true )
                return 0;

            unsigned int a = 0, b = 0, c = 0, d = 0;
            if( swscanf_s( ip.c_str(), L"%u.%u.%u.%u", &a, &b, &c, &d ) < 4 )
                return 0;

            return (unsigned long)((unsigned char)d << 24 | (unsigned char)c << 16 | (unsigned char)b << 8 | (unsigned char)a);
        }

        std::vector< std::string > getIPListFromRange( const std::string& startIP, const std::string& endIP )
        {
            std::vector< std::string > vecIPList;

            do 
            {
                for( unsigned long startA = nsCmnNetwork::getULONGFromIPV4( startIP );
                     startA <= nsCmnNetwork::getULONGFromIPV4( endIP ); ++startA )
                {
                    vecIPList.push_back( getIPAddressFromULONG( startA ) );
                }

            } while (false);

            return vecIPList;
        }


#ifdef USE_NETWORK_SUPPORT
        std::vector< NETWORK_INFO > GetNetworkInfos( ULONG defaultAddressFamily /* = AF_INET */ )
        {
            std::vector< NETWORK_INFO > vecResult;

            DWORD dwRetVal = 0;
            PIP_ADAPTER_ADDRESSES pAddresses = NULL;
            PIP_ADAPTER_ADDRESSES pCurrAddress = NULL;

            ULONG Iterations = 0;
            ULONG nOutBufferLen = 8192;

            do
            {
                pAddresses = (PIP_ADAPTER_ADDRESSES)::HeapAlloc( GetProcessHeap(), 0, nOutBufferLen );
                if( pAddresses == NULLPTR )
                    break;

                dwRetVal = GetAdaptersAddresses( defaultAddressFamily, 0
#if _WIN32_WINNT >= NTDDI_WINXPSP1
                                                 | GAA_FLAG_INCLUDE_PREFIX
#endif
#if _WIN32_WINNT >= NTDDI_VISTA
                                                 | GAA_FLAG_INCLUDE_WINS_INFO | GAA_FLAG_INCLUDE_GATEWAYS | GAA_FLAG_INCLUDE_ALL_INTERFACES
#endif
                                                 , NULL, pAddresses, &nOutBufferLen );


                if( dwRetVal == ERROR_SUCCESS )
                    break;

                if( dwRetVal == ERROR_BUFFER_OVERFLOW )
                {
                    if( pAddresses != NULL )
                        ::HeapFree( GetProcessHeap(), 0, pAddresses );
                    pAddresses = NULL;
                }
                else
                {
                    // 버퍼 문제가 아닌 다른 문제로 실패한다면 종료를 위함
                    break;
                }

                Iterations++;
                nOutBufferLen += nOutBufferLen;
            } while( (dwRetVal == ERROR_BUFFER_OVERFLOW) && ( Iterations < 5 ) );

            do
            {
                if( dwRetVal != ERROR_SUCCESS )
                    break;

                pCurrAddress = pAddresses;
                while( pCurrAddress )
                {
                    NETWORK_INFO networkInfo;

                    networkInfo.ipv4IfIndex = pCurrAddress->IfIndex;
                    networkInfo.IfType = pCurrAddress->IfType;
                    networkInfo.ipv6IfIndex = pCurrAddress->Ipv6IfIndex;
                    networkInfo.adapterName = pCurrAddress->AdapterName;
                    networkInfo.description = pCurrAddress->Description;
                    networkInfo.friendlyName = pCurrAddress->FriendlyName;

                    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddress->FirstUnicastAddress;
                    while( pUnicast )
                    {
                        // networkInfo.vecUnicastIP.push_back( 
                        ADDRESS_FAMILY af = pUnicast->Address.lpSockaddr->sa_family;
                        if( af == AF_INET )
                        {
                            networkInfo.vecUnicastIPv4.push_back(
                                inet_ntoa( ((sockaddr_in*)(pUnicast->Address.lpSockaddr))->sin_addr )
                                );
                        }
                        else if( af == AF_INET6 )
                        {
                            char ipv6[ 128 ] = { 0, };
                            DWORD nBufferLen = 128;
                            WSAAddressToStringA( pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength, NULL, ipv6, &nBufferLen );
                            networkInfo.vecUnicastIPv6.push_back( ipv6 );
                        }

                        pUnicast = pUnicast->Next;
                    }

                    PIP_ADAPTER_DNS_SERVER_ADDRESS pDnServer = pCurrAddress->FirstDnsServerAddress;
                    while( pDnServer )
                    {
                        char dnsServer[ 128 ] = { 0, };
                        DWORD nBufferLen = 128;
                        WSAAddressToStringA( pDnServer->Address.lpSockaddr, pDnServer->Address.iSockaddrLength, NULL, dnsServer, &nBufferLen );
                        if( pDnServer->Address.lpSockaddr->sa_family == AF_INET )
                            networkInfo.vecDNSServerIPv4.push_back( dnsServer );
                        else if( pDnServer->Address.lpSockaddr->sa_family == AF_INET6 )
                            networkInfo.vecDNSServerIPv6.push_back( dnsServer );

                        pDnServer = pDnServer->Next;
                    }

                    if( pCurrAddress->PhysicalAddressLength != 0 )
                    {
                        for( ULONG idx = 0; idx < pCurrAddress->PhysicalAddressLength; ++idx )
                        {
                            if( idx == (pCurrAddress->PhysicalAddressLength - 1) )
                                networkInfo.macAddress += format( "%.2X", (int)pCurrAddress->PhysicalAddress[ idx ] );
                            else
                                networkInfo.macAddress += format( "%.2X-", (int)pCurrAddress->PhysicalAddress[ idx ] );
                        }
                    }

                    pCurrAddress = pCurrAddress->Next;
                    vecResult.push_back( networkInfo );
                }
            } while( false );

            if( pAddresses != NULLPTR )
                ::HeapFree( GetProcessHeap(), 0, pAddresses );
            pAddresses = NULLPTR;

            return vecResult;
        }

        bool isTrusZoneVTN() // 이전 코드와의 호환을 위해 남겨둠
		{
			std::vector< std::wstring > vecVPNList;
			vecVPNList.push_back( L"TrusZone VTN driver" );
			return isIgnoringVPNExist( vecVPNList );
		}

		bool isIgnoringVPNExist( std::vector< std::wstring > vecVPNList )
        {
            bool isIgnore = false;

            DWORD dwErr, dwAdapterInfoSize = 0;
            PIP_ADAPTER_INFO	pAdapterInfo, pAdapt;

            if( (dwErr = GetAdaptersInfo( NULL, &dwAdapterInfoSize )) != 0 )
            {
                if( dwErr != ERROR_BUFFER_OVERFLOW )
                {
                    return isIgnore;
                }
            }

            // Allocate memory from sizing information
            if( (pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
                return isIgnore;

            // Get actual adapter information
            if( (dwErr = GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize )) != 0 )
            {
                GlobalFree( pAdapterInfo );
                return isIgnore;
            }

            for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
            {
                switch( pAdapt->Type )
                {
                    case MIB_IF_TYPE_ETHERNET:
                    case MIB_IF_TYPE_PPP:
                    case IF_TYPE_IEEE80211:
                    {
						for( auto it = vecVPNList.begin(); it != vecVPNList.end(); ++it )
						{
							std::wstring strDescription = format( L"%S", pAdapt->Description );

							if( strDescription.find( *it ) != std::wstring::npos )
							{
								isIgnore = true;
								break;
							}
						}
                    }
                        break;
                    default:
                        break;
                }
            }

            if( pAdapterInfo != NULL )
                GlobalFree( pAdapterInfo );

            return isIgnore;
        }

        std::wstring getIPAddress()
        {
            ULONG nBufferLength = 0;
            PBYTE pBuffer = NULL;

            std::wstring ipaddress;
            std::vector< std::pair< std::wstring, std::wstring > > vecIPAddress;

            if( ERROR_BUFFER_OVERFLOW == ::GetAdaptersInfo( NULL, &nBufferLength ) )
            {
                pBuffer = new BYTE[ nBufferLength ];
                ZeroMemory( pBuffer, nBufferLength );

                PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(pBuffer);

                if( GetAdaptersInfo( pAdapterInfo, &nBufferLength ) == ERROR_SUCCESS )
                {

                    while( pAdapterInfo != NULL )
                    {
                        DWORD dwGwIp = inet_addr( pAdapterInfo->GatewayList.IpAddress.String );

                        if( dwGwIp != 0 )
                        {
                            std::wstring strGateWayIP = nsCmnConvert::CA2U( pAdapterInfo->GatewayList.IpAddress.String );
                            std::wstring strIP = nsCmnConvert::CA2U( pAdapterInfo->IpAddressList.IpAddress.String );

                            std::wstring strMACAddress = format( _T( "%02x:%02x:%02x:%02x:%02x:%02x" ),
                                                                 pAdapterInfo->Address[ 0 ],
                                                                 pAdapterInfo->Address[ 1 ],
                                                                 pAdapterInfo->Address[ 2 ],
                                                                 pAdapterInfo->Address[ 3 ],
                                                                 pAdapterInfo->Address[ 4 ],
                                                                 pAdapterInfo->Address[ 5 ] );

                            vecIPAddress.push_back( std::make_pair( strIP, strMACAddress ) );
                        }

                        pAdapterInfo = pAdapterInfo->Next;
                    }

                    for( std::vector< std::pair< std::wstring, std::wstring > >::iterator iter = vecIPAddress.begin(); iter != vecIPAddress.end(); ++iter )
                    {
                        if( _wcsicmp( iter->first, L"0.0.0.0" ) == 0 )
                            continue;

                        if( vecIPAddress.size() > 1 )
                        {
                            if( StrStrIW( iter->second.c_str(), L"vmware" ) == NULL )
                                continue;

                            if( StrStrIW( iter->second.c_str(), L"virtual" ) == NULL )
                                continue;

                            ipaddress = iter->first;
                            break;
                        }
                        else
                        {
                            ipaddress = iter->first;
                        }
                    }
                }

                if( pBuffer != NULLPTR )
                    delete[] pBuffer;
            }

            return ipaddress;
        }

        std::wstring getAllMACAddress()
        {
            std::wstring sAllMac;

            DWORD dwErr, dwAdapterInfoSize = 0;
            PIP_ADAPTER_INFO	pAdapterInfo, pAdapt;
            PIP_ADDR_STRING		pAddrStr;

            if( (dwErr = GetAdaptersInfo( NULL, &dwAdapterInfoSize )) != 0 )
            {
                if( dwErr != ERROR_BUFFER_OVERFLOW )
                {
                    return sAllMac;
                }
            }

            // Allocate memory from sizing information
            if( (pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
            {
                return sAllMac;
            }

            // Get actual adapter information
            if( (dwErr = GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize )) != 0 )
            {
                GlobalFree( pAdapterInfo );
                return sAllMac;
            }

            for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
            {
                switch( pAdapt->Type )
                {
                    case MIB_IF_TYPE_ETHERNET:
                    case MIB_IF_TYPE_PPP:
                    case IF_TYPE_IEEE80211:
                    {
                                              std::wstring strDescription = format( L"%S", pAdapt->Description );

                                              if( StrStrIW( strDescription.c_str(), L"vm" ) != NULLPTR
                                                  || StrStrIW( strDescription.c_str(), L"virtual" ) != NULLPTR )
                                                  break;

                                              if( strlen( pAdapt->GatewayList.IpAddress.String ) > 0 )
                                              {
                                                  DWORD	dwGwIp, dwMask, dwIp, dwGwNetwork, dwNetwork;

                                                  dwGwIp = inet_addr( pAdapt->GatewayList.IpAddress.String );

                                                  for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
                                                  {
                                                      if( strlen( pAddrStr->IpAddress.String ) > 0 )
                                                      {
                                                          dwIp = inet_addr( pAddrStr->IpAddress.String );
                                                          dwMask = inet_addr( pAddrStr->IpMask.String );
                                                          dwNetwork = dwIp & dwMask;
                                                          dwGwNetwork = dwGwIp & dwMask;

                                                          if( dwGwNetwork == dwNetwork )
                                                          {
                                                              std::wstring strCurrentIP, strCurrentMAC;

                                                              strCurrentIP = format( _T( "%S" ), pAddrStr->IpAddress.String );
                                                              strCurrentMAC = format( _T( "%02x:%02x:%02x:%02x:%02x:%02x" ),
                                                                                      pAdapt->Address[ 0 ],
                                                                                      pAdapt->Address[ 1 ],
                                                                                      pAdapt->Address[ 2 ],
                                                                                      pAdapt->Address[ 3 ],
                                                                                      pAdapt->Address[ 4 ],
                                                                                      pAdapt->Address[ 5 ] );

                                                              if( sAllMac.empty() == false )
                                                                  sAllMac += L" ";

                                                              sAllMac += strCurrentMAC;
                                                          }
                                                      }
                                                  }
                                              }
                    }
                        break;
                    default:
                        break;
                }
            }

            if( pAdapterInfo != NULL )
                GlobalFree( pAdapterInfo );

            return sAllMac;
        }

        std::vector< std::wstring > GetDNSServerIPList()
        {
            std::vector< std::wstring > vecDNSServerIP;

            FIXED_INFO* pFixedInfo = (FIXED_INFO*)malloc( sizeof(FIXED_INFO) );
            DWORD dwRetValue = 0;
            ULONG ulOutBufLen = sizeof(FIXED_INFO);

            do
            {
                if( pFixedInfo == NULL )
                    break;

                if( GetNetworkParams( pFixedInfo, &ulOutBufLen ) == ERROR_BUFFER_OVERFLOW )
                {
                    if( pFixedInfo != NULL )
                        free( pFixedInfo );
                    pFixedInfo = NULL;
                    pFixedInfo = (FIXED_INFO *)malloc( ulOutBufLen );
                    if( pFixedInfo == NULL )
                        break;
                }

                dwRetValue = GetNetworkParams( pFixedInfo, &ulOutBufLen );
                if( dwRetValue != NO_ERROR )
                    break;

                IP_ADDR_STRING *pIPAddress = &pFixedInfo->DnsServerList;
                while( pIPAddress )
                {
                    vecDNSServerIP.push_back( nsCmnConvert::CA2U( pIPAddress->IpAddress.String ).c_str() );
                    pIPAddress = pIPAddress->Next;
                }

            } while( false );

            if( pFixedInfo != NULL )
                free( pFixedInfo );

            return vecDNSServerIP;
        }

        std::vector< std::wstring > GetGatewayIPList()
        {
            std::vector< std::wstring > vecGatewayIP;

            PIP_ADAPTER_INFO pAdapter = NULL;
            PIP_ADAPTER_INFO pAdapterInfo = NULL;

            do
            {
                DWORD dwRetVal = 0;

                ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
                pAdapterInfo = (IP_ADAPTER_INFO *)malloc( sizeof (IP_ADAPTER_INFO) );
                if( pAdapterInfo == NULL )
                    break;

                if( GetAdaptersInfo( pAdapterInfo, &ulOutBufLen ) == ERROR_BUFFER_OVERFLOW )
                {
                    free( pAdapterInfo );
                    pAdapterInfo = NULL;
                    pAdapterInfo = (IP_ADAPTER_INFO *)malloc( ulOutBufLen );
                    if( pAdapterInfo == NULL )
                        break;
                }

                dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen );
                if( dwRetVal != NO_ERROR )
                    break;

                pAdapter = pAdapterInfo;

                while( pAdapter )
                {
                    IP_ADDR_STRING* pIPAddrString = &pAdapter->GatewayList;
                    while( pIPAddrString != NULL )
                    {
                        if( _stricmp( pIPAddrString->IpAddress.String, "0.0.0.0" ) != 0 )
                            vecGatewayIP.push_back( nsCmnConvert::CA2U( pIPAddrString->IpAddress.String ).c_str() );
                        pIPAddrString = pIPAddrString->Next;
                    }

                    pAdapter = pAdapter->Next;
                }

            } while( false );

            if( pAdapterInfo )
                free( pAdapterInfo );

            return vecGatewayIP;
        }

        bool GetInternetConnectLanAddress( const std::wstring& strConnectIPAddress, std::wstring& strMAC, std::wstring& strIP, bool ignoreGatewayAddress )
        {
            detail::vecLancard vecLan;
            bool isResult = false;
            DWORD	iAddr = inet_addr( nsCmnConvert::CU2A(strConnectIPAddress).c_str() );
            DWORD	iIndex = 0;

            GetBestInterface( iAddr, &iIndex );

            DWORD dwErr, dwAdapterInfoSize = 0;
            PIP_ADAPTER_INFO	pAdapterInfo, pAdapt;
            PIP_ADDR_STRING		pAddrStr;

            if( (dwErr = GetAdaptersInfo( NULL, &dwAdapterInfoSize )) != 0 )
            {
                if( dwErr != ERROR_BUFFER_OVERFLOW )
                {
                    return isResult;
                }
            }

            // Allocate memory from sizing information
            if( (pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
            {
                return isResult;
            }

            // Get actual adapter information
            if( (dwErr = GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize )) != 0 )
            {
                GlobalFree( pAdapterInfo );
                return isResult;
            }

            for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
            {
                if( pAdapt->Index != iIndex )
                    continue;

                switch( pAdapt->Type )
                {
                    case MIB_IF_TYPE_ETHERNET:
                    case MIB_IF_TYPE_PPP:
                    case IF_TYPE_IEEE80211:
                        if( ignoreGatewayAddress == true )
                        {
                            for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
                            {
                                detail::LANCARD_ITEM item;
                                item.strName = format( L"%S", pAdapt->Description );

                                if( strlen( pAddrStr->IpAddress.String ) > 0 )
                                {
                                    std::wstring strCurrentIP, strCurrentMAC;

                                    strCurrentIP = format( _T( "%S" ), pAddrStr->IpAddress.String );
                                    strCurrentMAC = format( _T( "%02x:%02x:%02x:%02x:%02x:%02x" ),
                                                            pAdapt->Address[ 0 ],
                                                            pAdapt->Address[ 1 ],
                                                            pAdapt->Address[ 2 ],
                                                            pAdapt->Address[ 3 ],
                                                            pAdapt->Address[ 4 ],
                                                            pAdapt->Address[ 5 ] );

                                    isResult = true;

                                    item.strIP = strCurrentIP.c_str();
                                    item.strMAC = strCurrentMAC.c_str();
                                }

                                vecLan.push_back( item );
                            }
                        }
                        else
                        {
                            if( strlen( pAdapt->GatewayList.IpAddress.String ) > 0 )
                            {
                                DWORD	dwGwIp, dwMask, dwIp, dwGwNetwork, dwNetwork;

                                dwGwIp = inet_addr( pAdapt->GatewayList.IpAddress.String );

                                for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
                                {
                                    detail::LANCARD_ITEM item;
                                    item.strName = format( L"%S", pAdapt->Description );

                                    if( strlen( pAddrStr->IpAddress.String ) > 0 )
                                    {
                                        dwIp = inet_addr( pAddrStr->IpAddress.String );
                                        dwMask = inet_addr( pAddrStr->IpMask.String );
                                        dwNetwork = dwIp & dwMask;
                                        dwGwNetwork = dwGwIp & dwMask;

                                        if( dwGwNetwork == dwNetwork )
                                        {
                                            std::wstring strCurrentIP, strCurrentMAC;

                                            strCurrentIP = format( _T( "%S" ), pAddrStr->IpAddress.String );
                                            strCurrentMAC = format( _T( "%02x:%02x:%02x:%02x:%02x:%02x" ),
                                                                    pAdapt->Address[ 0 ],
                                                                    pAdapt->Address[ 1 ],
                                                                    pAdapt->Address[ 2 ],
                                                                    pAdapt->Address[ 3 ],
                                                                    pAdapt->Address[ 4 ],
                                                                    pAdapt->Address[ 5 ] );

                                            isResult = true;

                                            item.strIP = strCurrentIP.c_str();
                                            item.strMAC = strCurrentMAC.c_str();
                                        }
                                    }

                                    vecLan.push_back( item );

                                }
                            }
                        }

                        break;
                    default:
                        break;
                }

                if( isResult == true )
                    break;
            }

            GlobalFree( pAdapterInfo );

            if( !vecLan.empty() )
            {
                std::wstring strCurrentIP, strCurrentMAC;
                for( detail::vecLancardIter iter = vecLan.begin(); iter != vecLan.end(); ++iter )
                {
                    if( iter->strIP != _T( "0.0.0.0" ) )
                    {
                        if( vecLan.size() > 1 )
                        {
                            if( ::StrStrI( iter->strName.c_str(), L"vmware" ) == NULLPTR )
                            {
                                strCurrentIP = iter->strIP;
                                strCurrentMAC = iter->strMAC;
                            }
                        }
                        else
                        {
                            strCurrentIP = iter->strIP;
                            strCurrentMAC = iter->strMAC;
                        }
                    }
                }
                strMAC = strCurrentMAC;
                strIP = strCurrentIP;
                isResult = true;
            }

            return isResult;
        }

        bool GetInternetConnectLanInfo( std::wstring& strMAC, std::wstring& strIP, bool ignoreGatewayAddress )
        {
            ULONG nBufferLength = 0;
            PBYTE pBuffer = NULL;

            bool isResult = false;

            if( ERROR_BUFFER_OVERFLOW == ::GetAdaptersInfo( NULL, &nBufferLength ) )
            {
                pBuffer = new BYTE[ nBufferLength ];
                ZeroMemory( pBuffer, nBufferLength );

                PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(pBuffer);

                if( GetAdaptersInfo( pAdapterInfo, &nBufferLength ) == ERROR_SUCCESS )
                {
                    detail::vecLancard vecLan;

                    while( pAdapterInfo != NULL )
                    {
                        std::wstring strDescription = format( L"%S", pAdapterInfo->Description );
                        DWORD dwGwIp = inet_addr( pAdapterInfo->GatewayList.IpAddress.String );

                        if( ignoreGatewayAddress == true )
                            dwGwIp = 1;

                        if( ::StrStrI( strDescription.c_str(), L"vmware" ) == NULLPTR && dwGwIp != 0 )
                        {
                            detail::LANCARD_ITEM item;
                            item.strGateWayIP = format( L"%S", pAdapterInfo->GatewayList.IpAddress.String );

                            if( pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET )
                            {
                                if( ::StrStrI( strDescription.c_str(), L"wireless" ) == NULLPTR )
                                    item.isEthernet = true;
                                else
                                    item.isEthernet = false;
                            }
                            else
                                item.isEthernet = false;

                            item.strIP = format( L"%S", pAdapterInfo->IpAddressList.IpAddress.String );

                            std::wstring strMACAddress = format( _T( "%02x:%02x:%02x:%02x:%02x:%02x" ),
                                                                 pAdapterInfo->Address[ 0 ],
                                                                 pAdapterInfo->Address[ 1 ],
                                                                 pAdapterInfo->Address[ 2 ],
                                                                 pAdapterInfo->Address[ 3 ],
                                                                 pAdapterInfo->Address[ 4 ],
                                                                 pAdapterInfo->Address[ 5 ] );
                            item.strMAC = strMACAddress;

                            vecLan.push_back( item );
                        }

                        pAdapterInfo = pAdapterInfo->Next;
                    }

                    if( !vecLan.empty() )
                    {
                        std::wstring strCurrentIP, strCurrentMAC;
                        for( detail::vecLancardIter iter = vecLan.begin(); iter != vecLan.end(); ++iter )
                        {
                            if( iter->strIP != _T( "0.0.0.0" ) )
                            {
                                strCurrentIP = iter->strIP;
                                strCurrentMAC = iter->strMAC;
                                if( iter->isEthernet )
                                    break;
                            }
                        }
                        strMAC = strCurrentMAC;
                        strIP = strCurrentIP;
                        isResult = true;
                    }
                }

                delete[] pBuffer;
            }

            return isResult;
        }

        std::wstring getMACFromIP( const std::wstring& ipaddress, ADDRESS_FAMILY family /*= AF_INET */ )
        {
            std::wstring macaddress;

            DWORD dwRet;
            PIP_ADAPTER_ADDRESSES pAdapters = NULL;
            PIP_ADAPTER_ADDRESSES pCurrent = NULL;
            PIP_ADAPTER_UNICAST_ADDRESS pCurrentAddress = NULL;

            unsigned long ulBufLen = 0;

            dwRet = GetAdaptersAddresses( family, 0, NULL, pAdapters, &ulBufLen );

            if( dwRet == ERROR_NO_DATA )
                return macaddress;

            if( dwRet == ERROR_BUFFER_OVERFLOW )
            {
                pAdapters = (PIP_ADAPTER_ADDRESSES)malloc( ulBufLen );
                if( pAdapters == NULL )
                    return macaddress;
            }

            dwRet = GetAdaptersAddresses( family, 0, NULL, pAdapters, &ulBufLen );
            if( dwRet != ERROR_SUCCESS )
                return macaddress;

            for( pCurrent = pAdapters; pCurrent != NULL; pCurrent = pCurrent->Next )
            {
                for( pCurrentAddress = pCurrent->FirstUnicastAddress; pCurrentAddress != NULL; pCurrentAddress = pCurrentAddress->Next )
                {
                    LPSOCKADDR pAddr = (LPSOCKADDR)pCurrentAddress->Address.lpSockaddr;
                    if( pAddr == NULL )
                        continue;

                    WCHAR wszAddress[ 128 ] = { 0, };
                    DWORD chAddress = _countof( wszAddress );

                    dwRet = WSAAddressToString(
                        pCurrentAddress->Address.lpSockaddr,
                        pCurrentAddress->Address.iSockaddrLength,
                        NULL,
                        wszAddress,
                        &chAddress );

                    if( dwRet != 0 )
                        continue;

                    if( _wcsicmp( wszAddress, ipaddress ) == 0 )
                    {
                        for( DWORD i = 0; i < pCurrent->PhysicalAddressLength; i++ )
                        {
                            if( i == (pCurrent->PhysicalAddressLength - 1) )
                                macaddress += format( L"%.2X", (int)pCurrent->PhysicalAddress[ i ] );
                            else
                                macaddress += format( L"%.2X-", (int)pCurrent->PhysicalAddress[ i ] );
                        }
                    }
                }
            }

            if( pAdapters != NULL )
                free( (void *)pAdapters );

            return macaddress;
        }

        std::wstring getInternetConnectedAddress( const std::wstring& tryInternetIP /*= L"www.msn.com" */ )
        {
            std::wstring ipaddress;

            unsigned long ulTryIP = inet_addr( nsCmnConvert::CU2A( tryInternetIP ).c_str() );
            DWORD iIndex = 0;
            std::vector< std::pair< std::wstring, std::wstring > > vecIPAddress;

            DWORD dwErr = 0, dwAdapterInfoSize = 0;
            PIP_ADAPTER_INFO	pAdapterInfo = NULL, pAdapt = NULL;
            PIP_ADDR_STRING		pAddrStr = NULL;

            do
            {
                if( GetBestInterface( ulTryIP, &iIndex ) != NO_ERROR )
                    break;

                dwErr = GetAdaptersInfo( NULL, &dwAdapterInfoSize );
                if( dwErr != ERROR_BUFFER_OVERFLOW )
                    break;

                if( (pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
                    break;

                if( (dwErr = GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize )) != 0 )
                    break;

                for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
                {
                    if( pAdapt->Index != iIndex )
                        continue;

                    switch( pAdapt->Type )
                    {
                        case MIB_IF_TYPE_ETHERNET:
                        case MIB_IF_TYPE_PPP:
                        case IF_TYPE_IEEE80211:
                            if( strlen( pAdapt->GatewayList.IpAddress.String ) > 0 )
                            {
                                DWORD	dwGwIp, dwMask, dwIp, dwGwNetwork, dwNetwork;

                                dwGwIp = inet_addr( pAdapt->GatewayList.IpAddress.String );

                                for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
                                {
                                    if( strlen( pAddrStr->IpAddress.String ) <= 0 )
                                        continue;

                                    dwIp = inet_addr( pAddrStr->IpAddress.String );
                                    dwMask = inet_addr( pAddrStr->IpMask.String );
                                    dwNetwork = dwIp & dwMask;
                                    dwGwNetwork = dwGwIp & dwMask;

                                    if( dwGwNetwork != dwNetwork )
                                        continue;

                                    vecIPAddress.push_back(
                                        std::make_pair(
                                        format( L"%s", nsCmnConvert::CA2U( pAddrStr->IpAddress.String ).c_str() ),
                                        nsCmnConvert::CA2U( pAdapt->AdapterName ).c_str() )
                                        );
                                }
                            }
                            break;
                    }
                }

                for( std::vector< std::pair< std::wstring, std::wstring > >::iterator iter = vecIPAddress.begin(); iter != vecIPAddress.end(); ++iter )
                {
                    if( _wcsicmp( iter->first, L"0.0.0.0" ) == 0 )
                        continue;

                    if( vecIPAddress.size() > 1 )
                    {
                        if( StrStrIW( iter->second.c_str(), L"vmware" ) == NULL )
                            continue;

                        if( StrStrIW( iter->second.c_str(), L"virtual" ) == NULL )
                            continue;

                        ipaddress = iter->first;
                    }
                    else
                    {
                        ipaddress = iter->first;
                    }
                }

            } while( false );

            if( pAdapterInfo != NULL )
                ::GlobalFree( pAdapterInfo );

            return ipaddress;
        }

        CDownloaderByAPI::CDownloaderByAPI()
            : hDownloadThread( NULL ), hOnCompleteEvent( NULL ), hOnProgressEvent( NULL ), hResult_( S_OK )
        {
            hOnCompleteEvent = ::CreateEventW( NULL, FALSE, FALSE, NULL );
            hOnProgressEvent = ::CreateEventW( NULL, FALSE, FALSE, NULL );
        }

        CDownloaderByAPI::~CDownloaderByAPI()
        {
            if( hOnProgressEvent != NULL )
                CloseHandle( hOnProgressEvent );
            if( hOnCompleteEvent != NULL )
                CloseHandle( hOnCompleteEvent );
        }

        HRESULT CDownloaderByAPI::DownloadToFile( const std::wstring& remoteURL, const std::wstring& localPath )
        {
            remoteURL_ = remoteURL;
            localPath_ = localPath;

            boost::thread t( boost::bind( &CDownloaderByAPI::workerThread, this ) );

            HANDLE hWaitEvent[ 2 ] = { NULL, };
            hWaitEvent[ 0 ] = hOnProgressEvent;
            hWaitEvent[ 1 ] = hOnCompleteEvent;

            while( true )
            {
                DWORD dwWaitState = ::WaitForMultipleObjects( 2, hWaitEvent, FALSE, 1000 * 60 * 10 );
                if( dwWaitState == WAIT_OBJECT_0 ) // onProgress
                    continue;
                if( dwWaitState == (WAIT_OBJECT_0 + 1) )
                {
                    DeleteUrlCacheEntryW( remoteURL_.c_str() );
                    break;
                }
                if( dwWaitState == WAIT_TIMEOUT )
                {
                    t.interrupt();
                    break;
                }
            }

            return hResult_;
        }

        void CDownloaderByAPI::workerThread()
        {
            detail::CDownloaderStatusCallback callback( hOnProgressEvent );

            hResult_ = URLDownloadToFileW( NULL, remoteURL_.c_str(), localPath_.c_str(), 0, &callback );

            SetEvent( hOnCompleteEvent );
        }

    #ifdef _AFX
        bool DownloadFile( CString strRemoteAddr, CString strRemoteFile, CString strLocalPath, int downloadType /*= -1 */ )
        {
            bool isSuccess = false;

            CFile fpLocalPath;
            CInternetSession session;
            CFtpConnection* pFtpConn = NULL;
            CHttpConnection* pHttpConn = NULL;
            CHttpFile* pFile = NULL;
            CInternetFile* pFtpFile = NULL;
            CString strDebug;

            do
            {
                if( strRemoteAddr.IsEmpty() || strRemoteFile.IsEmpty() || strLocalPath.IsEmpty() )
                    break;

                session.SetOption( INTERNET_OPTION_CONNECT_TIMEOUT | INTERNET_OPTION_CONNECTED_STATE, 3000 );
                session.SetOption( INTERNET_OPTION_CONNECT_BACKOFF, 1000 );
                session.SetOption( INTERNET_OPTION_CONNECT_RETRIES, 3 );
                session.SetOption( INTERNET_OPTION_SECURITY_FLAGS, SECURITY_FLAG_IGNORE_CERT_CN_INVALID );
                session.SetOption( INTERNET_OPTION_SECURITY_FLAGS, SECURITY_FLAG_IGNORE_CERT_DATE_INVALID );

                INTERNET_PORT nPort = 80;
                CString strServer, strObject;
                DWORD dwServiceType = AFX_INET_SERVICE_HTTP;
                if( strRemoteFile.Left( 1 ).Compare( L"/" ) != 0 )
                    strRemoteFile = L"/" + strRemoteFile;

                // 주소에서 서비스 프로토콜 종류 검출
                if( downloadType == 0 )
                {
                    if( strRemoteAddr.Find( L"://" ) == -1 )
                        dwServiceType = AFX_INET_SERVICE_HTTP;
                }
                else if( downloadType <= 3 && downloadType >= 0 )   // downloadType = 1 과 2 는 AFX_INET_SERVICE_FTP, AFX_INET_SERVICE_HTTP 에 대응함
                    dwServiceType = downloadType == 2 ? AFX_INET_SERVICE_HTTPS : downloadType;
                else
                    break;

                if( AfxParseURL( strRemoteAddr + strRemoteFile, dwServiceType, strServer, strObject, nPort ) == 0 )
                    dwServiceType = downloadType == 2 ? AFX_INET_SERVICE_HTTPS : downloadType;

                try
                {
                    DWORD dwRet = 0;
                    const DWORD BUFFER_SIZE = 8192;
                    BYTE byBuffer[ BUFFER_SIZE ] = { 0, };
                    DWORD dwFlags = INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_TRANSFER_BINARY;
                    DWORD dwReadAmount = 0;

                    switch( dwServiceType )
                    {
                        case AFX_INET_SERVICE_HTTP:
                        case AFX_INET_SERVICE_HTTPS:
                            if( dwServiceType == AFX_INET_SERVICE_HTTP )
                                pHttpConn = session.GetHttpConnection( strServer, nPort );
                            else if( dwServiceType == AFX_INET_SERVICE_HTTPS )
                                pHttpConn = session.GetHttpConnection( strServer,
                                INTERNET_FLAG_SECURE |
                                INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
                                INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, nPort );

                            if( dwServiceType == AFX_INET_SERVICE_HTTPS )
                                dwFlags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

                            pFile = pHttpConn->OpenRequest( CHttpConnection::HTTP_VERB_GET, strRemoteFile, NULL, 1, NULL, NULL, dwFlags );
                            if( pFile == NULL )
                                break;

                            pFile->SetReadBufferSize( BUFFER_SIZE );

                            if( pFile->SendRequest() == FALSE )
                                break;

                            if( pFile->QueryInfoStatusCode( dwRet ) == FALSE )
                                break;

                            if( dwRet != HTTP_STATUS_OK )
                                break;

                            if( fpLocalPath.Open( strLocalPath, CFile::modeCreate | CFile::modeWrite, NULL ) == FALSE )
                                break;

                            while( (dwReadAmount = pFile->Read( byBuffer, BUFFER_SIZE )) > 0 )
                                fpLocalPath.Write( byBuffer, dwReadAmount );

                            isSuccess = true;
                            break;
                        case AFX_INET_SERVICE_FTP:
                            pFtpConn = session.GetFtpConnection( strRemoteAddr );

                            if( fpLocalPath.Open( strLocalPath, CFile::modeWrite | CFile::modeCreate, NULL ) == FALSE )
                                break;

                            pFtpFile = pFtpConn->OpenFile( strRemoteFile );
                            if( pFtpFile == NULL )
                                break;

                            while( (dwReadAmount = pFtpFile->Read( byBuffer, BUFFER_SIZE )) > 0 )
                                fpLocalPath.Write( byBuffer, dwReadAmount );

                            isSuccess = true;
                            break;
                    };
                }
                catch( CException* e )
                {
                    UNREFERENCED_PARAMETER( e );
                    if( fpLocalPath != INVALID_HANDLE_VALUE )
                        fpLocalPath.Close();
                    DeleteFile( strLocalPath );
                }

            } while( false );

            if( pFile != NULL )
            {
                pFile->Close(); delete pFile; pFile = NULL;
            }

            if( pFtpFile != NULL )
            {
                pFtpFile->Close(); delete pFtpFile; pFtpFile = NULL;
            }

            if( pHttpConn != NULL )
            {
                pHttpConn->Close(); delete pHttpConn; pHttpConn = NULL;
            }

            if( pFtpConn != NULL )
            {
                pFtpConn->Close(); delete pFtpConn; pFtpConn = NULL;
            }

            return isSuccess;
        }
    #endif
#endif

    } // nsCmnNetwork
} // nsCommon