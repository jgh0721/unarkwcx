#include "stdafx.h"
#include "unarkwcx_opts.h"

#include "typeDefs.h"
#include "commonXMLMapperV2.h"

using namespace nsCommon;
using namespace nsCommon::nsCmnPath;
using namespace nsCommon::nsCmnDateTime;
using namespace nsCommon::nsCmnLogger;
using namespace nsCommon::nsCmnConvert;
using namespace nsCommon::nsCmnFormatter;

#pragma execution_character_set( "utf-8" )

UnArkWCX_Opts::UnArkWCX_Opts(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    do
    {
        CXMLMapperV2& xmlMapper = GetXMLMapper();

        if( xmlMapper.LoadFile( CU2U8( GetCurrentPath() + L"\\" + CONFIG_FILENAME + L".sample" ) ) == false )
        {
            if( xmlMapper.LoadFile( CU2U8( GetCurrentPath() + L"\\" + CONFIG_FILENAME ) ) == false )
            {
                qApp->quit();
                break;
            }
        }

        ui.cbxCompressionLevel->clear();
        for( int compressionLevel = COMPRESS_LEVEL_DEFAULT; compressionLevel != COMPRESS_LEVEL_MAX; compressionLevel++ )
        {
            auto method = static_cast<eCompressionLevel>(compressionLevel);
            auto methodText = EnumString< eCompressionLevel >::From( method );

            ui.cbxCompressionLevel->addItem( QString::fromStdString( methodText ), method );
        }

        for( int encryptionMethod = ENCRYPTION_NONE; encryptionMethod != ENCRYPTION_MAX; encryptionMethod++ )
        {
            auto method = static_cast<eEncryptionMethod>(encryptionMethod);
            auto methodText = EnumString< eEncryptionMethod >::From( method );

            ui.cbxEncryptionMethod->addItem( QString::fromStdString( methodText ), method );
        }

        ui.cbxCompressionMethod->clear();
        for( int compressionMethod = COMPRESSION_METHOD_STORE; compressionMethod != COMPRESSION_METHOD_MAX; compressionMethod++ )
        {
            auto method = static_cast<eCompressionMethod>(compressionMethod);
            auto methodText = EnumString< eCompressionMethod >::From( method );

            ui.cbxCompressionMethod->addItem( QString::fromStdString( methodText ), method );
        }

        ui.cbxSplitArchiveSize->clear();
        for( int splitArchiveSize = SPLIT_ARCHIVE_NONE; splitArchiveSize != SPLIT_ARCHIVE_MAX; splitArchiveSize++ )
        {
            auto method = static_cast<eSplitArchiveSize>(splitArchiveSize);
            auto methodText = EnumString< eSplitArchiveSize >::From( method );

            ui.cbxSplitArchiveSize->addItem( QString::fromStdString( methodText ), method );
        }

        initGlobalFeatures();
        initFormatFeatures();
        registerCompressionFormats();

        loadOpts();
    } while( false );
}

UnArkWCX_Opts::~UnArkWCX_Opts()
{

}

