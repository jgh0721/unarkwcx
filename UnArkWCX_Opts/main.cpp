#include "stdafx.h"
#include "unarkwcx_opts.h"
#include <QtWidgets/QApplication>

using namespace nsCommon;
using namespace nsCommon::nsCmnPath;
using namespace nsCommon::nsCmnDateTime;
using namespace nsCommon::nsCmnLogger;
using namespace nsCommon::nsCmnConvert;
using namespace nsCommon::nsCmnFormatter;

#pragma execution_character_set( "utf-8" )

int main(int argc, char *argv[])
{
    QApplication::addLibraryPath( QString::fromStdWString( GetCurrentPath() ) );
    QApplication::addLibraryPath( QString::fromStdWString( tsFormat( L"%1\\plugins", GetCurrentPath() ) ) );
    if( _wgetenv( L"QTDIR" ) != NULL )
    {
        QApplication::addLibraryPath( QString::fromWCharArray( _wgetenv( L"QTDIR" ) ) );
        QApplication::addLibraryPath( QString::fromStdWString( tsFormat( L"%1\\plugins", _wgetenv( L"QTDIR" ) ) ) );
    }

    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed( true );

    UnArkWCX_Opts w;
    w.show();

    return a.exec();
}
