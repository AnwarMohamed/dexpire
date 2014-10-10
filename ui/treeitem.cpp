#include "treeitem.h"
#include <QIcon>

TreeItem::TreeItem(int type, TreeItem *parent, struct DEX_DECOMPILED_CLASS* dClass)
{
    parentItem = parent;
    itemType = type;

    switch(type)
    {
    case TI_CLASS:
        _icon = ":/icons/class_obj.png";
        break;
    case TI_DEX_ROOT:
        break;
    case TI_SRC_DIR:
        _icon = ":/icons/folder.png";
        break;
    case TI_PACKAGE_DIR:
        _icon = ":/icons/package_obj.png";
        break;
    }
}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
    for (int i=0; i<childCount(); i++)
        if (child(i)->getText().toString() == item->getText().toString())
            return;

    childItems.append(item);
}

TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

QVariant TreeItem::getText()
{
    return _text;
}

QVariant TreeItem::getIcon()
{
    return _icon;
}

void TreeItem::setText(QString& text)
{
    _text = text;
}

int TreeItem::getType()
{
    return itemType;
}

void TreeItem::removeChild(int row)
{
    childItems.removeAt(row);
}

void TreeItem::removeChilds()
{
    childItems.clear();
}

void TreeItem::setParent(TreeItem* parent)
{
    parentItem = parent;
}

bool toAssending(TreeItem* s1 , TreeItem* s2)
{
    return s1->getText().toString() < s2->getText().toString();
}

void TreeItem::sortChilds()
{
    qSort(childItems.begin() , childItems.end(), toAssending);
}
