#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vouw/vouw.h>
#include <QMainWindow>
#include <QTextEdit>
#include <QFile>

VOUW_NAMESPACE_BEGIN
class Encoder;
VOUW_NAMESPACE_END

class VouwWidget;
class VouwItem;
class VouwItemModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    void importImage( const QString& fileName, int levels );

public slots:
    void importImagePrompt();
    void quit();

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void vouwItemDoubleClicked(const QModelIndex&);

private:
    void updateConsole();
    void encode( Vouw::Encoder* );
    void setCurrentItem( VouwItem* );

    VouwWidget* vouwWidget;
    VouwItemModel* vouwModel;
    VouwItem* currentItem;
    QTextEdit* console;
    QFile console_stderr;
    QFile console_stdout;
};

#endif
