/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */
#include "importdialog.h"

#include <QtWidgets>


ImportDialog::ImportDialog( QWidget *parent ) : QDialog( parent ), opts( nullptr ) {

    lblQuantize = new QLabel( tr( "&Maximum number of singletons" ) );
    cmbQuantize = new QComboBox();
    lblQuantize->setBuddy( cmbQuantize );

    chkTabu = new QCheckBox( tr( "Mark most prevalent value as &tabu" ) );
    chkTabu->setChecked( true );

    btnOk = new QPushButton( tr( "&Import" ) );
    btnOk->setDefault( true );
    btnCancel = new QPushButton ( tr( "&Cancel" ) );

    connect( btnOk, &QAbstractButton::clicked, this, &ImportDialog::verifyAndAccept );
    connect( btnCancel, &QAbstractButton::clicked, this, &QDialog::reject );

    QDialogButtonBox *buttonBox = new QDialogButtonBox( Qt::Horizontal );
    buttonBox->addButton( btnCancel, QDialogButtonBox::ActionRole );
    buttonBox->addButton( btnOk, QDialogButtonBox::ActionRole );

    QHBoxLayout *layoutQuantize = new QHBoxLayout();
    layoutQuantize->addWidget( lblQuantize );
    layoutQuantize->addWidget( cmbQuantize );

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addLayout( layoutQuantize );
    layoutMain->addWidget( chkTabu );
    layoutMain->addWidget( buttonBox );

    setLayout( layoutMain );

    setWindowTitle( tr( "Import options" ) );
}

void
ImportDialog::verifyAndAccept() {
    bool ok;
    levels = cmbQuantize->currentText().toUInt( &ok );
    if( levels < lvlMin || levels > lvlMax || !ok ) {
        
        return;
    }
    tabu = chkTabu->isChecked();
    if( opts ) {
        opts->use_tabu = tabu;
        opts->levels = levels;
    }
    accept();
}

void
ImportDialog::setQuantizeLevels( int min, int max ) {
    cmbQuantize->clear();
    for( int i =min; i <= max; i = i << 1 ) {
        cmbQuantize->addItem( QString::number( i ) );
    }
    lvlMin =min; lvlMax =max;
}

void 
ImportDialog::setOptsPtr( QVouw::FileOpts* o ) {
    opts =o;
    if( opts ) setImportType( o->filetype );
}

void 
ImportDialog::setImportType( QVouw::FileType t ) {
    switch( t ) {
        case QVouw::IMAGE_IMPORT:
            setQuantizeLevels( 2, 256 ); // FIXME: un-hardcode
            break;
    }
    if( opts ) opts->filetype =t;
}


