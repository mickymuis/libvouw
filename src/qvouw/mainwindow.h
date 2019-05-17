/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */
#pragma once

#include <vouw/vouw.h>
#include "qvouw.h"
#include <QMainWindow>
#include <QTextEdit>
#include <QFile>

VOUW_NAMESPACE_BEGIN
class Encoder;
VOUW_NAMESPACE_END

class VouwWidget;
class VouwItem;
class VouwItemModel;
class MatrixWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    void import( const QVouw::FileOpts& opts);

public slots:
    void importImagePrompt();
    void quit();

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void vouwItemDoubleClicked(const QModelIndex&);
    void reencodeCurrent();

private:
    void updateConsole();
    void encode( QVouw::Handle* );
    void setCurrentItem( VouwItem* );

    MatrixWidget* vouwWidget;
    VouwItemModel* vouwModel;
    VouwItem* currentItem;
    QTextEdit* console;
    QFile console_stderr;
    QFile console_stdout;
    bool showProgress;
};

