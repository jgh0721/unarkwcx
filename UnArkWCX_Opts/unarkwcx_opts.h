#ifndef UNARKWCX_OPTS_H
#define UNARKWCX_OPTS_H

#include <QtWidgets/QDialog>
#include "ui_unarkwcx_opts.h"

#include "typeDefs.h"
#include "Ark.h"

#pragma execution_character_set( "utf-8" )

class UnArkWCX_Opts : public QDialog
{
    Q_OBJECT

public:
    UnArkWCX_Opts(QWidget *parent = 0);
    ~UnArkWCX_Opts();

    
    void                                loadOpts();
    void                                saveOpts();

public slots:
    void                                on_cbxCompressionFormats_currentIndexChanged( int index );
    void                                on_okButton_clicked( bool checked );
    void                                on_cancelButton_clicked( bool checked );

protected:

    virtual void                        closeEvent( QCloseEvent* ev );

private:
    void                                initGlobalFeatures();
    void                                initFormatFeatures();
    void                                registerCompressionFormats();

    void                                initCompressionFormatFeatures( ARK_FF arkFileFormat );

    void                                loadOpts( ARK_FF arkFileFormat );

    SArkGlobalOpt                       m_arkGlobalOpt;
    TyMapFileFormatToFeatures           m_mapFileFormatToFeatures;
    Ui::UnArkWCX_OptsClass              ui;
};

#endif // UNARKWCX_OPTS_H
