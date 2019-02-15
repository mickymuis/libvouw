#include "matrixwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMap>
#include <iostream>

MatrixWidget::MatrixWidget( QWidget *parent ) : QWidget( parent ) {

    mode =None;
    vs =NULL;
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
}

MatrixWidget::~MatrixWidget() {}

void 
MatrixWidget::showMatrix( Vouw::Matrix2D* mat ) {
    data.mat =mat;
    mode =InputMatrix;
    setViewstate( (void*) mat );
    update();
}

void 
MatrixWidget::showEncoded( Vouw::Encoder* v ) {
    data.enc =v;
    mode =InstanceMatrix;
    setViewstate( (void*) v );
    update();
}

void
MatrixWidget::panBy( float x, float y ) {
    if( !vs ) return;
    vs->xPan +=x;
    vs->yPan +=y;
    update();
}

void 
MatrixWidget::setPan( float x, float y ) {
    if( !vs ) return;
    vs->xPan = x; vs->yPan =y;
    update();
}

void 
MatrixWidget::setZoom( float f ) {
    if( !vs ) return;
    vs->zoom =f;
    update();
}

void 
MatrixWidget::zoomStepIn() {
    if( !vs ) return;
    vs->zoom *= 1.1f;
    update();
}

void 
MatrixWidget::zoomStepOut() {
    if( !vs ) return;
    vs->zoom *= .9f;
    vs->zoom = qMax( .2f, vs->zoom );
    update();
}

void 
MatrixWidget::zoomFill() {
    if( !vs ) return;
    vs->zoom =1.f;
    update();
}

void 
MatrixWidget::zoomFit() {
    if( !vs ) return;
    if( worldSize.width() / vs->zoom > width() ) {
        vs->zoom = width() / (worldSize.width() / vs->zoom);
    } else if( worldSize.height() / vs->zoom > height() ) {
        vs->zoom = height() / (worldSize.height() / vs->zoom);
    }
    update();
}

void 
MatrixWidget::resetPan() {
    if( !vs ) return;
    vs->xPan = vs->yPan = 0;
    update();
}

QSize 
MatrixWidget::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize 
MatrixWidget::sizeHint() const {
    //return QSize(200, 200);
    return worldSize;
}
    
