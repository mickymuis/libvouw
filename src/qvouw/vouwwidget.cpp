#include "vouwwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>

VouwWidget::VouwWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      clearColor(Qt::black),
      xPan(0),
      yPan(0),
      xRot(0),
      yRot(0),
      zRot(0),
      zoom(10),
      program(0)
{
    QPalette p;
    setClearColor( p.dark().color() );
}

VouwWidget::~VouwWidget()
{
    makeCurrent();
    vbo.destroy();
    delete program;
    doneCurrent();
}

QSize VouwWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize VouwWidget::sizeHint() const
{
    return QSize(200, 200);
}

void VouwWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
    xRot += xAngle;
    yRot += yAngle;
    zRot += zAngle;
    update();
}

void VouwWidget::panBy( float x, float y ) {
    xPan +=x;
    yPan +=y;
    update();
}

void VouwWidget::setClearColor(const QColor &color)
{
    clearColor = color;
    update();
}

void VouwWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_COLOR_ATTRIBUTE 1

// create Vertex Array Object (VAO)
    vao.create();
    vao.bind();

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    vshader->compileSourceFile(":/shaders/vertex.glsl");
    
    QOpenGLShader *gshader = new QOpenGLShader(QOpenGLShader::Geometry, this);
    gshader->compileSourceFile(":/shaders/geometry.glsl");

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    fshader->compileSourceFile(":/shaders/fragment.glsl");

    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(gshader);
    program->addShader(fshader);
    program->bindAttributeLocation("in_position", PROGRAM_VERTEX_ATTRIBUTE);
    program->bindAttributeLocation("in_color", PROGRAM_COLOR_ATTRIBUTE);
    program->link();

    program->bind();

//    makeObject();
    vbo.create();
    vbo.bind();
    vbo.setUsagePattern( QOpenGLBuffer::DynamicDraw );
}

void VouwWidget::paintGL()
{
    glClearColor(clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 projection;
    projection.ortho(-zoom * aspectRatio, +zoom * aspectRatio, +zoom, -zoom, .1f, 1000.0f);

    QMatrix4x4 m;
    m.translate(xPan, yPan, -10.0f);
    m.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    m.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

    QMatrix4x4 mvp = projection * m;

    program->setUniformValue("mat_mvp", mvp);
    program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    program->enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
    program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    program->setAttributeBuffer(PROGRAM_COLOR_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    for (int i = 0; i < vbo.size()/6; ++i) {
        glDrawArrays(GL_POINTS, i, 1);
    }
//    if( vbo.isCreated() )
//        glDrawArrays( GL_POINTS, 0, vbo.size()/6 );
}
void VouwWidget::resizeGL(int width, int height)
{
//    int side = qMin(width, height);
//    glViewport((width - side) / 2, (height - side) / 2, side, side);
    glViewport( 0, height, width, height );
    aspectRatio = (float)width / (float)(height);
}

void VouwWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void VouwWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    float precision = zoom / (float)height();

    if (event->buttons() & Qt::LeftButton) {
        //rotateBy(8 * dy, 8 * dx, 0);
        panBy( (float)dx * precision, (float)dy * precision );
    } else if (event->buttons() & Qt::RightButton) {
        rotateBy(8 * dy, 8 * dx, 0);
        //rotateBy(8 * dy, 0, 8 * dx);
    }
    lastPos = event->pos();
}

void VouwWidget::wheelEvent(QWheelEvent *event) {
    float vert = event->angleDelta().y();

    zoom += (vert>1) ? -1 : 1;
    zoom = (zoom<1) ? 1 : zoom;

    update();

    event->accept();
}

void 
VouwWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    emit clicked();
}

void 
VouwWidget::showMatrix( Vouw::Matrix2D* mat ) {
    if( vbo.isCreated() ) {
   //     vbo.release();
       // vbo.destroy();
    }

    float yCenter = mat->height() * .5f;
    float xCenter = mat->width() * .5f;
    zoom = (float)qMax( mat->height(), mat->width() ) / 4.f;

    QVector<GLfloat> vertData;
    for (int i = 0; i < mat->height(); ++i) {    
        Vouw::Matrix2D::ElementT *row = mat->rowPtr(i);
        for (int j = 0; j < mat->width(); ++j) {

        // vertex position
        vertData.append((float)j - xCenter);
        vertData.append((float)i - yCenter);
        vertData.append(0.0f);

        float value =(float)row[j] / (float)mat->base();

        // color
//        vertData.append( value < 0.25 ? 0.f : (value < 0.75 ? (value - 0.25f) * 2.f : 1.f ) ); // RED
        vertData.append( value < 0.5 ? value * 2.f : 1.f ); // RED
        vertData.append( value > 0.33 ? (value - 0.33f) * 1.5f : 0.f ); // GREEN
        //vertData.append( value > 0.5 ? 0.f : 1 - value * 2.f ); // BLUE
        vertData.append( value > 0.5 ? 0.f : ( value < 0.25 ? 0.5 + value * 2.f : 1 - (value-0.25) * 4.f ) ); // BLUE
        }
    }


    //vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
    update();
}

QColor
VouwWidget::colorLabel( int label ) {
    QColor c;
    c.setHsv( (240 + (label * 49)) % 360, 200, 255 - ((label >> 3) * 25) % 200 );
    return c;
}

void 
VouwWidget::showEncoded( Vouw::Encoder* v ) {
    if( vbo.isCreated() ) {
      //  vbo.release();
      //  vbo.destroy();
    }

    QVector<GLfloat> vertData;
    int height = v->matrix()->height();
    int width = v->matrix()->width();
    float yCenter = height * .5f;
    float xCenter = width * .5f;
    zoom = (float)qMax( height, width ) / 4.f;

    for( auto&& region : *v->instanceSet() ) {

        //region_apply( region, m );
        Vouw::Pattern* p =region.pattern();
        QColor color;
        if( p->isTabu() )
            color = clearColor;
        else 
            color = colorLabel( p->label() );
        for( auto&& elem : p->elements() ) {
            // For each offset, compute its location on the automaton
            Vouw::Coord2D c = elem.offset.abs( region.pivot() );
            // Set the buffer's value at c
            // vouw_matrix_setValue( m, c, (p->offsets[i].value + region->variant) % m->base );
            // float value = (float)((p->offsets[i].value + region->variant) % m->base) / (float)v->mat->base; 
            vertData.append((float)c.col() - xCenter);
            vertData.append((float)c.row() - yCenter);
            vertData.append(0.0f);


            vertData.append( color.redF() );
            vertData.append( color.greenF() );
            vertData.append( color.blueF() );
        }

    }

   // vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
    update();
}

void VouwWidget::makeObject()
{

    QVector<GLfloat> vertData;
        // vertex position
        vertData.append(0);
        vertData.append(0);
        vertData.append(0);
        // color
        vertData.append( 1 );
        vertData.append( 1 );
        vertData.append( 1 );
        
        vertData.append(1);
        vertData.append(1);
        vertData.append(0);
        // color
        vertData.append( 0 );
        vertData.append( 1 );
        vertData.append( 1 );

    vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
    update();
}
