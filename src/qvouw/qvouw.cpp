#include "qvouw.h"
#include <iostream>

VouwItem::VouwItem( Role r, VouwItem* parent ) 
    : matrix(0), handle(0), itemRole( r ), parentItem( parent ) {
    if( parent )
        parent->appendChild( this );
}

VouwItem::~VouwItem() {
    qDeleteAll( childList );
}

void 
VouwItem::appendChild(VouwItem *child) {
    childList.append( child );
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
        case ROOT:
            return QVariant();
            break;
        case PARENT:
            return name;
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

VouwItem::Role VouwItem::role() const {
    return itemRole;
}


QVouw::QVouw( QObject* parent ) : QAbstractItemModel( parent ) {

    rootItem =new VouwItem( VouwItem::ROOT );
    addEmpty( "Test" );
    addEmpty( "Test2" );
}

QVouw::~QVouw() {
    delete rootItem;
}

QModelIndex 
QVouw::index(int row, int column, const QModelIndex &parent) const {
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
QVouw::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    VouwItem *childItem = static_cast<VouwItem*>(index.internalPointer());
    VouwItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int 
QVouw::rowCount(const QModelIndex &parent) const {
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
QVouw::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return static_cast<VouwItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant 
QVouw::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    VouwItem *item = static_cast<VouwItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags 
QVouw::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant 
QVouw::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

bool 
QVouw::removeRows(int row, int count, const QModelIndex &parent) {
   // if( parent.isValid() )
   //     return false;

    beginRemoveRows( parent, row, row+count-1 );

    for( int i=0; i < count; i++ )
        delete rootItem->child( row + i );

    endRemoveRows();

    return true;
}

VouwItem* 
QVouw::addFromImage( const QString& filename, int levels ) {

}

VouwItem* 
QVouw::addEmpty( const QString& name ) {
    beginInsertRows( QModelIndex(), objList.count(), objList.count() );

    VouwItem *parent =new VouwItem( VouwItem::PARENT, rootItem );
    parent->name =name;
    new VouwItem( VouwItem::MATRIX, parent );
    new VouwItem( VouwItem::ENCODED, parent );
    new VouwItem( VouwItem::MODEL, parent );

    objList.append( parent );

    endInsertRows();
}
