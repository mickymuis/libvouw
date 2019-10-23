/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */
#include <QtWidgets>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>

#include "matrixwidget.h"
#include "vouwwidget.h"
#include "mainwindow.h"
#include "vouwitemmodel.h"
#include "importdialog.h"

#include <vouw/codetable.h>

#include <stdio.h>
#include <iostream>
#include <iomanip>

MainWindow::MainWindow() : QMainWindow(), currentItem( 0 )
{
  //  QGridLayout *mainLayout = new QGridLayout;
    vouwModel = new VouwItemModel( this );
    
    /* Create instance of the vouw viewer widget */
    vouwWidget = new MatrixWidget( this );
    
    setCentralWidget(vouwWidget);

    /* Prepare main window actions */
    QAction* actImportImage = new QAction(tr("&Import Image"), this);
    actImportImage->setShortcut(tr("CTRL+I"));
    actImportImage->setStatusTip(tr("Import image-type file and convert it to a matrix"));
    connect(actImportImage, &QAction::triggered, this, &MainWindow::importImagePrompt);

    QAction* actQuit = new QAction(tr("&Quit"), this);
    actQuit->setShortcuts(QKeySequence::Quit);
    actQuit->setStatusTip(tr("Quit application"));
    //connect(actQuit, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    connect(actQuit, &QAction::triggered, this, &MainWindow::quit);

    QAction* actShowProg = new QAction(tr("Show progress"), this );
    actShowProg->setCheckable( true );
    connect( actShowProg, &QAction::toggled, [=]( bool checked ){ showProgress =checked; } );
    actShowProg->setChecked( true );
    
    QAction* actHideSinglt = new QAction(tr("Hide singletons"), this );
    actHideSinglt->setCheckable( true );
    connect( actHideSinglt, &QAction::toggled, [=]( bool checked ){ vouwWidget->setOption( MatrixWidget::HideSingletons, checked ); } );
    actHideSinglt->setChecked( true );
    
    QAction* actShowPivots = new QAction(tr("Show pivots"), this );
    actShowPivots->setCheckable( true );
    connect( actShowPivots, &QAction::toggled, [=]( bool checked ){ vouwWidget->setOption( MatrixWidget::ShowPivots, checked ); } );
    actShowPivots->setChecked( false );

    QAction* actShowPeriph = new QAction(tr("Show peripheries"), this );
    actShowPeriph->setCheckable( true );
    connect( actShowPeriph, &QAction::toggled, [=]( bool checked ){ vouwWidget->setOption( MatrixWidget::ShowPeriphery, checked ); } );
    actShowPeriph->setChecked( true );

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
    
    QAction* actEncode = new QAction(tr("Encode"), this );
    actEncode->setShortcut( tr("F5") );
    connect( actEncode, &QAction::triggered, this, &MainWindow::encodeCurrent );
    
    QAction* actReencode = new QAction(tr("Re-encode"), this );
    actReencode->setShortcut( tr("F6") );
    connect( actReencode, &QAction::triggered, this, &MainWindow::reencodeCurrent );
    
    /* Actiongroup to select heuristic */
    QActionGroup* grpHeuristic = new QActionGroup( this );

    QAction* actBest1 = new QAction(tr("Best 1"), this );
    actBest1->setCheckable( true );
    actBest1->setActionGroup( grpHeuristic );
    connect( actBest1, &QAction::toggled, [=]( bool checked ){ heuristicMode = Vouw::Encoder::Best1; } );

    QAction* actBestN = new QAction(tr("Best N"), this );
    actBestN->setCheckable( true );
    actBestN->setActionGroup( grpHeuristic );
    connect( actBestN, &QAction::toggled, [=]( bool checked ){ heuristicMode = Vouw::Encoder::BestN; } );
    actBestN->setChecked( true ); 
    heuristicMode = Vouw::Encoder::BestN;
    
    /* Actiongroup to select local search mode */
    QActionGroup* grpLocal = new QActionGroup( this );

    QAction* actLocalNo = new QAction(tr("Disabled"), this );
    actLocalNo->setCheckable( true );
    actLocalNo->setActionGroup( grpLocal );
    connect( actLocalNo, &QAction::toggled, [=]( bool checked ){ localMode = Vouw::Encoder::NoLocalSearch; } );

    QAction* actLocalFF = new QAction(tr("Flood fill"), this );
    actLocalFF->setCheckable( true );
    actLocalFF->setActionGroup( grpLocal );
    connect( actLocalFF, &QAction::toggled, [=]( bool checked ){ localMode = Vouw::Encoder::FloodFill; } );
    actLocalFF->setChecked( true ); 
    localMode = Vouw::Encoder::FloodFill;

    
    /* Menubar */
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(actImportImage);
    fileMenu->addAction(actQuit);
    // fileMenu->addSeparator();

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(actShowProg);
    viewMenu->addAction(actHideSinglt);
    viewMenu->addAction(actShowPivots);
    viewMenu->addAction(actShowPeriph);
    viewMenu->addSeparator();
    viewMenu->addAction(actZoomIn);
    viewMenu->addAction(actZoomOut);
    viewMenu->addAction(actZoomFit);
    viewMenu->addAction(actZoomFill);
    viewMenu->addSeparator();
    viewMenu->addAction(actResetView);
    
    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction( actEncode );
    toolsMenu->addAction( actReencode );
    toolsMenu->addSection( tr( "Heuristic" ) );
    toolsMenu->addAction( actBest1 );
    toolsMenu->addAction( actBestN );
    toolsMenu->addSection( tr( "Local Search Mode" ) );
    toolsMenu->addAction( actLocalNo );
    toolsMenu->addAction( actLocalFF);

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

    startTimer( 100 );
    
    setWindowTitle(tr("QVouw"));
    resize(800,600);
}

