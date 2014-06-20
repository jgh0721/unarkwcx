#pragma once

#include <string>

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsPCInfo
    {
        // CPU 정보
        std::string                 GetCPUBrandString();
#ifdef _WINDOWS_
        // 로컬 컴퓨터의 NetBIOS 컴퓨터 이름을 반환
        std::wstring                GetLocalComputerName( );
        // 메모리 정보
        unsigned __int64            GetTotalPhysicalMEMbytes();
        // HDD 정보
        std::string                 GetHDDSerial( unsigned nHDDIndex = 0 );
        // 볼륨 정보
        std::wstring                GetVolumeSerial( const std::wstring& volumePath = L"C:\\" );
#endif
    }
}