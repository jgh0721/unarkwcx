#include "stdafx.h"

#include <intrin.h>

#ifdef _WINDOWS_
#include <WinIoCtl.h>
#endif

#include "cmnPCInfo.h"
#include "cmnUtils.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    namespace nsPCInfo
    {
        std::string GetCPUBrandString()
        {
            char CPUString[ 0x20 ] = { 0, };
            char CPUBrandString[ 0x40 ] = { 0, };
            int CPUInfo[ 4 ] = { -1 };
            unsigned nExIds, i;

            // __cpuid with an InfoType argument of 0 returns the number of
            // valid Ids in CPUInfo[0] and the CPU identification string in
            // the other three array elements. The CPU identification string is
            // not in linear order. The code below arranges the information 
            // in a human readable form.
            __cpuid( CPUInfo, 0 );
            memset( CPUString, 0, sizeof(CPUString) );
            *((int*)CPUString) = CPUInfo[ 1 ];
            *((int*)(CPUString + 4)) = CPUInfo[ 3 ];
            *((int*)(CPUString + 8)) = CPUInfo[ 2 ];

            // Calling __cpuid with 0x80000000 as the InfoType argument
            // gets the number of valid extended IDs.
            __cpuid( CPUInfo, 0x80000000 );
            nExIds = CPUInfo[ 0 ];
            memset( CPUBrandString, 0, sizeof(CPUBrandString) );

            // Get the information associated with each extended ID.
            for( i = 0x80000000; i <= nExIds; ++i )
            {
                __cpuid( CPUInfo, i );

                // Interpret CPU brand string and cache information.
                if( i == 0x80000002 )
                    memcpy( CPUBrandString, CPUInfo, sizeof(CPUInfo) );
                else if( i == 0x80000003 )
                    memcpy( CPUBrandString + 16, CPUInfo, sizeof(CPUInfo) );
                else if( i == 0x80000004 )
                    memcpy( CPUBrandString + 32, CPUInfo, sizeof(CPUInfo) );
            }

            if( nExIds >= 0x80000004 )
                return CPUBrandString;
            else
                return CPUString;
        }

#ifdef _WINDOWS_
        std::wstring GetLocalComputerName()
        {
            TCHAR wszComputerName[ MAX_COMPUTERNAME_LENGTH + 2 ] = { 0, };
            DWORD dwComputerBufferSize = _countof( wszComputerName );

            if( GetComputerName( wszComputerName, &dwComputerBufferSize ) == FALSE )
                return L"";

            return wszComputerName;
        }

        unsigned __int64 GetTotalPhysicalMEMbytes()
        {
            MEMORYSTATUSEX msex;
            memset( &msex, '\0', sizeof(msex) );
            msex.dwLength = sizeof(msex);

            ::GlobalMemoryStatusEx( &msex );

            return msex.ullTotalPhys;
        }

        std::string GetHDDSerial( unsigned nHDDIndex /*= 0 */ )
        {
            std::string strHDDSerial;

            //  GETVERSIONOUTPARAMS contains the data returned from the 
            //  Get Driver Version function.
            typedef struct _GETVERSIONOUTPARAMS
            {
                BYTE bVersion;      // Binary driver version.
                BYTE bRevision;     // Binary driver revision.
                BYTE bReserved;     // Not used.
                BYTE bIDEDeviceMap; // Bit map of IDE devices.
                DWORD fCapabilities; // Bit mask of driver capabilities.
                DWORD dwReserved[ 4 ]; // For future use.
            } GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

            const DWORD DFP_GET_VERSION = 0x00074080;
            const DWORD DFP_RECEIVE_DRIVE_DATA = 0x0007c088;

            HANDLE hPhysicalDriveIOCTL = INVALID_HANDLE_VALUE;

            do
            {
                hPhysicalDriveIOCTL = ::CreateFile(
                    nsCommon::format( L"\\\\.\\PhysicalDrive%d", nHDDIndex ).c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL );

                if( hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE )
                    break;

                DWORD cbBytesReturned = 0;
                GETVERSIONOUTPARAMS versionParams;
                ZeroMemory( &versionParams, sizeof(GETVERSIONOUTPARAMS) );

                if( DeviceIoControl( hPhysicalDriveIOCTL, DFP_GET_VERSION,
                    NULL, 0, &versionParams, sizeof(GETVERSIONOUTPARAMS),
                    &cbBytesReturned, NULL ) == FALSE )
                    break;

                if( versionParams.bIDEDeviceMap > 0 )
                {
                    BYTE bIDCmd = 0;
                    //  Valid values for the bCommandReg member of IDEREGS.
                    const BYTE IDE_ATAPI_IDENTIFY = 0xA1;
                    const BYTE IDE_ATA_IDENTIFY = 0xEC;

                    // Now, get the ID sector for all IDE devices in the system.
                    // If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
                    // otherwise use the IDE_ATA_IDENTIFY command
                    bIDCmd = (versionParams.bIDEDeviceMap >> nHDDIndex & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;
                    SENDCMDINPARAMS scip;
                    BYTE IdOutCmd[ sizeof (SENDCMDOUTPARAMS)+IDENTIFY_BUFFER_SIZE - 1 ] = { 0, };
                    ZeroMemory( &scip, sizeof(SENDCMDINPARAMS) );

                    // Set up data structures for IDENTIFY command.
                    scip.cBufferSize = IDENTIFY_BUFFER_SIZE;
                    scip.irDriveRegs.bFeaturesReg = 0;
                    scip.irDriveRegs.bSectorCountReg = 1;
                    scip.irDriveRegs.bSectorNumberReg = 1;
                    scip.irDriveRegs.bCylLowReg = 0;
                    scip.irDriveRegs.bCylHighReg = 0;

                    // Compute the drive number.
                    scip.irDriveRegs.bDriveHeadReg = 0xA0 | ((nHDDIndex & 1) << 4);

                    // The command can either be IDE identify or ATAPI identify.
                    scip.irDriveRegs.bCommandReg = bIDCmd;
                    scip.bDriveNumber = nHDDIndex;
                    scip.cBufferSize = IDENTIFY_BUFFER_SIZE;

                    if( DeviceIoControl( hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
                        (LPVOID)&scip,
                        sizeof(SENDCMDINPARAMS)-1,
                        (LPVOID)&IdOutCmd[ 0 ],
                        sizeof(SENDCMDOUTPARAMS)+IDENTIFY_BUFFER_SIZE - 1,
                        &cbBytesReturned, NULL ) == FALSE )
                        break;

                    DWORD diskData[ 256 ] = { 0, };
                    USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer;

                    for( int idx = 0; idx < _countof( diskData ); ++idx )
                        diskData[ idx ] = pIdSector[ idx ];

                    char szHDDSerial[ 1024 ] = { 0, };
                    int index = 0;
                    int position = 0;

                    //  each integer has two characters stored in it backwards
                    for( index = 10; index <= 19; index++ )
                    {
                        //  get high byte for 1st character
                        szHDDSerial[ position ] = (char)(diskData[ index ] / 256);
                        position++;

                        //  get low byte for 2nd character
                        szHDDSerial[ position ] = (char)(diskData[ index ] % 256);
                        position++;
                    }

                    //  end the string 
                    szHDDSerial[ position ] = '\0';

                    //  cut off the trailing blanks
                    for( index = position - 1; index > 0 && ' ' == szHDDSerial[ index ]; index-- )
                        szHDDSerial[ index ] = '\0';

                    strHDDSerial = szHDDSerial;
                }

            } while( false );

            if( hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE )
                CloseHandle( hPhysicalDriveIOCTL );
            hPhysicalDriveIOCTL = INVALID_HANDLE_VALUE;

            return strHDDSerial;
        }

        std::wstring GetVolumeSerial( const std::wstring& volumePath /*= L"C:\\" */ )
        {
            wchar_t nameBuffer[ MAX_PATH + 1 ] = { 0, };
            DWORD volumeSerial = 0;

            GetVolumeInformationW
                (
                volumePath.c_str(),
                (LPTSTR)nameBuffer, // volume label 버퍼
                MAX_PATH,                // volume label을 저장할 버퍼의 크기
                &volumeSerial,      // volume의 일련번호
                NULL,
                NULL,
                NULL,
                0                 // 파일 시스템 이름을 저장할 버퍼의 크기
                );

            return format( L"%d", volumeSerial );
        }

#endif
    } // nsPCInfo
}