void UnArkWCX_Opts::loadOpts()
{
    CXMLMapperV2& xmlMapper = GetXMLMapper();

    // GLOBAL
    m_arkGlobalOpt.bConvertNFD2NFCWhenMacOS = xmlMapper.GetDataFromItem( OPT_GLOBAL_CONVERT_NFD2NFC );
    m_arkGlobalOpt.bIgnoreMacOSXMetaFolder = xmlMapper.GetDataFromItem( OPT_GLOBAL_IGNORE_MACOS_META_FOLDER );
    m_arkGlobalOpt.bUseLongPathName = xmlMapper.GetDataFromItem( OPT_GLOBAL_USE_LONGPATH );
    m_arkGlobalOpt.bTreatUnixZipFileNameAsUTF8 = xmlMapper.GetDataFromItem( OPT_GLOBAL_TREAT_ZIP_AS_UTF8 );
    m_arkGlobalOpt.bTreatTGZAsSolidArchive = xmlMapper.GetDataFromItem( OPT_GLOBAL_TREAT_TGZ_AS_SOLID );
    m_arkGlobalOpt.bTreatTBZAsSolidArchive = xmlMapper.GetDataFromItem( OPT_GLOBAL_TREAT_TBZ_AS_SOLID );

    ui.chkConvertNFD2NFCWhenMACOS->setCheckState( m_arkGlobalOpt.bConvertNFD2NFCWhenMacOS == TRUE ? Qt::Checked : Qt::Unchecked );
    ui.chkIgnoreMACOSMetaFolder->setCheckState( m_arkGlobalOpt.bIgnoreMacOSXMetaFolder == TRUE ? Qt::Checked : Qt::Unchecked );
    ui.chkLongPathNameExceedsMAXPATH->setCheckState( m_arkGlobalOpt.bUseLongPathName == TRUE ? Qt::Checked : Qt::Unchecked );
    ui.chkTreatUnixZIPAsUTF8->setCheckState( m_arkGlobalOpt.bTreatUnixZipFileNameAsUTF8 == TRUE ? Qt::Checked : Qt::Unchecked );
    ui.chkTreatTGZAsSolidArchive->setCheckState( m_arkGlobalOpt.bTreatTGZAsSolidArchive == TRUE ? Qt::Checked : Qt::Unchecked );
    ui.chkTreatTBZAsSolidArchive->setCheckState( m_arkGlobalOpt.bTreatTBZAsSolidArchive == TRUE ? Qt::Checked : Qt::Unchecked );

    // COMPRESSION
    std::string compressionFormat = xmlMapper.GetDataFromItem( OPT_COMPRESS_COMPRESSION_FORMAT );

    if( compressionFormat.empty() == true )
        compressionFormat = PrFormatZIPToDesc.second;

    for( size_t idx = 0; idx < VecCompressionFormat.size(); ++idx )
    {
        if( u8sicmp( VecCompressionFormat[ idx ].second, compressionFormat ) == 0 )
        {
            ui.cbxCompressionFormats->setCurrentText( QString::fromStdString(VecCompressionFormat[ idx ].second.c_str()) );
            initCompressionFormatFeatures( VecCompressionFormat[ idx ].first );
            loadOpts( VecCompressionFormat[ idx ].first );
        }
    }
    
    ui.chkDeleteArchiveWhenFailed->setCheckState( xmlMapper.GetDataFromItem( OPT_COMPRESS_DELETE_ARCHIVE_WHEN_FAILED ) == true ? Qt::Checked : Qt::Unchecked );
    ui.chkSaveNTFSTime->setCheckState( xmlMapper.GetDataFromItem( OPT_COMPRESS_SAVE_NTFS_TIME ) == true ? Qt::Checked : Qt::Unchecked );
    ui.chkForceZIP64Format->setCheckState( xmlMapper.GetDataFromItem( OPT_COMPRESS_FORCE_ZIP64_FORMAT ) == true ? Qt::Checked : Qt::Unchecked );
    ui.chkFileNameUTF8InZIP->setCheckState( xmlMapper.GetDataFromItem( OPT_COMPRESS_FORCE_FILENAME_UTF8_ZIP ) == true ? Qt::Checked : Qt::Unchecked );
    ui.chkFileNameUTF8InTAR->setCheckState( xmlMapper.GetDataFromItem( OPT_COMPRESS_FORCE_FILENAME_UTF8_TAR ) == true ? Qt::Checked : Qt::Unchecked );
    ui.chkUseFileNameUTF8IfNeeded->setCheckState( xmlMapper.GetDataFromItem( OPT_COMPRESS_USE_UTF8_IF_NEEDED ) == true ? Qt::Checked : Qt::Unchecked );

    xmlMapper.GetDataFromItem( OPT_MULTI_THREAD_SUPPORT );

    // DECOMPRESSION
    xmlMapper.GetDataFromItem( OPT_EXTRACT_CODEPAGE_FILENAME );
}

