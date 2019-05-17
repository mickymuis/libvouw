/*
 * QVouw - Graphical User Interface for VOUW
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, 2019, Leiden Institute for Advanced Computer Science
 */
#include "vouwitemmodel.h"
#include <iostream>

/* VouwItem implementation */

VouwItem::VouwItem( Role r, VouwItem* parent ) 
    :itemRole( r ), parentItem( parent ), hnd( nullptr ) {
    if( parent )
        parent->appendChild( this );
}

VouwItem::~VouwItem() {
    qDeleteAll( childList );
    if( hnd && itemRole == ROOT )
        delete hnd;
}

void 
VouwItem::setHandle( QVouw::Handle *h ) {
    hnd =h;
}

QVouw::Handle* 
VouwItem::handle() const {
    if( !hnd )
        if( !parentItem )
            return nullptr;
        else
            return parentItem->handle();
    return hnd;
}

void 
VouwItem::appendChild(VouwItem *child) {
    childList.append( child );
}

void 
VouwItem::removeChildren( int startRow, int count ) {
    if( startRow < 0 || startRow + count > childList.size() )
        return;

    for( int row = 0; row < count; ++row )
        delete childList.takeAt( startRow );

    return;
}

VouwItem *VouwItem::child(int row) {
    return childList[row];
}

int VouwItem::childCount() const {
    return childList.count(); 
}

int VouwItem::columnCount() const {
    return 1;
}

QVariant VouwItem::data(int column) const {
    switch( itemRole ) {
        case INDEX:
            return QVariant();
            break;
        case ROOT:
            return str.isNull() ? QObject::tr( "(object)" ) : str;
            break;
        case MATRIX:
            return QString( QObject::tr( "Input matrix" ) );
            break;
        case ENCODED:
            return QString( QObject::tr( "Encoded matrix" ) );
            break;
        case MODEL:
            return QString( QObject::tr( "Model" ) );
            break;
        case PATTERN:
            return QString( QObject::tr( "Pattern #%1" ).arg( row() ) );
            break;
    };
    return QVariant();
}

int VouwItem::row() const {
    if (parentItem)
            return parentItem->childList.indexOf(const_cast<VouwItem*>(this));

    return 0;
}

VouwItem *VouwItem::parent() {
    return parentItem;
}

VouwItem *VouwItem::ancestor() {
    if( !parent() ) return this;
    return parent()->ancestor();
}

VouwItem::Role VouwItem::role() const {
    return itemRole;
}

/* VouwItemModel implementation */

VouwItemModel::VouwItemModel( QObject* parent ) : QAbstractItemModel( parent ) {

    rootItem =new VouwItem( VouwItem::INDEX );
   /* addEmpty( "Test" );
    addEmpty( "Test2" );*/
}

VouwItemModel::~VouwItemModel() {
    delete rootItem;
}

VouwItem* 
VouwItemModel::fromIndex( const QModelIndex& index ) const {
    if( index.isValid() /*&& index.parent().isValid()*/ ) {
        VouwItem* item =static_cast<VouwItem*>( index.internalPointer() );
        return item;
    }
    return 0;
}

QModelIndex 
VouwItemModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    VouwItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<VouwItem*>(parent.internalPointer());

    VouwItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex 
VouwItemModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    VouwItem *childItem = static_cast<VouwItem*>(index.internalPointer());
    VouwItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int 
VouwItemModel::rowCount(const QModelIndex &parent) const {
    VouwItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<VouwItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int 
VouwItemModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return static_cast<VouwItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant 
VouwItemModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    VouwItem *item = static_cast<VouwItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags 
VouwItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant 
VouwItemModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

bool 
VouwItemModel::removeRows(int row, int count, const QModelIndex &parent) {
   // if( parent.isValid() )
   //     return false;

    beginRemoveRows( parent, row, row+count-1 );

    for( int i=0; i < count; i++ )
        delete rootItem->child( row + i );

    endRemoveRows();

    return true;
}

VouwItem* 
VouwItemModel::add( QVouw::Handle* h, const QString& name ) {
    beginInsertRows( QModelIndex(), rootItem->childCount(), rootItem->childCount() );

    VouwItem *parent =new VouwItem( VouwItem::ROOT, rootItem );
    parent->setName( name );
    parent->setHandle( h );
    new VouwItem( VouwItem::MATRIX, parent );
    new VouwItem( VouwItem::ENCODED, parent );
    new VouwItem( VouwItem::MODEL, parent );

    endInsertRows();

    return parent;
}

VouwItem* 
VouwItemModel::addEmpty( const QString& name ) {
    beginInsertRows( QModelIndex(), rootItem->childCount(), rootItem->childCount() );

    VouwItem *parent =new VouwItem( VouwItem::ROOT, rootItem );
    parent->setName( name );

    endInsertRows();

    return parent;
}

/*void 
VouwItemModel::update( VouwItem* item ) {

}*/
