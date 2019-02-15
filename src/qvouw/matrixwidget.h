#ifndef MATRIXWIDGET_H
#define MATRIXWIDGET_H

#include <QWidget>
#include <QMap>

#include <vouw/encoder.h>
#include <vouw/matrix.h>

class MatrixWidget : public QWidget {
Q_OBJECT
public:
    enum Mode { None, InputMatrix, InstanceMatrix, Pattern };

    MatrixWidget( QWidget *parent =0 );
    ~MatrixWidget();

    void showMatrix( Vouw::Matrix2D* mat );
    void showEncoded( Vouw::Encoder* v );
    
    void panBy( float x, float y );
    void setPan( float x, float y );
    void setZoom( float f );

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

public slots:
    void zoomStepIn();
    void zoomStepOut();
    void zoomFill();
    void zoomFit();
    void resetPan();
    
signals:
    void clicked();

protected:
    void paintEvent( QPaintEvent *event ) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QColor colorLabel( int label );
    QColor colorValue( Vouw::Matrix2D::ElementT value, int base );
    int mode;
    union data_t {
        Vouw::Matrix2D* mat;
        Vouw::Encoder* enc;
    } data;
    QSize worldSize;
    qreal pixelSize;
    QPoint lastPos;

    struct viewstate_t {
        float zoom, yPan, xPan;
    };

    struct viewstate_t *vs;

    void setViewstate( void* ptr );
    QMap<void*, struct viewstate_t> viewstateHistory;
};

#endif
