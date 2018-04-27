#ifndef VOUW_HPP
#define VOUW_HPP

#include <QString>

#include <vouw/encoder.h>

class ImageEncoder : public Vouw::Encoder {
public:
    ImageEncoder( const QString& filename, int levels );
    ~ImageEncoder();


private:
};

#endif
