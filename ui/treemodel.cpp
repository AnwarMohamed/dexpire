/*
 *
 * Dexpire - treemodel.cpp
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 *
 */

#include "treeitem.h"
#include "treemodel.h"
#include <iostream>
#include <QStringList>
#include <QPixmap>
#include <QIcon>

TreeModel::TreeModel(cDexDecompiler* decompiledDex, cApkFile* apkFile, QObject *parent)
    : QAbstractItemModel(parent)
{
    this->decompiledDex = decompiledDex;
    this->apkFile = apkFile;

    if (apkFile)
    {
        rootItem = new TreeItem(TI_APK_ROOT);
        rootItem->setText(QString("APK Structure"));
    }
    else
    {
        rootItem = new TreeItem(TI_DEX_ROOT);
        rootItem->setText(QString("Dex Structure"));
    }

    setupApkModelData(rootItem);
    setupDexModelData(rootItem);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    switch(role)
    {
    case Qt::DisplayRole:
        return item->getText();
    case Qt::DecorationRole:
        return item->getIcon();
    default:
        return QVariant();
    }
}

TreeItem* TreeModel::getChild(const QModelIndex &index)
{
    return static_cast<TreeItem*>(index.internalPointer());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->getText();

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void TreeModel::setupApkModelData(TreeItem *parent)
{
    if (apkFile)
    {
        TreeItem * treeItem;
        for (unsigned int i=0; i<apkFile->FilesCount; i++)
        {
            treeItem = new TreeItem(TI_SRC_DIR, parent);
            parent->appendChild(treeItem, QString(apkFile->Files[i].Name).split("/"));
        }
    }
}

void TreeModel::setupDexModelData(TreeItem *parent)
{
    if (decompiledDex)
    {
        TreeItem * srcItem = new TreeItem(TI_SRC_DIR, parent);
        srcItem->setText(QString("src"));
        parent->appendChild(srcItem);
        for (unsigned int i=0; i<decompiledDex->nClasses; i++)
            appenedClassToParent(
                        srcItem,
                        &decompiledDex->Classes[i],
                        QString(decompiledDex->Classes[i].Package).split('.'));

        for (int i=srcItem->childCount()-1; i>=0; i--)
            shortenPackageName(srcItem->child(i));

        sortTree(srcItem);
    }
}

void TreeModel::sortTree(TreeItem* root)
{
    root->sortChilds();
    for (int i=0; i<root->childCount(); i++)
        sortTree(root->child(i));
}

void TreeModel::appenedClassToParent(TreeItem* parent, struct DEX_DECOMPILED_CLASS* dClass, QStringList& list)
{
    TreeItem * item;

    if (list.isEmpty())
    {
        item = new TreeItem(TI_CLASS, parent, dClass);
        item->setText(QString(dClass->Name));
        item->createClassTree();
        parent->appendChild(item);
    }
    else
    {
        for (int i=0; i<parent->childCount(); i++)
            if (parent->child(i)->getText().toString() == list.at(0))
            {
                list.removeFirst();
                return appenedClassToParent(parent->child(i), dClass, list);
            }

        item = new TreeItem(TI_PACKAGE_DIR, parent);
        item->setText(QString(list.at(0)));
        parent->appendChild(item);
        list.removeFirst();
        appenedClassToParent(item, dClass, list);
    }
}

void TreeModel::shortenPackageName(TreeItem* parent)
{
    if (parent->getType() == TI_PACKAGE_DIR &&
            parent->childCount() == 1 && parent->child(0)->getType() == TI_PACKAGE_DIR)
    {
        TreeItem* parentParent = parent->parent();
        TreeItem* parentChild = parent->child(0);
        parentChild->setText(parent->getText().toString().append(".").append(parentChild->getText().toString()));
        parentParent->appendChild(parentChild);
        parentParent->removeChild(parent->row());
        parentChild->setParent(parentParent);


        shortenPackageName(parentChild);
    }
    else
        for (int i=0; i<parent->childCount(); i++)
            shortenPackageName(parent->child(i));
}