void UnArkWCX_Opts::saveOpts()
{
    CXMLMapperV2& xmlMapper = GetXMLMapper();

    // GLOBAL

    xmlMapper.SetDataFromItem( OPT_GLOBAL_CONVERT_NFD2NFC, ui.chkConvertNFD2NFCWhenMACOS->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_GLOBAL_IGNORE_MACOS_META_FOLDER, ui.chkIgnoreMACOSMetaFolder->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_GLOBAL_USE_LONGPATH, ui.chkLongPathNameExceedsMAXPATH->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_GLOBAL_TREAT_ZIP_AS_UTF8, ui.chkTreatUnixZIPAsUTF8->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_GLOBAL_TREAT_TGZ_AS_SOLID, ui.chkTreatTGZAsSolidArchive->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_GLOBAL_TREAT_TBZ_AS_SOLID, ui.chkTreatTBZAsSolidArchive->checkState() == Qt::Checked ? true : false );

    // COMPRESSION

    xmlMapper.SetDataFromItem( OPT_COMPRESS_COMPRESSION_FORMAT, ui.cbxCompressionFormats->currentText().toStdString() );

    switch( static_cast<eCompressionMethod>(ui.cbxCompressionMethod->currentData().toInt()) )
    {
        case COMPRESSION_METHOD_STORE:
            xmlMapper.SetDataFromItem( OPT_COMPRESS_COMPRESSION_METHOD, std::string( "STORE" ) );
            break;
        case COMPRESSION_METHOD_DEFLATE:
            xmlMapper.SetDataFromItem( OPT_COMPRESS_COMPRESSION_METHOD, std::string( "DEFLATE" ) );
            break;
        case COMPRESSION_METHOD_LZMA:
            xmlMapper.SetDataFromItem( OPT_COMPRESS_COMPRESSION_METHOD, std::string( "LZMA" ) );
            break;
        case COMPRESSION_METHOD_LZMA2:
            xmlMapper.SetDataFromItem( OPT_COMPRESS_COMPRESSION_METHOD, std::string( "LZMA2" ) );
            break;
        case COMPRESSION_METHOD_LH6:
            xmlMapper.SetDataFromItem( OPT_COMPRESS_COMPRESSION_METHOD, std::string( "LH6" ) );
            break;
        case COMPRESSION_METHOD_LH7:
            xmlMapper.SetDataFromItem( OPT_COMPRESS_COMPRESSION_METHOD, std::string( "LH7" ) );
            break;
    }

    xmlMapper.SetDataFromItem( OPT_COMPRESS_COMPRESSION_LEVEL, ui.cbxCompressionLevel->currentData().toInt() );
    xmlMapper.SetDataFromItem( OPT_COMPRESS_ENCRYPTION_METHOD, ui.cbxEncryptionMethod->currentText().toStdString() );

    xmlMapper.SetDataFromItem( OPT_COMPRESS_DELETE_ARCHIVE_WHEN_FAILED, ui.chkDeleteArchiveWhenFailed->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_COMPRESS_SAVE_NTFS_TIME, ui.chkSaveNTFSTime->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_COMPRESS_FORCE_ZIP64_FORMAT, ui.chkForceZIP64Format->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_COMPRESS_FORCE_FILENAME_UTF8_ZIP, ui.chkFileNameUTF8InZIP->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_COMPRESS_FORCE_FILENAME_UTF8_TAR, ui.chkFileNameUTF8InTAR->checkState() == Qt::Checked ? true : false );
    xmlMapper.SetDataFromItem( OPT_COMPRESS_USE_UTF8_IF_NEEDED, ui.chkUseFileNameUTF8IfNeeded->checkState() == Qt::Checked ? true : false );

    xmlMapper.SetDataFromItem( OPT_SPLIT_ARCHIVE, ui.grpSplitArchive->isChecked() );
    xmlMapper.SetDataFromItem( OPT_SPLIT_ARCHIVE_ENUM, ui.cbxSplitArchiveSize->currentData().toInt() );
    xmlMapper.SetDataFromItem( OPT_SPLIT_ARCHIVE_MANUAL, ui.edtSplitArchiveSize->text().toInt() );

    // DECOMPRESSION

    xmlMapper.SaveFile();
    xmlMapper.RefreshXML();
}

//////////////////////////////////////////////////////////////////////////

void UnArkWCX_Opts::on_cbxCompressionFormats_currentIndexChanged( int index )
{
    if( index < 0 )
        return;

    QVariant vtData = ui.cbxCompressionFormats->itemData( index );
    if( vtData.isValid() == false )
        return;

    initCompressionFormatFeatures( static_cast<ARK_FF>(vtData.toInt()) );
}

void UnArkWCX_Opts::on_okButton_clicked( bool checked )
{
    saveOpts();

    accept();
}

void UnArkWCX_Opts::on_cancelButton_clicked( bool checked )
{
    rejected();
}

void UnArkWCX_Opts::closeEvent( QCloseEvent* ev )
{
    ev->accept();
}

//////////////////////////////////////////////////////////////////////////

