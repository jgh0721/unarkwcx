#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Ark.h"
#include "commonXMLMapperV2.h"
#include "EnumString.h"

#pragma execution_character_set( "utf-8" )

typedef std::pair< ARK_FF, std::string >                    TyPrFileFormatToDesc;

enum eCompressionLevel
{
    COMPRESS_LEVEL_DEFAULT = -1, 
    COMPRESS_LEVEL_0 = 0,
    COMPRESS_LEVEL_1,
    COMPRESS_LEVEL_2,
    COMPRESS_LEVEL_3,
    COMPRESS_LEVEL_4,
    COMPRESS_LEVEL_5,
    COMPRESS_LEVEL_6,
    COMPRESS_LEVEL_7,
    COMPRESS_LEVEL_8,
    COMPRESS_LEVEL_9,

    COMPRESS_LEVEL_MAX
};

Begin_Enum_String( eCompressionLevel )
{
    RegisterEnumerator( COMPRESS_LEVEL_DEFAULT, "Default" );
    RegisterEnumerator( COMPRESS_LEVEL_0, "Level 0" );
    RegisterEnumerator( COMPRESS_LEVEL_1, "Level 1" );
    RegisterEnumerator( COMPRESS_LEVEL_2, "Level 2" );
    RegisterEnumerator( COMPRESS_LEVEL_3, "Level 3" );
    RegisterEnumerator( COMPRESS_LEVEL_4, "Level 4" );
    RegisterEnumerator( COMPRESS_LEVEL_5, "Level 5" );
    RegisterEnumerator( COMPRESS_LEVEL_6, "Level 6" );
    RegisterEnumerator( COMPRESS_LEVEL_7, "Level 7" );
    RegisterEnumerator( COMPRESS_LEVEL_8, "Level 8" );
    RegisterEnumerator( COMPRESS_LEVEL_9, "Level 9" );
}
End_Enum_String;

enum eSplitArchiveSize
{
    SPLIT_ARCHIVE_NONE, 
    SPLIT_ARCHIVE_1MB,
    SPLIT_ARCHIVE_10MB,
    SPLIT_ARCHIVE_100MB,
    SPLIT_ARCHIVE_1GB,
    SPLIT_ARCHIVE_2GB,
    SPLIT_ARCHIVE_4GB,

    SPLIT_ARCHIVE_MAX,
};

Begin_Enum_String( eSplitArchiveSize )
{
    RegisterEnumerator( SPLIT_ARCHIVE_NONE, "None" );
    RegisterEnumerator( SPLIT_ARCHIVE_1MB, "1 MB" );
    RegisterEnumerator( SPLIT_ARCHIVE_10MB, "10 MB" );
    RegisterEnumerator( SPLIT_ARCHIVE_100MB, "100 MB" );
    RegisterEnumerator( SPLIT_ARCHIVE_1GB, "1 GB( 1024 MB )" );
    RegisterEnumerator( SPLIT_ARCHIVE_2GB, "2 GB( 2048 MB )" );
    RegisterEnumerator( SPLIT_ARCHIVE_4GB, "4 GB( DVD )" );
}
End_Enum_String;

enum eEncryptionMethod
{
    ENCRYPTION_NONE, 
    ENCRYPTION_ZIP, 
    ENCRYPTION_AES256,

    ENCRYPTION_MAX
};

Begin_Enum_String( eEncryptionMethod )
{
    RegisterEnumerator( ENCRYPTION_NONE, "None" );
    RegisterEnumerator( ENCRYPTION_ZIP, "ZIP" );
    RegisterEnumerator( ENCRYPTION_AES256, "AES256" );
}
End_Enum_String;

enum eCompressionMethod
{
    COMPRESSION_METHOD_STORE, 
    COMPRESSION_METHOD_DEFLATE,
    COMPRESSION_METHOD_LZMA,
    COMPRESSION_METHOD_LZMA2,
    COMPRESSION_METHOD_LH6,
    COMPRESSION_METHOD_LH7,

    COMPRESSION_METHOD_MAX
};

Begin_Enum_String( eCompressionMethod )
{
    RegisterEnumerator( COMPRESSION_METHOD_STORE, "STORE( ZIP,7Z,LZH )" );
    RegisterEnumerator( COMPRESSION_METHOD_DEFLATE, "DEFLATE( ZIP )" );
    RegisterEnumerator( COMPRESSION_METHOD_LZMA, "LZMA( ZIP,7Z )" );
    RegisterEnumerator( COMPRESSION_METHOD_LZMA2, "LZMA2( 7Z )" );
    RegisterEnumerator( COMPRESSION_METHOD_LH6, "LH6( LZH )" );
    RegisterEnumerator( COMPRESSION_METHOD_LH7, "LH7( LZH )" );
}
End_Enum_String;

const TyPrFileFormatToDesc PrFormatZIPToDesc = std::make_pair( ARK_FF_ZIP, "ZIP" );
const TyPrFileFormatToDesc PrFormatTARToDesc = std::make_pair( ARK_FF_TAR, "TAR" );
const TyPrFileFormatToDesc PrFormatTGZToDesc = std::make_pair( ARK_FF_TGZ, "TGZ" );
const TyPrFileFormatToDesc PrFormat7ZToDesc = std::make_pair( ARK_FF_7Z, "7Z" );
const TyPrFileFormatToDesc PrFormatLZHToDesc = std::make_pair( ARK_FF_LZH, "LZH" );
const TyPrFileFormatToDesc PrFormatISOToDesc = std::make_pair( ARK_FF_ISO, "ISO" );

