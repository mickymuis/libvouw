#include <QApplication>
#include <QSurfaceFormat>

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
