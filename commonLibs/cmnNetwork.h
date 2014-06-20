#pragma once

#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsCmnNetwork
    {
        namespace detail
        {
#ifdef USE_NETWORK_SUPPORT
            typedef struct tag_LANCARD_ITEM
            {
                std::wstring	strIP;
                std::wstring	strGateWayIP;
                std::wstring	strMAC;
                bool			isEthernet;
                std::wstring	strName;

                tag_LANCARD_ITEM() : isEthernet( false ) {}
            } LANCARD_ITEM, *PLANCARD_ITEM;

            typedef std::vector<LANCARD_ITEM>	vecLancard;
            typedef vecLancard::iterator		vecLancardIter;

            class CDownloaderStatusCallback : public IBindStatusCallback
            {
            public:

                virtual HRESULT STDMETHODCALLTYPE OnStartBinding( /* [in] */ DWORD dwReserved, /* [in] */ IBinding *pib )
                {
                    return S_OK;
                }

                virtual HRESULT STDMETHODCALLTYPE GetPriority( /* [out] */ LONG *pnPriority )
                {
                    return S_OK;
                }

                virtual HRESULT STDMETHODCALLTYPE OnLowResource( /* [in] */ DWORD reserved )
                {
                    return S_OK;
                }

                virtual HRESULT STDMETHODCALLTYPE OnProgress( /* [in] */ ULONG ulProgress, /* [in] */ ULONG ulProgressMax, /* [in] */ ULONG ulStatusCode, /* [in] */ LPCWSTR szStatusText )
                {
                    SetEvent( itsProgressEvent );
                    return S_OK;
                }

                virtual HRESULT STDMETHODCALLTYPE OnStopBinding( /* [in] */ HRESULT hresult, /* [unique][in] */ LPCWSTR szError )
                {
                    return S_OK;
                }

                virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetBindInfo( /* [out] */ DWORD *grfBINDF, /* [unique][out][in] */ BINDINFO *pbindinfo )
                {
                    return S_OK;
                }

                virtual /* [local] */ HRESULT STDMETHODCALLTYPE OnDataAvailable( /* [in] */ DWORD grfBSCF, /* [in] */ DWORD dwSize, /* [in] */ FORMATETC *pformatetc, /* [in] */ STGMEDIUM *pstgmed )
                {
                    return S_OK;
                }

                virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable( /* [in] */ REFIID riid, /* [iid_is][in] */ IUnknown *punk )
                {
                    return S_OK;
                }

                STDMETHOD_( ULONG, AddRef )() { return 0; }
                STDMETHOD_( ULONG, Release )() { return 0; }

                STDMETHOD( QueryInterface )( /* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
                {
                    return E_NOTIMPL;
                }

            public:
                CDownloaderStatusCallback( HANDLE progressEvent ) { itsProgressEvent = progressEvent; }

            private:
                HANDLE itsProgressEvent;
            };
#endif
        }

        // IP 주소로부터 해당하는 U_INT 숫자를 반환, 네트워크 바이트 순서로 반환됨
        unsigned long               getULONGFromIPAddress( unsigned char lsb, unsigned char lsbBy1, unsigned char lsbBy2, unsigned char lsbBy3 );
        // IP 주소(Unsinged long, 네트워크 바이트 오더 ) 로부터 IP 주소 문자열을 반환
        std::string                 getIPAddressFromULONG( unsigned long ulIPaddress );
        // 맥 주소를 나타내는 메모리 배열에서 맥 주소 문자열로 변환
        std::wstring			    getMACFromHDR( const unsigned char* pEtherHder );
        // IP v4 주소 문자열로부터 IP 숫자 반환, 네트워크 바이트 순서로 반환됨
        unsigned long               getULONGFromIPV4( const std::string& ip );
        unsigned long               getULONGFromIPV4( const std::wstring& ip );
        std::vector< std::string >  getIPListFromRange( const std::string& startIP, const std::string& endIP );

        typedef struct tagNETWORK_INFO
        {
            unsigned int                            ipv4IfIndex;    // ipv4 를 사용할 수 없는 인터페이스라면(ipv6전용등) 해당 인덱스는 0을 사용한다
            unsigned int                            ipv6IfIndex;    // ipv6 를 사용할 수 없는 인터페이스라면 해당 인덱스는 0 을 사용한다
            unsigned int                            IfType;         // IF_TYPE_ETHERNET_CSMACD, IF_TYPE_SOFTWARE_LOOPBACK, IF_TYPE_IEEE80211 ...
            std::string                             adapterName;
            std::wstring                            description;
            std::wstring                            friendlyName;

            std::string                             macAddress;

            std::vector< std::string >              vecUnicastIPv4;
            std::vector< std::string >              vecUnicastIPv6;

            std::vector< std::string >              vecDNSServerIPv4;
            std::vector< std::string >              vecDNSServerIPv6;

            std::vector< std::wstring >             vecGatewayIP;

        } NETWORK_INFO, *PNETWORK_INFO;

#ifdef USE_NETWORK_SUPPORT
        // 네트워크 정보, 2 = AF_INET, 23 = AF_INET6, 0 = AF_UNSPEC
        std::vector< NETWORK_INFO > GetNetworkInfos( ULONG defaultAddressFamily = 0 );
        bool                        isTrusZoneVTN( );
		bool						isIgnoringVPNExist( std::vector< std::wstring > vecVPNList );

        // IPv4 형식으로 IP 주소를 가져옴, 이때 VMWare 또는 Virtual Box, Virtual PC 등의 어대텁를 제외한 첫번째 어댑터의 주소를 가져옴
        std::wstring			    getIPAddress();
        // 가상화 솔루션(VMware, VirtualBox 등) 으로 생성된 어댑터를 제외한 모든 어댑터의 맥 주소를 공백으로 구분하여 반환
        std::wstring                getAllMACAddress();

        // 인터넷 연결에 필요한 DNS 서버의 IP 목록을 가져온다
        std::vector< std::wstring > GetDNSServerIPList( );
        std::vector< std::wstring > GetGatewayIPList( );

        /** strConnectIPAddress 로의 라우트 정보를 이용하여서 자신의 인터넷 연결 IP 주소를 찾는다. * */
        bool                        GetInternetConnectLanAddress( const std::wstring& strConnectIPAddress, std::wstring& strMAC, std::wstring& strIP, bool ignoreGatewayAddress = false );
        /** 호스트에서 인터넷에 연결되어 있는 IP 주소를 가져온다. */
        bool                        GetInternetConnectLanInfo( std::wstring& strMAC, std::wstring& strIP, bool ignoreGatewayAddress = false );

        /*!
            넘겨준 IP 에 해당하는 MAC 주소 값을 반환. IPv4, IPv6 모두 대응함
            Method:    getMACFromIP
            @param: const std::wstring & ipaddress
            @param: ADDRESS_FAMILY family
            @return:   std::wstring
        */
        std::wstring                getMACFromIP( const std::wstring& ipaddress, ADDRESS_FAMILY family = AF_INET );
        // 인터넷에 연결을 시도하여 실제로 인터넷에 연결된 로컬 컴퓨터의 IP 를 반환, 랜카드가 여러개 있거나 할 때 유용
        std::wstring                getInternetConnectedAddress( const std::wstring& tryInternetIP = L"www.msn.co.kr" );

        class CDownloaderByAPI
        {
        public:
            CDownloaderByAPI();
            ~CDownloaderByAPI();

            HRESULT                 DownloadToFile( const std::wstring& remoteURL, const std::wstring& localPath );

        private:
            void                    workerThread();

            std::wstring        remoteURL_;
            std::wstring        localPath_;

            HRESULT             hResult_;

            HANDLE              hOnProgressEvent;
            HANDLE              hOnCompleteEvent;
            HANDLE              hDownloadThread;
        };

    #ifdef _AFX
        /*!
            HTTP, HTTPS, FTP 로부터 파일을 내려받습니다.

            @param strRemoteAddr 원격지의 주소
            @param strRemoteFile 내려받을 원격지의 파일이름
            @param strLocalPath 내려받은 파일을 저장할 로컬 경로 ( 파일이름은 변경없이 그대로 저장됨 )

            포트가 일반적인 포트가 아닐 경우 해당 서버의 프로토콜 형식을 직접 지정할 수 있음
            downloadType = 0( AUTO ), 1 = FTP, 2 = HTTPS, 3 = HTTP,

            @return bool 파일 내려받기 성공여부 반환

            http://www.msn.co.kr/index.html 을 내려받는다면
            strRemoteAddr = http://www.msn.co.kr
            strRemoteFile = /index.html
            strLocalPath = C:/
        */
        bool                        DownloadFile( CString strRemoteAddr, CString strRemoteFile, CString strLocalPath, int downloadType = 2 );
    #endif

#endif
    }
}
