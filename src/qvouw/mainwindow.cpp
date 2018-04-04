#include <QtWidgets>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>

#include "vouwwidget.h"
#include "mainwindow.h"
#include "vouwitemmodel.h"

#include <stdio.h>
#include <iostream>
#include <iomanip>

MainWindow::MainWindow() : QMainWindow(), currentItem( 0 )
{
  //  QGridLayout *mainLayout = new QGridLayout;
    vouwModel = new VouwItemModel( this );

    /* Prepare main window actions */
    QAction* actImportImage = new QAction(tr("&Import Image"), this);
    //newAct->setShortcuts(QKeySequence::New);
    actImportImage->setStatusTip(tr("Import image-type file and convert it to a matrix"));
    connect(actImportImage, &QAction::triggered, this, &MainWindow::importImage);

    QAction* actQuit = new QAction(tr("&Quit"), this);
    actQuit->setShortcuts(QKeySequence::Quit);
    actQuit->setStatusTip(tr("Quit application"));
    connect(actQuit, &QAction::triggered, this, &MainWindow::quit);
    
    /* Menubar */
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(actImportImage);
    fileMenu->addAction(actQuit);
    // fileMenu->addSeparator();

    /* Create instance of the vouw viewer widget */
    vouwWidget = new VouwWidget( this );
    
    setCentralWidget(vouwWidget);

//    setLayout(mainLayout);
    /* List panel */
    QDockWidget* modelWindow = new QDockWidget(tr("Models"), this);
    modelWindow->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea );
    modelWindow->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::RightDockWidgetArea, modelWindow);

    QTreeView* view = new QTreeView( modelWindow );
    modelWindow->setWidget( view );
    view->setModel( vouwModel );
    connect( view, &QAbstractItemView::doubleClicked, this, &MainWindow::vouwItemDoubleClicked );

    /* Console */
    QDockWidget* consoleWindow = new QDockWidget(tr("Console"), this);
    consoleWindow->setAllowedAreas(Qt::BottomDockWidgetArea);
    consoleWindow->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, consoleWindow);

    console = new QTextEdit(consoleWindow);
    console->setReadOnly( true );
    consoleWindow->setWidget(console);

    /* Redirect stdout and stderr */
    *stdout =*fopen( ".stdout.tmp", "w" );
    console_stdout.setFileName( ".stdout.tmp" );
    console_stdout.open( QIODevice::ReadOnly );

    *stderr =*fopen( ".stderr.tmp", "w" );
    console_stderr.setFileName( ".stderr.tmp" );
    console_stderr.open( QIODevice::ReadOnly );

    startTimer( 100 );
    
    setWindowTitle(tr("QVouw"));
    resize(800,600);

   /* std::cout << "Poep" <<std::endl;
    std::cerr << "Fout!"<<std::endl;

    std::cout << "Test" << std::endl;
    std::cout << "Bla";
    std::cout << "Troep!" << std::endl; */
}

void
MainWindow::importImage() {
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Import image..."),
                                "",
                                tr("Images (*.jpg *.jpeg *.png *.bmp *.tiff)"),
                                &selectedFilter);
    if (fileName.isEmpty())
        return;
    
    QStringList items;
    items << "2" << "4" << "8" << "16" << "32";

    bool ok;
    QString item = QInputDialog::getItem(this, tr("Import options"),
                                         tr("Quantization levels:"), items, 2, false, &ok);
    if (ok && !item.isEmpty()) {
        int levels = item.toInt();

        std::cerr << levels << std::endl;

        Vouw* v= Vouw::createFromImage( fileName, levels );
        if( !v ) {
            QMessageBox::critical(this, tr("Error"),
            tr("Could not load selected image. Please see log for details."),
            QMessageBox::Ok);
        }
        VouwItem* item = vouwModel->add( v );
        if( !currentItem )
            setCurrentItem( item );
    }
}

void
MainWindow::quit() {
    QApplication::quit();
}

void 
MainWindow::updateConsole() {
    console->setTextColor( Qt::darkCyan );
    while( console_stderr.bytesAvailable() )
        console->append( console_stderr.readLine().trimmed() );

    console->setTextColor( Qt::darkBlue );
    while( console_stdout.bytesAvailable() )
        console->append( console_stdout.readLine().trimmed() );
}

void
MainWindow::timerEvent(QTimerEvent *event) {
    updateConsole();
}

void
MainWindow::encode( Vouw* v ) {
    while( v->encodeStep() ){
        updateConsole();
        vouwWidget->showEncoded( v->handle );
        qApp->processEvents();
    }

    std::cout << "Compression ratio: " << std::setprecision(4) << v->ratio() << "%" << std::endl;
}

void 
MainWindow::setCurrentItem( VouwItem* item ) {
    Vouw* v =item->object();
    if( !v ) return;

    switch( item->role() ) {
        case VouwItem::ENCODED:
            if( !v->isEncoded() )
                encode( v );
            vouwWidget->showEncoded( v->handle );
            break;
        default:
            vouwWidget->showMatrix( v->matrix );
    }

    currentItem = item;

}

void 
MainWindow::vouwItemDoubleClicked( const QModelIndex& index ) {
    VouwItem* item =vouwModel->fromIndex( index );
    if( !item ) return;

    setCurrentItem( item );
}
