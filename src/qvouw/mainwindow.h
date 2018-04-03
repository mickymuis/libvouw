#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QFile>

class VouwWidget;
class VouwItemModel;

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

private slots:
    void vouwItemDoubleClicked(const QModelIndex&);

private:

    VouwWidget* vouwWidget;
    VouwItemModel* vouwModel;
    QTextEdit* console;
    QFile console_stderr;
    QFile console_stdout;
};

#endif
