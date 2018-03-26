#ifndef VOUWWIDGET_H
#define VOUWWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class VouwWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit VouwWidget(QWidget *parent = 0);
    ~VouwWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void rotateBy(int xAngle, int yAngle, int zAngle);
    void setClearColor(const QColor &color);

signals:
    void clicked();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private:
    void makeObject();

    int zoom;
    QColor clearColor;
    QPoint lastPos;
    int xRot;
    int yRot;
    int zRot;
//    QOpenGLTexture *textures[6];
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;
};

#endif