void UnArkWCX_Opts::initGlobalFeatures()
{

    m_arkGlobalOpt.bAzoSupport = TRUE;
    m_arkGlobalOpt.bConvertNFD2NFCWhenMacOS = OPT_GLOBAL_CONVERT_NFD2NFC.xmlDefaultValue == true ? TRUE : FALSE;;
    m_arkGlobalOpt.bIgnoreMacOSXMetaFolder = OPT_GLOBAL_IGNORE_MACOS_META_FOLDER.xmlDefaultValue == true ? TRUE : FALSE;;
    m_arkGlobalOpt.bOpenArchiveWithShareDelete = FALSE;
    m_arkGlobalOpt.bPrintAssert = FALSE;
    m_arkGlobalOpt.bTreatTBZAsSolidArchive = OPT_GLOBAL_TREAT_TBZ_AS_SOLID.xmlDefaultValue == true ? TRUE : FALSE;;
    m_arkGlobalOpt.bTreatTGZAsSolidArchive = OPT_GLOBAL_TREAT_TGZ_AS_SOLID.xmlDefaultValue == true ? TRUE : FALSE;;
    m_arkGlobalOpt.bTreatUnixZipFileNameAsUTF8 = OPT_GLOBAL_TREAT_ZIP_AS_UTF8.xmlDefaultValue == true ? TRUE : FALSE;;
    m_arkGlobalOpt.bUseLongPathName = OPT_GLOBAL_USE_LONGPATH.xmlDefaultValue == true ? TRUE : FALSE;;
}

void UnArkWCX_Opts::initFormatFeatures()
{
    m_mapFileFormatToFeatures[ ARK_FF_ZIP ] = SF_ZIP_FEATURES;
    m_mapFileFormatToFeatures[ ARK_FF_TAR ] = SF_TAR_FEATURES;
    m_mapFileFormatToFeatures[ ARK_FF_TGZ ] = SF_TGZ_FEATURES;
    m_mapFileFormatToFeatures[ ARK_FF_7Z ] = SF_7Z_FEATURES;
    m_mapFileFormatToFeatures[ ARK_FF_LZH ] = SF_LZH_FEATURES;
    m_mapFileFormatToFeatures[ ARK_FF_ISO ] = SF_ISO_FEATURES;

}

void UnArkWCX_Opts::registerCompressionFormats()
{
    ui.cbxCompressionFormats->clear();

    ui.cbxCompressionFormats->addItem( QString::fromStdString( PrFormatZIPToDesc.second ), PrFormatZIPToDesc.first );
    ui.cbxCompressionFormats->addItem( QString::fromStdString( PrFormatTARToDesc.second ), PrFormatTARToDesc.first );
    ui.cbxCompressionFormats->addItem( QString::fromStdString( PrFormatTGZToDesc.second ), PrFormatTGZToDesc.first );
    ui.cbxCompressionFormats->addItem( QString::fromStdString( PrFormat7ZToDesc.second ), PrFormat7ZToDesc.first );
    ui.cbxCompressionFormats->addItem( QString::fromStdString( PrFormatLZHToDesc.second ), PrFormatLZHToDesc.first );
    ui.cbxCompressionFormats->addItem( QString::fromStdString( PrFormatISOToDesc.second ), PrFormatISOToDesc.first );

}

void UnArkWCX_Opts::initCompressionFormatFeatures( ARK_FF arkFileFormat )
{
    switch( arkFileFormat )
    {
        case ARK_FF_ISO:
        case ARK_FF_TAR:
        case ARK_FF_TGZ:
            ui.cbxCompressionMethod->setEnabled( false );
            ui.cbxCompressionLevel->setEnabled( false );
            ui.cbxEncryptionMethod->setEnabled( false );

            ui.chkDeleteArchiveWhenFailed->setEnabled( true );
            ui.chkSaveNTFSTime->setEnabled( false );
            ui.chkForceZIP64Format->setEnabled( false );
            ui.chkFileNameUTF8InZIP->setEnabled( false );
            ui.chkFileNameUTF8InTAR->setEnabled( false );
            ui.chkUseFileNameUTF8IfNeeded->setEnabled( false );

            ui.cbxSplitArchiveSize->setEnabled( false );
            ui.edtSplitArchiveSize->setEnabled( false );
            ui.grpSplitArchive->setEnabled( false );
            break;
        case ARK_FF_ZIP:
        case ARK_FF_7Z:
            ui.cbxCompressionMethod->setEnabled( true );
            ui.cbxCompressionLevel->setEnabled( true );
            ui.cbxEncryptionMethod->setEnabled( true );

            ui.chkDeleteArchiveWhenFailed->setEnabled( true );
            ui.chkSaveNTFSTime->setEnabled( true );
            ui.chkForceZIP64Format->setEnabled( true );
            ui.chkFileNameUTF8InZIP->setEnabled( true );
            ui.chkFileNameUTF8InTAR->setEnabled( false );
            ui.chkUseFileNameUTF8IfNeeded->setEnabled( true );

            ui.cbxSplitArchiveSize->setEnabled( true );
            ui.edtSplitArchiveSize->setEnabled( true );
            ui.grpSplitArchive->setEnabled( true );
            break;
        case ARK_FF_LZH:
            ui.cbxCompressionMethod->setEnabled( true );
            ui.cbxCompressionLevel->setEnabled( false );
            ui.cbxEncryptionMethod->setEnabled( false );

            ui.chkDeleteArchiveWhenFailed->setEnabled( true );
            ui.chkSaveNTFSTime->setEnabled( false );
            ui.chkForceZIP64Format->setEnabled( false );
            ui.chkFileNameUTF8InZIP->setEnabled( false );
            ui.chkFileNameUTF8InTAR->setEnabled( false );
            ui.chkUseFileNameUTF8IfNeeded->setEnabled( false );

            ui.cbxSplitArchiveSize->setEnabled( false );
            ui.edtSplitArchiveSize->setEnabled( false );
            ui.grpSplitArchive->setEnabled( false );
            break;
    }

}

