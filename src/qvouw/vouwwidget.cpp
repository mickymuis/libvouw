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

    makeObject();

    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_COLOR_ATTRIBUTE 1

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

// create Vertex Array Object (VAO)
    vao.create();
    vao.bind();
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

//    for (int i = 0; i < 6; ++i) {
//        glDrawArrays(GL_POINTS, i*4, 4);
//    }
    glDrawArrays( GL_POINTS, 0, vbo.size() );
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

    if (event->buttons() & Qt::LeftButton) {
        //rotateBy(8 * dy, 8 * dx, 0);
        panBy( (float)dx * .001f, (float)dy * .001f );
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

void VouwWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    emit clicked();
}

void VouwWidget::showMatrix( vouw_matrix_t* mat ) {
    if( vbo.isCreated() ) {
        vbo.release();
        //vbo.destroy();
    }

    int height = mat->height;
    float yCenter = height * .5f;
    float xCenter = mat->width * .5f;

    QVector<GLfloat> vertData;
    for (int i = 0; i < height; ++i) {    
        vouw_row_t* row = &mat->rows[i];
        for (int j = 0; j < row->size; ++j) {

        // vertex position
        vertData.append((float)j - xCenter);
        vertData.append((float)i - yCenter);
        vertData.append(0.0f);

        // color
        vertData.append( (float)row->cols[j] * 8.0f );
        vertData.append( (float)row->cols[j] * 8.0f );
        vertData.append( (float)row->cols[j] * 8.0f );
        }
    }


    vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
    update();
}

void VouwWidget::makeObject()
{
   /* static const int coords[4][3] = {
        { +1, 1, 1 }, { -1, -1, 1 }, { -1, +1, 1 }, { +1, +1, -1 }
    };*/

static const int coords[6][4][3] = {
        { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
        { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
        { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
        { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
        { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
        { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };

    QVector<GLfloat> vertData;
    for (int i = 0; i < 6; ++i) {    
        for (int j = 0; j < 4; ++j) {
        // vertex position
        vertData.append(0.2 * coords[i][j][0]);
        vertData.append(0.2 * coords[i][j][1]);
        vertData.append(0.2 * coords[i][j][2]);
        // color
        vertData.append( (float)i / 3.0f );
        vertData.append( (float)(3-i) / 3.0f );
        vertData.append( (float)i / 3.0f );
        }
    }

    vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
    update();
}
