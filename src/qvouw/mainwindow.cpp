#include <QtWidgets>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>

#include "matrixwidget.h"
#include "vouwwidget.h"
#include "mainwindow.h"
#include "vouwitemmodel.h"

#include <vouw/codetable.h>

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
    connect(actImportImage, &QAction::triggered, this, &MainWindow::importImagePrompt);

    QAction* actQuit = new QAction(tr("&Quit"), this);
    actQuit->setShortcuts(QKeySequence::Quit);
    actQuit->setStatusTip(tr("Quit application"));
    connect(actQuit, &QAction::triggered, this, &MainWindow::quit);

    QAction* actShowProg = new QAction(tr("Show progress"), this );
    actShowProg->setCheckable( true );
    connect( actShowProg, &QAction::toggled, [=]( bool checked ){ showProgress =checked; } );
    actShowProg->setChecked( true );
    
    QAction* actZoomIn = new QAction(tr("Zoom in"), this );
    actZoomIn->setShortcut( QKeySequence::ZoomIn );
    connect( actZoomIn, &QAction::triggered, [=]( void ){ vouwWidget->zoomStepIn(); } );
    
    QAction* actZoomOut = new QAction(tr("Zoom out"), this );
    actZoomOut->setShortcut( QKeySequence::ZoomOut );
    connect( actZoomOut, &QAction::triggered, [=]( void ){ vouwWidget->zoomStepOut(); } );
    
    QAction* actZoomFit = new QAction(tr("Zoom fit"), this );
    connect( actZoomFit, &QAction::triggered, [=]( void ){ vouwWidget->zoomFit(); } );
    
    QAction* actZoomFill = new QAction(tr("Zoom fill"), this );
    connect( actZoomFill, &QAction::triggered, [=]( void ){ vouwWidget->zoomFill(); } );
    
    QAction* actResetView = new QAction(tr("Reset view"), this );
    connect( actResetView, &QAction::triggered, [=]( void ){ vouwWidget->zoomFill(); vouwWidget->setPan(0,0); } );
    
    /* Menubar */
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(actImportImage);
    fileMenu->addAction(actQuit);
    // fileMenu->addSeparator();

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(actShowProg);
    viewMenu->addSeparator();
    viewMenu->addAction(actZoomIn);
    viewMenu->addAction(actZoomOut);
    viewMenu->addAction(actZoomFit);
    viewMenu->addAction(actZoomFill);
    viewMenu->addSeparator();
    viewMenu->addAction(actResetView);
    
    /* Create instance of the vouw viewer widget */
    vouwWidget = new MatrixWidget( this );
    
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

   /* *stderr =*fopen( ".stderr.tmp", "w" );
    console_stderr.setFileName( ".stderr.tmp" );
    console_stderr.open( QIODevice::ReadOnly );*/

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
MainWindow::importImagePrompt() {
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
        importImage( fileName, levels );
    }
}

void
MainWindow::importImage( const QString& fileName, int levels ) {

    Vouw::Encoder* v= new ImageEncoder( fileName, levels );
    if( !v->isValid() ) {
        delete v;
        QMessageBox::critical(this, tr("Error"),
        tr("Could not load selected image. Please see log for details."),
        QMessageBox::Ok);
        QMessageBox::critical(this, tr("Error"),
                fileName,
                QMessageBox::Ok);
        return;
    }
    VouwItem* item = vouwModel->add( v, QFileInfo( fileName ).fileName() );
    if( !currentItem )
        setCurrentItem( item );
}

void
MainWindow::quit() {
    QApplication::quit();
}

void 
MainWindow::updateConsole() {
   /* console->setTextColor( Qt::darkCyan );
    while( console_stderr.bytesAvailable() )
        console->append( console_stderr.readLine().trimmed() );*/

    console->setTextColor( Qt::darkBlue );
    while( console_stdout.bytesAvailable() )
        console->append( console_stdout.readLine().trimmed() );
}

void
MainWindow::timerEvent(QTimerEvent *event) {
    updateConsole();
}

void
MainWindow::encode( Vouw::Encoder* v ) {
    while( v->encodeStep() ){
        updateConsole();
        vouwWidget->showEncoded( v );
        if( showProgress )
            qApp->processEvents();
    }

    std::cout << "Compression ratio: " << std::setprecision(4) << v->ratio() * 100.0 << "%" << std::endl;
    std::cout << "Model: " << v->codeTable()->countIfActive() << " patterns, Instance Set: " << v->instanceSet()->size() << " regions." << std::endl;
}

void 
MainWindow::setCurrentItem( VouwItem* item ) {
    Vouw::Encoder* v =item->object();
    if( !v ) return;

    switch( item->role() ) {
        case VouwItem::ENCODED:
            if( !v->isEncoded() )
                encode( v );
            vouwWidget->showEncoded( v );
            break;
        default:
            vouwWidget->showMatrix( v->matrix() );
    }

    currentItem = item;

}

void 
MainWindow::vouwItemDoubleClicked( const QModelIndex& index ) {
    VouwItem* item =vouwModel->fromIndex( index );
    if( !item ) return;

    setCurrentItem( item );
}
