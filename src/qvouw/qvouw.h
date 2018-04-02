#ifndef QVOUW_H
#define QVOUW_H

#include <QObject>
#include <QList>
#include <QAbstractItemModel>

#include <vouw/vouw.h>
#include <vouw/matrix.h>

class VouwItem;
class VouwItem {
public:
    enum Role {
        ROOT,
        PARENT,
        MATRIX,
        ENCODED,
        MODEL,
        PATTERN
    };
    explicit VouwItem( Role, VouwItem* parent =0 );
    ~VouwItem();

    vouw_matrix_t* matrix;
    vouw_t* handle;
    QString name;

    void appendChild(VouwItem *child);

    VouwItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    VouwItem *parent();
    Role role() const;

private:
    Role itemRole;
    QList<VouwItem*> childList;
    VouwItem *parentItem;

};

class QVouw : public QAbstractItemModel {
    Q_OBJECT

public:
    QVouw( QObject* parent =0 );
    ~QVouw();

    VouwItem* addFromImage( const QString& filename, int levels );
    VouwItem* addEmpty( const QString& name );

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
    QList<VouwItem*> objList;

};

#endif
