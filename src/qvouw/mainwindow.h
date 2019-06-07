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
#include <QVector>
#include <cstdio>

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

    void addLogFile( const QString& filename );

public slots:
    void importImagePrompt();
    void quit();

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void vouwItemDoubleClicked(const QModelIndex&);
    void encodeCurrent();
    void reencodeCurrent();

private:
    void updateConsole();
    void encode( QVouw::Handle* );
    void setCurrentItem( VouwItem* );

    MatrixWidget* vouwWidget;
    VouwItemModel* vouwModel;
    VouwItem* currentItem;
    QTextEdit* console;
    QVector<QFile*> logfiles;
    bool showProgress;

    Vouw::Encoder::LocalSearch localMode;
    Vouw::Encoder::Heuristic heuristicMode;
};

