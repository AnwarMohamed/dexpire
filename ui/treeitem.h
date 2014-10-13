#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <cDexDecompiler.h>
#include <QIcon>
#include <QPainter>

enum
{
    TI_DIR = 0,
    TI_PACKAGE_DIR,
    TI_SRC_DIR,
    TI_CLASS,
    TI_CLASS_FIELD,
    TI_CLASS_METHOD,
    TI_DEX_ROOT
};

class TreeItem
{
public:
    explicit TreeItem(int type, TreeItem *parent = 0, struct DEX_DECOMPILED_CLASS* dClass = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);
    void removeChild(int row);
    void removeChilds();

    void setParent(TreeItem* parent);

    TreeItem *child(int row);
    int childCount() const;
    int row() const;
    TreeItem *parent();

    void setText(QString& text);

    QVariant getText();
    QVariant getIcon();
    int getType();

    void sortChilds();

    void setAsClassNode(int index);
    void createClassTree();

private:
    QList<TreeItem*> childItems;
    TreeItem* _parent;

    QVariant _text;
    QPixmap _icon;
    QString _icon_path;

    int _type;
    DEX_DECOMPILED_CLASS* _class;
};

#endif // TREEITEM_H
