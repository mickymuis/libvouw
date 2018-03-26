#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class VouwWidget;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();

private:

    VouwWidget *vouwWidget;
};

#endif
