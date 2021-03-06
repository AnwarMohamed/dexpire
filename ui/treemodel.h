/*
 *
 * Dexpire - treemodel.h
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 *
 */

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include "treeitem.h"
#include <cDexDecompiler.h>
#include <cApkFile.h>

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(cDexDecompiler* DexDecompiler, cApkFile* ApkFile=0, QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &) const;

    TreeItem* getChild(const QModelIndex &index);

private:
    void setupDexModelData(TreeItem* parent);
    void setupApkModelData(TreeItem *parent);

    void appenedClassToParent(TreeItem* parent, struct DEX_DECOMPILED_CLASS* dClass, QStringList& list);
    void shortenPackageName(TreeItem* parent);
    void sortTree(TreeItem* root);

    TreeItem *rootItem;
    cDexDecompiler* decompiledDex;
    cApkFile* apkFile;
};

#endif // TREEMODEL_H