void
MainWindow::importImagePrompt() {
    QVouw::FileOpts opts;
    opts.filetype = QVouw::IMAGE_IMPORT;
    QString selectedFilter;
    opts.filename = QFileDialog::getOpenFileName(this,
                                tr("Import image..."),
                                "",
                                tr("Images (*.jpg *.jpeg *.png *.bmp *.tiff *.pgm *.pbm *.pnm)"),
                                &selectedFilter);
    if (opts.filename.isEmpty())
        return;

    ImportDialog dialog(this);
    dialog.setOptsPtr( &opts );
    dialog.exec();
    
    if (dialog.result() == QDialog::Accepted ) {
        import( opts );
    }
}

void
MainWindow::import( const QVouw::FileOpts& opts ) {

    QVouw::Handle *h =new QVouw::Handle( {0} );
    h->opts =opts;

    switch( opts.filetype ) {
        case QVouw::IMAGE_IMPORT:
            h->matrix = QVouw::importImage( opts );
        break;
    }
    
    if( !h->matrix ) {
        delete h;
        QMessageBox::critical(this, tr("Error"),
        tr("Could not import selected file. Please see log for details."),
        QMessageBox::Ok);
        QMessageBox::critical(this, tr("Error"),
                opts.filename,
                QMessageBox::Ok);
        return;
    }
    VouwItem* item = vouwModel->add( h, QFileInfo( opts.filename ).fileName() );
    if( !currentItem )
        setCurrentItem( item );
}

void
MainWindow::addLogFile( const QString& filename ) {
    QFile *file = new QFile();;
    file->setFileName( filename );
    file->open( QIODevice::ReadOnly );
    if( !file->isOpen() ) {
        std::cerr << "Could not open logfile " << filename.unicode() << std::endl;
        delete file;
        return;
    }
    logfiles.append( file );
}

void
MainWindow::quit() {
    for( int i =0; i < logfiles.size(); i++ ) {
        logfiles[i]->close();
        delete logfiles[i];
    }
    qApp->closeAllWindows();
}

void 
MainWindow::updateConsole() {
   // console->setTextColor( Qt::darkCyan );
    console->setTextColor( Qt::darkBlue );

    for( int i =0; i < logfiles.size(); i++ ) {
        while( logfiles[i]->bytesAvailable() )
            console->append( logfiles[i]->readLine().trimmed() );
    }
}

void
MainWindow::timerEvent(QTimerEvent *event) {
    updateConsole();
}

void
MainWindow::encode( QVouw::Handle* h ) {
    Vouw::Encoder *v = h->encoder;
    if( !v ) return;
    v->setHeuristic( heuristicMode );
    v->setLocalSearchMode( localMode );

    if( showProgress ) {
    // To enable visualisation within the encoding steps, we call encodeStep() ourselves
        while( v->encodeStep() ){
            updateConsole();
            vouwWidget->showEncoded( v );
            qApp->processEvents();
        }
    } else
        v->encode();

    std::cout << "Compression ratio: " << std::setprecision(4) << v->ratio() * 100.0 << "%" << std::endl;
    std::cout << "Model: " << v->codeTable()->countIfActiveNonSingleton() << " patterns (excluding singletons), Instance Set: " << v->instanceSet()->size() << " regions." << std::endl;
}

void 
MainWindow::setCurrentItem( VouwItem* item ) {
    QVouw::Handle* h =item->handle();
    if( !h ) return;
    
    currentItem = item;

    switch( item->role() ) {
        case VouwItem::ENCODED:
            if( h->encoder != nullptr )
                vouwWidget->showEncoded( h->encoder );
            else
                encodeCurrent();
            break;
        case VouwItem::ERROR:
            if( h->encoder != nullptr )
                vouwWidget->showError( h->encoder );
            break;
        default:
            vouwWidget->showMatrix( h->matrix );
    }
}

void 
MainWindow::vouwItemDoubleClicked( const QModelIndex& index ) {
    VouwItem* item =vouwModel->fromIndex( index );
    if( !item ) return;

    setCurrentItem( item );
}

void
MainWindow::reencodeCurrent() { 
    if( !currentItem ) return;
    
    QVouw::Handle* h =currentItem->handle();
    if( !h ) return;

    Vouw::Encoder* v =h->encoder;
    if( !v ) return;

    if( !v->isEncoded() ) return;

    v->reencode();
    vouwWidget->showEncoded( v );
    std::cout << "Compression ratio: " << std::setprecision(4) << v->ratio() * 100.0 << "%" << std::endl;
    std::cout << "Model: " << v->codeTable()->countIfActive() << " patterns, Instance Set: " << v->instanceSet()->size() << " regions." << std::endl;

}

void
MainWindow::encodeCurrent() { 
    if( !currentItem ) return;
    QVouw::Handle* h =currentItem->handle();
    if( !h ) return;
    
    if( !h->matrix ) return;
    if( !h->encoder ) {
        h->encoder = new Vouw::Encoder();
    } else {
        h->encoder->clear();
    }
    if( h->encoder->matrix() != h->matrix ) {
        h->encoder->setFromMatrix( h->matrix, h->opts.use_tabu ); 
    }
    if( !h->encoder->isEncoded() )
        encode( h );

}
