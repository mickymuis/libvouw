/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017-2019, Leiden Institute for Advanced Computer Science
 */

#pragma once

#include <vouw/matrix.h>
#include <map>
#include <string>

class MatrixWriter {
    public:
        virtual ~MatrixWriter() {}

        virtual bool writeMatrix( Vouw::Matrix2D& mat, const std::string& path ) = 0;

        virtual std::string fileTypeString() = 0;
        
        static void registerWriter( MatrixWriter* );
        static MatrixWriter* getWriter( std::string fileType );

        static void destroy();

    private:
        typedef std::map<std::string,MatrixWriter*> WriterMapT;
        static WriterMapT map;
};

void registerBuiltinWriters();

class PGMWriter : public MatrixWriter { 
    public:
        PGMWriter() {}
        ~PGMWriter() {}

        bool writeMatrix( Vouw::Matrix2D& mat, const std::string& path );

        std::string fileTypeString();

};
