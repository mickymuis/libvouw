#include <QtWidgets>

#include "vouwwidget.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    QGridLayout *mainLayout = new QGridLayout;

    vouwWidget = new VouwWidget( this );
    
    mainLayout->addWidget(vouwWidget, 0, 0);

    setLayout(mainLayout);

    setWindowTitle(tr("QVouw"));
}
