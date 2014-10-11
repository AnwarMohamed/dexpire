#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cDexFile.h>
#include <QTreeView>
#include <QTabWidget>
#include "treemodel.h"
#include <QTableView>

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

    cDexFile* dexFile;
    cDexDecompiler* dexDecompiler;

    QTableView* stringsTab;
    QTableView* headerTab;
    QTableView* methodsTab;
    QTableView* fieldsTab;
    QTableView* typesTab;
     QTableView* protoTab;

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
};

#endif // MAINWINDOW_H
