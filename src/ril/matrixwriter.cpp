/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#include "matrixwriter.h"
#include <stdexcept>     
#include <fstream>
#include <iostream>
#include <vouw/massfunction.h>

// MatrixWriter
        
MatrixWriter::WriterMapT MatrixWriter::map;
        
void
MatrixWriter::registerWriter( MatrixWriter* w ) {
    if( w->fileTypeString().empty() || map.find( w->fileTypeString() ) != map.end() ) {
        throw std::runtime_error( "Invalid filetype string when registering new MatrixWriter class." );
    }
    map[w->fileTypeString()] = w;
}

MatrixWriter* 
MatrixWriter::getWriter( std::string fileType ) {
    auto it = map.find( fileType );
    if( it == map.end() ) return nullptr;
    return it->second;
}

void
MatrixWriter::destroy() {
    for( auto it : map ) 
        delete it.second;
    map.clear();
}


void 
registerBuiltinWriters() {
    MatrixWriter::registerWriter( new PGMWriter() );
}

// PGMWriter


bool 
PGMWriter::writeMatrix( Vouw::Matrix2D& mat, const std::string& path ) {
    const Vouw::MassFunction& dist =mat.distribution();

    int floor =0;

    if( dist.upperBound() > 255 ) {
        floor =dist.lowerBound();
        if( dist.upperBound() - floor > 255 ) {
            std::cerr << "PGMWriter: data distribution does not fit PGM file format." << std::endl;
            return false;
        }
    }

    std::ofstream file;
    
    try {
        file.open( path, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary );
    } catch( std::ios_base::failure e ) {
        std::cerr << e.what() << std::endl;
        return false;
    }


    file << "P5\n" << mat.width() << " " << mat.height() << "\n" << dist.upperBound()-floor << "\n";

    // Matrix2D stores its data as uint32_t so we cannot write it directly
    // Therefore we use the slow per-byte method
    for( int i =0; i < mat.count(); i++ ) {
        char c =(char)((mat.data()[i] & ~VOUW_UNODE32_FLAGGED) - floor);
        file.write( &c, sizeof(char) );
    }

    file.close();

    return true;
}

std::string 
PGMWriter::fileTypeString() { return "pgm"; }

