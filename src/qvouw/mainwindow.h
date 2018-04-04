#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QFile>

class Vouw;
class VouwWidget;
class VouwItem;
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
    void updateConsole();
    void encode( Vouw* );
    void setCurrentItem( VouwItem* );

    VouwWidget* vouwWidget;
    VouwItemModel* vouwModel;
    VouwItem* currentItem;
    QTextEdit* console;
    QFile console_stderr;
    QFile console_stdout;
};

#endif
