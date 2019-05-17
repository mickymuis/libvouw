/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */
#pragma once

#include <QDialog>
#include "qvouw.h"

class QLabel;
class QCheckBox;
class QComboBox;

class ImportDialog : public QDialog {
    Q_OBJECT
    public:
        ImportDialog( QWidget* parent =0 );

        void setOptsPtr( QVouw::FileOpts* o );
        void setImportType( QVouw::FileType );
        void setQuantizeLevels( int min, int max );
      
        int quantizeLevels() const { return levels; }
        bool tabuChecked() const { return tabu; }

    public slots:
        void verifyAndAccept();

    private:
        int lvlMin, lvlMax, levels;
        bool tabu;
        QVouw::FileType type;
        QVouw::FileOpts *opts;

        QLabel *lblQuantize;
        QComboBox *cmbQuantize;
        QCheckBox *chkTabu;
        QPushButton *btnOk, *btnCancel;
};
