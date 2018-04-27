#include "vouw.h"
#include <iostream>
#include <QImage>
#include <QFileInfo>

ImageEncoder::~ImageEncoder() {
    if( m_mat )
        delete m_mat;
}

ImageEncoder::ImageEncoder( const QString& filename, int levels ) : Vouw::Encoder() {
    QImage img( filename );
    if( img.isNull() ) {
        std::cerr << "Unable to load image `" << filename.unicode() << "'" << std::endl;
        return;
    }

    QImage gray =img.convertToFormat( QImage::Format_Grayscale8 );
    int N = gray.width() * gray.height();
    int shift =8;
    while( (levels /= 2) > 0 ) {
        shift--;
    }
    int levels2 = 256 >> shift;

    std::cout << "Import image: width= " << gray.width() << " height=" << gray.height() << " levels=" << levels2 << " shift=" << shift << std::endl;

    Vouw::Matrix2D *mat = new Vouw::Matrix2D( gray.width(), gray.height(), levels2 );
    for( int i =0; i < N; i++ )
        mat->data()[i] = gray.bits()[i] >> shift;

    setFromMatrix( mat );

    std::cerr << std::flush;
    std::cout << std::flush;
}

