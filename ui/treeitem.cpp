#include "treeitem.h"
#include <QIcon>
#include <QPainter>

TreeItem::TreeItem(int type, TreeItem *parent, struct DEX_DECOMPILED_CLASS* Class)
{
    _parent = parent;
    _type = type;
    _class = NULL;
    _dex_tab = NULL;
    _java_tab = NULL;

    switch(type)
    {
    case TI_CLASS:
    case TI_CLASS_FIELD:
    case TI_CLASS_METHOD:
        _class = Class;
        _icon_path = ":/icons/class_obj.png";
        break;
    case TI_DEX_ROOT:
        break;
    case TI_SRC_DIR:
        _icon_path = ":/icons/packagefolder_obj.gif";
        break;
    case TI_PACKAGE_DIR:
        _icon_path = ":/icons/package_obj.png";
        break;
    }

    _icon = QPixmap(_icon_path);
}

void TreeItem::setAsClassNode(int index)
{
    switch(_type)
    {
    case TI_CLASS_FIELD:
        _icon_path = ":/icons/field_";
        _text = QString(_class->Fields[index]->Name).append(" : ").append(_class->Fields[index]->ReturnType);

        if (_class->Fields[index]->Ref->AccessFlags & ACC_PUBLIC)
            _icon_path = _icon_path.append("public");
        else if (_class->Fields[index]->Ref->AccessFlags & ACC_PRIVATE)
            _icon_path = _icon_path.append("private");
        else if (_class->Fields[index]->Ref->AccessFlags & ACC_PROTECTED)
            _icon_path = _icon_path.append("protected");
        else
            _icon_path = _icon_path.append("default");

        _icon_path = _icon_path.append("_obj.gif");
        _icon = QPixmap(_icon_path);
        break;

    case TI_CLASS_METHOD:
        _icon_path = ":/icons/meth";
        _text = QString(_class->Methods[index]->Name).append("(");
        for (unsigned int i=0; i<_class->Methods[index]->ArgumentsSize; i++)
        {
            if (i) _text = _text.toString().append(", ");
            _text = _text.toString().append(cDexString::GetShortType(_class->Methods[index]->Arguments[i]->Type));
        }
        _text = _text.toString().append(") : ").append(cDexString::GetShortType(_class->Methods[index]->ReturnType));

        if (_class->Methods[index]->Ref->AccessFlags & ACC_PUBLIC)
            _icon_path = _icon_path.append("pub");
        else if (_class->Methods[index]->Ref->AccessFlags & ACC_PRIVATE)
            _icon_path = _icon_path.append("pri");
        else if (_class->Methods[index]->Ref->AccessFlags & ACC_PROTECTED)
            _icon_path = _icon_path.append("pro");
        else
            _icon_path = _icon_path.append("def");

        _icon_path = _icon_path.append("_obj.gif");
        _icon = QPixmap(_icon_path);
        break;
    }
}

void TreeItem::createClassTree()
{
    TreeItem* item;
    for (unsigned int i=0; i<_class->FieldsSize; i++)
    {
        item = new TreeItem(TI_CLASS_FIELD, this, _class);
        item->setAsClassNode(i);
        appendChild(item);
    }

    for (unsigned int i=0; i<_class->MethodsSize; i++)
    {
        item = new TreeItem(TI_CLASS_METHOD, this, _class);
        item->setAsClassNode(i);
        appendChild(item);
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
    if (_parent)
        return _parent->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

TreeItem *TreeItem::parent()
{
    return _parent;
}

QVariant TreeItem::getText()
{
    return _text;
}

QVariant TreeItem::getIcon()
{
    return QVariant(_icon);
}

void TreeItem::setText(QString& text)
{
    _text = text;
}

int TreeItem::getType()
{
    return _type;
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
    _parent = parent;
}

bool toAssending(TreeItem* s1 , TreeItem* s2)
{
    return s1->getText().toString() < s2->getText().toString();
}

void TreeItem::sortChilds()
{
    qSort(childItems.begin() , childItems.end(), toAssending);
}

DEX_DECOMPILED_CLASS* TreeItem::getClass()
{
    return _class;
}

void TreeItem::setDexTabWidget(QWidget* tab)
{
    _dex_tab = tab;
}

QWidget* TreeItem::getDexTabWidget()
{
    return _dex_tab;
}


void TreeItem::setJavaTabWidget(QWidget* tab)
{
    _java_tab = tab;
}

QWidget* TreeItem::getJavaTabWidget()
{
    return _java_tab;
}
