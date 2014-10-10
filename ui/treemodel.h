#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include "treeitem.h"
#include <cDexDecompiler.h>

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(cDexDecompiler* DexDecompiler, QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    TreeItem* getChild(const QModelIndex &index);

private:
    void setupModelData(TreeItem* parent);
    void appenedClassToParent(TreeItem* parent, struct DEX_DECOMPILED_CLASS* dClass, QStringList& list);
    void shortenPackageName(TreeItem* parent);
    void sortTree(TreeItem* root);

    TreeItem *rootItem;
    cDexDecompiler* decompiledDex;
};

#endif // TREEMODEL_H