void 
MatrixWidget::paintEvent( QPaintEvent *event ) {
    if( mode == None || !vs ) return;
    float stroke =.0f;

    /* We will draw each element as a 1x1 rect, which directly gives us the window size */
    QRect window;
    if( mode == InputMatrix ) {
        //window =QRect( -data.mat->width()/2, -data.mat->height()/2, data.mat->width(), data.mat->height() );
        window =QRect( 0, 0, data.mat->width(), data.mat->height() );
    } else {
        window =QRect( 0, 0, data.enc->matrix()->width(), data.enc->matrix()->height() );

    }
    
    QPainter painter(this);
    painter.setWindow( window );
    
    /* In addition, we set up the viewport to match the ratio of the window */

    float s =.0f;

    if( width() > height() ) {     
        float h = ((float)window.height() / (float)window.width()) * (float)width();
        int delta = (int)h - height();
        painter.setViewport( 0, -delta / 2, width(), (int)h );
        s = h / window.height();

    } else {
        float w = ((float)window.width() / (float)window.height()) * (float)height();
        int delta = (int)w - width();
        painter.setViewport( -delta / 2, 0, (int)w, height() );
        s = w / window.width();
    }

    /* Update the size of the displayed content and some properties of it */
    
    pixelSize = s * vs->zoom;   // size of one element in device pixels
    worldSize = QSize( pixelSize * window.width(), pixelSize * window.height() ); // size of image in device pixels

    if( s * vs->zoom > 6.f ) {
        stroke =1.f / pixelSize;
        painter.setRenderHint( QPainter::Antialiasing, true );
    } else {
        painter.setRenderHint( QPainter::Antialiasing, false );

    }


    /* Apply transformations */

    painter.translate( window.width() / 2.f, window.height() / 2.f  );
    painter.scale( vs->zoom, vs->zoom );
    painter.translate( -window.width() / 2.f, -window.height() / 2.f  );
    painter.translate( vs->xPan, vs->yPan );

    /* Compute the clipping rect
       In order to do this, we need to transform the visible viewport to world coordinates (i.e., inverse of the above)*/
    QRectF clip( 0, 0, width(), height() );

    // Size of the widget in terms of world elements (i.e. matrix cells)
    clip.setSize( clip.size() / pixelSize );
    // Inverse world transformation (this is cheaper that computing the inverse matrix)
    clip.translate( -vs->xPan, -vs->yPan );
    clip.translate( (window.width() / 2.f) , (window.height() / 2.f) );
    clip.translate( -(window.width() / 2.f) / vs->zoom, -(window.height() / 2.f) / vs->zoom );
    clip.translate( -painter.viewport().left() / pixelSize, -painter.viewport().top() / pixelSize );
    // Clamp to the rendered image
    clip.setCoords( qMax(0.0,clip.x()), 
                  qMax(0.0,clip.y()), 
                  qMin((qreal)window.width()+window.x(),clip.right()), 
                  qMin((qreal)window.height()+window.y(),clip.bottom()) );

//    std::cout << "Clip: " << clip.size() * s << " " << width() << "x" << height() <<std::endl;
    /* Render the widget depending on the type of content */
    if( mode == InputMatrix ) {

        for( int i = (int)clip.top(); i < qMin(data.mat->height(), (unsigned int)clip.bottom()+1); ++i ) {    
            Vouw::Matrix2D::ElementT *row = data.mat->rowPtr( i );

            for( int j = (int)clip.left(); j < qMin(data.mat->width(), (unsigned int)clip.right()+1); ++j ) {
                QRectF rect( j+stroke, i+stroke, 1-stroke, 1-stroke );
                QColor color = colorValue( row[j], data.mat->base() );

                painter.fillRect( rect, color );
            }
        }
    } else { 

        painter.setPen( QPen( Qt::black,stroke ) );
        QMap<int, QPolygonF> patternPoly;

        for( auto&& region : *data.enc->instanceSet() ) {

            Vouw::Pattern* p =region.pattern();
            if( p->isTabu() )
                continue;
            
            QColor color;
            QPolygonF poly;
            color = colorLabel( p->label() );
            painter.setBrush( color );

            if( stroke != .0f ) {
                poly = patternPoly[p->label()];
            }

            if( stroke == .0f || poly.isEmpty() ) {
                for( auto&& elem : p->elements() ) {
                    Vouw::Coord2D c; 
                    
                    if( stroke == .0f ) {
                        // For each offset, compute its absolute location
                        c = elem.offset.abs( region.pivot() );
                    
                        QRectF rect( QPointF( c.col(), c.row() ), QPointF( c.col() + 1, c.row() + 1 ) );
                        painter.fillRect( rect, color );
                    } else {
                        // Use the relative location within the pattern
                        c = elem.offset;

                        QVector<QPointF> points;
                        points.append( QPointF( c.col(), c.row() ) );
                        points.append( QPointF( c.col() + 1.0001, c.row() ) );
                        points.append( QPointF( c.col() + 1.0001, c.row() + 1.0001 ) );
                        points.append( QPointF( c.col(), c.row() + 1.0001) );
                        poly = poly.united( QPolygonF( points ) );
                    }
                }
            }

            if( stroke != .0f ) {
                patternPoly[p->label()] = poly;
                poly.translate( region.pivot().col(), region.pivot().row() );
                painter.drawPolygon( poly );
            }
        }
    }

    painter.setBrush( Qt::NoBrush );
    painter.setPen( QPen( Qt::black,2.f / pixelSize ) );
    painter.drawRect( clip );


}

void 
MatrixWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void 
MatrixWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    float precision =.01f;

    if (event->buttons() & Qt::LeftButton) {
        //rotateBy(8 * dy, 8 * dx, 0);
        panBy( (float)dx * precision, (float)dy * precision );
    } else if (event->buttons() & Qt::RightButton) {
   //     rotateBy(8 * dy, 8 * dx, 0);
        //rotateBy(8 * dy, 0, 8 * dx);
    }
    lastPos = event->pos();
}

void 
MatrixWidget::wheelEvent(QWheelEvent *event) {
    float vert = event->angleDelta().y();

    if( vs ) {
        if( vert > 1 && worldSize.width() <= width() && worldSize.height() <= height() ) return;

        if( vert > 1 ) zoomStepOut();
        else zoomStepIn();
    }

    event->accept();
}

void 
MatrixWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    emit clicked();
}

QColor
MatrixWidget::colorLabel( int label ) {
    QColor c;
    c.setHsv( (240 + (label * 49)) % 360, 200, 255 - ((label >> 3) * 25) % 200 );
    return c;
}

QColor 
MatrixWidget::colorValue( Vouw::Matrix2D::ElementT value, int base ) {
    float valuef = (float)value / (float)base;
    QColor c;
    c.setRgbF( valuef < 0.5 ? valuef * 2.f : 1.f,   
               valuef > 0.33 ? (valuef - 0.33f) * 1.5f : 0.f,
               valuef > 0.5 ? 0.f : ( valuef < 0.25 ? 0.5 + valuef * 2.f : 1 - (valuef-0.25) * 4.f ) );
    return c;
}

void 
MatrixWidget::setViewstate( void* ptr ) {
    if( !viewstateHistory.contains( ptr ) ) {
        vs = &viewstateHistory[ptr];
        *vs = { .zoom =1.f, .yPan =.0f, .xPan =.0f };
    } else
        vs = &viewstateHistory[ptr];
}
