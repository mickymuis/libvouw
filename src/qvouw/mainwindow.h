#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QFile>
#include "qvouw.h"

class VouwWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void importImage();
    void quit();

protected:
    void timerEvent(QTimerEvent *event);

private:

    VouwWidget* vouwWidget;
    QVouw* vouw;
    QTextEdit* console;
    QFile console_stderr;
    QFile console_stdout;
};

#endif
