/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#include "qvouw.h"
#include <iostream>
#include <QImage>
#include <QFileInfo>

namespace QVouw {

    Vouw::Matrix2D* 
    importImage( const FileOpts& opts ) {
        QImage img( opts.filename );
        if( img.isNull() ) {
            std::cerr << "Unable to load image `" << opts.filename.unicode() << "'" << std::endl;
            return nullptr;
        }

        QImage gray =img.convertToFormat( QImage::Format_Grayscale8 );
        int N = gray.width() * gray.height();
        int shift =8;
        int levels =opts.levels;
        while( (levels /= 2) > 0 && shift > 0 ) {
            shift--;
        }
        int levels2 = 256 >> shift;

        std::cout << "Import image: width= " << gray.width() << " height=" << gray.height() << " levels=" << levels2 << " shift=" << shift << std::endl;

        Vouw::Matrix2D *mat = new Vouw::Matrix2D( gray.width(), gray.height(), levels2 );
        for( int i =0; i < N; i++ )
            mat->data()[i] = gray.bits()[i] >> shift;

        std::cerr << std::flush;
        std::cout << std::flush;

        return mat;
    }
};
