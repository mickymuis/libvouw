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

    bool encodeStep();
    void encode();
    bool isEncoded() const;

    double compressedSize() const;
    double uncompressedSize() const;
    double ratio() const;

    vouw_matrix_t* matrix;
    vouw_t* handle;
    QString name;

private:

    double initialBits;
    bool encoded;
};

#endif
