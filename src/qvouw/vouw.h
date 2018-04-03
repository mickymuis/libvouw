#ifndef VOUW_HPP
#define VOUW_HPP

#include <QString>

#include <vouw/vouw.h>
#include <vouw/matrix.h>

class Vouw {
public:
    Vouw( const QString& name = "" );
    ~Vouw();

    static Vouw* createFromImage( const QString& filename, int levels );

    vouw_matrix_t* matrix;
    vouw_t* handle;
    QString name;
};

#endif
