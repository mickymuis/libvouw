/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */
#pragma once

#include <QObject>
#include <QList>
#include <QAbstractItemModel>

#include "qvouw.h"

class VouwItem {
public:
    enum Role {
        INDEX,
        ROOT,
        MATRIX,
        ENCODED,
        MODEL,
        PATTERN,
        ERROR
    };
    explicit VouwItem( Role, VouwItem* parent =0 );
    ~VouwItem();

    void setHandle( QVouw::Handle* );
    QVouw::Handle* handle() const;

    void setName( const QString& n ) { str = n; }
    QString name() const { return str; }

    void appendChild(VouwItem *child);

    void removeChildren( int startRow, int count ); 

    VouwItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    VouwItem *parent();
    VouwItem *ancestor();
    Role role() const;

private:
    Role itemRole;
    QList<VouwItem*> childList;
    VouwItem *parentItem;
    QString str;
    QVouw::Handle* hnd;

};

class VouwItemModel : public QAbstractItemModel {
    Q_OBJECT

public:
    VouwItemModel( QObject* parent =0 );
    ~VouwItemModel();

    VouwItem* add( QVouw::Handle*, const QString& name );
    VouwItem* addEmpty( const QString& name );

    //void update( VouwItem* );

    VouwItem* fromIndex( const QModelIndex& index ) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    VouwItem* rootItem;

};

