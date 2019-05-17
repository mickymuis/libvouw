/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#pragma once

#include <QWidget>
#include <QMap>

#include <vouw/encoder.h>
#include <vouw/matrix.h>

class MatrixWidget : public QWidget {
Q_OBJECT
public:
    enum Mode { None, InputMatrix, InstanceMatrix, Pattern };
    enum Option { HideSingletons =1, ShowPivots =2, ShowPeriphery =4 };

    MatrixWidget( QWidget *parent =0 );
    ~MatrixWidget();

    void showMatrix( Vouw::Matrix2D* mat );
    void showEncoded( Vouw::Encoder* v );
    
    void panBy( float x, float y );
    void setPan( float x, float y );
    void setZoom( float f );

    void setOption( Option, bool active );
    bool hasOption( Option ) const;

    bool hasSelection() const { return selectionCenter != QPoint(-1,-1); }

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
    void drawCross( QPainter& painter, QPoint pos );
    int mode, opts;
    union data_t {
        Vouw::Matrix2D* mat;
        Vouw::Encoder* enc;
    } data;
    QSize worldSize;
    qreal pixelSize;
    QPoint lastPos;
    QPointF selectionCenter;
    bool hasSelectionEvent;
    QBrush selectionBrush;

    struct viewstate_t {
        float zoom, yPan, xPan;
    };

    struct viewstate_t *vs;
    QRect window;
    QRect viewport;

    void setViewstate( void* ptr );
    QMap<void*, struct viewstate_t> viewstateHistory;
};