const std::vector< TyPrFileFormatToDesc > VecCompressionFormat = {
    PrFormatZIPToDesc,
    PrFormatTARToDesc,
    PrFormatTGZToDesc,
    PrFormat7ZToDesc,
    PrFormatLZHToDesc,
    PrFormatISOToDesc
};

typedef struct tagSupportFeatures
{
    bool                            hasCompression;

    bool                            hasCompressionMethod;
    bool                            hasCompressionLevel;
    bool                            hasEncryptionMethod;
    bool                            hasSplitArchive;
    bool                            hasMultiThreadSupport;

    std::vector< eCompressionMethod >   vecCompressMethods;
    std::vector< eEncryptionMethod >    vecEncryptionMethods;
} SUPPORT_FEATURES;

const SUPPORT_FEATURES SF_ZIP_FEATURES = 
{ 
    true, // hasCompression
    true, // hasCompressionMethod
    true, // hasCompressionLevel
    true, // hasEncryptionMethod
    true, // hasSplitArchive
    true, // hasMultiThreadSupport

    { COMPRESSION_METHOD_STORE, COMPRESSION_METHOD_DEFLATE, COMPRESSION_METHOD_LZMA }, 
    { ENCRYPTION_ZIP, ENCRYPTION_AES256 }
};

const SUPPORT_FEATURES SF_TAR_FEATURES = 
{
    true,   // hasCompression
    false,  // hasCompressionMethod
    false,  // hasCompressionLevel
    false,  // hasEncryptionMethod
    false,  // hasSplitArchive
    false,  // hasMultiThreadSupport

    { COMPRESSION_METHOD_STORE },
    { ENCRYPTION_NONE }
};

const SUPPORT_FEATURES SF_TGZ_FEATURES = 
{
    true,   // hasCompression
    false,  // hasCompressionMethod
    false,  // hasCompressionLevel
    false,  // hasEncryptionMethod
    false,  // hasSplitArchive
    false,  // hasMultiThreadSupport

    { COMPRESSION_METHOD_STORE },
    { ENCRYPTION_NONE }
};

const SUPPORT_FEATURES SF_7Z_FEATURES = 
{
    true, // hasCompression
    true, // hasCompressionMethod
    true, // hasCompressionLevel
    true, // hasEncryptionMethod
    true, // hasSplitArchive
    true, // hasMultiThreadSupport

    { COMPRESSION_METHOD_STORE, COMPRESSION_METHOD_LZMA, COMPRESSION_METHOD_LZMA2 },
    { ENCRYPTION_ZIP, ENCRYPTION_AES256 }
};

const SUPPORT_FEATURES SF_LZH_FEATURES = 
{
    true, // hasCompression
    true, // hasCompressionMethod
    false, // hasCompressionLevel
    false, // hasEncryptionMethod
    false, // hasSplitArchive
    false, // hasMultiThreadSupport

    { COMPRESSION_METHOD_STORE, COMPRESSION_METHOD_LH6, COMPRESSION_METHOD_LH7 },
    { ENCRYPTION_NONE }
};

const SUPPORT_FEATURES SF_ISO_FEATURES = 
{
    true,   // hasCompression
    false,  // hasCompressionMethod
    false,  // hasCompressionLevel
    false,  // hasEncryptionMethod
    false,  // hasSplitArchive
    false,  // hasMultiThreadSupport

    { COMPRESSION_METHOD_STORE },
    { ENCRYPTION_NONE }
};

typedef std::map< ARK_FF, SUPPORT_FEATURES > TyMapFileFormatToFeatures;

//////////////////////////////////////////////////////////////////////////

const std::wstring CONFIG_FILENAME = L"UnArkWCX.xml";
const std::wstring CONFIG_EXENAME = L"UnArkWCX_Opts.exe";
const std::wstring PACKER_EXTENSION = L"ARKWCX";

const tagXMLItemV2< std::string > OPT_COMPRESSION_FORMAT    = { "/UnArkWCX/compressionFormat", PrFormatZIPToDesc.second };
const tagXMLItemV2< std::string > OPT_COMPRESSION_METHOD    = { "/UnArkWCX/compressionMethod", "DEFLATE" };
const tagXMLItemV2< int >         OPT_COMPRESSION_LEVEL     = { "/UnArkWCX/compressionLevel", -1 };
const tagXMLItemV2< std::string > OPT_ENCRYPTION_METHOD     = { "/UnArkWCX/encryptionMethod", "None" };
const tagXMLItemV2< bool >        OPT_SPLIT_ARCHIVE         = { "/UnArkWCX/splitArchive", false };
const tagXMLItemV2< int >         OPT_SPLIT_ARCHIVE_ENUM    = { "/UnArkWCX/splitArchiveSizeEnum", 0 };
const tagXMLItemV2< int >         OPT_SPLIT_ARCHIVE_MANUAL  = { "/UnArkWCX/splitArhciveSizeManual", 0 };
const tagXMLItemV2< bool >        OPT_MULTI_THREAD_SUPPORT  = { "/UnArkWCX/multithreadSupport", true };
const tagXMLItemV2< std::string > OPT_EXTRACT_CODEPAGE      = { "/UnArkWCX/multithreadSupport", "ANSI" };