void UnArkWCX_Opts::loadOpts( ARK_FF arkFileFormat )
{
    CXMLMapperV2& xmlMapper = GetXMLMapper();

    QStringList supportMethods;

    if( m_mapFileFormatToFeatures.count( arkFileFormat ) <= 0 )
        return;

    auto supportFeatures = m_mapFileFormatToFeatures[ arkFileFormat ];

    int compressionLevel = COMPRESS_LEVEL_DEFAULT;
    if( supportFeatures.hasCompressionLevel == true )
    {
        compressionLevel = xmlMapper.GetDataFromItem( OPT_COMPRESS_COMPRESSION_LEVEL );
        if( compressionLevel < -1 || compressionLevel >= COMPRESS_LEVEL_MAX )
            compressionLevel = COMPRESS_LEVEL_DEFAULT;
    }

    ui.cbxCompressionLevel->setCurrentText(
        QString::fromStdString( EnumString<eCompressionLevel>::From( static_cast<eCompressionLevel>(compressionLevel) ) )
        );
    
    std::string compressionMethod = EnumString<eCompressionMethod>::From( COMPRESSION_METHOD_STORE );
    int compressionIndex = -1;
    if( supportFeatures.hasCompressionMethod == true )
    {
        compressionMethod = xmlMapper.GetDataFromItem( OPT_COMPRESS_COMPRESSION_METHOD );
        compressionIndex = ui.cbxCompressionMethod->findText(
            QString::fromUtf8( compressionMethod.c_str() ), Qt::MatchContains | Qt::MatchFixedString );
    }
    ui.cbxCompressionMethod->setCurrentIndex( compressionIndex );
    
    std::string encryptionMethod = EnumString<eEncryptionMethod>::From( ENCRYPTION_NONE );
    int encryptionIndex = -1;
    if( supportFeatures.hasEncryptionMethod == true )
    {
        encryptionMethod = xmlMapper.GetDataFromItem( OPT_COMPRESS_ENCRYPTION_METHOD );
        encryptionIndex = ui.cbxEncryptionMethod->findText(
            QString::fromUtf8( encryptionMethod.c_str() ), Qt::MatchContains | Qt::MatchFixedString );
    }
    ui.cbxEncryptionMethod->setCurrentIndex( encryptionIndex );

    bool isUseSplitArchive = OPT_SPLIT_ARCHIVE.xmlDefaultValue;
    int splitArchiveEnum = OPT_SPLIT_ARCHIVE_ENUM.xmlDefaultValue;
    int splitArchiveManual = OPT_SPLIT_ARCHIVE_MANUAL.xmlDefaultValue;

    if( supportFeatures.hasSplitArchive == true )
    {
        isUseSplitArchive = xmlMapper.GetDataFromItem( OPT_SPLIT_ARCHIVE );
        splitArchiveEnum = xmlMapper.GetDataFromItem( OPT_SPLIT_ARCHIVE_ENUM );
        if( splitArchiveEnum < 0 || splitArchiveEnum >= SPLIT_ARCHIVE_MAX )
            splitArchiveEnum = SPLIT_ARCHIVE_NONE;

        splitArchiveManual = xmlMapper.GetDataFromItem( OPT_SPLIT_ARCHIVE_MANUAL );
    }
    
    ui.grpSplitArchive->setChecked( isUseSplitArchive );
    ui.cbxSplitArchiveSize->setCurrentText(
        QString::fromStdString( EnumString<eSplitArchiveSize>::From( static_cast< eSplitArchiveSize >(splitArchiveEnum) ).c_str() )
        );
    ui.edtSplitArchiveSize->setText( QString( "%1" ).arg( splitArchiveManual ) );

}
