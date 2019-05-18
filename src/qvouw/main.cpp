/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */
#include <QApplication>
#include <QSurfaceFormat>
#include <cstdio>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qvouw);

    QApplication app(argc, argv);

  /*  QSurfaceFormat format;
    //format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setDepthBufferSize(24);
    format.setMajorVersion(3);
    format.setMinorVersion(2);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);    
    QSurfaceFormat::setDefaultFormat(format);*/

    MainWindow window;
    window.show();

    // This is not a great way to redirect stdout, but it seems to be complicated in general
    freopen( ".stdout.tmp", "w", stdout );
    window.addLogFile( ".stdout.tmp" );
    
    //freopen( ".stderr.tmp", "w", stderr );
    //window.addLogFile( ".stderr.tmp" );

   // QStringList args = app.arguments();
    for( int i =1; i < argc; i++ ) {
        // TODO: make commandline flags
        QVouw::FileOpts opts;
        opts.filename = QString( argv[i] );
        opts.filetype = QVouw::IMAGE_IMPORT;
        opts.levels = 8;
        opts.use_tabu = true;
        window.import( opts );
    }
    return app.exec();
}
