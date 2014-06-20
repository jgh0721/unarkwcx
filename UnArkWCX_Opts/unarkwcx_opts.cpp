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

    std::string compressionFormat = xmlMapper.GetDataFromItem( OPT_COMPRESSION_FORMAT );

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

    xmlMapper.GetDataFromItem( OPT_MULTI_THREAD_SUPPORT );
    xmlMapper.GetDataFromItem( OPT_EXTRACT_CODEPAGE );
}

void UnArkWCX_Opts::saveOpts()
{
    CXMLMapperV2& xmlMapper = GetXMLMapper();

    xmlMapper.SetDataFromItem( OPT_COMPRESSION_FORMAT, ui.cbxCompressionFormats->currentText().toStdString() );

    switch( static_cast<eCompressionMethod>(ui.cbxCompressionMethod->currentData().toInt()) )
    {
        case COMPRESSION_METHOD_STORE:
            xmlMapper.SetDataFromItem( OPT_COMPRESSION_METHOD, std::string("STORE") );
            break;
        case COMPRESSION_METHOD_DEFLATE:
            xmlMapper.SetDataFromItem( OPT_COMPRESSION_METHOD, std::string( "DEFLATE" ) );
            break;
        case COMPRESSION_METHOD_LZMA:
            xmlMapper.SetDataFromItem( OPT_COMPRESSION_METHOD, std::string( "LZMA" ) );
            break;
        case COMPRESSION_METHOD_LZMA2:
            xmlMapper.SetDataFromItem( OPT_COMPRESSION_METHOD, std::string( "LZMA2" ) );
            break;
        case COMPRESSION_METHOD_LH6:
            xmlMapper.SetDataFromItem( OPT_COMPRESSION_METHOD, std::string( "LH6" ) );
            break;
        case COMPRESSION_METHOD_LH7:
            xmlMapper.SetDataFromItem( OPT_COMPRESSION_METHOD, std::string( "LH7" ) );
            break;
    }

    xmlMapper.SetDataFromItem( OPT_COMPRESSION_LEVEL, ui.cbxCompressionLevel->currentData().toInt() );
    xmlMapper.SetDataFromItem( OPT_ENCRYPTION_METHOD, ui.cbxEncryptionMethod->currentText().toStdString() );
    xmlMapper.SetDataFromItem( OPT_SPLIT_ARCHIVE, ui.grpSplitArchive->isChecked() );
    xmlMapper.SetDataFromItem( OPT_SPLIT_ARCHIVE_ENUM, ui.cbxSplitArchiveSize->currentData().toInt() );
    xmlMapper.SetDataFromItem( OPT_SPLIT_ARCHIVE_MANUAL, ui.edtSplitArchiveSize->text().toInt() );

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

void UnArkWCX_Opts::closeEvent( QCloseEvent* ev )
{
    saveOpts();

    ev->accept();
}

//////////////////////////////////////////////////////////////////////////

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
            ui.cbxSplitArchiveSize->setEnabled( false );
            ui.edtSplitArchiveSize->setEnabled( false );
            ui.grpSplitArchive->setEnabled( false );
            break;
        case ARK_FF_ZIP:
        case ARK_FF_7Z:
            ui.cbxCompressionMethod->setEnabled( true );
            ui.cbxCompressionLevel->setEnabled( true );
            ui.cbxEncryptionMethod->setEnabled( true );
            ui.cbxSplitArchiveSize->setEnabled( true );
            ui.edtSplitArchiveSize->setEnabled( true );
            ui.grpSplitArchive->setEnabled( true );
            break;
        case ARK_FF_LZH:
            ui.cbxCompressionMethod->setEnabled( true );
            ui.cbxCompressionLevel->setEnabled( false );
            ui.cbxEncryptionMethod->setEnabled( false );
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
        compressionLevel = xmlMapper.GetDataFromItem( OPT_COMPRESSION_LEVEL );
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
        compressionMethod = xmlMapper.GetDataFromItem( OPT_COMPRESSION_METHOD );
        compressionIndex = ui.cbxCompressionMethod->findText(
            QString::fromUtf8( compressionMethod.c_str() ), Qt::MatchContains | Qt::MatchFixedString );
    }
    ui.cbxCompressionMethod->setCurrentIndex( compressionIndex );
    
    std::string encryptionMethod = EnumString<eEncryptionMethod>::From( ENCRYPTION_NONE );
    int encryptionIndex = -1;
    if( supportFeatures.hasEncryptionMethod == true )
    {
        encryptionMethod = xmlMapper.GetDataFromItem( OPT_ENCRYPTION_METHOD );
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
