/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */
#pragma once

#include <QString>
#include <vouw/matrix.h>
#include <vouw/encoder.h>

namespace QVouw {
    
    enum FileType {
        IMAGE_IMPORT
    };

    struct FileOpts {
        QString filename;
        FileType filetype;
        int levels;
        bool use_tabu;
    };

    struct Handle {
        FileOpts opts;
        Vouw::Matrix2D *matrix;
        Vouw::Encoder *encoder;
    };

    Vouw::Matrix2D* importImage( const FileOpts& opts );

};
