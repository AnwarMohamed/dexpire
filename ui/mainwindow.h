/*
 *
 * Dexpire - mainwindows.h
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cDexFile.h>
#include <QTreeView>
#include <QTabWidget>
#include "treemodel.h"
#include <QTableView>
#include <QEvent>
#include <iostream>
#include <QMouseEvent>
#include <QMenu>
#include "codeeditor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QToolBar* toolbar;
    TreeModel* treeModel;

    void uiCenterScreen();
    void uiSetupToolbar();
    void loadFileDialog();
    void uiSetupSplitter();
    void uiSetupWorkspace();

    void cleanCurrentWorkspace();
    void prepareDexWorkspace();
    //void insertNewTab(QWidget* tab, )
    int tabOpened(QString& name);

    void addDexTab(TreeItem* item);
    void addJavaTab(TreeItem* item);

    void setClassToolbarEnabled(bool enable);

    cDexFile* dexFile;
    cDexDecompiler* dexDecompiler;

    QTableView* stringsTab;
    QTableView* headerTab;
    QTableView* methodsTab;
    QTableView* fieldsTab;
    QTableView* typesTab;
    QTableView* protoTab;

    bool treeViewSignalsRegistered;

    void printClassDexData(CodeEditor* editor, struct DEX_DECOMPILED_CLASS* dexClass);
    void printClassJavaData(CodeEditor* editor, struct DEX_DECOMPILED_CLASS* dexClass);

protected:
    void resizeEvent(QResizeEvent * event);
    //void closeEvent(QCloseEvent *event);

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_Class_triggered();
    void on_actionSave_All_triggered();
    void on_tabWidget_tabCloseRequested(int index);
    void on_tabWidget_currentChanged(int index);
    void on_actionDex_Disassembly_triggered();
    void on_treeView_clicked(const QModelIndex &index);
    void on_actionStrings_Table_triggered();
    void on_actionHeader_triggered();

    void on_actionTypes_Table_triggered();

    void on_actionFields_Table_triggered();
    void with_fieldsTab_doubleClicked(const QModelIndex &index);

    void with_protoTab_doubleClicked(const QModelIndex &index);
    void on_actionPrototypes_Table_triggered();

    void with_methodsTab_doubleClicked(const QModelIndex &index);
    void on_actionMethods_Table_triggered();

    void with_treeView_collapsed(const QModelIndex &index);
    void with_treeView_expanded(const QModelIndex &index);
    void on_tabWidget_customContextMenuRequested(const QPoint &pos);
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_actionJava_Source_triggered();
};

class TabWidgetEventFilter: public QObject
{
    Q_OBJECT

public:
    TabWidgetEventFilter(QTabBar* _tabBar):QObject()
    {
        this->_tabBar = _tabBar;
        this->_tabWidget = (QTabWidget*)_tabBar->parent();
        setupRightDropdownMenu();
    }

    ~TabWidgetEventFilter(){}

    bool eventFilter(QObject* object, QEvent* event)
    {
        if(event->type() == QEvent::MouseButtonRelease)
        {
            _clickedItem = -1;
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QPoint position = mouseEvent->pos();

            for (int i=0; i<_tabBar->count(); i++)
                if (_tabBar->tabRect(i).contains(position))
                {
                    _clickedItem = i;
                    break;
                }

            if (_clickedItem == -1)
                return QObject::eventFilter(object, event);

            switch(mouseEvent->button())
            {
                //case Qt::LeftButton:
                //    return QObject::eventFilter(obj, event);
                //    break;

                case Qt::RightButton:
                    popupRightDropdownMenu(position, _clickedItem);
                    break;

                //case Qt::MidButton:
                //    on_middleMouse_pressed( clickedItem, position );
                //    break;

                default:
                    return QObject::eventFilter(object, event);
            }

            return true;
        }
        else
            return QObject::eventFilter(object,event);
    }

private:
    QTabBar* _tabBar;
    QTabWidget* _tabWidget;
    QMenu* _rightDropdownMenu;
    int _clickedItem;

    void popupRightDropdownMenu(const QPoint& pos, int index)
    {
        if (index + 1 == _tabWidget->count())
            _rightDropdownMenu->actions()[3]->setEnabled(false);
        else
            _rightDropdownMenu->actions()[3]->setEnabled(true);

        if (_tabWidget->count() == 1)
            _rightDropdownMenu->actions()[1]->setEnabled(false);
        else
            _rightDropdownMenu->actions()[1]->setEnabled(true);

        _rightDropdownMenu->popup(_tabBar->mapToGlobal(pos));
    }

    void setupRightDropdownMenu()
    {
        QAction* action;
        _rightDropdownMenu = new QMenu();

        action = new QAction("Close tab", _rightDropdownMenu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_tabBar_closeTab()));
        _rightDropdownMenu->addAction(action);

        action = new QAction("Close other tabs", _rightDropdownMenu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_tabBar_closeOtherTabs()));
        _rightDropdownMenu->addAction(action);

        action = new QAction("Close all tabs", _rightDropdownMenu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_tabBar_closeAllTabs()));
        _rightDropdownMenu->addAction(action);

        action = new QAction("Close tabs to the right", _rightDropdownMenu);
        connect(action, SIGNAL(triggered()), this, SLOT(on_tabBar_closeRightTabs()));
        _rightDropdownMenu->addAction(action);
    }

private slots:
    void on_tabBar_closeTab()
    {
        _tabWidget->removeTab(_clickedItem);
    }

    void on_tabBar_closeAllTabs()
    {
        _tabWidget->clear();
    }

    void on_tabBar_closeOtherTabs()
    {
        on_tabBar_closeRightTabs();

        for (int i=0; i<_clickedItem; i++)
            _tabWidget->removeTab(i);
    }

    void on_tabBar_closeRightTabs()
    {
        while(_clickedItem+1 != _tabWidget->count())
            _tabWidget->removeTab(_tabWidget->count()-1);
    }
};

#endif // MAINWINDOW_H
