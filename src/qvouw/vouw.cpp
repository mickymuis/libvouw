#include "vouw.h"
#include <iostream>
#include <QImage>
#include <QFileInfo>

Vouw::Vouw( const QString& n ) : matrix(0), handle(0), name( n ) {

}

Vouw::~Vouw() {
    if( matrix );
        vouw_matrix_free( matrix );
    if( handle )
        vouw_free( handle );
}

Vouw* 
Vouw::createFromImage( const QString& filename, int levels ) {
    QImage img( filename );
    if( img.isNull() ) {
        std::cerr << "Unable to load image `" << filename.unicode() << "'" << std::endl;
        return 0;
    }

    QImage gray =img.convertToFormat( QImage::Format_Grayscale8 );
    int N = gray.width() * gray.height();
    int shift =8;
    while( (levels /= 2) > 0 ) {
        shift--;
    }
    int levels2 = 256 >> shift;


    Vouw* v =new Vouw( QFileInfo( filename ).fileName() );

    std::cout << "Import image: width= " << gray.width() << " height=" << gray.height() << " levels=" << levels2 << " shift=" << shift << std::endl;

    v->matrix = vouw_matrix_create( gray.width(), gray.height(), levels2 );
    for( int i =0; i < N; i++ )
        v->matrix->buffer[i] = gray.bits()[i] >> shift;

    v->handle = vouw_createFrom( v->matrix );
//    vouw_encode( v->handle );

    std::cerr << std::flush;
    std::cout << std::flush;
    
    return v;
}
